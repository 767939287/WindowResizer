// ChromiumViewport.h: Chromium 系浏览器视口探测与精确尺寸反推
//

#pragma once

/// Chromium 系浏览器视口探测与精确尺寸反推（无状态工具函数集）。
/// 所有函数在探测失败时返回 false/NULL 而不抛异常，调用方据此降级到客户区口径。
namespace Chromium
{
	/// 取窗口所属进程的映像名（含扩展名，如 "chrome.exe"）。
	/// @param hWnd 目标窗口句柄。
	/// @param szImageName [out] 映像名；失败时置空。
	/// @return true 表示取到；进程已退出或权限不足时返回 false（静默，不提示用户）。
	bool GetProcessImageName(HWND hWnd, CString& szImageName);

	/// 判断窗口是否为 Chromium 系浏览器主窗口。
	/// 类名与进程名需同时命中白名单，避免把 Electron 应用（VS Code/Discord 等）误判进来。
	/// @param hWnd 待判定窗口。
	/// @param szProcess [out] 命中时返回其进程映像名，便于绑定档案。
	/// @return true 表示是 Chromium 系浏览器主窗口。
	bool IsChromiumWindow(HWND hWnd, CString& szProcess);

	/// 在指定顶层窗口下递归查找网页渲染子窗口。
	/// @param hTop 顶层窗口句柄。
	/// @return 面积最大的、类名为 "Chrome_RenderWidgetHostHWND" 的子窗口；找不到返回 NULL。
	HWND FindRenderWidget(HWND hTop);

	/// 实测浏览器 UI 占位与边框厚度（全部为物理像素）。
	/// @param hTop 顶层窗口。
	/// @param nViewportW [out] 当前渲染子窗口宽度。
	/// @param nViewportH [out] 当前渲染子窗口高度。
	/// @param nDeltaW [out] 客户区宽 − 视口宽（垂直滚动条等宽度损耗），不小于 0。
	/// @param nDeltaH [out] 客户区高 − 视口高（标签栏、地址栏、书签栏等 UI 占高）。
	/// @param nBorderW [out] 窗口宽 − 客户区宽（左右边框）。
	/// @param nBorderH [out] 窗口高 − 客户区高（标题栏 + 上下边框）。
	/// @return true 表示测量成功；渲染子窗口缺失时返回 false 且所有输出置 0。
	bool Measure(HWND hTop, int& nViewportW, int& nViewportH,
		int& nDeltaW, int& nDeltaH, int& nBorderW, int& nBorderH);

	/// 按目标视口尺寸反推应设置的父窗口尺寸（物理像素）。
	/// @param hTop 顶层窗口。
	/// @param nTargetViewportW 目标视口宽。
	/// @param nTargetViewportH 目标视口高。
	/// @param nOutWindowW [out] 父窗口宽。
	/// @param nOutWindowH [out] 父窗口高。
	/// @return true 表示反推成功；测量失败时返回 false，输出保持不变。
	bool ComputeWindowSizeForViewport(HWND hTop, int nTargetViewportW, int nTargetViewportH,
		int& nOutWindowW, int& nOutWindowH);

	/// 回读渲染子窗口尺寸，判断设定后是否被浏览器布局覆盖回弹。
	/// @param hTop 顶层窗口。
	/// @param nTargetW 期望视口宽。
	/// @param nTargetH 期望视口高。
	/// @param nTolerance 允许误差（物理像素）。
	/// @return true 表示实际尺寸在容差内，视为设置生效。
	bool VerifyViewportSize(HWND hTop, int nTargetW, int nTargetH, int nTolerance = 2);
}
