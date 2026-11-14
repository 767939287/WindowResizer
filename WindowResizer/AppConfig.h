// AppConfig.h: ini 配置文件读写与分辨率档案内存模型
//

#pragma once

#include <vector>

/// 一套分辨率档案：档案名 + 绑定的进程映像名 + 该档案自己的分辨率列表。
struct RESOLUTION_PROFILE
{
	CString szName;                  ///< 档案显示名，同时作为 ini 节名 [Profile:名称]
	CString szProcess;               ///< 绑定的进程映像名，多个用半角逗号分隔；空串表示不参与自动切换
	std::vector<CSize> resolutions;  ///< 分辨率列表（逻辑像素）；已去重并按宽度升序稳定排序
};

/// ini 配置文件读写与档案内存模型。
/// 文件位于 exe 同目录下的 WindowResizer.ini；不存在时自动生成带中文注释的默认文件。
/// 写入采用「内存模型整体序列化 + 整文件覆盖」的方式，以保留注释头（profile API 会抹掉注释）。
class CAppConfig
{
public:
	CAppConfig();
	~CAppConfig();

	/// 载入配置文件。文件缺失则生成默认文件；内容损坏则备份为 .bak 并回退内置默认。
	/// @return true 表示至少有一套可用档案（含回退情形）；即使磁盘不可写也返回 true（此时仅内存默认值可用）。
	bool Load();

	/// 把当前内存模型整体序列化重写回 ini（含中文注释头），会覆盖用户的格式调整。
	/// @return true 表示写盘成功；false 表示创建/写入失败（调用方提示 IDS_ERR_INI_WRITE）。
	bool Save();

	/// 取当前激活档案指针。
	/// @return 激活档案名不存在时返回第一套；档案列表为空时返回 nullptr。
	const RESOLUTION_PROFILE* GetActiveProfile() const;

	/// 取全部档案名，用于填充档案下拉框。
	std::vector<CString> GetProfileNames() const;

	/// 设置当前激活档案，仅改内存，不写盘。
	/// @param szName 档案名，必须与已有档案名精确匹配。
	/// @return true 表示匹配成功。
	bool SetActiveProfile(LPCTSTR szName);

	/// 是否启用「按进程自动切换档案」。
	/// 现已改为内置常开：分辨率按目标 exe 隔离，不再提供开关。
	/// @return 恒为 true。
	bool IsAutoSwitchEnabled() const;

	/// 设置自动切换开关，仅改内存，不写盘。
	/// @param bEnable true 表示启用。
	/// @note 该状态已不再影响行为（自动切换恒开），保留仅为 ini 字段兼容。
	void SetAutoSwitchEnabled(bool bEnable);

	/// 按进程映像名在全部档案中查找匹配项（大小写不敏感，支持逗号分隔的多进程）。
	/// @param szProcess 进程映像名，如 "chrome.exe"；空串直接返回空。
	/// @return 命中的档案名；无命中返回空串。
	CString MatchProfileByProcess(LPCTSTR szProcess) const;

	/// 新增或覆盖一套档案，并立即写盘。
	/// @param szName 档案名，同名即覆盖；传空串时自动以 szProcess 的第一个进程名作为节名，
	///               从而让每个目标 exe 各自拥有独立的 [Profile:xxx] 节。
	/// @param szProcess 绑定的进程映像名，可为空串；szName 为空时此项不能为空。
	/// @param resolutions 分辨率列表，内部会做合法性校验、去重、按宽度升序排序。
	/// @return true 表示写盘成功；szName 与 szProcess 均为空、或列表归一化后为空时返回 false。
	bool SaveProfile(LPCTSTR szName, LPCTSTR szProcess, const std::vector<CSize>& resolutions);

	/// 取配置文件绝对路径（%APPDATA%\WindowResizer\WindowResizer.ini）。
	/// 目录不存在时自动创建；%APPDATA% 不可用时回退到 exe 同目录。
	static CString GetIniPath();

private:
	/// 填充内置默认档案（沿用旧版硬编码的全部分辨率项 + 空进程绑定）。
	void LoadDefaults();

	/// 解析形如 "1920x1080" 的文本（半角小写 x，两段均为正整数）。
	/// @param szText 待解析文本，允许首尾空格。
	/// @param size 输出：解析结果（宽、高）。
	/// @return true 表示格式合法且数值在 (0, 16384] 内。
	bool ParseResolution(LPCTSTR szText, CSize& size) const;

	/// 对档案的分辨率列表做校验、去重、按宽度升序稳定排序；无效项丢弃。
	void NormalizeResolutions(RESOLUTION_PROFILE& profile) const;

	/// 按 BOM 自动识别编码（UTF-16LE / UTF-8 / ANSI）读取整个文本文件。
	bool ReadTextFile(LPCTSTR szPath, CString& content) const;

	/// 以 UTF-16LE + BOM 写出整个文本文件（保证中文注释不乱码）。
	bool WriteTextFile(LPCTSTR szPath, const CString& content) const;

	/// 把内存模型序列化为 ini 文本（注释头 + General 节 + 各 Profile 节）。
	CString SerializeToIniText() const;

	std::vector<RESOLUTION_PROFILE> m_profiles;  ///< 全部档案
	CString m_szActiveProfile;                   ///< 当前激活档案名
	bool m_bAutoSwitch;                          ///< 按进程自动切换的持久化状态；行为上恒为开启，仅用于 ini 字段兼容
};
