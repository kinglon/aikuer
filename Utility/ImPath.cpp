#include <Windows.h>
#include "ImPath.h"
#include <shlobj.h>
#include <string>
#include <shlwapi.h>
#include <sys\stat.h>

using namespace std;

std::wstring CImPath::m_appName = L"AkoolCam";

std::wstring CImPath::GetDataPath()
{
	static std::wstring wstrPath;
	if (!wstrPath.empty())
	{
		return wstrPath;
	}

    wstrPath = GetLocalAppDataPath() + L"data\\";
	if (!PathFileExists(wstrPath.c_str()))
	{		
		CreateDirectory(wstrPath.c_str(), NULL);
	}
	
	return wstrPath;
}

std::wstring CImPath::GetCachePath()
{
	static std::wstring wstrPath;
	if (!wstrPath.empty())
	{
		return wstrPath;
	}

	wstrPath = GetSystemTempPath();

	return wstrPath;
}

std::wstring CImPath::GetSoftInstallPath()
{
    static std::wstring strSoftInstallPath;
    if (!strSoftInstallPath.empty())
    {
        return strSoftInstallPath;
    }

    wchar_t szModulePath[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szModulePath, MAX_PATH);
    for (int i = wcslen(szModulePath); i >= 0; --i)
    {
        if (szModulePath[i] == '\\')
        {
            szModulePath[i] = 0;
            break;
        }
    }

    strSoftInstallPath = std::wstring(szModulePath) + L"\\";

    return strSoftInstallPath;
}

std::wstring CImPath::GetLocalAppDataPath()
{
    static std::wstring localAppDataPath;
    if (!localAppDataPath.empty())
    {
        return localAppDataPath;
    }

	TCHAR szPath[MAX_PATH];
	if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)!=S_OK)
	{
		return L"";
	}

    localAppDataPath = std::wstring(szPath) + L"\\" + CImPath::m_appName + L"\\";
    if (!PathFileExists(localAppDataPath.c_str()))
    {
        CreateDirectory(localAppDataPath.c_str(), NULL);
    }
    return localAppDataPath;
}

std::wstring CImPath::GetSystemTempPath()
{
	TCHAR szPath[MAX_PATH];

	if (GetTempPath(MAX_PATH, szPath) == 0)
	{
		return L"";	
	}
	return std::wstring(szPath);// 默认路径已经带"\"
}

std::wstring CImPath::GetConfPath()
{
    static std::wstring strConfPath = L"";
    if (!strConfPath.empty())
    {
        return strConfPath;
    }

    strConfPath = GetSoftInstallPath() + L"Configs\\";
    if (!PathFileExists(strConfPath.c_str()))
    {
        CreateDirectory(strConfPath.c_str(), NULL);
    }
    return strConfPath;
}

std::wstring CImPath::GetLogPath()
{
    static std::wstring wstrPath;
    if (!wstrPath.empty())
    {
        return wstrPath;
    }

    wstrPath = GetLocalAppDataPath() + L"Log\\";
    if (!PathFileExists(wstrPath.c_str()))
    {
        CreateDirectory(wstrPath.c_str(), NULL);
    }

    return wstrPath;
}

std::wstring CImPath::GetDumpPath()
{
    static std::wstring wstrPath;
    if (!wstrPath.empty())
    {
        return wstrPath;
    }

    wstrPath = GetLocalAppDataPath() + L"dump\\";
    if (!PathFileExists(wstrPath.c_str()))
    {
        CreateDirectory(wstrPath.c_str(), NULL);
    }

    return wstrPath;
}
