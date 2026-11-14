// PickerOverlay.cpp: 拾取目标窗口时的全屏蒙版实现
//

#include "stdafx.h"
#include "PickerOverlay.h"

#include <algorithm>

namespace
{
	// 整屏压暗用的颜色：BGRA 全 0 为黑色，alpha=110 控制压暗程度（约 43%）
	const DWORD kDarkPixel = 0x6E000000;

	// 目标窗口描边色：BGRA 形式，B=0、G=120、R=255、A=255（橙色）
	// 注意：DWORD 按小端解析，最低字节是 B；原值 0xFF0078FF 实际画出来是蓝色
	const DWORD kEdgePixel = 0xFFFF7800;

	// 描边宽度（像素）
	const int kEdgeWidth = 2;

	// 目标窗口挖空区的像素：alpha=1 而非 0。
	// 分层窗口 alpha=0 的像素区域不参与命中测试，鼠标消息会穿透到下层窗口，
	// 导致点击目标窗口时蒙版收不到 WM_LBUTTONDOWN，从而无法完成拾取。
	// alpha=1 的视觉差异（1/255 不透明度）肉眼几乎不可见，但能正常接收鼠标消息。
	const DWORD kHolePixel = 0x00000001;

	// Z 序遍历的防御上限，避免异常窗口链导致死循环
	const int kMaxWindowWalk = 10000;
}

BEGIN_MESSAGE_MAP(CPickerOverlay, CWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CPickerOverlay::CPickerOverlay()
{
	m_sizeVirtual.cx = 0;
	m_sizeVirtual.cy = 0;
	m_ptOrigin.x = 0;
	m_ptOrigin.y = 0;
}

CPickerOverlay::~CPickerOverlay()
{
	ReleaseDib();
}

bool CPickerOverlay::IsActive() const
{
	return m_hWnd != NULL && ::IsWindow(m_hWnd);
}

bool CPickerOverlay::Begin(CWnd* pSink)
{
	if (IsActive())
		return true;
	if (pSink == nullptr || pSink->GetSafeHwnd() == NULL)
		return false;

	m_pSink = pSink;
	m_hSinkWnd = pSink->GetSafeHwnd();
	m_hTarget = nullptr;
	m_bLButtonWasDown = false;

	// 用虚拟屏幕而非主屏：多显示器时原点可能为负（左侧副屏），写死 (0,0) 会漏掉那块区域
	m_sizeVirtual.cx = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
	m_sizeVirtual.cy = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
	m_ptOrigin.x = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_ptOrigin.y = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
	if (m_sizeVirtual.cx <= 0 || m_sizeVirtual.cy <= 0)
		return false;

	LPCTSTR szClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
		::LoadCursor(NULL, IDC_CROSS), NULL, NULL);

	// WS_EX_NOACTIVATE 保证蒙版不抢焦点（主对话框不失活）；
	// WS_EX_TOOLWINDOW 让它不出现在 Alt+Tab；WS_EX_LAYERED 用于逐像素 alpha
	if (!CreateEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
		szClass, _T("WindowResizerPicker"), WS_POPUP,
		m_ptOrigin.x, m_ptOrigin.y, m_sizeVirtual.cx, m_sizeVirtual.cy, NULL, NULL))
	{
		return false;
	}

	if (!EnsureDib())
	{
		DestroyWindow();
		return false;
	}

	::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);

	// 60ms 检测 Esc：蒙版不抢焦点，收不到 WM_KEYDOWN，只能读全局键态
	SetTimer(1, 60, NULL);

	RedrawMask();
	::SetCursor(::LoadCursor(NULL, IDC_CROSS));
	return true;
}

void CPickerOverlay::End()
{
	if (m_hWnd != NULL && ::IsWindow(m_hWnd))
	{
		KillTimer(1);
		ShowWindow(SW_HIDE);
		DestroyWindow();
	}
	ReleaseDib();
	m_hTarget = nullptr;
	m_pSink = nullptr;
	m_hSinkWnd = nullptr;
	m_bLButtonWasDown = false;
}

bool CPickerOverlay::EnsureDib()
{
	ReleaseDib();

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = m_sizeVirtual.cx;
	// 高度取负值：让 DIB 自上而下排列，便于按行用指针直接填充
	bmi.bmiHeader.biHeight = -m_sizeVirtual.cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hScreen = ::GetDC(NULL);
	if (hScreen == NULL)
		return false;
	m_hMemDC = ::CreateCompatibleDC(hScreen);
	::ReleaseDC(NULL, hScreen);
	if (m_hMemDC == NULL)
		return false;

	void* pBits = nullptr;
	m_hBmp = ::CreateDIBSection(m_hMemDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	if (m_hBmp == NULL || pBits == nullptr)
	{
		ReleaseDib();
		return false;
	}

	m_pBits = pBits;
	::SelectObject(m_hMemDC, m_hBmp);
	return true;
}

void CPickerOverlay::ReleaseDib()
{
	if (m_hMemDC != NULL)
	{
		::DeleteDC(m_hMemDC);
		m_hMemDC = nullptr;
	}
	if (m_hBmp != NULL)
	{
		::DeleteObject(m_hBmp);
		m_hBmp = nullptr;
	}
	m_pBits = nullptr;
}

void CPickerOverlay::RedrawMask()
{
	if (m_pBits == nullptr || m_sizeVirtual.cx <= 0 || m_sizeVirtual.cy <= 0)
		return;

	DWORD* pdwBase = reinterpret_cast<DWORD*>(m_pBits);
	const int nW = m_sizeVirtual.cx;
	const int nH = m_sizeVirtual.cy;

	// 先把整屏压暗，再在目标窗口处挖空
	std::fill(pdwBase, pdwBase + static_cast<size_t>(nW) * nH, kDarkPixel);

	// 按行填充一个水平区间的像素，自动裁剪到屏幕范围之外的部分
	auto FillSpan = [&](int y, int x1, int x2, DWORD clr)
	{
		if (y < 0 || y >= nH)
			return;
		if (x1 < 0)
			x1 = 0;
		if (x2 > nW)
			x2 = nW;
		if (x1 >= x2)
			return;
		DWORD* row = pdwBase + static_cast<size_t>(y) * nW;
		std::fill(row + x1, row + x2, clr);
	};

	if (m_hTarget != nullptr && ::IsWindow(m_hTarget))
	{
		CRect rc;
		::GetWindowRect(m_hTarget, &rc);

		// 屏幕坐标换算成蒙版本地坐标（虚拟屏幕原点可能为负）
		int L = rc.left - m_ptOrigin.x;
		int T = rc.top - m_ptOrigin.y;
		int R = rc.right - m_ptOrigin.x;
		int B = rc.bottom - m_ptOrigin.y;

		// 挖空：目标窗口保持原始亮度（实时内容，不是截图快照）。
		// 使用 kHolePixel（alpha=1）而非 0，避免鼠标穿透导致点击丢失
		for (int y = T; y < B; ++y)
			FillSpan(y, L, R, kHolePixel);

		// 描边：在挖空区内侧四边各画 kEdgeWidth 像素，形成橙色高亮框
		for (int y = T; y < T + kEdgeWidth; ++y)
			FillSpan(y, L, R, kEdgePixel);
		for (int y = B - kEdgeWidth; y < B; ++y)
			FillSpan(y, L, R, kEdgePixel);
		for (int y = T + kEdgeWidth; y < B - kEdgeWidth; ++y)
		{
			FillSpan(y, L, L + kEdgeWidth, kEdgePixel);
			FillSpan(y, R - kEdgeWidth, R, kEdgePixel);
		}
	}

	// 主对话框区域挖空：此处刻意用 alpha=0（与上面的目标窗口挖空区相反）。
	// alpha=0 会让鼠标穿透到主对话框，使用户能直接点到「取消拾取」按钮退出，
	// 而不必被迫按 Esc；同时该区域不被压暗，用户能看清自己的程序。
	if (m_hSinkWnd != NULL && ::IsWindow(m_hSinkWnd))
	{
		CRect rcSink;
		::GetWindowRect(m_hSinkWnd, &rcSink);
		const int nL = rcSink.left - m_ptOrigin.x;
		const int nT = rcSink.top - m_ptOrigin.y;
		const int nR = rcSink.right - m_ptOrigin.x;
		const int nB = rcSink.bottom - m_ptOrigin.y;
		for (int y = nT; y < nB; ++y)
			FillSpan(y, nL, nR, 0);
	}

	// 提交到分层窗口：BLENDFUNCTION 使用逐像素 alpha
	HDC hScreen = ::GetDC(NULL);
	if (hScreen == NULL)
		return;

	POINT ptSrc = { 0, 0 };
	SIZE sizeWin = { nW, nH };
	POINT ptDst = { m_ptOrigin.x, m_ptOrigin.y };
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

	::UpdateLayeredWindow(m_hWnd, hScreen, &ptDst, &sizeWin, m_hMemDC, &ptSrc, 0, &bf, ULW_ALPHA);
	::ReleaseDC(NULL, hScreen);
}

bool CPickerOverlay::IsExcludedWindow(HWND hWnd) const
{
	if (hWnd == NULL)
		return true;
	// 自身与主对话框不作为目标：主对话框被蒙版覆盖，鼠标划过时不能把自己选进去
	if (hWnd == m_hWnd || hWnd == m_hSinkWnd)
		return true;
	if (!::IsWindowVisible(hWnd))
		return true;
	if (::IsIconic(hWnd))
		return true;

	// 只接受顶层窗口，子窗口由父窗口代表
	if ((::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) != 0)
		return true;

	TCHAR szClass[64] = { 0 };
	::GetClassName(hWnd, szClass, _countof(szClass));
	// 桌面与任务栏不算可操作窗口，否则鼠标停在空白处会高亮整个桌面
	if (_tcsicmp(szClass, _T("Progman")) == 0 ||
		_tcsicmp(szClass, _T("WorkerW")) == 0 ||
		_tcsicmp(szClass, _T("Shell_TrayWnd")) == 0 ||
		_tcsicmp(szClass, _T("Shell_SecondaryTrayWnd")) == 0)
	{
		return true;
	}

	return false;
}

HWND CPickerOverlay::WindowFromPointTopMost(CPoint ptScreen) const
{
	// 按 Z 序自上而下找第一个命中点且不被排除的顶层窗口
	HWND hWnd = ::GetTopWindow(NULL);
	int nWalk = 0;
	while (hWnd != NULL && nWalk++ < kMaxWindowWalk)
	{
		if (!IsExcludedWindow(hWnd))
		{
			CRect rc;
			if (::GetWindowRect(hWnd, &rc) && rc.PtInRect(ptScreen))
				return hWnd;
		}
		hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
	}
	return NULL;
}

void CPickerOverlay::OnMouseMove(UINT nFlags, CPoint point)
{
	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);

	HWND hHit = WindowFromPointTopMost(ptScreen);
	// 仅当目标变化时才整屏重绘：1920x1080x4 约 8MB，逐帧重算会明显占用 CPU
	if (hHit != m_hTarget)
	{
		m_hTarget = hHit;
		RedrawMask();
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CPickerOverlay::OnLButtonDown(UINT nFlags, CPoint point)
{
	UNREFERENCED_PARAMETER(nFlags);

	// 回传前按点击位置重算一次：若鼠标未产生移动事件，m_hTarget 可能仍为空
	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);
	HWND hPick = WindowFromPointTopMost(ptScreen);
	if (hPick == NULL)
		hPick = m_hTarget;   // 兜底

	CWnd* pSink = m_pSink;
	End();

	if (pSink != nullptr && pSink->GetSafeHwnd() != NULL)
		pSink->SendMessage(WM_PICKER_RESULT, reinterpret_cast<WPARAM>(hPick), 0);
}

void CPickerOverlay::OnRButtonDown(UINT nFlags, CPoint point)
{
	UNREFERENCED_PARAMETER(nFlags);
	UNREFERENCED_PARAMETER(point);

	// 右键视为取消
	CWnd* pSink = m_pSink;
	End();

	if (pSink != nullptr && pSink->GetSafeHwnd() != NULL)
		pSink->SendMessage(WM_PICKER_RESULT, 0, 0);
}

BOOL CPickerOverlay::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	UNREFERENCED_PARAMETER(pWnd);
	UNREFERENCED_PARAMETER(nHitTest);
	UNREFERENCED_PARAMETER(message);

	// 蒙版覆盖整屏且不主动改变光标形状，这里固定维持十字，避免被子控件覆盖成箭头
	::SetCursor(::LoadCursor(NULL, IDC_CROSS));
	return TRUE;
}

void CPickerOverlay::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		// 蒙版不抢焦点，收不到 WM_KEYDOWN，只能读全局键态判断 Esc
		if ((::GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
		{
			CWnd* pSink = m_pSink;
			End();
			if (pSink != nullptr && pSink->GetSafeHwnd() != NULL)
				pSink->SendMessage(WM_PICKER_RESULT, 0, 0);
			return;
		}

		// 用轮询而非依赖 WM_LBUTTONDOWN：分层窗口按像素 alpha 做命中测试，
		// 挖空区（目标窗口）的点击会被判定为穿透，OnLButtonDown 根本收不到，
		// 导致拾取永远无法完成。这里直接读全局左键状态即可绕开命中测试。
		const bool bDownNow = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		const bool bPressed = bDownNow && !m_bLButtonWasDown;
		m_bLButtonWasDown = bDownNow;

		if (bPressed)
		{
			POINT pt = { 0, 0 };
			::GetCursorPos(&pt);

			// 挖空区穿透后，WindowFromPoint 返回的正是用户真正点到的那个窗口
			HWND hHit = ::WindowFromPoint(pt);
			if (hHit != NULL)
				hHit = ::GetAncestor(hHit, GA_ROOT);   // 子控件归到其顶层窗口

			// 命中主对话框（含其子控件）时不拾取，交给「取消拾取」按钮等自行处理
			const bool bOnSink = (hHit != NULL &&
				(hHit == m_hSinkWnd || ::IsChild(m_hSinkWnd, hHit)));

			if (hHit != NULL && hHit != m_hWnd && !bOnSink)
			{
				CWnd* pSink = m_pSink;
				End();
				if (pSink != nullptr && pSink->GetSafeHwnd() != NULL)
					pSink->SendMessage(WM_PICKER_RESULT, reinterpret_cast<WPARAM>(hHit), 0);
				return;
			}
		}

		// 鼠标停在其他窗口上方时对方的 WM_SETCURSOR 会把光标改回箭头，这里周期性拉回
		::SetCursor(::LoadCursor(NULL, IDC_CROSS));
	}

	CWnd::OnTimer(nIDEvent);
}

void CPickerOverlay::OnDestroy()
{
	KillTimer(1);
	ReleaseDib();
	CWnd::OnDestroy();
}
