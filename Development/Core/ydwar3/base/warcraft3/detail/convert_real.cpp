#include <base/warcraft3/jass.h>

namespace base { namespace warcraft3 { namespace jass {
	/*
	Jass��ʵ������ת����
	Ϊ�˷�ֹ���������Ż�������ѱ���ѡ���еġ�����ģ�͡���Ϊ���ϸ񡱡�
	*/
	float from_real(jreal_t val)
	{
		return *(float*)&val;
	}

	jreal_t to_real(float val)
	{
		return *(jreal_t*)&val;
	}
}}}
