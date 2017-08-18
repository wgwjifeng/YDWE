#include "BlpConv.h"
#include <algorithm> 
#include <sstream>  
#include <vector>

#define LOG(s)

namespace IMAGE { namespace BLP {

const int MAX_NR_OF_BLP_MIP_MAPS = 16;

struct BLP_HEADER
{
	BLP_HEADER()
	{
		MagicNumber = '1PLB';
		Compression = 0;
		AlphaBits = 0;
		Width = 0;
		Height = 0;
		Unknown1 = 0;
		Unknown2 = 0;
		memset(Offset, 0, MAX_NR_OF_BLP_MIP_MAPS * sizeof(uint32_t));
		memset(Size, 0, MAX_NR_OF_BLP_MIP_MAPS * sizeof(uint32_t));
	}

	uint32_t MagicNumber;
	uint32_t Compression;
	uint32_t AlphaBits;
	uint32_t Width;
	uint32_t Height;
	uint32_t Unknown1;
	uint32_t Unknown2;
	uint32_t Offset[MAX_NR_OF_BLP_MIP_MAPS];
	uint32_t Size[MAX_NR_OF_BLP_MIP_MAPS];
};

struct BLP_RGBA
{
	uint8_t Red;
	uint8_t Green;
	uint8_t Blue;
	uint8_t Alpha;
};

typedef uint8_t BLP_PIXEL;

static bool LoadCompressed(BLP_HEADER& Header, const BUFFER& SourceBuffer, BUFFER& TargetBuffer)
{
	BUFFER TempBuffer;
	uint32_t  JpegHeaderSize;

	memcpy(reinterpret_cast<char*>(&JpegHeaderSize), SourceBuffer.data() + sizeof(BLP_HEADER), sizeof(uint32_t));

	TempBuffer.resize(Header.Size[0] + JpegHeaderSize);

	memcpy(TempBuffer.data(), SourceBuffer.data() + sizeof(BLP_HEADER) + sizeof(uint32_t), JpegHeaderSize);
	memcpy(TempBuffer.data() + JpegHeaderSize, SourceBuffer.data() + Header.Offset[0], Header.Size[0]);

	if (!JPEG::Read(TempBuffer, TargetBuffer, Header.Width, Header.Height))
	{
		LOG("Unable to load  blp file, BLP reading failed!");
		return false;
	}

	return true;
}

static bool LoadUncompressed(BLP_HEADER& Header, const BUFFER& SourceBuffer, BUFFER& TargetBuffer)
{
	static const int PALETTE_SIZE = 256;
	BLP_RGBA const* Palette = reinterpret_cast<BLP_RGBA const*>(SourceBuffer.data() + sizeof(BLP_HEADER));
	BLP_PIXEL const* SourcePixel = reinterpret_cast<BLP_PIXEL const*>(SourceBuffer.data() + Header.Offset[0]);
	int Size = Header.Width * Header.Height;
	TargetBuffer.resize(Size * 4);
	BLP_RGBA* TargetPixel = reinterpret_cast<BLP_RGBA*>(TargetBuffer.data());
	BLP_PIXEL const* SourceAlpha = SourcePixel + Size;
	switch (Header.AlphaBits)
	{
	default:
	case 0:
		for (int i = 0; i < Size; i++)
		{
			TargetPixel[i] = Palette[SourcePixel[i]];
			TargetPixel[i].Alpha = 255;
		}
		break;
	case 1:
		for (int i = 0; i < Size; i++)
		{
			TargetPixel[i] = Palette[SourcePixel[i]];
			TargetPixel[i].Alpha = (SourceAlpha[i >> 3] & (1 << (i & 7))) ? 1 : 0;
		}
		break;
	case 4:
		for (int i = 0; i < Size; i++)
		{
			TargetPixel[i] = Palette[SourcePixel[i]];
			switch (i & 1)
			{
			case 0: TargetPixel[i].Alpha = SourceAlpha[i >> 1] & 0x0F; break;
			case 1: TargetPixel[i].Alpha = (SourceAlpha[i >> 1] & 0xF0) >> 4; break;
			}
		}
		break;
	case 8:
		for (int i = 0; i < Size; i++)
		{
			TargetPixel[i] = Palette[SourcePixel[i]];
			TargetPixel[i].Alpha = SourceAlpha[i];
		}
		break;
	}
	return true;
}

bool Write(const BUFFER& SourceBuffer, BUFFER& TargetBuffer, int Width, int Height, int Quality)
{
	int32_t i;
	int32_t X;
	int32_t Y;
	int32_t Size;
	int32_t Index;
	int32_t BufferIndex;
	int32_t TotalSize;
	int32_t NrOfMipMaps;
	int32_t TextureSize;
	int32_t CurrentWidth;
	int32_t CurrentHeight;
	int32_t CurrentOffset;
	BUFFER TempBuffer;
	BLP_HEADER Header;
	const unsigned char* Pointer;
	uint32_t JpegHeaderSize;
	std::stringstream Stream;
	std::vector<BUFFER> MipMapBufferList;

	JpegHeaderSize = 4;
	MipMapBufferList.resize(MAX_NR_OF_BLP_MIP_MAPS);

	Header.Compression = 0;
	Header.AlphaBits = 8;
	Header.Width = Width;
	Header.Height = Height;
	Header.Unknown1 = 4;
	Header.Unknown2 = 1;

	NrOfMipMaps = 0;

	Size = std::max(Header.Width, Header.Height);
	while (Size >= 1)
	{
		Size /= 2;
		NrOfMipMaps++;
	}

	if (NrOfMipMaps > MAX_NR_OF_BLP_MIP_MAPS)
	{
		NrOfMipMaps = MAX_NR_OF_BLP_MIP_MAPS;
	}

	if (NrOfMipMaps < 1)
	{
		return false;
	}

	CurrentWidth = Header.Width;
	CurrentHeight = Header.Height;
	CurrentOffset = sizeof(BLP_HEADER) + sizeof(uint32_t) + JpegHeaderSize;
	for (i = 0; i < NrOfMipMaps; i++)
	{
		TempBuffer.resize(CurrentWidth * CurrentHeight * 4);

		Index = 0;
		BufferIndex = 0;
		Pointer = reinterpret_cast<const unsigned char*>(SourceBuffer.data());

		for (Y = 0; Y < static_cast<int32_t>(CurrentHeight); Y++)
		{
			for (X = 0; X < static_cast<int32_t>(CurrentWidth); X++)
			{
				TempBuffer[BufferIndex++] = Pointer[Index++];
				TempBuffer[BufferIndex++] = Pointer[Index++];
				TempBuffer[BufferIndex++] = Pointer[Index++];
				TempBuffer[BufferIndex++] = Pointer[Index++];
			}
		}

		if (!JPEG::Write(TempBuffer, MipMapBufferList[i], CurrentWidth, CurrentHeight, Quality))
		{
			return false;
		}

		TextureSize = MipMapBufferList[i].size();

		Header.Offset[i] = CurrentOffset;
		Header.Size[i] = TextureSize - JpegHeaderSize;

		CurrentWidth /= 2;
		CurrentHeight /= 2;
		CurrentOffset += Header.Size[i];

		if (CurrentWidth < 1) CurrentWidth = 1;
		if (CurrentHeight < 1) CurrentHeight = 1;
	}

	TotalSize = sizeof(BLP_HEADER) + sizeof(uint32_t) + JpegHeaderSize;
	for (i = 0; i < NrOfMipMaps; i++)
	{
		if (MipMapBufferList[i].size() <= 0) break;
		TotalSize += Header.Size[i];
	}

	TargetBuffer.resize(TotalSize);

	CurrentOffset = 0;

	memcpy(&TargetBuffer[CurrentOffset], &Header, sizeof(BLP_HEADER));
	CurrentOffset += sizeof(BLP_HEADER);

	memcpy(&TargetBuffer[CurrentOffset], &JpegHeaderSize, sizeof(uint32_t));
	CurrentOffset += sizeof(uint32_t);

	Size = Header.Size[0] + JpegHeaderSize;
	memcpy(&TargetBuffer[CurrentOffset], &((MipMapBufferList[0])[0]), Size);
	CurrentOffset += Size;

	for (i = 1; i < NrOfMipMaps; i++)
	{
		if (MipMapBufferList[i].size() <= 0) break;

		memcpy(&TargetBuffer[CurrentOffset], &((MipMapBufferList[i])[JpegHeaderSize]), Header.Size[i]);
		CurrentOffset += Header.Size[i];
	}
	return true;
}

bool Read(const BUFFER& SourceBuffer, BUFFER& TargetBuffer, int* Width, int* Height)
{
	BLP_HEADER Header;

	memcpy(reinterpret_cast<char*>(&Header), SourceBuffer.data(), sizeof(BLP_HEADER));
	if (Header.MagicNumber != '1PLB')
	{
		LOG("The file is not a BLP file!");
		return false;
	}

	switch (Header.Compression)
	{
	default:
	case 0:
		if (!LoadCompressed(Header, SourceBuffer, TargetBuffer)) return false;
		break;
	case 1:
		if (!LoadUncompressed(Header, SourceBuffer, TargetBuffer)) return false;
		break;
	}

	if (Width != NULL) (*Width) = Header.Width;
	if (Height != NULL) (*Height) = Header.Height;

	return true;
}

}}
