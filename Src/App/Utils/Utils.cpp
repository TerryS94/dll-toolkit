#include "Utils.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <regex>
#include <fstream>

namespace StringUtils
{
	std::string TransformIntoValidWindowsFilename(const std::string& name)
	{
		std::regex reg(R"(\/|<|>|\"|:|\?|\*|\\|\|)");
		return std::regex_replace(name, reg, "_");
	}
	std::vector<std::string> Split(const std::string& s, char delim)
	{
		std::vector<std::string> out;
		std::stringstream ss(s);
		std::string tok;
		while (std::getline(ss, tok, delim)) if (!tok.empty()) out.push_back(tok);
		return out;
	}
	std::string ToLowercase(const std::string& input)
	{
		std::string result = input;
		for (char& c : result)
		{
			unsigned char uc = static_cast<unsigned char>(c);
			if (std::isalpha(uc)) c = static_cast<char>(std::tolower(uc));
		}
		return result;
	}
	std::string ToUppercase(const std::string& input)
	{
		std::string result = input;
		for (char& c : result)
		{
			unsigned char uc = static_cast<unsigned char>(c);
			if (std::isalpha(uc)) c = static_cast<char>(std::toupper(uc));
		}
		return result;
	}
	bool HasUppercase(const std::string& s)
	{
		return std::any_of(s.begin(), s.end(), [](unsigned char c) { return std::isupper(c); });
	}
	bool HasLowercase(const std::string& s)
	{
		return std::any_of(s.begin(), s.end(), [](unsigned char c) { return std::islower(c); });
	}
	std::string Trim(std::string str)
	{
		str.erase(str.find_last_not_of(' ') + 1);
		str.erase(0, str.find_first_not_of(' '));
		return str;
	}
	bool IsEmptyOrWhitespaceOnly(const std::string& str)
	{
		return str.empty() || std::all_of(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
	}
	std::string EscapeRegexChars(const std::string& input)
	{
		std::string output;
		for (char c : input)
		{
			if (c == '.' || c == '+' || c == '*' || c == '?' || c == '(' || c == ')' ||
				c == '[' || c == ']' || c == '{' || c == '}' || c == '\\' || c == '|' ||
				c == '^' || c == '$')
			{
				output += '\\';
			}
			output += c;
		}
		return output;
	}
	std::string Reverse(const std::string& text)
	{
		std::string reversedText = text;
		std::reverse(reversedText.begin(), reversedText.end());
		return reversedText;
	}
	std::string Utf16ToUtf8(const std::wstring& wstr)
	{
		if (wstr.empty()) return {};
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (size_needed <= 0) return {};
		std::string result(size_needed - 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size_needed, nullptr, nullptr);
		return result;
	}
	std::wstring Utf8ToUtf16(const std::string& str)
	{
		if (str.empty()) return std::wstring();
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
		std::wstring result(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], size_needed);
		return result;
	}
}//namespace StringUtils
namespace Utils
{
	std::string GetCurrentUtcTimestamp()
	{
		auto now = std::chrono::system_clock::now();
		auto now_t = std::chrono::system_clock::to_time_t(now);
		std::tm utc_tm;
		gmtime_s(&utc_tm, &now_t);
		// format RFC3339
		std::ostringstream oss;
		oss << std::put_time(&utc_tm, "%FT%TZ");
		return oss.str();
	}
	uintptr_t GetTargetProcessBase()
	{
		return (uintptr_t)GetModuleHandleA(nullptr);
	}
	bool CopyToClipboard(const std::string_view& text)
	{
		bool succeeded = false;

		if (HANDLE clipdata = GlobalAlloc(GMEM_MOVEABLE, text.length() + 1))
		{
			if (auto clipdataptr = GlobalLock(clipdata))
			{
				std::memcpy(clipdataptr, text.data(), text.length() + 1);
				GlobalUnlock(clipdata);

				if (OpenClipboard(nullptr))
				{
					if (EmptyClipboard() && SetClipboardData(CF_TEXT, clipdata))
						succeeded = true;

					CloseClipboard();
				}
			}
			if (!succeeded)
				GlobalFree(clipdata);
		}
		return succeeded;
	}
	int GetSecondsSinceEpochNow()
	{
		return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}
	int64_t GetMillisecondsSinceEpochNow()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	std::string GetSecondsSinceEpochNow_ToString()
	{
		return std::to_string(Utils::GetSecondsSinceEpochNow());
	}
	std::string GetMillisecondsSinceEpochNow_ToString()
	{
		return std::to_string(Utils::GetMillisecondsSinceEpochNow());
	}
	//the time that windows shows at the bottom right of your desktop
	std::string GetTimeNow()
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		char buffer[100];
		int result = GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, "hh':'mm tt", buffer, sizeof(buffer));
		if (result > 0) return std::string(buffer);
		return "";
	}
	//the date that windows shows at the bottom right of your desktop
	std::string GetDateNow()
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		char buffer[100];
		int result = GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, "MM'-'dd'-'yy", buffer, sizeof(buffer));
		if (result > 0) return std::string(buffer);
		return "";
	}
	std::string GetActiveWindowTitle()
	{
		char wnd_title[256];
		HWND hwnd = GetForegroundWindow();
		GetWindowText(hwnd, wnd_title, sizeof(wnd_title));
		return wnd_title;
	}
	std::string FileBinaryToString(const std::string& path)
	{
		std::ifstream in(path, std::ios::binary | std::ios::ate);
		if (!in) return {};
		std::streamsize size = in.tellg();
		in.seekg(0, std::ios::beg);
		std::string buf;
		buf.resize(static_cast<size_t>(size));
		if (size > 0) in.read(&buf[0], size);
		return buf;
	}
	int64_t GetFileTimestamp(const std::filesystem::path& path)
	{
		using namespace std::chrono;
		try
		{
			auto ftime = std::filesystem::last_write_time(path);
			auto sctp = time_point_cast<milliseconds>(ftime);
			return sctp.time_since_epoch().count();
		}
		catch (...)
		{
			return 0ll;
		}
	}
	std::string GetFileLastModifiedDate(const std::string& path)
	{
		namespace fs = std::filesystem;
		std::error_code ec;
		auto ftime = fs::last_write_time(path, ec);
		if (ec) return std::string { "[error getting timestamp: " } + ec.message() + "]";
		using namespace std::chrono;
		auto sctp = time_point_cast<system_clock::duration>(ftime - fs::file_time_type::clock::now() + system_clock::now());
		std::time_t tt = system_clock::to_time_t(sctp);
		std::tm tm;
		if (localtime_s(&tm, &tt) != 0) return "[error in localtime_s]";
		char buf[32];
		if (std::strftime(buf, sizeof(buf), "%m/%d/%y", &tm) == 0) return "[strftime error]";
		return std::string { buf };
	}
	std::string GetDateTimeStringFromEpochMs(int64_t epochMs)
	{
		static constexpr int64_t EPOCH_DIFF_MILLISECONDS = 11644473600000LL;
		unsigned __int64 ft100ns;
		long long adj = epochMs + EPOCH_DIFF_MILLISECONDS;
		if (adj < 0) return {};
		ft100ns = static_cast<unsigned __int64>(adj) * 10000ULL;
		FILETIME fileTimeUtc{};
		fileTimeUtc.dwLowDateTime = static_cast<DWORD>(ft100ns & 0xFFFFFFFFULL);
		fileTimeUtc.dwHighDateTime = static_cast<DWORD>((ft100ns >> 32) & 0xFFFFFFFFULL);
		FILETIME fileTimeLocal{};
		if (!FileTimeToLocalFileTime(&fileTimeUtc, &fileTimeLocal)) return {};
		SYSTEMTIME localSt{};
		if (!FileTimeToSystemTime(&fileTimeLocal, &localSt)) return {};
		char dateBuf[64] = { 0 };
		char timeBuf[64] = { 0 };
		int dateLen = GetDateFormatA(LOCALE_USER_DEFAULT, 0, &localSt, "MM'/'dd'/'yyyy", dateBuf, static_cast<int>(sizeof(dateBuf)));
		if (dateLen <= 0) return {};
		int timeLen = GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &localSt, "hh':'mm tt", timeBuf, static_cast<int>(sizeof(timeBuf)));
		if (timeLen <= 0) return {};
		std::string out{};
		out.reserve(dateLen + 1 + timeLen);
		out = dateBuf;
		out.push_back(' ');
		out += timeBuf;
		return out;
	}
}//namespace Utils