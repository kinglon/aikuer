#include "virtualcameramanager.h"
#include "Utility/ImPath.h"
#include "rtmpmanager.h"
#include <QFile>
#include "softcam.h"
#include "settingmanager.h"
#include "Utility/LogMacro.h"

VirtualCameraManager::VirtualCameraManager(QObject *parent)
    : QObject{parent}
{
    RtmpManager::getInstance()->setRtmpFrameArriveCallback(this);
}

VirtualCameraManager* VirtualCameraManager::getInstance()
{
    static VirtualCameraManager* instance = new VirtualCameraManager();
    return instance;
}

bool VirtualCameraManager::enableVirtualCamera(bool enable)
{
    if (!enable)
    {
        m_enableVirtualCamera = false;
        return true;
    }

    // 检查虚拟摄像头是否已经安装
    bool install = true;
    std::wstring dataPath = CImPath::GetLocalAppDataPath() + L"JericCamera\\";
    std::wstring dllFilePath = dataPath + L"jericcam.dll";
    std::wstring newDllFilePath = CImPath::GetSoftInstallPath() + L"x64\\jericcam.dll";
    std::wstring flagFilePath = dataPath + L"install";
    QFile flagFile(QString::fromStdWString(flagFilePath));
    QFile dllFile(QString::fromStdWString(dllFilePath));
    if (!flagFile.exists() || !dllFile.exists())
    {
        install = false;
    }

    if (!install)
    {
        CreateDirectory(dataPath.c_str(), nullptr);
        if (!CopyFile(newDllFilePath.c_str(), dllFilePath.c_str(), FALSE))
        {
            qCritical("failed to copy the virtual camera dll");
            return false;
        }

        // 启动安装程序
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);

        std::wstring installerFilePath = CImPath::GetSoftInstallPath() + L"x64\\jericcam_installer.exe";
        wchar_t command[3*MAX_PATH];
        _snwprintf_s(command, 3*MAX_PATH, L"\"%s\" register \"%s\" ", installerFilePath.c_str(), dllFilePath.c_str());
        if (!CreateProcess(NULL, (LPWSTR)command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        {
            qCritical("failed to start installer process, error is %d", GetLastError());
            return false;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        // 创建标志文件
        if (flagFile.open(QFile::WriteOnly))
        {
            flagFile.close();
        }
    }

    m_enableVirtualCamera = true;
    return true;
}

void VirtualCameraManager::onRtmpFrameArrive(AVFrame* frame)
{
    sendFrame(frame);
    av_frame_free(&frame);
}

void VirtualCameraManager::sendFrame(const AVFrame* frame)
{
    if (!m_enableVirtualCamera)
    {        
        return;
    }

    static scCamera camera = nullptr;
    static int width = 0;
    static int height = 0;
    if (camera == nullptr)
    {
        width = frame->width;
        height = frame->height;
        camera = scCreateCamera(width, height, 60);
        if (camera == nullptr)
        {            
            return;
        }
    }

    if (frame->width != width || frame->height != height)
    {
        if (SettingManager::getInstance()->m_nLogLevel >= (int)ELogLevel::LOG_LEVEL_DEBUG)
        {
            qDebug("frame size is (%d, %d), desired size is (%d, %d)", frame->width, frame->height, width, height);
        }
        return;
    }

    int dataSize = frame->width * frame->height * 4; // 4 bytes per pixel for BGRA format
    BYTE* pixelData = new BYTE[dataSize];
    if (pixelData == nullptr)
    {
        return;
    }

    for (int y = 0; y < frame->height; y++)
    {
        for (int x = 0; x < frame->width; x++)
        {
            uint8_t *ptr = pixelData + y * frame->width + x * 4;
            ptr[0] = frame->data[0][y * frame->linesize[0] + x * 3 + 2];
            ptr[1] = frame->data[0][y * frame->linesize[0] + x * 3 + 1];
            ptr[2] = frame->data[0][y * frame->linesize[0] + x * 3];
            ptr[3] = 255;
        }
    }

    scSendFrame(camera, pixelData, dataSize);
    delete[] pixelData;
}
