// WindowResizerDlg.h: 头文件
//

#pragma once


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

	/// 「快捷功能」区的尺寸口径开关：勾选=以整个窗口(含标题栏/边框)为尺寸目标，不勾=以客户区为目标。
	CButton m_checkClientArea;

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

	/// 尺寸口径开关：true=以客户区(viewport)为尺寸目标；false=以整个窗口(含标题栏/边框)为目标。
	/// 默认 true，对应复选框默认不勾（即默认按客户区）。
	bool m_bUseClientArea{ true };

	void UpdateSizeShow(int width,int height); //更新对话框上宽度和高度的显示(参数为物理像素)
	bool IsWindowValid();                      //判断已选中的窗口是否有效
	void SetPercentageEditEnable();	           //设置按比例缩放处的编辑框是否可用
	void IsEnableAllControls(bool bIsEnable);  //设置除“开始查找窗口”按钮之外的所有控件的可用性
	void SetMaxSizeButtonEnable();             //设置最大化按钮是否可用
	UINT GetWindowDpi();                        //获取目标窗口所在显示器的DPI(Win10 1607+，失败回退程序DPI)
	int PhysicalToLogical(int value, bool bUseLogical);  //物理像素转逻辑尺寸(除以DPI/96)
	int LogicalToPhysical(int value, bool bUseLogical);  //逻辑尺寸转物理像素(乘以DPI/96)
	void UpdateDpiInfoShow();                  //更新“当前缩放”的显示文本
	void CreateDynamicControls();              //运行时动态创建新增控件（复选框、下拉框、缩放提示）

	/// 按当前口径取目标尺寸（物理像素）：客户区模式用 GetClientRect，否则用 GetWindowRect。
	/// @param nWidth  [out] 目标宽度（物理像素），无有效窗口时输出 0。
	/// @param nHeight [out] 目标高度（物理像素），无有效窗口时输出 0。
	void GetTargetSizePhysical(int& nWidth, int& nHeight);

	/// 由「客户区尺寸(物理像素)」反推「SetWindowPos 所需的窗口尺寸」。
	/// @param nClientWidth  客户区宽度（物理像素）。
	/// @param nClientHeight 客户区高度（物理像素）。
	/// @return 对应的窗口尺寸；窗口句柄无效或样式获取失败时回退为直接返回入参尺寸。
	CSize WindowSizeFromClientSize(int nClientWidth, int nClientHeight);

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
	afx_msg void OnBnClickedCheckClientArea();
};






