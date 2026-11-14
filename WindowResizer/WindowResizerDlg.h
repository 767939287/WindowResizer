// WindowResizerDlg.h: 头文件
//

#pragma once

#include "AppConfig.h"
#include "ChromiumViewport.h"
#include "PickerOverlay.h"

/// 尺寸口径：决定用户看到/输入的数字代表窗口的哪一部分。
enum SizeMode
{
	ModeWindow = 0,   ///< 整个窗口（含边框、标题栏）
	ModeClient = 1,   ///< 客户区（不含边框标题栏）
	ModeViewport = 2  ///< 浏览器网页视口（仅 Chromium 系可用）
};

// CWindowResizerDlg 对话框
class CWindowResizerDlg : public CDialogEx
{
// 构造
public:
	CWindowResizerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WINDOWRESIZER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
private:
	// 界面中所有的编辑框控件变量
	CEdit m_editShowHandle;
	CEdit m_editShowTitle;
	CEdit m_editShowCurWidth;
	CEdit m_editShowCurHeight;
	CEdit m_editSetWidth;
	CEdit m_editSetHeight;
		CEdit m_editSetPercentage;

	// 运行时动态创建的控件
	CStatic m_staticDpiInfo;
	CButton m_checkDpiView;      //“窗口数据”区的100% DPI开关，仅控制显示
	CButton m_checkDpiSet;       //“设置大小”区的100% DPI开关，控制输入与设置
	CComboBox m_comboResolution;
	CStatic m_staticResLabel;    //“设置为主流分辨率”标签

	// 资源文件中已声明的三态口径与分辨率档案控件
	CComboBox m_comboSizeMode;   ///< 尺寸口径下拉框（下标与 SizeMode 枚举一一对应）
	CComboBox m_comboProfile;    ///< 分辨率档案下拉框
	CButton m_checkAutoSwitch;   ///< 「按进程自动切换」复选框
	CButton m_btnSaveProfile;    ///< 「保存为档案」按钮
	CStatic m_staticViewportInfo;///< 浏览器视口测量结果文本

	// 从指定窗口获取到的一些原始数据，不会被修改
	HWND m_hWindowHandle;       //查找到的窗口的句柄
	CString m_szWindowTitle;    //查找到的窗口的标题
	CWnd* m_pWnd{ nullptr };	//查找到的窗口的指针
	CSize m_sizeOriginalWindow; //查找到的窗口的原始大小

	int m_nScalePercentage{ 100 };          //窗口要缩放的百分比
	bool m_bScaleIsCustomized{ false };     //按比例缩放处是否选择了自定义
	bool m_bClickedSetPercentage{ false };  //最后一次设置大小是否为百分比缩放
	int m_nTempOriginalWidth;               //按比例缩放时需要临时记录当前窗口的大小作为缩放基准
	int m_nTempOriginalHeight;
	int m_nRadioSelect;                     //按比例缩放处控制选取哪一个复选框

	bool m_bIsSetMaxSize{ false };          //是否设置过最大化

	bool m_bUseLogicalSizeView{ false };    //“窗口数据”区是否按100% DPI显示
	bool m_bUseLogicalSizeSet{ false };     //“设置大小”区是否按100% DPI输入与设置
	UINT m_nTargetDpi{ 96 };                //目标窗口所在显示器的DPI，未选中窗口时为96（100%）

	CAppConfig m_config;                    // ini 配置文件与档案内存模型
	int m_nSizeMode{ ModeClient };          // 当前尺寸口径，默认客户区（与旧版默认行为一致）
	std::vector<CSize> m_currentResolutions;// 当前档案的分辨率列表（逻辑像素）

	bool m_bPicking{ false };               // 是否正在拾取目标窗口
	HCURSOR m_hCursorPick{ nullptr };       // 拾取时使用的十字光标（系统 IDC_CROSS）
	CPickerOverlay m_overlay;               // 拾取时的全屏蒙版（屏幕压暗 + 目标窗口凸显）

	void UpdateSizeShow(int width,int height); //更新对话框上宽度和高度的显示(参数为父窗口物理尺寸)
	bool IsWindowValid();                      //判断已选中的窗口是否有效
	void SetPercentageEditEnable();	           //设置按比例缩放处的编辑框是否可用
	void IsEnableAllControls(bool bIsEnable);  //设置除“开始查找窗口”按钮之外的所有控件的可用性
	void SetMaxSizeButtonEnable();             //设置最大化按钮是否可用
	UINT GetWindowDpi();                        //获取目标窗口所在显示器的DPI(Win10 1607+，失败回退程序DPI)
	int PhysicalToLogical(int value, bool bUseLogical);  //物理像素转逻辑尺寸(除以DPI/96)
	int LogicalToPhysical(int value, bool bUseLogical);  //逻辑尺寸转物理像素(乘以DPI/96)
	void UpdateDpiInfoShow();                  //更新“当前缩放”的显示文本
	void CreateDynamicControls();              //运行时动态创建新增控件（复选框、下拉框、缩放提示）

	/// 按当前口径取目标尺寸（物理像素）。
	/// @param nWidth  [out] 目标宽度（物理像素）；ModeViewport 下为渲染子窗口宽度。
	/// @param nHeight [out] 目标高度（物理像素）；ModeViewport 下为渲染子窗口高度。
	/// @return true 表示取到有效尺寸；无有效窗口时返回 false 且输出置 0。
	bool GetTargetSizePhysical(int& nWidth, int& nHeight);

	/// 由「客户区尺寸(物理像素)」反推「SetWindowPos 所需的窗口尺寸」。
	/// @param nClientWidth  客户区宽度（物理像素）。
	/// @param nClientHeight 客户区高度（物理像素）。
	/// @return 对应的窗口尺寸；窗口句柄无效或样式获取失败时回退为直接返回入参尺寸。
	CSize WindowSizeFromClientSize(int nClientWidth, int nClientHeight);

	/// 按当前口径把窗口设置为指定尺寸（物理像素，含义随口径变化）。
	/// @param nWidth  目标宽度（口径内）。
	/// @param nHeight 目标高度（口径内）。
	/// @return true 表示设置成功；ModeViewport 下回读校验不通过会内部回退客户区并返回 false。
	bool ApplyTargetSize(int nWidth, int nHeight);

	/// 按当前口径把父窗口物理尺寸换算成「用户口径下的显示值」（物理像素，未做 DPI 换算）。
	/// @param nWindowWidth  父窗口矩形宽。
	/// @param nWindowHeight 父窗口矩形高。
	/// @param nOutWidth  [out] 口径内宽。
	/// @param nOutHeight [out] 口径内高。
	void WindowSizeToModeSize(int nWindowWidth, int nWindowHeight, int& nOutWidth, int& nOutHeight);

	/// 按当前口径把「用户口径下的尺寸」换算成父窗口物理尺寸。
	/// @param nModeWidth  口径内宽（物理像素）。
	/// @param nModeHeight 口径内高（物理像素）。
	/// @param nOutWidth  [out] 父窗口宽。
	/// @param nOutHeight [out] 父窗口高。
	/// @return ModeViewport 下找不到渲染子窗口时返回 false。
	bool ModeSizeToWindowSize(int nModeWidth, int nModeHeight, int& nOutWidth, int& nOutHeight);

	/// 切换尺寸口径：同步下拉框选中项、刷新显示、按需移除/恢复「浏览器视口」项。
	/// @param nMode 目标口径。
	/// @param bSyncCombo true 时把下拉框选中项同步为 nMode。
	void SetSizeMode(int nMode, bool bSyncCombo);

	/// 根据当前目标窗口是否为 Chromium，决定「浏览器视口」项是否可选择。
	void UpdateViewportModeAvailability();

	/// 重新填充分辨率下拉框（先清空），数据来自当前档案。
	void RefreshResolutionCombo();

	/// 重新填充档案下拉框并选中当前激活档案。
	void RefreshProfileCombo();

	/// 按实测值刷新「浏览器视口测量」区的文本。
	void UpdateViewportInfoShow();

	/// 进入拾取状态：切十字光标、按钮文案改为「取消拾取」、提高轮询频率。
	void StartPicking();

	/// 退出拾取状态并复位所有视觉与定时器状态。
	/// @param bAccepted true 表示已成功拾取到目标窗口；false 表示用户取消或按 Esc 放弃。
	void StopPicking(bool bAccepted);

	/// 把拾取到的窗口信息填入界面（供蒙版拾取路径复用）。
	/// @param hTarget 目标窗口句柄，调用方需保证其有效。
	void FillTargetWindowInfo(HWND hTarget);

public:
	afx_msg void OnBnClickedButtonFindWindow();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonSetSize();
	afx_msg void OnBnClickedRadio50();
	afx_msg void OnBnClickedRadio100();
	afx_msg void OnBnClickedRadio125();
	afx_msg void OnBnClickedRadio150();
	afx_msg void OnBnClickedRadio200();
	afx_msg void OnBnClickedRadioCustom();
	afx_msg void OnBnClickedButtonSetPercentage();
	afx_msg void OnBnClickedButtonMaxSize();
	afx_msg void OnBnClickedButtonRestore();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedCheckDpi100View();
	afx_msg void OnBnClickedCheckDpi100Set();
	afx_msg void OnCbnSelchangeComboResolution();
	afx_msg void OnCbnSelchangeComboSizeMode();
	afx_msg void OnCbnSelchangeComboProfile();
	afx_msg void OnBnClickedCheckAutoswitch();
	afx_msg void OnBnClickedButtonSaveProfile();
	afx_msg LRESULT OnOverlayPickWindow(WPARAM wParam, LPARAM lParam);
};
