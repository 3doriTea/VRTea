#pragma once
#include <string>
#include <string_view>

namespace mtgb
{
	/// <summary>
	/// UTF8 文字数(バイト数ではない)を取得する
	/// </summary>
	/// <param name="_string">utf8の文字列</param>
	/// <returns>文字数</returns>
	size_t GetSizeUTF8Characters(const std::u8string& _string);

	/// <summary>
	/// utf8文字列を切り取る
	/// </summary>
	/// <param name="_string">utf8文字列</param>
	/// <param name="_count">文字数(バイト数ではない)</param>
	/// <returns>切り取られたutf8文字列</returns>
	std::u8string SubStrBegin(const std::u8string& _string, const int _count);

	/// <summary>
	/// ワイド文字列に変換する
	/// </summary>
	/// <param name="_string"></param>
	/// <returns></returns>
	inline std::wstring ToWString(const std::string& _string)
	{
		return {_string.begin(), _string.end()};
	}

	/// <summary>
	/// ワイド文字列に変換する
	/// </summary>
	/// <param name="_stringView"></param>
	/// <returns></returns>
	inline std::wstring ToWString(const std::string_view& _stringView)
	{
		return {_stringView.begin(), _stringView.end()};
	}

	/// <summary>
	/// マルチバイト文字列からワイド文字列へ変換
	/// </summary>
	/// <param name="_string">変換する文字列</param>
	/// <returns></returns>
	std::wstring MultiToWide(const std::string& _string);
	/// <summary>
	/// マルチバイト文字列からワイド文字列へ変換
	/// </summary>
	/// <param name="_string">文字列</param>
	/// <returns></returns>
	std::wstring MultiToWide(const char* _string);

	std::string WideToMulti(const std::wstring& _wstring);
	std::string WideToMulti(const wchar_t* _wstring);

	/// <summary>
	/// マルチバイト文字列からワイド文字列へ変換
	/// </summary>
	/// <param name="_string">変換する文字列</param>
	/// <returns></returns>
	std::string MultiToUTF8(const std::string& _string);
	/// <summary>
	/// マルチバイト文字列からワイド文字列へ変換
	/// </summary>
	/// <param name="_string">変換する文字列</param>
	/// <returns></returns>
	std::string MultiToUTF8(const char* _string);

	std::wstring UTF8ToWide(const std::string& _string);
	std::wstring UTF8ToWide(const std::u8string& _string);
	std::wstring UTF8ToWide(const char* _string);

	std::string WideToUTF8(const std::wstring& _wstring);
	std::string WideToUTF8(const wchar_t* _wstring);

	std::string UTF8ToMulti(const std::string& _string);
	std::string UTF8ToMulti(const char* _string);

	/// <summary>
	/// インスタンス名から生成番号を削除し、クラス名を返す
	/// </summary>
	/// <param name="_fullName"> インスタンス名</param>
	/// <returns></returns>
	std::string ExtractClassName(const std::string& _fullName);
} // namespace mtgb
