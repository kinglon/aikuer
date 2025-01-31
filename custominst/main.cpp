#include <QCoreApplication>
#include <Windows.h>

std::wstring GetSoftInstallPath()
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

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 2)
    {
        qCritical("too less params");
        return 1;
    }

    bool isInstall = true;
    if (strcmp(argv[1], "install") == 0)
    {
        isInstall = true;
    }
    else if (strcmp(argv[1], "uninstall") == 0)
    {
        isInstall = false;
    }
    else
    {
        qCritical("wrong param");
        return 2;
    }

    // 安装或卸载虚拟摄像头
    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"runas";
    std::wstring installerFilePath = GetSoftInstallPath() + L"x64\\jericcam_installer.exe";
    sei.lpFile = installerFilePath.c_str();
    std::wstring dllFilePath = GetSoftInstallPath() + L"x64\\jericcam.dll";
    wchar_t parameter[2*MAX_PATH];
    if (isInstall)
    {
        _snwprintf_s(parameter, 2*MAX_PATH, L"register \"%s\" 1", dllFilePath.c_str());
    }
    else
    {
        _snwprintf_s(parameter, 2*MAX_PATH, L"unregister \"%s\"", dllFilePath.c_str());
    }
    sei.lpParameters = parameter;

    if (!ShellExecuteEx(&sei))
    {
        qCritical("failed to start virtual camera installer, error is %d", GetLastError());
        return 3;
    }

    WaitForSingleObject(sei.hProcess, INFINITE);
    CloseHandle(sei.hProcess);

    // 注册应用scheme，先删除，如果是安装再创建
    HKEY hRootKey;
    LPCWSTR subKey = L"akool";
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, NULL, 0, KEY_WRITE, &hRootKey) == ERROR_SUCCESS)
    {
        RegDeleteTree(hRootKey, subKey);
        RegCloseKey(hRootKey);
    }

    if (isInstall)
    {
        HKEY hKey;
        LPCWSTR defaultValue = L"URL:akool";
        std::wstring mainFilePath = GetSoftInstallPath() + L"main.exe";
        std::wstring command = std::wstring(L"\"") + mainFilePath + L"\" --open-url=\"%1\"";
        LPCWSTR commandValue = command.c_str();
        if (RegCreateKeyEx(HKEY_CLASSES_ROOT, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
        {
            // Set the default value (@)
            if (RegSetValueEx(hKey, L"", 0, REG_SZ, (const BYTE*)defaultValue, (wcslen(defaultValue)+1)*2) != ERROR_SUCCESS)
            {
                qCritical("Failed to set default value.");
            }

            if (RegSetValueEx(hKey, L"URL Protocol", 0, REG_SZ, (const BYTE*)"", 2) != ERROR_SUCCESS)
            {
                qCritical("Failed to set URL Protocol value.");
            }

            // Create the "shell" subkey
            HKEY hShellKey;
            if (RegCreateKeyEx(hKey, L"shell", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hShellKey, NULL) == ERROR_SUCCESS)
            {
                HKEY hOpenKey;
                if (RegCreateKeyEx(hShellKey, L"open", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hOpenKey, NULL) == ERROR_SUCCESS)
                {
                    HKEY hCommandKey;
                    if (RegCreateKeyEx(hOpenKey, L"command", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hCommandKey, NULL) == ERROR_SUCCESS)
                    {
                        // Set the default value for the "command" key
                        if (RegSetValueEx(hCommandKey, L"", 0, REG_SZ, (const BYTE*)commandValue, (wcslen(commandValue)+1)*2) != ERROR_SUCCESS)
                        {
                            qCritical("Failed to set command value.");
                        }
                        RegCloseKey(hCommandKey);
                    }
                    else
                    {
                        qCritical("Failed to create 'command' key.");
                    }
                    RegCloseKey(hOpenKey);
                }
                else
                {
                    qCritical("Failed to create 'open' key.");
                }
                RegCloseKey(hShellKey);
            }
            else
            {
                qCritical("Failed to create 'shell' key.");
            }
            RegCloseKey(hKey);
        }
        else
        {
            qCritical("Failed to create or open 'akoolcamera' key.");
        }
    }

    return 0;
}
