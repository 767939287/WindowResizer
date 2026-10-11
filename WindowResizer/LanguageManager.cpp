// LanguageManager.cpp: 界面语言管理器的实现
//

#include "stdafx.h"
#include "LanguageManager.h"

// 注册表与语言标记的常量定义（静态数据成员需在实现文件中定义）
LPCTSTR const LanguageManager::kSettingsSubKey = _T("Settings");
LPCTSTR const LanguageManager::kLanguageValue = _T("Language");
LPCTSTR const LanguageManager::kLangZh = _T("zh");
LPCTSTR const LanguageManager::kLangEn = _T("en");

//获取应使用的界面语言：优先注册表偏好，未保存或值非法时跟随系统 UI 语言
AppLanguage LanguageManager::GetSavedOrSystemLanguage()
{
	CWinApp* pApp = AfxGetApp();
	if (pApp != nullptr)
	{
		// GetProfileString 在值不存在时返回默认值，因此这里用空串作为“未保存”的标记
		CString szSaved = pApp->GetProfileString(kSettingsSubKey, kLanguageValue, _T(""));
		if (szSaved.CompareNoCase(kLangEn) == 0)
			return AppLanguage::English;
		if (szSaved.CompareNoCase(kLangZh) == 0)
			return AppLanguage::Chinese;
		// 其余情况（含手工改成非法值）一律视为未保存，继续走系统语言判定
	}

	// 中文系语言判为中文，其余判为英文，避免为非中英文系统回退到中文界面
	return (PRIMARYLANGID(::GetUserDefaultUILanguage()) == LANG_CHINESE)
		? AppLanguage::Chinese : AppLanguage::English;
}

//将用户手动选择的语言写入注册表持久化
void LanguageManager::SaveLanguage(AppLanguage lang)
{
	CWinApp* pApp = AfxGetApp();
	if (pApp == nullptr)
		return;   // 理论上仅在 InitInstance 之前调用才会发生，静默忽略

	// 写入失败时 CWinApp 内部会吞掉错误，不影响程序继续运行
	pApp->WriteProfileString(kSettingsSubKey, kLanguageValue,
		(lang == AppLanguage::English) ? kLangEn : kLangZh);
}

//按指定语言应用运行时环境：设置线程 UI 语言并使 MFC 使用本模块资源
void LanguageManager::ApplyLanguage(AppLanguage lang)
{
	LANGID langId = (lang == AppLanguage::English)
		? MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
		: MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);

	// 设置线程 UI 语言后，FindResource 会按该语言在模块资源目录中挑选对应的 LANGUAGE 资源块，
	// 因此这里必须在创建任何对话框/菜单之前完成
	::SetThreadUILanguage(langId);

	// SetThreadPreferredUILanguages 为 Vista+ 才有，动态加载以保证旧系统仍可运行
	typedef BOOL(WINAPI *SetThreadPreferredUILanguagesProc)(DWORD dwFlags, const WCHAR* pwszLanguagesBuffer, PULONG pulNumLanguages);
	static SetThreadPreferredUILanguagesProc pSetThreadPreferredUILanguages =
		(SetThreadPreferredUILanguagesProc)::GetProcAddress(
			::GetModuleHandle(_T("kernel32.dll")), "SetThreadPreferredUILanguages");
	if (pSetThreadPreferredUILanguages != nullptr)
	{
		// MUI_LANGUAGE_ID 要求传入十六进制语言 ID 字符串（如 "0409" = en-US、"0804" = zh-CN），
		// 且整个列表必须以双 null 结尾；这里手工拼装字符以避免额外引入格式化函数头文件
		wchar_t szLangList[8] = { 0 };
		static const wchar_t szHex[] = L"0123456789ABCDEF";
		szLangList[0] = szHex[(langId >> 12) & 0xF];
		szLangList[1] = szHex[(langId >> 8) & 0xF];
		szLangList[2] = szHex[(langId >> 4) & 0xF];
		szLangList[3] = szHex[langId & 0xF];
		// szLangList[4]、[5] 保持为 0，正好构成 MUI 需要的双 null 结尾
		ULONG nCount = 0;
		pSetThreadPreferredUILanguages(MUI_LANGUAGE_ID, szLangList, &nCount);
	}

	// .rc 中中/英两个语言块都编入本模块，显式指定资源句柄可确保 MFC 从本模块取资源
	AfxSetResourceHandle(AfxGetInstanceHandle());
}

//判断当前线程 UI 语言是否为英文
bool LanguageManager::IsEnglishUi()
{
	LANGID langId = ::GetThreadUILanguage();
	if (langId == 0)   // 返回 0 表示尚未显式设置线程 UI 语言，此时跟随系统语言
		langId = ::GetUserDefaultUILanguage();
	return (PRIMARYLANGID(langId) == LANG_ENGLISH);
}
