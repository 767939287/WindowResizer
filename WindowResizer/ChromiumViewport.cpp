// ChromiumViewport.cpp: Chromium 系浏览器视口探测与精确尺寸反推的实现
//

#include "stdafx.h"
#include "ChromiumViewport.h"

namespace
{
	// 渲染子窗口的类名（Chrome/Edge/Brave/Opera/Vivaldi 共用同一套窗口结构）
	const TCHAR* const kRenderWidgetClass = _T("Chrome_RenderWidgetHostHWND");

	// 递归枚举上下文：记录面积最大的渲染子窗口
	struct FindRenderCtx
	{
		HWND hBest;      ///< 当前找到的最大面积渲染子窗口
		LONG nBestArea;  ///< 对应的面积（像素平方）
		int nDepth;      ///< 当前递归深度
	};

	// EnumChildWindows 回调：命中渲染子窗口则记录，并继续向下递归
	BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
	{
		FindRenderCtx* pCtx = reinterpret_cast<FindRenderCtx*>(lParam);
		if (pCtx == nullptr)
			return FALSE;

		TCHAR szClass[64] = { 0 };
		::GetClassName(hWnd, szClass, _countof(szClass));

		if (_tcsicmp(szClass, kRenderWidgetClass) == 0)
		{
			CRect rc;
			::GetWindowRect(hWnd, &rc);
			// 取面积最大者：页面里可能存在隐藏的 0 尺寸同 Class 窗口，用面积即可排除
			LONG nArea = static_cast<LONG>(rc.Width()) * rc.Height();
			if (nArea > pCtx->nBestArea)
			{
				pCtx->nBestArea = nArea;
				pCtx->hBest = hWnd;
			}
		}

		// Chromium 的内部窗口有多层嵌套，需要继续向下找；限制深度以防御异常结构导致栈溢出
		if (pCtx->nDepth < 8)
		{
			FindRenderCtx sub = { nullptr, pCtx->nBestArea, pCtx->nDepth + 1 };
			::EnumChildWindows(hWnd, EnumChildProc, reinterpret_cast<LPARAM>(&sub));
			if (sub.nBestArea > pCtx->nBestArea)
			{
				pCtx->nBestArea = sub.nBestArea;
				pCtx->hBest = sub.hBest;
			}
		}

		return TRUE;
	}
}

bool Chromium::GetProcessImageName(HWND hWnd, CString& szImageName)
{
	szImageName.Empty();
	if (hWnd == NULL || !::IsWindow(hWnd))
		return false;

	DWORD dwPid = 0;
	::GetWindowThreadProcessId(hWnd, &dwPid);
	if (dwPid == 0)
		return false;

	// 只在需要查询路径时申请最小权限，受保护进程会返回 NULL，此时静默失败
	HANDLE hProc = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPid);
	if (hProc == NULL)
		return false;

	typedef BOOL(WINAPI *QueryFullProcessImageNameWProc)(HANDLE, DWORD, LPWSTR, PDWORD);
	static QueryFullProcessImageNameWProc pQuery = (QueryFullProcessImageNameWProc)::GetProcAddress(
		::GetModuleHandle(_T("kernel32.dll")), "QueryFullProcessImageNameW");

	bool bOk = false;
	if (pQuery != nullptr)
	{
		WCHAR szPath[MAX_PATH * 2] = { 0 };
		DWORD dwSize = static_cast<DWORD>(_countof(szPath));
		if (pQuery(hProc, 0, szPath, &dwSize) && dwSize > 0)
		{
			// 只保留文件名的最后一段（去掉完整路径）
			CString szFull(szPath);
			int nPos = szFull.ReverseFind(_T('\\'));
			szImageName = (nPos >= 0) ? szFull.Mid(nPos + 1) : szFull;
			bOk = !szImageName.IsEmpty();
		}
	}

	::CloseHandle(hProc);
	return bOk;
}

bool Chromium::IsChromiumWindow(HWND hWnd, CString& szProcess)
{
	szProcess.Empty();
	if (hWnd == NULL || !::IsWindow(hWnd))
		return false;

	TCHAR szClass[64] = { 0 };
	::GetClassName(hWnd, szClass, _countof(szClass));
	if (_tcsicmp(szClass, _T("Chrome_WidgetWin_1")) != 0 &&
		_tcsicmp(szClass, _T("Chrome_WidgetWin_0")) != 0)
	{
		return false;
	}

	CString szImage;
	if (!GetProcessImageName(hWnd, szImage))
		return false;

	// Electron 应用（VS Code/Discord 等）也用 Chrome_WidgetWin_1，其内容区层级结构不同，
	// 反推会算出离谱的边框，故必须用进程名二次确认
	static const TCHAR* const kExeWhitelist[] = {
		_T("chrome.exe"), _T("msedge.exe"), _T("brave.exe"),
		_T("opera.exe"), _T("vivaldi.exe"), _T("chromium.exe")
	};

	for (int i = 0; i < _countof(kExeWhitelist); ++i)
	{
		if (szImage.CompareNoCase(kExeWhitelist[i]) == 0)
		{
			szProcess = szImage;
			return true;
		}
	}

	return false;
}

HWND Chromium::FindRenderWidget(HWND hTop)
{
	if (hTop == NULL || !::IsWindow(hTop))
		return NULL;

	FindRenderCtx ctx = { nullptr, 0, 0 };
	::EnumChildWindows(hTop, EnumChildProc, reinterpret_cast<LPARAM>(&ctx));
	return ctx.hBest;
}

bool Chromium::Measure(HWND hTop, int& nViewportW, int& nViewportH,
	int& nDeltaW, int& nDeltaH, int& nBorderW, int& nBorderH)
{
	nViewportW = 0;
	nViewportH = 0;
	nDeltaW = 0;
	nDeltaH = 0;
	nBorderW = 0;
	nBorderH = 0;

	if (hTop == NULL || !::IsWindow(hTop))
		return false;

	HWND hRender = FindRenderWidget(hTop);
	if (hRender == NULL)
		return false;

	CRect rcWin, rcClient, rcRender;
	::GetWindowRect(hTop, &rcWin);
	::GetClientRect(hTop, &rcClient);
	::GetWindowRect(hRender, &rcRender);

	// 全部取物理像素，不做 DPI 换算：GetWindowRect/GetClientRect 本身就返回物理像素，
	// 再掺入换算反而会让反推结果偏离
	nViewportW = rcRender.Width();
	nViewportH = rcRender.Height();

	nDeltaW = rcClient.Width() - nViewportW;
	nDeltaH = rcClient.Height() - nViewportH;
	nBorderW = rcWin.Width() - rcClient.Width();
	nBorderH = rcWin.Height() - rcClient.Height();

	// 出现滚动条等情况时差值仍应非负；负值说明窗口结构异常，按 0 处理避免反推出更小的窗口
	if (nDeltaW < 0)
		nDeltaW = 0;
	if (nDeltaH < 0)
		nDeltaH = 0;
	if (nBorderW < 0)
		nBorderW = 0;
	if (nBorderH < 0)
		nBorderH = 0;

	return true;
}

bool Chromium::ComputeWindowSizeForViewport(HWND hTop, int nTargetViewportW, int nTargetViewportH,
	int& nOutWindowW, int& nOutWindowH)
{
	if (nTargetViewportW <= 0 || nTargetViewportH <= 0)
		return false;

	int nVW = 0, nVH = 0, nDW = 0, nDH = 0, nBW = 0, nBH = 0;
	if (!Measure(hTop, nVW, nVH, nDW, nDH, nBW, nBH))
		return false;

	// 用现测差值而非固定公式：开了书签栏/侧边栏/下载面板后 nDeltaH 完全不同
	nOutWindowW = nTargetViewportW + nDW + nBW;
	nOutWindowH = nTargetViewportH + nDH + nBH;
	return true;
}

bool Chromium::VerifyViewportSize(HWND hTop, int nTargetW, int nTargetH, int nTolerance)
{
	if (nTargetW <= 0 || nTargetH <= 0)
		return false;

	HWND hRender = FindRenderWidget(hTop);
	if (hRender == NULL)
		return false;

	CRect rc;
	::GetWindowRect(hRender, &rc);

	return (abs(rc.Width() - nTargetW) <= nTolerance &&
		abs(rc.Height() - nTargetH) <= nTolerance);
}
