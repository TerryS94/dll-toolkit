#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <random>

namespace StringUtils
{
	std::string TransformIntoValidWindowsFilename(const std::string& name);
	std::vector<std::string> Split(const std::string& s, char delim);
	std::string ToLowercase(const std::string& input);
	std::string ToUppercase(const std::string& input);
	bool HasUppercase(const std::string& s);
	bool HasLowercase(const std::string& s);
	std::string Trim(std::string str);
	bool IsEmptyOrWhitespaceOnly(const std::string& str);
	std::string EscapeRegexChars(const std::string& input);
	std::string Reverse(const std::string& text);
	std::string Utf16ToUtf8(const std::wstring& wstr);
	std::wstring Utf8ToUtf16(const std::string& str);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Utils
{
	unsigned char FloatToByte(float f);
	std::string GetCurrentUtcTimestamp();
	uintptr_t GetTargetProcessBase();
	bool CopyToClipboard(const std::string_view& text);
	int GetSecondsSinceEpochNow();
	int64_t GetMillisecondsSinceEpochNow();
	std::string GetSecondsSinceEpochNow_ToString();
	std::string GetMillisecondsSinceEpochNow_ToString();
	std::string GetTimeNow();
	std::string GetDateNow();
	std::string GetActiveWindowTitle();
	std::string FileBinaryToString(const std::string& path);
	int64_t GetFileTimestamp(const std::filesystem::path& path);
	std::string GetFileLastModifiedDate(const std::string& path);
	std::string GetDateTimeStringFromEpochMs(int64_t epochMs);

	template<typename T> inline T RandomValueRange(T start, T end)
	{
		thread_local static std::mt19937 gen(std::random_device {}());
		std::uniform_real_distribution<T> dist(start, end);
		return dist(gen);
	}
}