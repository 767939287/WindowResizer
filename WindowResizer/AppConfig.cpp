// AppConfig.cpp: ini 配置文件读写与分辨率档案内存模型的实现
//

#include "stdafx.h"
#include "AppConfig.h"

#include <shlobj.h>   // SHGetFolderPath：取 %APPDATA% 的规范方式

#include <algorithm>

#pragma comment(lib, "shell32.lib")

namespace
{
	// 内置默认分辨率列表（宽度升序、宽度相同按高度升序），与旧版硬编码列表保持一致
	const TCHAR* const kDefaultResolutions[] = {
		_T("640x480"),   _T("800x600"),   _T("1024x768"),  _T("1152x864"),
		_T("1280x720"),  _T("1280x800"),  _T("1280x960"),  _T("1280x1024"),
		_T("1366x768"),  _T("1400x1050"), _T("1440x900"),  _T("1600x900"),
		_T("1600x1200"), _T("1680x1050"), _T("1920x1080"), _T("1920x1200"),
		_T("2048x1536"), _T("2560x1080"), _T("2560x1440"), _T("2560x1600"),
		_T("2880x1800"), _T("3200x1800"), _T("3440x1440"), _T("3840x1600"),
		_T("3840x2160"), _T("3840x2400"), _T("5120x2160"), _T("5120x2880"),
		_T("7680x4320")
	};

	// 内置默认档案名（中英文界面上都用同一个名字，避免语言切换后找不到档案）
	const TCHAR* const kDefaultProfileName = _T("默认");

	// 档案绑定的进程列表支持半角逗号与中文逗号两种分隔
	bool ContainsProcess(LPCTSTR szList, LPCTSTR szProcess)
	{
		if (szList == nullptr || szProcess == nullptr)
			return false;

		CString szRest(szList);
		int nPos = 0;
		while (nPos < szRest.GetLength())
		{
			int nComma = szRest.Find(_T(','), nPos);
			CString szItem;
			if (nComma < 0)
			{
				szItem = szRest.Mid(nPos);
				nPos = szRest.GetLength();
			}
			else
			{
				szItem = szRest.Mid(nPos, nComma - nPos);
				nPos = nComma + 1;
			}
			szItem.Trim();
			szItem.Replace(_T("，"), _T(","));   // 容错：把中文逗号残留也去掉
			if (!szItem.IsEmpty() && szItem.CompareNoCase(szProcess) == 0)
				return true;
		}
		return false;
	}
}

CAppConfig::CAppConfig()
	: m_bAutoSwitch(false)
{
}

CAppConfig::~CAppConfig()
{
}

/// 取配置文件绝对路径（%APPDATA%\WindowResizer\WindowResizer.ini）。
/// 目录不存在时会一并创建，确保后续 Save() 能成功写入。
/// %APPDATA% 取不到时回退到 exe 同目录，保证任何情况下都能返回可用路径。
CString CAppConfig::GetIniPath()
{
	TCHAR szPath[MAX_PATH] = { 0 };
	if (FAILED(::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szPath)))
	{
		// 极端环境（无用户配置文件）下回退到 exe 目录，避免返回空路径导致后续读写全失败
		::GetModuleFileName(NULL, szPath, MAX_PATH);
		CString szFallback(szPath);
		int nPos = szFallback.ReverseFind(_T('\\'));
		if (nPos >= 0)
			szFallback = szFallback.Left(nPos + 1);
		szFallback += _T("WindowResizer.ini");
		return szFallback;
	}

	CString szDir(szPath);
	szDir += _T("\\WindowResizer");

	// 目录已存在时 CreateDirectory 返回失败但无需处理，这里只保证首次运行时能建出来
	::CreateDirectory(szDir, NULL);

	szDir += _T("\\WindowResizer.ini");
	return szDir;
}

//按 BOM 自动识别编码读取整个文本文件
bool CAppConfig::ReadTextFile(LPCTSTR szPath, CString& content) const
{
	content.Empty();

	CFile file;
	if (!file.Open(szPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
		return false;

	ULONGLONG nLen = file.GetLength();
	if (nLen == 0)
	{
		file.Close();
		return true;   // 空文件视为「内容为空」，由调用方决定是否重建
	}

	std::vector<BYTE> buf(static_cast<size_t>(nLen));
	file.Read(buf.data(), static_cast<UINT>(nLen));
	file.Close();

	// UTF-16LE：BOM 为 FF FE
	if (nLen >= 2 && buf[0] == 0xFF && buf[1] == 0xFE)
	{
		int nChars = static_cast<int>((nLen - 2) / sizeof(WCHAR));
		content = CString(reinterpret_cast<LPCWSTR>(buf.data() + 2), nChars);
		return true;
	}

	// 其余情况按字节流处理：优先当 UTF-8（严格模式），失败再回退系统 ANSI
	size_t nOffset = 0;
	if (nLen >= 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
		nOffset = 3;   // 跳过 UTF-8 BOM

	const char* pData = reinterpret_cast<const char*>(buf.data() + nOffset);
	int nBytes = static_cast<int>(nLen - nOffset);
	if (nBytes <= 0)
		return true;

	int nNeed = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pData, nBytes, nullptr, 0);
	UINT nCodePage = CP_UTF8;
	if (nNeed <= 0)
	{
		// 不是合法 UTF-8，改用当前 ANSI 代码页（记事本另存为 ANSI 的情形）
		nCodePage = CP_ACP;
		nNeed = ::MultiByteToWideChar(CP_ACP, 0, pData, nBytes, nullptr, 0);
		if (nNeed <= 0)
			return false;
	}

	std::vector<WCHAR> wbuf(static_cast<size_t>(nNeed) + 1, 0);
	::MultiByteToWideChar(nCodePage, nCodePage == CP_UTF8 ? MB_ERR_INVALID_CHARS : 0,
		pData, nBytes, wbuf.data(), nNeed);
	wbuf[nNeed] = 0;
	content = wbuf.data();
	return true;
}

//以 UTF-16LE + BOM 写出整个文本文件（保证中文注释不乱码）
bool CAppConfig::WriteTextFile(LPCTSTR szPath, const CString& content) const
{
	CFile file;
	// 用 shareDenyNone 而非 shareExclusive：用户常常开着记事本查看 ini 后再点保存，
	// 独占模式会导致写入直接失败且难以察觉
	if (!file.Open(szPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone))
		return false;

	const BYTE bom[2] = { 0xFF, 0xFE };
	file.Write(bom, 2);

	// CString 在 Unicode 构建下就是 UTF-16，可直接按字节写出
	int nChars = content.GetLength();
	if (nChars > 0)
		file.Write(static_cast<LPCWSTR>(content), static_cast<UINT>(nChars * sizeof(WCHAR)));

	file.Close();
	return true;
}

//解析 "宽x高" 文本；要求恰好一个半角小写 x，两段均为非零开头的正整数
bool CAppConfig::ParseResolution(LPCTSTR szText, CSize& size) const
{
	if (szText == nullptr)
		return false;

	CString sz(szText);
	sz.Trim();
	if (sz.IsEmpty())
		return false;

	int nPos = sz.Find(_T('x'));
	// 必须恰好出现一次，否则 "1920x1080x2" 之类会被错误接受
	if (nPos <= 0 || nPos != sz.ReverseFind(_T('x')))
		return false;

	CString szW = sz.Left(nPos);
	CString szH = sz.Mid(nPos + 1);
	szW.Trim();
	szH.Trim();
	if (szW.IsEmpty() || szH.IsEmpty())
		return false;

	// 只允许纯数字，且首字符不能是 '0'（排除 01920 这类写法）
	for (int i = 0; i < szW.GetLength(); ++i)
		if (!_istdigit(szW[i]))
			return false;
	for (int i = 0; i < szH.GetLength(); ++i)
		if (!_istdigit(szH[i]))
			return false;
	if (szW[0] == _T('0') || szH[0] == _T('0'))
		return false;

	int nW = _ttoi(szW);
	int nH = _ttoi(szH);
	// 上限按 8K 分辨率留余量，防止误填超大数值把目标窗口搞坏
	if (nW <= 0 || nH <= 0 || nW > 16384 || nH > 16384)
		return false;

	size.cx = nW;
	size.cy = nH;
	return true;
}

//去重并按「宽度升序、宽度相同按高度升序」重排
void CAppConfig::NormalizeResolutions(RESOLUTION_PROFILE& profile) const
{
	std::vector<CSize> result;
	for (size_t i = 0; i < profile.resolutions.size(); ++i)
	{
		const CSize& s = profile.resolutions[i];
		if (s.cx <= 0 || s.cy <= 0 || s.cx > 16384 || s.cy > 16384)
			continue;

		bool bDup = false;
		for (size_t j = 0; j < result.size(); ++j)
		{
			if (result[j].cx == s.cx && result[j].cy == s.cy)
			{
				bDup = true;
				break;
			}
		}
		if (!bDup)
			result.push_back(s);
	}

	std::stable_sort(result.begin(), result.end(), [](const CSize& a, const CSize& b)
	{
		if (a.cx != b.cx)
			return a.cx < b.cx;
		return a.cy < b.cy;
	});

	profile.resolutions.swap(result);
}

//填充内置默认档案
void CAppConfig::LoadDefaults()
{
	m_profiles.clear();
	m_szActiveProfile = kDefaultProfileName;
	m_bAutoSwitch = false;

	RESOLUTION_PROFILE prof;
	prof.szName = kDefaultProfileName;
	prof.szProcess.Empty();

	for (int i = 0; i < _countof(kDefaultResolutions); ++i)
	{
		CSize sz;
		if (ParseResolution(kDefaultResolutions[i], sz))
			prof.resolutions.push_back(sz);
	}
	NormalizeResolutions(prof);

	m_profiles.push_back(prof);
}

//把内存模型序列化为 ini 文本
CString CAppConfig::SerializeToIniText() const
{
	CString sz;

	sz += _T("; ============================================================\r\n");
	sz += _T("; WindowResizer 配置文件\r\n");
	sz += _T("; 「保存为档案」按钮会重写本文件，但保留这些注释\r\n");
	sz += _T("; ------------------------------------------------------------\r\n");
	sz += _T("; 【基本规则】\r\n");
	sz += _T(";   1. 每套档案一个节，节名格式：[Profile:档案名]\r\n");
	sz += _T(";   2. Process = 该档案绑定的进程映像名，多个用半角逗号分隔，可留空\r\n");
	sz += _T(";   3. ResolutionCount = 本档案分辨率条数，应与实际条数一致（不一致时以实际行数为准）\r\n");
	sz += _T(";   4. ResolutionN = 分辨率，格式「宽x高」，x 必须是半角小写 x，两段均为正整数\r\n");
	sz += _T(";   5. 非法行会被忽略；重复项自动去重；列表按宽度升序自动排序\r\n");
	sz += _T(";   6. 加一项：把 ResolutionCount 加 1，再补一行 ResolutionN=宽x高\r\n");
	sz += _T(";   7. 减一项：删掉对应行，把 ResolutionCount 改成实际条数\r\n");
	sz += _T("; ------------------------------------------------------------\r\n");
	sz += _T("; 【测试步骤】\r\n");
	sz += _T(";   1. 把 ResolutionCount 改成 3 只留前三行 -> 重启程序，下拉框只剩 3 项\r\n");
	sz += _T(";   2. 加一行 Resolution99=abc -> 重启程序，该项被忽略，其余正常\r\n");
	sz += _T(";   3. 把 ActiveProfile 改成不存在的名字 -> 重启后自动退回第一套档案\r\n");
	sz += _T(";   4. 把 AutoSwitch 改成 1 -> 打开浏览器点「开始查找窗口」，档案自动切到绑定该进程的那套\r\n");
	sz += _T("; ============================================================\r\n");
	sz += _T("\r\n");

	sz += _T("[General]\r\n");
	sz += _T("; 当前激活的档案名，必须是下面某个 [Profile:xxx] 的名字\r\n");
	sz.AppendFormat(_T("ActiveProfile=%s\r\n"), static_cast<LPCTSTR>(m_szActiveProfile));
	sz += _T("; 1 = 按目标窗口进程自动切换档案；0 = 只手动切换（默认 0）\r\n");
	sz.AppendFormat(_T("AutoSwitch=%d\r\n"), m_bAutoSwitch ? 1 : 0);

	for (size_t i = 0; i < m_profiles.size(); ++i)
	{
		const RESOLUTION_PROFILE& prof = m_profiles[i];
		sz.AppendFormat(_T("\r\n[Profile:%s]\r\n"), static_cast<LPCTSTR>(prof.szName));
		sz += _T("; 绑定进程（留空表示不参与自动切换），多个用半角逗号分隔\r\n");
		sz.AppendFormat(_T("Process=%s\r\n"), static_cast<LPCTSTR>(prof.szProcess));
		sz.AppendFormat(_T("ResolutionCount=%d\r\n"), static_cast<int>(prof.resolutions.size()));
		for (size_t j = 0; j < prof.resolutions.size(); ++j)
		{
			sz.AppendFormat(_T("Resolution%d=%dx%d\r\n"), static_cast<int>(j + 1),
				prof.resolutions[j].cx, prof.resolutions[j].cy);
		}
	}

	return sz;
}

//载入配置：缺失则生成默认文件，损坏则备份后回退
bool CAppConfig::Load()
{
	m_profiles.clear();
	m_szActiveProfile.Empty();
	m_bAutoSwitch = false;

	CString szPath = GetIniPath();
	CString szContent;

	if (!ReadTextFile(szPath, szContent) || szContent.IsEmpty())
	{
		// 文件不存在或为空：直接生成带注释的默认文件
		LoadDefaults();
		return Save();
	}

	RESOLUTION_PROFILE* pCur = nullptr;
	bool bInGeneral = false;
	bool bSawProfileSection = false;
	int nStart = 0;

	while (nStart <= szContent.GetLength())
	{
		int nLineEnd = szContent.Find(_T('\n'), nStart);
		CString szLine;
		if (nLineEnd < 0)
		{
			szLine = szContent.Mid(nStart);
			nStart = szContent.GetLength() + 1;
		}
		else
		{
			szLine = szContent.Mid(nStart, nLineEnd - nStart);
			nStart = nLineEnd + 1;
		}

		szLine.Trim();   // 顺带去掉行尾的 \r
		if (szLine.IsEmpty() || szLine[0] == _T(';') || szLine[0] == _T('#'))
			continue;

		// 节头
		if (szLine[0] == _T('['))
		{
			int nClose = szLine.Find(_T(']'));
			if (nClose <= 0)
				continue;

			CString szSection = szLine.Mid(1, nClose - 1);
			szSection.Trim();

			bInGeneral = (szSection.CompareNoCase(_T("General")) == 0);
			pCur = nullptr;

			if (!bInGeneral)
			{
				const CString szPrefix = _T("Profile:");
				if (szSection.GetLength() > szPrefix.GetLength()
					&& szSection.Left(szPrefix.GetLength()).CompareNoCase(szPrefix) == 0)
				{
					CString szName = szSection.Mid(szPrefix.GetLength());
					szName.Trim();
					if (!szName.IsEmpty())
					{
						RESOLUTION_PROFILE prof;
						prof.szName = szName;
						m_profiles.push_back(prof);
						pCur = &m_profiles.back();
						bSawProfileSection = true;
					}
				}
			}
			continue;
		}

		// key=value
		int nEq = szLine.Find(_T('='));
		if (nEq <= 0)
			continue;

		CString szKey = szLine.Left(nEq);
		CString szValue = szLine.Mid(nEq + 1);
		szKey.Trim();
		szValue.Trim();

		if (bInGeneral)
		{
			if (szKey.CompareNoCase(_T("ActiveProfile")) == 0)
				m_szActiveProfile = szValue;
			else if (szKey.CompareNoCase(_T("AutoSwitch")) == 0)
				m_bAutoSwitch = (_ttoi(szValue) != 0);
		}
		else if (pCur != nullptr)
		{
			if (szKey.CompareNoCase(_T("Process")) == 0)
			{
				pCur->szProcess = szValue;
			}
			else if (szKey.GetLength() >= 10 && szKey.Left(10).CompareNoCase(_T("Resolution")) == 0)
			{
				// ResolutionCount 也以此前缀开头，需排除
				if (szKey.CompareNoCase(_T("ResolutionCount")) != 0)
				{
					CSize szRes;
					if (ParseResolution(szValue, szRes))
						pCur->resolutions.push_back(szRes);
				}
			}
		}
	}

	// 归一化并按有效性过滤
	for (size_t i = 0; i < m_profiles.size(); ++i)
		NormalizeResolutions(m_profiles[i]);

	std::vector<RESOLUTION_PROFILE> validProfiles;
	for (size_t i = 0; i < m_profiles.size(); ++i)
	{
		if (!m_profiles[i].resolutions.empty())
			validProfiles.push_back(m_profiles[i]);
	}
	m_profiles.swap(validProfiles);

	if (!bSawProfileSection || m_profiles.empty())
	{
		// 内容无任何可用档案：备份原文件后重建，避免静默吞掉用户改坏的内容
		CString szBak = szPath + _T(".bak");
		::DeleteFile(szBak);
		::MoveFile(szPath, szBak);

		LoadDefaults();
		Save();
		return true;
	}

	// 确保激活档案有效
	if (m_szActiveProfile.IsEmpty() || !SetActiveProfile(m_szActiveProfile))
		m_szActiveProfile = m_profiles[0].szName;

	return true;
}

//整体序列化并覆盖写回 ini
bool CAppConfig::Save()
{
	// 防御：保存前保证至少有一套档案，否则写出的文件下次无法解析
	if (m_profiles.empty())
		LoadDefaults();

	return WriteTextFile(GetIniPath(), SerializeToIniText());
}

const RESOLUTION_PROFILE* CAppConfig::GetActiveProfile() const
{
	if (m_profiles.empty())
		return nullptr;

	for (size_t i = 0; i < m_profiles.size(); ++i)
	{
		if (m_profiles[i].szName == m_szActiveProfile)
			return &m_profiles[i];
	}
	return &m_profiles[0];
}

std::vector<CString> CAppConfig::GetProfileNames() const
{
	std::vector<CString> names;
	names.reserve(m_profiles.size());
	for (size_t i = 0; i < m_profiles.size(); ++i)
		names.push_back(m_profiles[i].szName);
	return names;
}

bool CAppConfig::SetActiveProfile(LPCTSTR szName)
{
	if (szName == nullptr)
		return false;

	for (size_t i = 0; i < m_profiles.size(); ++i)
	{
		if (m_profiles[i].szName == szName)
		{
			m_szActiveProfile = m_profiles[i].szName;
			return true;
		}
	}
	return false;
}

bool CAppConfig::IsAutoSwitchEnabled() const
{
	// 分辨率按目标 exe 隔离已改为内置常开行为，不再受开关控制
	return true;
}

void CAppConfig::SetAutoSwitchEnabled(bool bEnable)
{
	// 保留字段以兼容 ini 中的 AutoSwitch 键；行为上已恒为开启
	m_bAutoSwitch = bEnable;
}

CString CAppConfig::MatchProfileByProcess(LPCTSTR szProcess) const
{
	if (szProcess == nullptr || szProcess[0] == _T('\0'))
		return CString();

	for (size_t i = 0; i < m_profiles.size(); ++i)
	{
		if (ContainsProcess(m_profiles[i].szProcess, szProcess))
			return m_profiles[i].szName;
	}
	return CString();
}

bool CAppConfig::SaveProfile(LPCTSTR szName, LPCTSTR szProcess, const std::vector<CSize>& resolutions)
{
	// 档案名缺省时用绑定的第一个进程名兜底：调用方只需给出「当前目标进程」，
	// 无需了解「档案」概念，即可让每个 exe 各自拥有独立的 [Profile:xxx] 节
	CString szActualName = (szName != nullptr) ? CString(szName) : CString();
	szActualName.Trim();
	if (szActualName.IsEmpty())
	{
		CString szProc = (szProcess != nullptr) ? CString(szProcess) : CString();
		szProc.Trim();
		int nComma = szProc.Find(_T(','));
		if (nComma > 0)
			szProc = szProc.Left(nComma);   // 多进程绑定时取第一个作为节名
		szProc.Trim();
		if (szProc.IsEmpty())
			return false;
		szActualName = szProc;
	}

	RESOLUTION_PROFILE prof;
	prof.szName = szActualName;
	prof.szProcess = (szProcess != nullptr) ? szProcess : _T("");
	prof.resolutions = resolutions;
	NormalizeResolutions(prof);

	// 归一化后若一条都不剩，说明调用方传入了非法数据，直接拒绝以免写坏配置
	if (prof.resolutions.empty())
		return false;

	bool bReplaced = false;
	for (size_t i = 0; i < m_profiles.size(); ++i)
	{
		if (m_profiles[i].szName == prof.szName)
		{
			m_profiles[i] = prof;
			bReplaced = true;
			break;
		}
	}
	if (!bReplaced)
		m_profiles.push_back(prof);

	m_szActiveProfile = prof.szName;
	return Save();
}
