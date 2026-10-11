
// WindowResizerDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowResizer.h"
#include "WindowResizerDlg.h"
#include "afxdialogex.h"
#include "LanguageManager.h"

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
	DDX_Control(pDX, IDC_CHECK_CLIENT_AREA, m_checkClientArea);
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
	ON_BN_CLICKED(IDC_CHECK_CLIENT_AREA, &CWindowResizerDlg::OnBnClickedCheckClientArea)
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

		// 追加“语言”子菜单，供用户在中文/英文之间切换（切换后需重启生效）
		CMenu menuLang;
		menuLang.CreatePopupMenu();
		CString szLangZh, szLangEn, szMenuTitle;
		szLangZh.LoadString(IDS_LANG_ZH);
		szLangEn.LoadString(IDS_LANG_EN);
		szMenuTitle.LoadString(IDS_MENU_LANGUAGE);
		menuLang.AppendMenu(MF_STRING, IDM_LANGUAGE_ZH, szLangZh);
		menuLang.AppendMenu(MF_STRING, IDM_LANGUAGE_EN, szLangEn);
		// 勾选当前生效的语言（来自注册表偏好或系统语言）
		const AppLanguage curLang = LanguageManager::GetSavedOrSystemLanguage();
		menuLang.CheckMenuItem(IDM_LANGUAGE_ZH,
			MF_BYCOMMAND | (curLang == AppLanguage::Chinese ? MF_CHECKED : MF_UNCHECKED));
		menuLang.CheckMenuItem(IDM_LANGUAGE_EN,
			MF_BYCOMMAND | (curLang == AppLanguage::English ? MF_CHECKED : MF_UNCHECKED));
		// Detach 后子菜单由系统菜单托管，避免局部 CMenu 析构销毁仍被引用的菜单句柄
		pSysMenu->AppendMenu(MF_POPUP, (UINT_PTR)menuLang.Detach(), szMenuTitle);
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
	else if (nID == IDM_LANGUAGE_ZH || nID == IDM_LANGUAGE_EN)
	{
		// 注意：此处必须用精确相等判断。32771/32772 经 0xFFF0 掩码后为 0x8000，
		// 与自身不相等，用 (nID & 0xFFF0) 会误判；且二者不在 SC_* 段内，无冲突。
		LanguageManager::SaveLanguage(nID == IDM_LANGUAGE_EN ? AppLanguage::English : AppLanguage::Chinese);

		// 采用“提示重启生效”策略：不即刻遍历控件刷新，避免半新半旧与重建对话框的复杂度
		CString szTip, szCaption;
		szTip.LoadString(IDS_RESTART_TIP);
		szCaption.LoadString(IDS_RESTART_TIP_CAPTION);
		::MessageBox(m_hWnd, szTip, szCaption, MB_OK | MB_ICONINFORMATION);
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

//由「客户区尺寸(物理像素)」反推「SetWindowPos 所需的窗口尺寸」
CSize CWindowResizerDlg::WindowSizeFromClientSize(int nClientWidth, int nClientHeight)
{
	// 目标窗口无效时不具备样式信息，直接原样返回，由调用方判断有效性
	if (m_hWindowHandle == NULL)
		return CSize(nClientWidth, nClientHeight);

	DWORD dwStyle = (DWORD)::GetWindowLongPtr(m_hWindowHandle, GWL_STYLE);
	DWORD dwExStyle = (DWORD)::GetWindowLongPtr(m_hWindowHandle, GWL_EXSTYLE);

	// 动态加载 AdjustWindowRectExForDpi(Win10 1607+)，与 GetWindowDpi 同风格，保证旧系统可用
	typedef BOOL(WINAPI *AdjustWindowRectExForDpiProc)(LPRECT, DWORD, BOOL, DWORD, UINT);
	static AdjustWindowRectExForDpiProc pAdjustWindowRectExForDpi = (AdjustWindowRectExForDpiProc)::GetProcAddress(
		::GetModuleHandle(_T("user32.dll")), "AdjustWindowRectExForDpi");

	RECT rc = { 0, 0, nClientWidth, nClientHeight };
	BOOL bOk = FALSE;
	if (pAdjustWindowRectExForDpi != nullptr)
		bOk = pAdjustWindowRectExForDpi(&rc, dwStyle, FALSE, dwExStyle, m_nTargetDpi);
	// 旧系统或高 DPI API 调用失败时，回退到系统 DPI 的经典实现
	// 注意：bMenu 固定传 FALSE —— 本项目目标为普通顶层窗口，菜单栏不参与客户区尺寸计算
	if (!bOk)
		::AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

	return CSize(rc.right - rc.left, rc.bottom - rc.top);
}

//按当前口径取目标尺寸（物理像素）：客户区模式用 GetClientRect，否则用 GetWindowRect
void CWindowResizerDlg::GetTargetSizePhysical(int& nWidth, int& nHeight)
{
	nWidth = 0;
	nHeight = 0;
	if (m_hWindowHandle == NULL || !::IsWindow(m_hWindowHandle))
		return;   // 无效窗口交由调用方判断（与 IsWindowValid 职责分离）

	if (m_bUseClientArea)
	{
		CRect rc;
		::GetClientRect(m_hWindowHandle, &rc);
		nWidth = rc.Width();    // 客户区宽高
		nHeight = rc.Height();
	}
	else
	{
		CRect rc;
		::GetWindowRect(m_hWindowHandle, &rc);
		nWidth = rc.Width();    // 整个窗口宽高
		nHeight = rc.Height();
	}
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
	{
		// 格式串取自资源，随界面语言自动切换
		CString szFmt;
		szFmt.LoadString(IDS_DPI_INFO_FORMAT);
		szInfo.Format(szFmt, m_nTargetDpi * 100 / 96);
	}
	else
	{
		// 无效窗口时显示占位文案，同样取自资源
		szInfo.LoadString(IDS_DPI_INFO_NONE);
	}
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
	CString szDpiInfoNone;
	szDpiInfoNone.LoadString(IDS_DPI_INFO_NONE);
	m_staticDpiInfo.Create(szDpiInfoNone,
		WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, IDC_STATIC_DPIINFO);
	if (pFont != nullptr)
		m_staticDpiInfo.SetFont(pFont);

	//“窗口数据”区的100% DPI开关（仅控制显示），放在“窗口数据”分组框底行右侧
	rc.SetRect(96, 78, 160, 89);
	MapDialogRect(&rc);
	CString szDpiView;
	szDpiView.LoadString(IDS_DPI100_VIEW);
	m_checkDpiView.Create(szDpiView,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, rc, this, IDC_CHECK_DPI100_VIEW);
	if (pFont != nullptr)
		m_checkDpiView.SetFont(pFont);

	//=================== “设置大小”分组框 ===================
	//“设置大小”区的100% DPI开关（控制输入与设置），放在“设置大小”框宽度编辑框右侧
	rc.SetRect(110, 118, 164, 129);
	MapDialogRect(&rc);
	CString szDpiSet;
	szDpiSet.LoadString(IDS_DPI100_SET);
	m_checkDpiSet.Create(szDpiSet,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, rc, this, IDC_CHECK_DPI100_SET);
	if (pFont != nullptr)
		m_checkDpiSet.SetFont(pFont);

	//“设置为主流分辨率”标签，放在“设置大小”分组框底行左侧
	rc.SetRect(18, 160, 92, 169);
	MapDialogRect(&rc);
	CString szResLabel;
	szResLabel.LoadString(IDS_RESOLUTION_LABEL);
	m_staticResLabel.Create(szResLabel,
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

	//=================== 英文排版自适应 ===================
	// 英文文案普遍长于中文，此处按实际文本宽度扩展标签控件，避免英文界面下文字被控件矩形裁剪；
	// 文本本身已能容纳时不做任何调整，因此完全不影响中文布局
	if (LanguageManager::IsEnglishUi())
	{
		CClientDC dc(this);
		CFont* pOldFont = dc.SelectObject(pFont != nullptr ? pFont : GetFont());
		// 对话框单位 96 换算成像素后，即标签允许扩展到的最大右边界（右侧需留给复选框/下拉框）
		CRect rcLimit(0, 0, 96, 0);
		MapDialogRect(&rcLimit);

		//“当前缩放”标签：右边界不得越过同行的“100% DPI”复选框
		CRect rcDpi;
		m_staticDpiInfo.GetWindowRect(&rcDpi);
		ScreenToClient(&rcDpi);
		int nWantRight = rcDpi.left + dc.GetTextExtent(szDpiInfoNone).cx + 4;   // 文本宽 + 少量余量
		if (nWantRight > rcDpi.right && nWantRight <= rcLimit.right)
			m_staticDpiInfo.SetWindowPos(nullptr, 0, 0, nWantRight - rcDpi.left, rcDpi.Height(),
				SWP_NOMOVE | SWP_NOZORDER);

		//“设置为主流分辨率”标签：右边界不得越过右侧的分辨率下拉框
		CRect rcRes;
		m_staticResLabel.GetWindowRect(&rcRes);
		ScreenToClient(&rcRes);
		nWantRight = rcRes.left + dc.GetTextExtent(szResLabel).cx + 4;
		if (nWantRight > rcRes.right && nWantRight <= rcLimit.right)
			m_staticResLabel.SetWindowPos(nullptr, 0, 0, nWantRight - rcRes.left, rcRes.Height(),
				SWP_NOMOVE | SWP_NOZORDER);

		dc.SelectObject(pOldFont);
	}
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

			//获取窗口的大小：m_sizeOriginalWindow 固定记录窗口矩形尺寸（物理像素），作为精确还原基准，不随尺寸口径变化
			CRect rect;
			::GetWindowRect(m_hWindowHandle, rect);
			m_sizeOriginalWindow = rect.Size();

			m_nTargetDpi = GetWindowDpi();   //获取目标窗口所在显示器的DPI

			//显示则按当前口径取值（客户区模式显示客户区尺寸）
			int nW = 0, nH = 0;
			GetTargetSizePhysical(nW, nH);
			UpdateSizeShow(nW, nH);
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
		//客户区模式下，把目标客户区尺寸反推为 SetWindowPos 所需的窗口尺寸
		CSize szSet(nPhysicalWidth, nPhysicalHeight);
		if (m_bUseClientArea)
			szSet = WindowSizeFromClientSize(nPhysicalWidth, nPhysicalHeight);
		::SetWindowPos(m_hWindowHandle, NULL, 0, 0, szSet.cx, szSet.cy, SWP_NOZORDER | SWP_NOMOVE);

		UpdateSizeShow(nPhysicalWidth, nPhysicalHeight);   //显示目标口径尺寸（即用户预期值）
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
		int nBaseW = 0, nBaseH = 0;
		GetTargetSizePhysical(nBaseW, nBaseH);   //缩放基准按当前尺寸口径采样
		//“设置大小”区开启逻辑尺寸时以100% DPI等效值作为缩放基准，与输入保持一致
		m_nTempOriginalWidth = PhysicalToLogical(nBaseW, m_bUseLogicalSizeSet);
		m_nTempOriginalHeight = PhysicalToLogical(nBaseH, m_bUseLogicalSizeSet);
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
	//客户区模式下，把目标客户区尺寸反推为 SetWindowPos 所需的窗口尺寸
	CSize szSet(nPhysicalWidth, nPhysicalHeight);
	if (m_bUseClientArea)
		szSet = WindowSizeFromClientSize(nPhysicalWidth, nPhysicalHeight);
	::SetWindowPos(m_hWindowHandle, NULL, 0, 0, szSet.cx, szSet.cy, SWP_NOZORDER | SWP_NOMOVE);

	UpdateSizeShow(nPhysicalWidth, nPhysicalHeight);   //显示目标口径尺寸（即用户预期值）
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

	//重新获取窗口的大小（按当前尺寸口径）
	int nW = 0, nH = 0;
	GetTargetSizePhysical(nW, nH);
	UpdateSizeShow(nW, nH);
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

	// 重新获取窗口大小并显示出来（还原后仍按当前尺寸口径显示）
	int nW = 0, nH = 0;
	GetTargetSizePhysical(nW, nH);
	UpdateSizeShow(nW, nH);
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

	//切换后刷新“窗口数据”处当前大小的显示（按当前尺寸口径）
	if (IsWindowValid())
	{
		int nW = 0, nH = 0;
		GetTargetSizePhysical(nW, nH);
		UpdateSizeShow(nW, nH);
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

//勾选/取消「整个窗口（含边框）」开关时，切换尺寸口径并刷新显示
void CWindowResizerDlg::OnBnClickedCheckClientArea()
{
	// 勾选=按整个窗口，不勾=按客户区，故此处对勾选状态取反
	m_bUseClientArea = (m_checkClientArea.GetCheck() != BST_CHECKED);

	//口径变化后，按比例缩放的采样基准失效，下次按比例缩放时重新采样
	m_bClickedSetPercentage = false;

	//切换后刷新「窗口数据」处当前尺寸的显示
	if (IsWindowValid())
	{
		int nW = 0, nH = 0;
		GetTargetSizePhysical(nW, nH);
		UpdateSizeShow(nW, nH);
	}
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
	m_checkClientArea.EnableWindow(bIsEnable);   //未选中窗口时禁用整个窗口开关
	SetMaxSizeButtonEnable();
	SetPercentageEditEnable();
	UpdateDpiInfoShow();
}



void CAboutDlg::OnNMClickSyslinkCheckUpdate(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(NULL, _T("open"), _T("https://github.com/inkuang/WindowResizer/releases"), NULL, NULL, SW_SHOW);
	*pResult = 0;
}
