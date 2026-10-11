// LanguageManager.h: 界面语言管理器（语言判定、注册表记忆、资源句柄切换）
//

#pragma once

/// 应用支持的界面语言。
enum class AppLanguage
{
	Chinese,    ///< 简体中文
	English     ///< 英语（美国）
};

/// 界面语言管理器：负责语言偏好的注册表读写、系统语言判定，
/// 以及按语言切换线程 UI 语言与 MFC 资源句柄。
/// 全部为静态方法，不持有实例状态，可在启动早期安全调用。
class LanguageManager
{
public:
	/// 获取应使用的界面语言：优先注册表已保存的偏好，未保存时跟随系统 UI 语言。
	/// @return 解析得到的语言；注册表读取失败或值非法时回退到系统语言判定结果。
	static AppLanguage GetSavedOrSystemLanguage();

	/// 将用户手动选择的语言写入注册表持久化。
	/// @param lang 要保存的语言。
	static void SaveLanguage(AppLanguage lang);

	/// 按指定语言应用运行时环境：设置线程 UI 语言并使 MFC 使用本模块资源。
	/// @param lang 目标语言。
	static void ApplyLanguage(AppLanguage lang);

	/// 判断当前线程 UI 语言是否为英文，供英文排版适配逻辑使用。
	/// @return true 表示当前为英文界面。
	static bool IsEnglishUi();

private:
	/// 注册表子键名（相对 SetRegistryKey 设定的应用键）。
	static LPCTSTR const kSettingsSubKey;   ///< 存储界面相关设置的子键名："Settings"
	/// 注册表值名，保存语言偏好。
	static LPCTSTR const kLanguageValue;    ///< 值名："Language"
	/// 中文语言标记值。
	static LPCTSTR const kLangZh;           ///< 中文标记："zh"
	/// 英文语言标记值。
	static LPCTSTR const kLangEn;           ///< 英文标记："en"
};
