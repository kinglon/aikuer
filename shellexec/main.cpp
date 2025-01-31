#include <QCoreApplication>
#include <Windows.h>

std::wstring UTF8ToUnicode(const char* src)
{
    if (src == NULL)
    {
        return L"";
    }
    std::string strSrc(src);
    int length = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, NULL, 0);
    wchar_t *buf = new wchar_t[length + 1];
    MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, buf, length);
    buf[length] = L'\0';
    std::wstring dest = buf;
    delete[] buf;
    return dest;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 2)
    {
        qCritical("too less params");
        return 1;
    }

    std::wstring programFile = UTF8ToUnicode(argv[1]);
    std::wstring param;
    for (int i=2; i<argc; i++)
    {
        param += UTF8ToUnicode(argv[i]);
    }

    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"runas";
    sei.lpFile = programFile.c_str();
    sei.lpParameters = param.c_str();

    if (!ShellExecuteEx(&sei))
    {
        qCritical("failed to start program, error is %d", GetLastError());
        return 3;
    }

    WaitForSingleObject(sei.hProcess, INFINITE);
    CloseHandle(sei.hProcess);
    return 0;
}
