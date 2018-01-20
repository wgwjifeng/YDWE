#include "stdafx.h"
#include <base/i18n-2/gettext.h>
#include <base/util/unicode.h>

namespace DuiLib
{
	CFrameWindow::CFrameWindow() { };

	const wchar_t* CFrameWindow::GetWindowClassName() const { return L"FrameWindow"; };
	UINT CFrameWindow::GetClassStyle() const { return UI_CLASSSTYLE_FRAME | CS_DBLCLKS | CS_DROPSHADOW; };
	void CFrameWindow::OnFinalMessage(HWND /*hWnd*/) { };

	LRESULT CFrameWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// ȥ��ϵͳ�ı�����(��Ϊ�Ի�)
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		// ����Ƥ����
		m_pm.Init(m_hWnd);
		m_pm.SetResourceZip(GetSkinZip());

		CDialogBuilder builder;
		CControlUI* pRoot = builder.Create(GetSkinXml(), NULL, &m_pm);
		if (!pRoot) 
		{
			::MessageBoxW(NULL, base::u2w(base::i18n::v2::get_text("ERROR_LOAD_SKINS")).c_str(), L"YDWEConfig", MB_OK|MB_ICONERROR);
			::PostQuitMessage(0L);
			return 0;
		}
		m_pm.AttachDialog(pRoot);

		// ���������ʼ��
		InitWindow();

		return 0;
	}

	LRESULT CFrameWindow::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// ʵ�ֱ���������(���϶����ڵ�����)
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		RECT rcCaption = m_pm.GetCaptionRect();
		if (pt.x >= rcClient.left + rcCaption.left 
			&& pt.x < rcClient.right - rcCaption.right 
			&& pt.y >= rcCaption.top 
			&& pt.y < rcCaption.bottom) 
		{
			CControlUI* pControl = m_pm.FindControl(pt);
			if (pControl && _wcsicmp(pControl->GetClass(), DUI_CTR_BUTTON) != 0 && 
				_wcsicmp(pControl->GetClass(), DUI_CTR_CHECKBOX) != 0 &&
				_wcsicmp(pControl->GetClass(), DUI_CTR_RADIOBUTTON) != 0 &&
				_wcsicmp(pControl->GetClass(), DUI_CTR_TEXT) != 0)
			{
				return HTCAPTION;
			}
		}

		return HTCLIENT;
	}

	LRESULT CFrameWindow::OnDestory(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		::PostQuitMessage(0L);
		bHandled = FALSE;
		return 0;
	}

	LRESULT CFrameWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// ʵ��Բ�Ǵ���
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) 
		{
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT CFrameWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_ERASEBKGND) { return 1; }

		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch (uMsg)
		{
		case WM_CREATE:     lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:  lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:    lRes = OnDestory(uMsg, wParam, lParam, bHandled); break;
		case WM_SIZE:       lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		default: bHandled = FALSE;
		}

		if (bHandled) return lRes;
		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}
}
