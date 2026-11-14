// PickerOverlay.h: 拾取目标窗口时的全屏蒙版（屏幕压暗 + 目标窗口挖空凸显）
//

#pragma once

/// 拾取结果回传消息：wParam 为选中的窗口句柄，为 NULL 表示用户取消。
#define WM_PICKER_RESULT (WM_APP + 100)

/// 拾取时的全屏蒙版窗口。
/// 职责单一：只负责绘制蒙版、计算鼠标下的目标窗口并把结果回传给主对话框，
/// 不做尺寸口径、配置档案等任何业务判断。
class CPickerOverlay : public CWnd
{
public:
	CPickerOverlay();
	virtual ~CPickerOverlay();

	/// 创建并显示全屏蒙版。
	/// @param pSink 结果接收方（主对话框），需响应 WM_PICKER_RESULT 消息。
	/// @return true 表示创建成功；虚拟屏幕尺寸异常或位图分配失败时返回 false。
	bool Begin(CWnd* pSink);

	/// 销毁蒙版并释放位图资源。可重复调用。
	void End();

	/// 蒙版当前是否已创建且有效。
	bool IsActive() const;

protected:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

private:
	/// 取屏幕坐标点下最上层的可见窗口。
	/// 按 Z 序自上而下遍历顶层窗口取第一个矩形包含该点的窗口。
	/// 不能用 WindowFromPoint——它只会返回蒙版自身。
	/// @param ptScreen 屏幕坐标（物理像素）。
	/// @return 命中窗口句柄；无命中返回 NULL。
	HWND WindowFromPointTopMost(CPoint ptScreen) const;

	/// 判断窗口是否应被排除在拾取目标之外。
	/// 排除项：自身、主对话框、不可见/最小化窗口、桌面与任务栏、子窗口。
	/// @param hWnd 待判断窗口。
	/// @return true 表示不作为拾取目标。
	bool IsExcludedWindow(HWND hWnd) const;

	/// 按虚拟屏幕尺寸创建或重建 32bpp DIB 及内存 DC。
	/// @return true 表示准备成功。
	bool EnsureDib();

	/// 释放 DIB 与内存 DC。可重复调用。
	void ReleaseDib();

	/// 重建蒙版位图并提交到分层窗口。
	/// 整屏压暗、目标窗口矩形挖空（保持原亮度）、并绘制橙色描边。
	void RedrawMask();

	bool   m_bLButtonWasDown{ false }; ///< 上次轮询时左键是否按下，用于取「按下沿」
	HWND   m_hTarget { nullptr };   ///< 当前凸显的目标窗口
	CWnd*  m_pSink   { nullptr };   ///< 结果接收方（主对话框）
	HWND   m_hSinkWnd{ nullptr };   ///< 结果接收方窗口句柄，用于排除自身程序
	HBITMAP m_hBmp   { nullptr };   ///< 32bpp DIB 位图
	HDC    m_hMemDC  { nullptr };   ///< 与位图绑定的内存 DC
	void*  m_pBits   { nullptr };   ///< DIB 像素首地址（自上而下排列）
	CSize  m_sizeVirtual;           ///< 虚拟屏幕尺寸（物理像素）
	CPoint m_ptOrigin;              ///< 虚拟屏幕左上角坐标（多显示器时可能为负）
};
