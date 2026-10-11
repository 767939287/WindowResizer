
// WindowResizerDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowResizer.h"
#include "WindowResizerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClickSyslinkCheckUpdate(NMHDR *pNMHDR, LRESULT *pResult);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_CHECK_UPDATE, &CAboutDlg::OnNMClickSyslinkCheckUpdate)
END_MESSAGE_MAP()


// CWindowResizerDlg 对话框



CWindowResizerDlg::CWindowResizerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WINDOWRESIZER_DIALOG, pParent)
	, m_nRadioSelect(1)      //启动时按比例缩放处默认选取100%
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWindowResizerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_HANDLE, m_editShowHandle);
	DDX_Control(pDX, IDC_EDIT_TITLE, m_editShowTitle);
	DDX_Control(pDX, IDC_EDIT_WIDTH, m_editShowCurWidth);
	DDX_Control(pDX, IDC_EDIT_HEIGHT, m_editShowCurHeight);
	DDX_Control(pDX, IDC_EDIT_SET_WIDTH, m_editSetWidth);
	DDX_Control(pDX, IDC_EDIT_SET_HEIGHT, m_editSetHeight);
	DDX_Control(pDX, IDC_EDIT_PERCENTAGE_CUSTOM, m_editSetPercentage);
	DDX_Radio(pDX, IDC_RADIO_50, m_nRadioSelect);
}

BEGIN_MESSAGE_MAP(CWindowResizerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_FIND_WINDOW, &CWindowResizerDlg::OnBnClickedButtonFindWindow)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SET_SIZE, &CWindowResizerDlg::OnBnClickedButtonSetSize)
	ON_BN_CLICKED(IDC_RADIO_50, &CWindowResizerDlg::OnBnClickedRadio50)
	ON_BN_CLICKED(IDC_RADIO_100, &CWindowResizerDlg::OnBnClickedRadio100)
	ON_BN_CLICKED(IDC_RADIO_125, &CWindowResizerDlg::OnBnClickedRadio125)
	ON_BN_CLICKED(IDC_RADIO_150, &CWindowResizerDlg::OnBnClickedRadio150)
	ON_BN_CLICKED(IDC_RADIO_200, &CWindowResizerDlg::OnBnClickedRadio200)
	ON_BN_CLICKED(IDC_RADIO_CUSTOM, &CWindowResizerDlg::OnBnClickedRadioCustom)
	ON_BN_CLICKED(IDC_BUTTON_SET_PERCENTAGE, &CWindowResizerDlg::OnBnClickedButtonSetPercentage)
	ON_BN_CLICKED(IDC_BUTTON_MAX_SIZE, &CWindowResizerDlg::OnBnClickedButtonMaxSize)
	ON_BN_CLICKED(IDC_BUTTON_RESTORE, &CWindowResizerDlg::OnBnClickedButtonRestore)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CWindowResizerDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_CHECK_DPI100_VIEW, &CWindowResizerDlg::OnBnClickedCheckDpi100View)
	ON_BN_CLICKED(IDC_CHECK_DPI100_SET, &CWindowResizerDlg::OnBnClickedCheckDpi100Set)
	ON_CBN_SELCHANGE(IDC_COMBO_RESOLUTION, &CWindowResizerDlg::OnCbnSelchangeComboResolution)
END_MESSAGE_MAP()


// CWindowResizerDlg 消息处理程序

BOOL CWindowResizerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//资源文件未包含以下新增控件，这里在运行时动态创建，避免修改资源的编码
	CreateDynamicControls();

	IsEnableAllControls(false);   //初始状态未选中窗口所有控件都不可用

	// 已弃用，现在通过修改int型变量m_nRadioSelect的值（已默认设为1）来修改Radio Button选中状态
	//((CButton*)GetDlgItem(IDC_RADIO_100))->SetCheck(TRUE); //比例缩放默认选中100%

	SetTimer(2, 1000, NULL);     //设置定时器，每隔一秒检查一次窗口的有效性

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CWindowResizerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWindowResizerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CWindowResizerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



//判断目标窗口是否有效，并刷新其所在显示器的DPI。返回DPI值（96的整数倍，失败回退程序DPI）
UINT CWindowResizerDlg::GetWindowDpi()
{
	UINT nDpi = 0;
	if (IsWindowValid())
	{
		//动态加载GetDpiForWindow(Win10 1607+)，以便在旧系统上也能正常运行
		typedef UINT(WINAPI *GetDpiForWindowProc)(HWND);
		static GetDpiForWindowProc pGetDpiForWindow = (GetDpiForWindowProc)::GetProcAddress(
			::GetModuleHandle(_T("user32.dll")), "GetDpiForWindow");
		if (pGetDpiForWindow != nullptr)
			nDpi = pGetDpiForWindow(m_hWindowHandle);
	}
	if (nDpi == 0)   //未选中窗口或系统不支持时，回退到程序自身窗口的DPI
	{
		HDC hdc = ::GetDC(NULL);
		if (hdc != NULL)
		{
			nDpi = (UINT)::GetDeviceCaps(hdc, LOGPIXELSX);
			::ReleaseDC(NULL, hdc);
		}
	}
	if (nDpi == 0)
		nDpi = 96;
	return nDpi;
}

//物理像素转逻辑尺寸（100% DPI等效值）；仅当bUseLogical为true且DPI非96时才换算
int CWindowResizerDlg::PhysicalToLogical(int value, bool bUseLogical)
{
	if (!bUseLogical || m_nTargetDpi == 96)
		return value;
	return (int)((double)value * 96.0 / m_nTargetDpi + 0.5);
}

//逻辑尺寸转物理像素；仅当bUseLogical为true且DPI非96时才换算
int CWindowResizerDlg::LogicalToPhysical(int value, bool bUseLogical)
{
	if (!bUseLogical || m_nTargetDpi == 96)
		return value;
	return (int)((double)value * m_nTargetDpi / 96.0 + 0.5);
}

//更新“当前缩放”的显示文本
void CWindowResizerDlg::UpdateDpiInfoShow()
{
	CString szInfo;
	if (IsWindowValid())
		szInfo.Format(_T("当前缩放: %u%%"), m_nTargetDpi * 100 / 96);
	else
		szInfo = _T("当前缩放: --");
	if (m_staticDpiInfo.GetSafeHwnd() != NULL)
		m_staticDpiInfo.SetWindowTextW(szInfo);
}

//运行时动态创建新增的控件（复选框、分辨率下拉框、当前缩放提示）
//资源文件（UTF-16）不便直接编辑，故在此以代码创建，坐标使用对话框单位
void CWindowResizerDlg::CreateDynamicControls()
{
	CRect rc;
	CFont* pFont = GetFont();   //对话框字体，动态创建的控件需手动继承，否则字体大小不一致

	//=================== “窗口数据”分组框 ===================
	//“当前缩放”提示，放在“窗口数据”分组框底行左侧
	rc.SetRect(18, 79, 80, 88);
	MapDialogRect(&rc);
	m_staticDpiInfo.Create(_T("当前缩放: --"),
		WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, IDC_STATIC_DPIINFO);
	if (pFont != nullptr)
		m_staticDpiInfo.SetFont(pFont);

	//“窗口数据”区的100% DPI开关（仅控制显示），放在“窗口数据”分组框底行右侧
	rc.SetRect(96, 78, 160, 89);
	MapDialogRect(&rc);
	m_checkDpiView.Create(_T("100% DPI"),
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, rc, this, IDC_CHECK_DPI100_VIEW);
	if (pFont != nullptr)
		m_checkDpiView.SetFont(pFont);

	//=================== “设置大小”分组框 ===================
	//“设置大小”区的100% DPI开关（控制输入与设置），放在“设置大小”框宽度编辑框右侧
	rc.SetRect(110, 118, 164, 129);
	MapDialogRect(&rc);
	m_checkDpiSet.Create(_T("100% DPI"),
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, rc, this, IDC_CHECK_DPI100_SET);
	if (pFont != nullptr)
		m_checkDpiSet.SetFont(pFont);

	//“设置为主流分辨率”标签，放在“设置大小”分组框底行左侧
	rc.SetRect(18, 160, 92, 169);
	MapDialogRect(&rc);
	m_staticResLabel.Create(_T("设置为主流分辨率"),
		WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, IDC_STATIC_RESOLUTION_LABEL);
	if (pFont != nullptr)
		m_staticResLabel.SetFont(pFont);

	//主流分辨率下拉框，放在标签右侧
	rc.SetRect(96, 159, 164, 214);
	MapDialogRect(&rc);
	m_comboResolution.Create(
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST,
		rc, this, IDC_COMBO_RESOLUTION);
	if (pFont != nullptr)
		m_comboResolution.SetFont(pFont);

	//初始化主流分辨率下拉框
	static const TCHAR* szResolutions[] = {
		// 按宽度升序排列（宽度相同时按高度升序）
		_T("640x480"),   _T("800x600"),   _T("1024x768"),  _T("1152x864"),
		_T("1280x720"),  _T("1280x800"),  _T("1280x960"),  _T("1280x1024"),
		_T("1366x768"),  _T("1400x1050"), _T("1440x900"),  _T("1600x900"),
		_T("1600x1200"), _T("1680x1050"), _T("1920x1080"), _T("1920x1200"),
		_T("2048x1536"), _T("2560x1080"), _T("2560x1440"), _T("2560x1600"),
		_T("2880x1800"), _T("3200x1800"), _T("3440x1440"), _T("3840x1600"),
		_T("3840x2160")
	};
	for (int i = 0; i < _countof(szResolutions); i++)
		m_comboResolution.AddString(szResolutions[i]);
}


void CWindowResizerDlg::UpdateSizeShow(int width,int height)
{
	CString szWidth,szHeight;
	//参数为物理像素，按“窗口数据”区的显示开关换算后再显示
	szWidth.Format(_T("%d"), PhysicalToLogical(width, m_bUseLogicalSizeView));
	szHeight.Format(_T("%d"), PhysicalToLogical(height, m_bUseLogicalSizeView));
	m_editShowCurWidth.SetWindowTextW(szWidth);
	m_editShowCurHeight.SetWindowTextW(szHeight);
}


void CWindowResizerDlg::OnBnClickedButtonFindWindow()
{
	SetTimer(1, 1000, NULL);
}




void CWindowResizerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		CWnd* pWnd = GetForegroundWindow();
		m_hWindowHandle = pWnd->GetSafeHwnd();
		if (m_hWindowHandle != NULL && m_hWindowHandle != m_hWnd)//只有处于前台的窗口不是程序本身的窗口，才说明找到了
		{
			m_pWnd = pWnd;
			CString szHandle;
			szHandle.Format(_T("%.8x"), m_hWindowHandle);
			szHandle.MakeUpper();
			m_editShowHandle.SetWindowTextW(szHandle);

			pWnd->GetWindowTextW(m_szWindowTitle);
			m_editShowTitle.SetWindowTextW(m_szWindowTitle);

			//获取窗口的大小
			CRect rect;
			::GetWindowRect(m_hWindowHandle, rect);
			m_sizeOriginalWindow = rect.Size();

			m_nTargetDpi = GetWindowDpi();   //获取目标窗口所在显示器的DPI

			UpdateSizeShow(m_sizeOriginalWindow.cx, m_sizeOriginalWindow.cy);
			UpdateDpiInfoShow();

			IsEnableAllControls(IsWindowValid());
			
			KillTimer(1);       //找到窗口之后停掉定时器
		}
		// 不管窗口是否有效，每次点击“查找窗口”按钮之后这些值都需要重新初始化
		m_bClickedSetPercentage = false;
		m_bIsSetMaxSize = false;
	}
	else if (nIDEvent == 2)
	{
		//每隔1秒钟检查一次要操作的窗口是否有效，并调用控件的启用或禁用状态
		IsEnableAllControls(IsWindowValid());
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CWindowResizerDlg::OnBnClickedButtonSetSize()
{
	if (IsWindowValid())
	{
		int nWidth, nHeight;
		CString szWidth, szHeight;
		m_editSetWidth.GetWindowTextW(szWidth);
		m_editSetHeight.GetWindowTextW(szHeight);
		nWidth = _ttoi(szWidth);
		nHeight = _ttoi(szHeight);
		if (nWidth == 0 || nHeight == 0)
			return;

		//输入值在开启“设置大小”区的逻辑尺寸开关时是100% DPI等效值，需要换算成物理像素再设置
		int nPhysicalWidth = LogicalToPhysical(nWidth, m_bUseLogicalSizeSet);
		int nPhysicalHeight = LogicalToPhysical(nHeight, m_bUseLogicalSizeSet);
		::SetWindowPos(m_hWindowHandle, NULL, 0, 0, nPhysicalWidth, nPhysicalHeight, SWP_NOZORDER | SWP_NOMOVE);

		UpdateSizeShow(nPhysicalWidth, nPhysicalHeight);
		m_bClickedSetPercentage = false;
	}

}


void CWindowResizerDlg::OnBnClickedRadio50()
{
	m_nScalePercentage = 50;
	m_bScaleIsCustomized = false;
	SetPercentageEditEnable();
}


void CWindowResizerDlg::OnBnClickedRadio100()
{
	m_nScalePercentage = 100;
	m_bScaleIsCustomized = false;
	SetPercentageEditEnable();
}


void CWindowResizerDlg::OnBnClickedRadio125()
{
	m_nScalePercentage = 125;
	m_bScaleIsCustomized = false;
	SetPercentageEditEnable();
}


void CWindowResizerDlg::OnBnClickedRadio150()
{
	m_nScalePercentage = 150;
	m_bScaleIsCustomized = false;
	SetPercentageEditEnable();
}


void CWindowResizerDlg::OnBnClickedRadio200()
{
	m_nScalePercentage = 200;
	m_bScaleIsCustomized = false;
	SetPercentageEditEnable();
}


void CWindowResizerDlg::OnBnClickedRadioCustom()
{
	m_bScaleIsCustomized = true;
	SetPercentageEditEnable();
}


void CWindowResizerDlg::OnBnClickedButtonSetPercentage()
{
	if (!IsWindowValid())
		return;
	if (!m_bClickedSetPercentage)       //如果最后一次改变窗口大小不是按比例缩放的则记录当前窗口大小作为后续按比例缩放的基准
	{
		CRect rect;
		::GetWindowRect(m_hWindowHandle, rect);
		//“设置大小”区开启逻辑尺寸时以100% DPI等效值作为缩放基准，与输入保持一致
		m_nTempOriginalWidth = PhysicalToLogical(rect.Size().cx, m_bUseLogicalSizeSet);
		m_nTempOriginalHeight = PhysicalToLogical(rect.Size().cy, m_bUseLogicalSizeSet);
	}
	if (m_bScaleIsCustomized)           //如果选中了自定义，则比例调整为自定义的值
	{
		CString szPercentage;
		m_editSetPercentage.GetWindowText(szPercentage);
		m_nScalePercentage = _ttoi(szPercentage);
		if (m_nScalePercentage <= 0)
			return;
	}
	int nWidth;
	int nHeight;
	nWidth = m_nTempOriginalWidth * m_nScalePercentage / 100;
	nHeight = m_nTempOriginalHeight * m_nScalePercentage / 100;
	//nWidth/nHeight为逻辑尺寸，需要换算成物理像素再设置
	int nPhysicalWidth = LogicalToPhysical(nWidth, m_bUseLogicalSizeSet);
	int nPhysicalHeight = LogicalToPhysical(nHeight, m_bUseLogicalSizeSet);
	::SetWindowPos(m_hWindowHandle, NULL, 0, 0, nPhysicalWidth, nPhysicalHeight, SWP_NOZORDER | SWP_NOMOVE);

	UpdateSizeShow(nPhysicalWidth, nPhysicalHeight);
	m_bClickedSetPercentage = true;
}


void CWindowResizerDlg::OnBnClickedButtonMaxSize()
{
	if (!IsWindowValid())
		return;
	if (m_pWnd->IsZoomed())  //只有窗口本身不是最大化的才有效
		return;
	m_pWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_bIsSetMaxSize = true;

	//重新获取窗口的大小
	CRect rect;
	::GetWindowRect(m_hWindowHandle, rect);
	UpdateSizeShow(rect.Size().cx, rect.Size().cy);
	m_bClickedSetPercentage = false;
}


void CWindowResizerDlg::OnBnClickedButtonRestore()
{
	if (!IsWindowValid())
		return;
	if(m_bIsSetMaxSize)
		m_pWnd->ShowWindow(SW_RESTORE);      //取消最大化

	// 还原初始大小
	::SetWindowPos(m_hWindowHandle, NULL, 0, 0, m_sizeOriginalWindow.cx, m_sizeOriginalWindow.cy, SWP_NOZORDER | SWP_NOMOVE);

	// 按比例缩放处恢复到选中100%
	m_nRadioSelect = 1;
	UpdateData(FALSE);
	OnBnClickedRadio100();

	// 重新获取窗口大小并显示出来
	CRect rect;
	::GetWindowRect(m_hWindowHandle, rect);
	UpdateSizeShow(rect.Size().cx, rect.Size().cy);
	m_bClickedSetPercentage = false;
}


void CWindowResizerDlg::OnBnClickedButtonAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

//勾选/取消“窗口数据”区的100% DPI开关时，切换宽高的显示模式
void CWindowResizerDlg::OnBnClickedCheckDpi100View()
{
	m_bUseLogicalSizeView = (m_checkDpiView.GetCheck() == BST_CHECKED);

	//切换后刷新“窗口数据”处当前窗口大小的显示
	if (IsWindowValid())
	{
		CRect rect;
		::GetWindowRect(m_hWindowHandle, rect);
		UpdateSizeShow(rect.Size().cx, rect.Size().cy);
	}
}

//勾选/取消“设置大小”区的100% DPI开关时，切换输入值的含义（100% DPI等效值 / 物理像素）
void CWindowResizerDlg::OnBnClickedCheckDpi100Set()
{
	m_bUseLogicalSizeSet = (m_checkDpiSet.GetCheck() == BST_CHECKED);
}

//选择主流分辨率后，将其填入“设置大小”的宽度和高度
void CWindowResizerDlg::OnCbnSelchangeComboResolution()
{
	int nSel = m_comboResolution.GetCurSel();
	if (nSel == CB_ERR)
		return;
	CString szItem;
	m_comboResolution.GetLBText(nSel, szItem);   //格式形如 "1920x1080"
	int nSplit = szItem.Find(_T('x'));
	if (nSplit <= 0)
		return;
	CString szWidth = szItem.Left(nSplit);
	CString szHeight = szItem.Mid(nSplit + 1);
	m_editSetWidth.SetWindowTextW(szWidth);
	m_editSetHeight.SetWindowTextW(szHeight);
}

// 判断已选中的窗口是否可用
bool CWindowResizerDlg::IsWindowValid()
{
	if (m_pWnd == nullptr)                 //还未选中窗口
		return false;
	HWND handle = m_pWnd->GetSafeHwnd();   //获取所选中窗口的句柄
	if (handle == NULL)
		return false;
	if (!IsWindow(handle))
		return false;
	return true;
}

// 按比例缩放处的编辑框只有在选中自定义时才可以使用
void CWindowResizerDlg::SetPercentageEditEnable()
{
	if (IsWindowValid())
		m_editSetPercentage.EnableWindow(m_bScaleIsCustomized);
	else
		m_editSetPercentage.EnableWindow(false);
}


// 只有窗口本身不是最大化才能够使用最大化按钮
void CWindowResizerDlg::SetMaxSizeButtonEnable()
{
	if (IsWindowValid())
		GetDlgItem(IDC_BUTTON_MAX_SIZE)->EnableWindow(!m_pWnd->IsZoomed());
	else
		GetDlgItem(IDC_BUTTON_MAX_SIZE)->EnableWindow(false);
}


void CWindowResizerDlg::IsEnableAllControls(bool bIsEnable)
{
	GetDlgItem(IDC_EDIT_HANDLE)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_EDIT_TITLE)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_EDIT_HEIGHT)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_EDIT_HEIGHT)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_EDIT_WIDTH)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_EDIT_HEIGHT)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_EDIT_SET_WIDTH)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_EDIT_SET_HEIGHT)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_BUTTON_SET_SIZE)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_RADIO_50)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_RADIO_100)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_RADIO_125)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_RADIO_150)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_RADIO_200)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_RADIO_CUSTOM)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_BUTTON_SET_PERCENTAGE)->EnableWindow(bIsEnable);
	GetDlgItem(IDC_BUTTON_RESTORE)->EnableWindow(bIsEnable);
	m_checkDpiView.EnableWindow(bIsEnable);
	m_checkDpiSet.EnableWindow(bIsEnable);
	m_comboResolution.EnableWindow(bIsEnable);
	SetMaxSizeButtonEnable();
	SetPercentageEditEnable();
	UpdateDpiInfoShow();
}



void CAboutDlg::OnNMClickSyslinkCheckUpdate(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(NULL, _T("open"), _T("https://github.com/inkuang/WindowResizer/releases"), NULL, NULL, SW_SHOW);
	*pResult = 0;
}
