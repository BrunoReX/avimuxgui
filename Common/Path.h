#pragma once

#include "Windows.h"
#include "UTF-8.h"

#include <vector>
#include <deque>
#include <stdexcept>

#ifndef MAX_LONG_PATH
#define MAX_LONG_PATH 32768
#endif

class CPath
{
private:
	// TODO: Find better solution than this...
	template<class TChar>
	static TChar PathSeperator();

	template<>
	static char PathSeperator() {
		return '\\';
	}

	template<>
	static wchar_t PathSeperator() {
		return L'\\';
	}

	template<class TChar>
	static TChar CurrentDirectory();

	template<>
	static char CurrentDirectory() {
		return '.';
	}

	template<>
	static std::string CurrentDirectory() {
		return ".";
	}

	template<>
	static wchar_t CurrentDirectory() {
		return L'.';
	}

	template<>
	static std::wstring CurrentDirectory() {
		return L".";
	}

	template<class TChar>
	static std::basic_string<TChar> UpperDirectory();

	template<>
	static std::string UpperDirectory() {
		return "..";
	}

	template<>
	static std::wstring UpperDirectory() {
		return L"..";
	}

	template<class TChar>
	static TChar DriveSeperator();

	template<>
	static char DriveSeperator() {
		return ':';
	}

	template<>
	static wchar_t DriveSeperator() {
		return L':';
	}

	template<class TChar>
	static std::basic_string<TChar> NetworkResourceSeperator();

	template<>
	static std::string NetworkResourceSeperator() {
		return "\\\\";
	}

	template<>
	static std::wstring NetworkResourceSeperator() {
		return L"\\\\";
	}

	template<class TChar>
	static std::basic_string<TChar> ShortUNC();

	template<>
	static std::string ShortUNC() {
		return "\\\\?\\";
	}

	template<>
	static std::wstring ShortUNC() {
		return L"\\\\?\\";
	}

	template<class TChar>
	static std::basic_string<TChar> LongUNC();

	template<>
	static std::string LongUNC() {
		return "\\\\?\\UNC\\";
	}

	template<>
	static std::wstring LongUNC() {
		return L"\\\\?\\UNC\\";
	}

private:
	template<class TChar, class _in_it>
	static void split(const std::basic_string<TChar>& in, TChar sep, _in_it in_it)
	{
		std::basic_string<TChar>::size_type lastPos = 0;
		std::basic_string<TChar>::size_type nextPos;

		if (in.empty())
			return;

		do 
		{
			nextPos = in.find(sep, lastPos);
	
			std::basic_string<TChar> nextPart;
			if (nextPos != std::basic_string<TChar>::npos)
				nextPart = in.substr(lastPos, nextPos - lastPos);
			else
				nextPart = in.substr(lastPos);

			*in_it = nextPart;
			lastPos = nextPos+1;
		} while (nextPos != std::basic_string<TChar>::npos);
	}

private:
	template<class T, DWORD (WINAPI _GetModuleFileName)(HMODULE, T*, DWORD)>
	static std::basic_string<T> GetAbsolutePath(const std::basic_string<T>& path) 
	{
		std::basic_string<T> result = path;
		std::basic_string<T> pathSeperator(1, PathSeperator<T>());
		std::basic_string<T> driveSeperator(1, DriveSeperator<T>());
		std::basic_string<T> networkResourceStart = NetworkResourceSeperator<T>();

		if (result.size() > 2 && (result.substr(0, networkResourceStart.size()) == networkResourceStart || result.substr(1, driveSeperator.size()) == driveSeperator))
		{
		}
		else
		{
			std::basic_string<T> appDir = CPath::GetAppDir<T, _GetModuleFileName>(NULL);
			result = CPath::Combine(appDir, result);
		}

		return result;
	}	

	template<class T, BOOL (WINAPI _GetVolumePathName)(const T*, T*, DWORD), DWORD (WINAPI _GetModuleFileName)(HMODULE, T*, DWORD)>
	static std::basic_string<T> Combine(
		const std::basic_string<T>& first, 
		const std::basic_string<T>& second)
	{
		if (second.empty())
			return first;

		std::basic_string<T> driveSeperator(1, DriveSeperator<T>());
		std::basic_string<T> networtResourceSeperator = NetworkResourceSeperator<T>();
		std::basic_string<T> pathSeperator(1, PathSeperator<T>());
		std::basic_string<T> result;
		
		if (first.empty()) {
			std::basic_string<T> appDir = GetAppDir<T, _GetModuleFileName>(NULL);
			result = Combine(appDir, second);
			return result;
		}

		if (second.size() > 2 && (second.substr(0, networtResourceSeperator.size()) == networtResourceSeperator || second.substr(1, driveSeperator.size()) == driveSeperator))
		{
			result = second;
		}
		else
		{
			// absolute path without drive -> get volume mount point from first 
			if (second.substr(0, pathSeperator.size()) == pathSeperator) {
				T buf[MAX_LONG_PATH]; buf[0]=0;
				if (_GetVolumePathName(first.c_str(), buf, MAX_LONG_PATH)) {
					result = buf;
				} else {
					result = first;
				}
			} else {
				result = first;
			}


			if (result[result.size()-1] != PathSeperator<T>())
				result = result.append(pathSeperator);

			std::basic_string<T> toAppend;
			if (second.substr(0, pathSeperator.size()) == pathSeperator)
				toAppend = second.substr(pathSeperator.size());
			else
				toAppend = second;

			result.append(toAppend);
		}

		return result;
	}

	template<class T, class TFindData, HANDLE (WINAPI _FindFirst)(const T*, TFindData*), BOOL (WINAPI _FindNext)(HANDLE, TFindData*)>
	static int FindFiles(const std::basic_string<T>& searchPattern, std::vector<std::basic_string<T>>& result)
	{
		std::basic_string<T>::size_type indexOfLastSlash = searchPattern.find_last_of(PathSeperator<T>());
		if (indexOfLastSlash == std::basic_string<T>::npos)
			return 0;

		std::basic_string<T> path = searchPattern.substr(0, indexOfLastSlash + 1);

		TFindData findData;
		HANDLE hFind = _FindFirst(searchPattern.c_str(), &findData);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				std::basic_string<T> fullFilePath = path;
				fullFilePath.append(findData.cFileName);
				result.push_back(fullFilePath);
			} while (_FindNext(hFind, &findData));
			FindClose(hFind);
		}

		return 1;
	}

	template<class TChar, DWORD (WINAPI _GetModuleFileName)(HMODULE, TChar*, DWORD)>
	static std::basic_string<TChar> GetAppDir(std::basic_string<TChar>* appFile) {
		TChar appExecuteableFile[32768]; appExecuteableFile[0]=0;
		_GetModuleFileName(NULL, appExecuteableFile, 32768);
		if (appFile)
			*appFile = (const TChar*)appExecuteableFile;
		std::basic_string<TChar> file = CUTF8(appExecuteableFile);

		size_t index = file.find_last_of(PathSeperator<TChar>());
		std::basic_string<TChar> result = file.substr(0, index);
		return result;
	}

public:
	/** \brief Ruft zu einem Pfad den verfügbaren freien Speicher ab.
	 */
	static unsigned __int64 GetAvailableSpace(const CUTF8& path) {
		std::wstring strPath = path;

		wchar_t buf[MAX_LONG_PATH]; buf[0]=0;
		if (!GetVolumePathNameW(strPath.c_str(), buf, MAX_LONG_PATH))
			return 0;

		ULARGE_INTEGER bytesAvailable;
		if (GetDiskFreeSpaceExW(buf, &bytesAvailable, NULL, NULL)) {
			return bytesAvailable.QuadPart;
		}

		return 0;
	}

/*	template<class TChar>
	static std::basic_string<TChar> GetAppDir(std::basic_string<TChar>* appFile) {
		TChar appExecuteableFile[32768]; appExecuteableFile[0]=0;
		GetModuleFileName(NULL, appExecuteableFile, 32768);
		if (appFile)
			*appFile = (const TChar*)appExecuteableFile;
		std::basic_string<TChar> file = CUTF8(appExecuteableFile);

		size_t index = file.find_last_of(PathSeperator<TChar>());
		std::basic_string<TChar> result = file.substr(0, index);
		return result;
	}
*/
	static std::wstring GetAbsolutePath(const std::wstring& path) 
	{
		return GetAbsolutePath<wchar_t, GetModuleFileNameW>(path);
	}

	static std::string GetAbsolutePath(const std::string& path) 
	{
		return GetAbsolutePath<char, GetModuleFileNameA>(path);
	}

	static std::wstring Combine(const std::wstring& first, const std::wstring& second)
	{
		return Combine<wchar_t, GetVolumePathNameW, GetModuleFileNameW>(first, second);
	}

	static std::string Combine(const std::string& first, const std::string& second)
	{
		return Combine<char, GetVolumePathNameA, GetModuleFileNameA>(first, second);
	}

	static int FindFiles(const std::string& filePath, std::vector<std::string>& result)
	{
		return FindFiles<char, WIN32_FIND_DATAA, FindFirstFileA, FindNextFileA>(filePath, result);
	}

	static int FindFiles(const std::wstring& filePath, std::vector<std::wstring>& result)
	{
		return FindFiles<wchar_t, WIN32_FIND_DATAW, FindFirstFileW, FindNextFileW>(filePath, result);
	}

	static std::string GetAppDir(std::string* appFile) 
	{
		return GetAppDir<char, GetModuleFileNameA>(appFile);
	}

	static std::wstring GetAppDir(std::wstring* appFile) 
	{
		return GetAppDir<wchar_t, GetModuleFileNameW>(appFile);
	}

	template<class TChar> 
	static bool IsLongPath(const std::basic_string<TChar>& path)
	{
		if (path.size() < ShortUNC<TChar>().size())
			return false;
		if (path.substr(0, ShortUNC<TChar>().size()) == ShortUNC<TChar>())
			return true;

		return false;
	}

	template<class TChar>
	static std::basic_string<TChar> GetFileName(const std::basic_string<TChar>& fullPath)
	{
		std::basic_string<TChar>::size_type posOfLastBackslash = fullPath.find_last_of(PathSeperator<TChar>());
		if (posOfLastBackslash == std::basic_string<TChar>::npos)
			return fullPath;

		if (posOfLastBackslash == fullPath.size()-1)
			return std::basic_string<TChar>();

		std::basic_string<TChar> result = fullPath.substr(posOfLastBackslash+1);
		return result;
	}

	template<class TChar>
	static std::basic_string<TChar> GetLongFilename(const std::basic_string<TChar>& in)
	{
		std::deque<std::basic_string<TChar>> in_components;
		std::deque<std::basic_string<TChar>> out_components;
		bool inputIsNetworkShare = false;
		split(in, PathSeperator<TChar>(), std::insert_iterator<std::deque<std::basic_string<TChar>>>(
			in_components, in_components.end()));

		// if the input string starts with \\ and is not UNC, it is a network share. Those leading backslashes must
		// be removed to create a UNC path
		if (in_components.size() >= 2)
		{
			if (!IsLongPath(in)) {
				if (in_components[0].empty() && in_components[1].empty()) {
					inputIsNetworkShare = true;
					in_components.pop_front();
					in_components.pop_front();
				}
			}
		}

		std::deque<std::basic_string<TChar>>::iterator iter;
		for (iter = in_components.begin(); iter != in_components.end(); iter++) {
			std::basic_string<TChar>& component = *iter;
			// slash means nothing at all
			if (component != CurrentDirectory<std::basic_string<TChar>>()) {
				// path up
				if (component == UpperDirectory<TChar>()) {
					if (!out_components.empty())
						out_components.pop_back();
					else {
						// there is a .. that cannot be evaluated
					}
				} else {
					out_components.push_back(component);
				}
			}
		}

		std::deque<std::basic_string<TChar>>::iterator iter_out;
		std::basic_string<TChar> result;
		for (iter_out = out_components.begin(); iter_out != out_components.end(); iter_out++) {
			if (iter_out != out_components.begin())
				result.append(1, PathSeperator<TChar>());
			result.append(*iter_out);
		}

		if (IsLongPath(in))
			return result;
		else {
			if (inputIsNetworkShare) {
				return LongUNC<TChar>() + result;
			} else {
				return ShortUNC<TChar>() + result;
			}
		}
	}

	template<class TChar>
	static std::basic_string<TChar> ReplaceIllegalFilenameCharacters(const std::basic_string<TChar>& fileName, TChar replacementChar)
	{
		CUTF8 utf8IllegalCharacters("\\\"?/<>:*|");
		std::basic_string<TChar> illegalCharacters = utf8IllegalCharacters;
		if (illegalCharacters.find(replacementChar) != std::basic_string<TChar>::npos)
			throw std::invalid_argument("The replacement character must be a valid file name character");

		std::basic_string<TChar>::size_type startPos = 0;
		std::basic_string<TChar>::size_type nextPos = 0;

		std::basic_string<TChar> result = fileName;
		while (std::basic_string<TChar>::npos != (nextPos = result.find_first_of(utf8IllegalCharacters, startPos))) {
			result[nextPos] = replacementChar;
			startPos = nextPos;
		}
		return result;
	}
};