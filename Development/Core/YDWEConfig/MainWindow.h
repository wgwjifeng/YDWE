#pragma once

#include <DuiLib/UIlib.h>
#include <base/util/ini.h>

class CMainWindow : public DuiLib::CFrameWindow, public DuiLib::INotifyUI
{
public:
	CMainWindow();
	LPCTSTR GetWindowClassName() const;
	void OnFinalMessage(HWND /*hWnd*/);

	void InitWindow();
	fs::path GetSkinZip() const;
	const wchar_t* GetSkinXml() const { return L"DialogConfig.xml"; };

	void ContrlSelected(std::string const& name, bool bSelect);
	void ContrlSetEnabled(std::string const& name, bool bEnable);
	void EnableMapSave(bool bEnable);
	void EnableScriptInjection(bool bEnable); 
	void EnableHostTest(bool bEnable);
	void DisableCJass(bool bEnable);
	void EnableJassHelper(bool bEnable);
	void Notify(DuiLib::TNotifyUI& msg);

	void ResetConfig(base::ini::table& table);
	bool LoadConfig(base::ini::table& table);
	bool SaveConfig(base::ini::table const& table);
	void ConfigToUI(base::ini::table& table);
	void UIToConfig(base::ini::table& table);
	void InitOSHelpUI();
	void DoneOSHelpUI();
	void InitRegistryUI();
	void DoneRegistryUI();	
	void InitPatchUI(base::ini::table& table);
	void DonePatchUI(base::ini::table& table);
	void InitPluginUI();
	void DonePluginUI();
	void InitFontUI();
	void UpdateWarcraft3Directory();

private:
	std::map<std::string, DuiLib::CCheckBoxUI*> m_controls;
	std::map<std::string, DuiLib::CComboUI*>    m_comboboxs;

	DuiLib::CCheckBoxUI*       m_pEnableJassHelper;
	DuiLib::CCheckBoxUI*       m_pEnableCJass;
	DuiLib::CCheckBoxUI*       m_pLaunchWindowed;
	DuiLib::CCheckBoxUI*       m_pLaunchWideScreenSupport;
	DuiLib::CCheckBoxUI*       m_pLaunchFixedRatioWindowed;
	DuiLib::CCheckBoxUI*       m_pEnableHost;
	DuiLib::CCheckBoxUI*       m_pFileAssociation_w3x;
	DuiLib::CCheckBoxUI*       m_pFileAssociation_w3m;
	DuiLib::CCheckBoxUI*       m_pShortcuts_desktop;
	DuiLib::CCheckBoxUI*       m_pShortcuts_taskbar;
	DuiLib::CCheckBoxUI*       m_pAllowLocalFiles;
	DuiLib::CVerticalLayoutUI* m_pWar3PatchList;
	DuiLib::CVerticalLayoutUI* m_pWar3PluginList;
	DuiLib::CLabelUI*          m_pWarcraft3Directory;
	DuiLib::CCheckBoxUI*       m_pFontEnable;
	DuiLib::CComboUI*          m_pFontName;
	DuiLib::CComboUI*          m_pFontSize;
	DuiLib::CLabelUI*          m_pFontPreview;

	fs::path                   m_ydwe_path;
	std::string                m_username;
	std::string                m_virtualmpq;
};
