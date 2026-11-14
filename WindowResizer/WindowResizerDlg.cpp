
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
	DDX_Control(pDX, IDC_COMBO_SIZE_MODE, m_comboSizeMode);
	DDX_Control(pDX, IDC_COMBO_PROFILE, m_comboProfile);
	DDX_Control(pDX, IDC_CHECK_AUTOSWITCH, m_checkAutoSwitch);
	DDX_Control(pDX, IDC_BUTTON_SAVE_PROFILE, m_btnSaveProfile);
	DDX_Control(pDX, IDC_STATIC_VIEWPORT_INFO, m_staticViewportInfo);
	DDX_Radio(pDX, IDC_RADIO_50, m_nRadioSelect);
}

BEGIN_MESSAGE_MAP(CWindowResizerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SETCURSOR()
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
	ON_CBN_SELCHANGE(IDC_COMBO_SIZE_MODE, &CWindowResizerDlg::OnCbnSelchangeComboSizeMode)
	ON_CBN_SELCHANGE(IDC_COMBO_PROFILE, &CWindowResizerDlg::OnCbnSelchangeComboProfile)
	ON_BN_CLICKED(IDC_CHECK_AUTOSWITCH, &CWindowResizerDlg::OnBnClickedCheckAutoswitch)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_PROFILE, &CWindowResizerDlg::OnBnClickedButtonSaveProfile)
	ON_MESSAGE(WM_PICKER_RESULT, &CWindowResizerDlg::OnOverlayPickWindow)
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

	// 拾取态使用系统内置的「精确选择」十字光标（IDC_CROSS）：自带高 DPI 版本，且跟随用户的指针方案，
	// 无需打包自绘 .cur 文件（自绘在 125%/150% 缩放下会发虚）
	m_hCursorPick = ::LoadCursor(NULL, IDC_CROSS);

	// TODO: 在此添加额外的初始化代码

	// 载入 ini 配置：文件不存在时会自动生成带中文注释与示例的默认文件
	m_config.Load();

	// 尺寸口径下拉：下标 0/1/2 与 SizeMode 枚举严格一一对应，SetCurSel/GetCurSel 可直接互换
	for (int i = 0; i < 3; i++)
	{
		CString szMode;
		szMode.LoadString(IDS_MODE_WINDOW + i);   // IDS_MODE_WINDOW/CLIENT/VIEWPORT 三个 ID 连续
		m_comboSizeMode.AddString(szMode);
	}
	m_comboSizeMode.SetCurSel(m_nSizeMode);

	//资源文件未包含以下新增控件，这里在运行时动态创建，避免修改资源的编码
	CreateDynamicControls();

	RefreshProfileCombo();
	m_checkAutoSwitch.SetCheck(m_config.IsAutoSwitchEnabled() ? BST_CHECKED : BST_UNCHECKED);

	IsEnableAllControls(false);   //初始状态未选中窗口所有控件都不可用

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

//按当前口径取目标尺寸（物理像素）：整个窗口用 GetWindowRect，客户区/浏览器视口用 GetClientRect 或渲染子窗口矩形
bool CWindowResizerDlg::GetTargetSizePhysical(int& nWidth, int& nHeight)
{
	nWidth = 0;
	nHeight = 0;
	if (m_hWindowHandle == NULL || !::IsWindow(m_hWindowHandle))
		return false;   // 无效窗口交由调用方判断（与 IsWindowValid 职责分离）

	if (m_nSizeMode == ModeViewport)
	{
		int nViewportW = 0, nViewportH = 0, nDW = 0, nDH = 0, nBW = 0, nBH = 0;
		if (Chromium::Measure(m_hWindowHandle, nViewportW, nViewportH, nDW, nDH, nBW, nBH))
		{
			nWidth = nViewportW;
			nHeight = nViewportH;
			return true;
		}
		// 找不到渲染子窗口（非浏览器或页面未加载）时，退化为客户区口径取值
	}

	if (m_nSizeMode == ModeWindow)
	{
		CRect rc;
		::GetWindowRect(m_hWindowHandle, &rc);
		nWidth = rc.Width();    // 整个窗口宽高
		nHeight = rc.Height();
	}
	else
	{
		CRect rc;
		::GetClientRect(m_hWindowHandle, &rc);
		nWidth = rc.Width();    // 客户区宽高
		nHeight = rc.Height();
	}
	return true;
}

//按当前口径把父窗口物理尺寸换算成「用户口径下的显示值」
void CWindowResizerDlg::WindowSizeToModeSize(int nWindowWidth, int nWindowHeight, int& nOutWidth, int& nOutHeight)
{
	nOutWidth = nWindowWidth;
	nOutHeight = nWindowHeight;

	if (m_hWindowHandle == NULL || !::IsWindow(m_hWindowHandle))
		return;

	if (m_nSizeMode == ModeWindow)
		return;

	CRect rcWin, rcClient;
	::GetWindowRect(m_hWindowHandle, &rcWin);
	::GetClientRect(m_hWindowHandle, &rcClient);
	const int nBorderW = rcWin.Width() - rcClient.Width();
	const int nBorderH = rcWin.Height() - rcClient.Height();

	// 客户区口径：扣掉标题栏与边框
	nOutWidth = nWindowWidth - nBorderW;
	nOutHeight = nWindowHeight - nBorderH;

	if (m_nSizeMode != ModeViewport)
		return;

	// 视口口径：再扣掉浏览器自身的 UI 占高（标签栏/地址栏/书签栏）
	int nVW = 0, nVH = 0, nDW = 0, nDH = 0, nBW = 0, nBH = 0;
	if (Chromium::Measure(m_hWindowHandle, nVW, nVH, nDW, nDH, nBW, nBH))
	{
		nOutWidth = nWindowWidth - nBorderW - nDW;
		nOutHeight = nWindowHeight - nBorderH - nDH;
	}
}

//按当前口径把「用户口径下的尺寸」换算成父窗口物理尺寸
bool CWindowResizerDlg::ModeSizeToWindowSize(int nModeWidth, int nModeHeight, int& nOutWidth, int& nOutHeight)
{
	nOutWidth = nModeWidth;
	nOutHeight = nModeHeight;

	if (m_hWindowHandle == NULL || !::IsWindow(m_hWindowHandle))
		return false;

	if (m_nSizeMode == ModeWindow)
		return true;

	if (m_nSizeMode == ModeClient)
	{
		CSize sz = WindowSizeFromClientSize(nModeWidth, nModeHeight);
		nOutWidth = sz.cx;
		nOutHeight = sz.cy;
		return true;
	}

	// 视口口径：按实测的 UI 占高与边框反推父窗口尺寸
	return Chromium::ComputeWindowSizeForViewport(m_hWindowHandle, nModeWidth, nModeHeight, nOutWidth, nOutHeight);
}

//按当前口径设置窗口尺寸
bool CWindowResizerDlg::ApplyTargetSize(int nWidth, int nHeight)
{
	if (!IsWindowValid() || nWidth <= 0 || nHeight <= 0)
		return false;

	if (m_nSizeMode == ModeViewport)
	{
		int nWinW = 0, nWinH = 0;
		if (Chromium::ComputeWindowSizeForViewport(m_hWindowHandle, nWidth, nHeight, nWinW, nWinH))
		{
			::SetWindowPos(m_hWindowHandle, NULL, 0, 0, nWinW, nWinH, SWP_NOZORDER | SWP_NOMOVE);

			// 立即回读渲染子窗口：Chromium 若因标签栏/侧边栏变化重排，会在本次调用内同步覆盖设置
			if (Chromium::VerifyViewportSize(m_hWindowHandle, nWidth, nHeight, 2))
				return true;

			CString szMsg, szCaption;
			szMsg.LoadString(IDS_VIEWPORT_FALLBACK);
			szCaption.LoadString(IDS_RESTART_TIP_CAPTION);   // 复用既有的「提示」标题
			::MessageBox(m_hWnd, szMsg, szCaption, MB_OK | MB_ICONINFORMATION);
			SetSizeMode(ModeClient, true);   // 回弹：自动回退到客户区，避免「点了没反应」
		}
		else
		{
			// 找不到渲染子窗口（非浏览器或页面尚未加载）：静默回退客户区，不打扰用户
			SetSizeMode(ModeClient, true);
		}
	}

	int nTargetW = nWidth, nTargetH = nHeight;
	if (!ModeSizeToWindowSize(nWidth, nHeight, nTargetW, nTargetH))
		return false;

	::SetWindowPos(m_hWindowHandle, NULL, 0, 0, nTargetW, nTargetH, SWP_NOZORDER | SWP_NOMOVE);
	return true;
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

	//初始化主流分辨率下拉框：数据来自 ini 配置的当前档案（内置默认项由 CAppConfig 生成）
	RefreshResolutionCombo();

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
	//参数为「父窗口矩形」的物理尺寸，先按当前尺寸口径换算成用户口径下的值，再做 DPI 逻辑化
	int nModeWidth = 0, nModeHeight = 0;
	WindowSizeToModeSize(width, height, nModeWidth, nModeHeight);

	CString szWidth,szHeight;
	szWidth.Format(_T("%d"), PhysicalToLogical(nModeWidth, m_bUseLogicalSizeView));
	szHeight.Format(_T("%d"), PhysicalToLogical(nModeHeight, m_bUseLogicalSizeView));
	m_editShowCurWidth.SetWindowTextW(szWidth);
	m_editShowCurHeight.SetWindowTextW(szHeight);
}


void CWindowResizerDlg::OnBnClickedButtonFindWindow()
{
	// 按钮在拾取态已变成「取消拾取」，再点即退出：
	// 这解决了旧版「开始拾取后点到自己窗口就退不出来」的问题
	if (m_bPicking)
	{
		StopPicking(false);
		return;
	}
	StartPicking();
}

//进入拾取状态：显示全屏蒙版并切换按钮文案
void CWindowResizerDlg::StartPicking()
{
	if (m_bPicking)
		return;

	// 蒙版创建失败（虚拟屏幕尺寸异常或位图分配失败）时直接返回，
	// 避免界面进入「已拾取」的假状态而用户看不到任何提示
	if (!m_overlay.Begin(this))
		return;

	m_bPicking = true;

	// 按钮文字实时切换：这是「正在拾取」最明确的反馈，同时提供退出入口
	CString szBtn;
	szBtn.LoadString(IDS_BTN_PICKING);
	CWnd* pBtn = GetDlgItem(IDC_BUTTON_FIND_WINDOW);
	if (pBtn != nullptr)
		pBtn->SetWindowTextW(szBtn);
}

//退出拾取状态并复位所有视觉状态
void CWindowResizerDlg::StopPicking(bool bAccepted)
{
	UNREFERENCED_PARAMETER(bAccepted);

	// 蒙版可能已由自身（点击/右键/Esc）先行关闭，End 内部做了幂等处理
	m_overlay.End();

	if (!m_bPicking)
		return;

	m_bPicking = false;

	CString szBtn;
	szBtn.LoadString(IDS_BTN_FIND);
	CWnd* pBtn = GetDlgItem(IDC_BUTTON_FIND_WINDOW);
	if (pBtn != nullptr)
		pBtn->SetWindowTextW(szBtn);

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}

//拾取期间拦截光标：覆盖本窗口范围内的光标（蒙版未覆盖到的区域）
BOOL CWindowResizerDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bPicking && m_hCursorPick != nullptr)
	{
		::SetCursor(m_hCursorPick);
		return TRUE;
	}
	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

//把拾取到的窗口信息填入界面（蒙版拾取与定时器两条路径共用）
void CWindowResizerDlg::FillTargetWindowInfo(HWND hTarget)
{
	m_hWindowHandle = hTarget;
	m_pWnd = CWnd::FromHandle(hTarget);

	CString szHandle;
	szHandle.Format(_T("%.8x"), hTarget);
	szHandle.MakeUpper();
	m_editShowHandle.SetWindowTextW(szHandle);

	m_pWnd->GetWindowTextW(m_szWindowTitle);
	m_editShowTitle.SetWindowTextW(m_szWindowTitle);

	// m_sizeOriginalWindow 固定记录窗口矩形尺寸（物理像素），作为精确还原基准，不随尺寸口径变化
	CRect rect;
	::GetWindowRect(hTarget, rect);
	m_sizeOriginalWindow = rect.Size();

	m_nTargetDpi = GetWindowDpi();   //获取目标窗口所在显示器的DPI

	//显示入口统一传「父窗口矩形」尺寸，由 UpdateSizeShow 按当前口径反算显示值
	UpdateSizeShow(rect.Width(), rect.Height());
	UpdateDpiInfoShow();
	UpdateViewportModeAvailability();

	IsEnableAllControls(IsWindowValid());

	// 每次选中新窗口后，这两个标志都需要重新初始化
	m_bClickedSetPercentage = false;
	m_bIsSetMaxSize = false;
}

//接收蒙版回传的拾取结果：wParam 为窗口句柄，NULL 表示取消
LRESULT CWindowResizerDlg::OnOverlayPickWindow(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	HWND hTarget = reinterpret_cast<HWND>(wParam);
	if (hTarget != NULL && ::IsWindow(hTarget))
	{
		FillTargetWindowInfo(hTarget);

		// 按进程自动匹配档案：仅在用户显式开启「按进程自动切换」时才动作，默认关闭
		if (m_config.IsAutoSwitchEnabled())
		{
			CString szProc;
			if (Chromium::GetProcessImageName(hTarget, szProc))
			{
				CString szMatched = m_config.MatchProfileByProcess(szProc);
				const RESOLUTION_PROFILE* pCur = m_config.GetActiveProfile();
				// 命中且与当前激活档案不同才切换，避免反复重填下拉框
				if (!szMatched.IsEmpty() && (pCur == nullptr || szMatched != pCur->szName))
				{
					m_config.SetActiveProfile(szMatched);
					RefreshProfileCombo();
					RefreshResolutionCombo();
				}
			}
		}

		StopPicking(true);
	}
	else
	{
		StopPicking(false);
	}

	return 0;
}




void CWindowResizerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 2)
	{
		//每隔1秒钟检查一次要操作的窗口是否有效，并调用控件的启用或禁用状态
		IsEnableAllControls(IsWindowValid());
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CWindowResizerDlg::OnBnClickedButtonSetSize()
{
	if (!IsWindowValid())
		return;

	CString szWidth, szHeight;
	m_editSetWidth.GetWindowTextW(szWidth);
	m_editSetHeight.GetWindowTextW(szHeight);
	int nWidth = _ttoi(szWidth);
	int nHeight = _ttoi(szHeight);
	if (nWidth == 0 || nHeight == 0)
		return;

	//输入值在开启“设置大小”区的逻辑尺寸开关时是100% DPI等效值，需要换算成物理像素再设置
	int nPhysicalWidth = LogicalToPhysical(nWidth, m_bUseLogicalSizeSet);
	int nPhysicalHeight = LogicalToPhysical(nHeight, m_bUseLogicalSizeSet);

	ApplyTargetSize(nPhysicalWidth, nPhysicalHeight);

	//按实际结果刷新显示：视口口径发生回弹时会回退到客户区，此处能反映真实值
	CRect rcWin;
	::GetWindowRect(m_hWindowHandle, rcWin);
	UpdateSizeShow(rcWin.Width(), rcWin.Height());
	m_bClickedSetPercentage = false;
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
	ApplyTargetSize(nPhysicalWidth, nPhysicalHeight);

	//按实际结果刷新显示：视口口径发生回弹时会回退到客户区，此处能反映真实值
	CRect rcWin;
	::GetWindowRect(m_hWindowHandle, rcWin);
	UpdateSizeShow(rcWin.Width(), rcWin.Height());
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

	//重新获取窗口的大小（传父窗口矩形，由 UpdateSizeShow 按当前口径反算）
	CRect rcWin;
	::GetWindowRect(m_hWindowHandle, rcWin);
	UpdateSizeShow(rcWin.Width(), rcWin.Height());
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
	CRect rcWin;
	::GetWindowRect(m_hWindowHandle, rcWin);
	UpdateSizeShow(rcWin.Width(), rcWin.Height());
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

	//切换后刷新“窗口数据”处当前大小的显示（传父窗口矩形，按当前口径反算）
	if (IsWindowValid())
	{
		CRect rcWin;
		::GetWindowRect(m_hWindowHandle, rcWin);
		UpdateSizeShow(rcWin.Width(), rcWin.Height());
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

//切换尺寸口径：同步下拉框、刷新显示、按需调整「浏览器视口」项
void CWindowResizerDlg::SetSizeMode(int nMode, bool bSyncCombo)
{
	// 非 Chromium 窗口（或找不到渲染子窗口）时禁止进入视口口径
	if (nMode == ModeViewport && Chromium::FindRenderWidget(m_hWindowHandle) == NULL)
		nMode = ModeClient;

	m_nSizeMode = nMode;

	if (bSyncCombo && m_comboSizeMode.GetSafeHwnd() != NULL)
		m_comboSizeMode.SetCurSel(nMode);   // 下拉项下标与枚举值严格一一对应

	// 口径变化后按比例缩放的采样基准失效，下次按比例缩放时重新采样
	m_bClickedSetPercentage = false;

	UpdateViewportInfoShow();

	// 切换后刷新「窗口数据」处当前尺寸的显示：同一窗口在不同口径下应显示不同数字
	if (IsWindowValid())
	{
		CRect rcWin;
		::GetWindowRect(m_hWindowHandle, rcWin);
		UpdateSizeShow(rcWin.Width(), rcWin.Height());
	}
}

//根据目标窗口是否为 Chromium 决定「浏览器视口」项是否可选
//说明：CComboBox 无法单独把某一项置灰（需自绘），故采用「不可用时移除该项」的等效做法，
//既达到「非浏览器下不能选视口口径」的效果，又无需引入自绘代码
void CWindowResizerDlg::UpdateViewportModeAvailability()
{
	if (m_comboSizeMode.GetSafeHwnd() == NULL)
		return;

	const bool bAvailable = (Chromium::FindRenderWidget(m_hWindowHandle) != NULL);
	const int nCount = m_comboSizeMode.GetCount();

	if (bAvailable && nCount < 3)
	{
		CString szViewport;
		szViewport.LoadString(IDS_MODE_VIEWPORT);
		m_comboSizeMode.AddString(szViewport);
	}
	else if (!bAvailable && nCount >= 3)
	{
		// 索引 2 固定对应 ModeViewport；移除前先退出该口径，避免选中项指向已删除的索引
		if (m_nSizeMode == ModeViewport)
			SetSizeMode(ModeClient, true);
		m_comboSizeMode.DeleteString(2);
	}

	m_comboSizeMode.SetCurSel(m_nSizeMode);
	UpdateViewportInfoShow();
}

//重新填充分辨率下拉框
void CWindowResizerDlg::RefreshResolutionCombo()
{
	if (m_comboResolution.GetSafeHwnd() == NULL)
		return;

	m_comboResolution.ResetContent();
	m_currentResolutions.clear();

	const RESOLUTION_PROFILE* pProfile = m_config.GetActiveProfile();
	if (pProfile == nullptr)
		return;

	m_currentResolutions = pProfile->resolutions;
	for (size_t i = 0; i < m_currentResolutions.size(); i++)
	{
		CString szItem;
		// 必须是半角小写 x：OnCbnSelchangeComboResolution 依赖它拆分宽高
		szItem.Format(_T("%dx%d"), m_currentResolutions[i].cx, m_currentResolutions[i].cy);
		m_comboResolution.AddString(szItem);
	}
	m_comboResolution.SetCurSel(-1);
}

//重新填充档案下拉框并选中当前激活档案
void CWindowResizerDlg::RefreshProfileCombo()
{
	if (m_comboProfile.GetSafeHwnd() == NULL)
		return;

	m_comboProfile.ResetContent();
	std::vector<CString> names = m_config.GetProfileNames();
	for (size_t i = 0; i < names.size(); i++)
		m_comboProfile.AddString(names[i]);

	const RESOLUTION_PROFILE* pProfile = m_config.GetActiveProfile();
	if (pProfile != nullptr)
		m_comboProfile.SelectString(-1, pProfile->szName);
}

//刷新「浏览器视口测量」区文本
void CWindowResizerDlg::UpdateViewportInfoShow()
{
	if (m_staticViewportInfo.GetSafeHwnd() == NULL)
		return;

	CString szText(_T("--"));
	if (m_hWindowHandle != NULL && ::IsWindow(m_hWindowHandle))
	{
		int nVW = 0, nVH = 0, nDW = 0, nDH = 0, nBW = 0, nBH = 0;
		if (Chromium::Measure(m_hWindowHandle, nVW, nVH, nDW, nDH, nBW, nBH))
		{
			// 实测的 UI 占高与边框，便于用户判断反推是否合理（数字与语言无关，无需本地化）
			szText.Format(_T("UI +%dpx  Border %dx%d"), nDH, nBW, nBH);
		}
	}
	m_staticViewportInfo.SetWindowTextW(szText);
}

//选择尺寸口径
void CWindowResizerDlg::OnCbnSelchangeComboSizeMode()
{
	int nSel = m_comboSizeMode.GetCurSel();
	if (nSel == CB_ERR)
		return;
	SetSizeMode(nSel, false);
}

//手动切换分辨率档案
void CWindowResizerDlg::OnCbnSelchangeComboProfile()
{
	int nSel = m_comboProfile.GetCurSel();
	if (nSel == CB_ERR)
		return;

	CString szName;
	m_comboProfile.GetLBText(nSel, szName);
	m_config.SetActiveProfile(szName);
	RefreshResolutionCombo();

	// 用户一旦手动选过就关掉自动切换：否则下一秒定时器又切回去，等于下拉框失灵
	if (m_config.IsAutoSwitchEnabled())
	{
		m_config.SetAutoSwitchEnabled(false);
		m_checkAutoSwitch.SetCheck(BST_UNCHECKED);
		m_config.Save();
	}
}

//勾选/取消「按进程自动切换」
void CWindowResizerDlg::OnBnClickedCheckAutoswitch()
{
	m_config.SetAutoSwitchEnabled(m_checkAutoSwitch.GetCheck() == BST_CHECKED);
	m_config.Save();
}

//把当前分辨率列表与当前目标进程绑定保存到当前档案
void CWindowResizerDlg::OnBnClickedButtonSaveProfile()
{
	const RESOLUTION_PROFILE* pProfile = m_config.GetActiveProfile();
	if (pProfile == nullptr)
		return;

	// 先复制出名字与绑定，避免后续写入使内部 vector 扩容而导致指针失效
	CString szProfileName = pProfile->szName;
	CString szProcess = pProfile->szProcess;

	// 绑定「当前目标窗口的进程名」；目标窗口无效时保留原绑定不变
	if (m_hWindowHandle != NULL && ::IsWindow(m_hWindowHandle))
	{
		CString szCur;
		if (Chromium::GetProcessImageName(m_hWindowHandle, szCur))
			szProcess = szCur;   // 单进程覆盖；仍可在 ini 里手工改回逗号分隔的多进程
	}

	CString szCaption;
	szCaption.LoadString(IDS_RESTART_TIP_CAPTION);

	if (m_config.SaveProfile(szProfileName, szProcess, m_currentResolutions))
	{
		CString szMsg;
		szMsg.LoadString(IDS_PROFILE_SAVED);
		::MessageBox(m_hWnd, szMsg, szCaption, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		CString szMsg;
		szMsg.LoadString(IDS_ERR_INI_WRITE);
		::MessageBox(m_hWnd, szMsg, szCaption, MB_OK | MB_ICONWARNING);
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
	m_comboSizeMode.EnableWindow(bIsEnable);     //未选中窗口时禁用尺寸口径下拉框
	m_comboProfile.EnableWindow(bIsEnable);      //未选中窗口时禁用档案下拉框
	m_checkAutoSwitch.EnableWindow(bIsEnable);   //未选中窗口时禁用自动切换开关
	m_btnSaveProfile.EnableWindow(bIsEnable);    //未选中窗口时禁用保存档案按钮
	SetMaxSizeButtonEnable();
	SetPercentageEditEnable();
	UpdateDpiInfoShow();
}



void CAboutDlg::OnNMClickSyslinkCheckUpdate(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(NULL, _T("open"), _T("https://github.com/767939287/WindowResizer/releases"), NULL, NULL, SW_SHOW);
	*pResult = 0;
}
