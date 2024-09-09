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
        RtmpManager::getInstance()->setRtmpFrameArriveCallback(nullptr);
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
        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei, sizeof(sei));
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = L"runas";
        std::wstring installerFilePath = CImPath::GetSoftInstallPath() + L"x64\\jericcam_installer.exe";
        sei.lpFile = installerFilePath.c_str();
        wchar_t parameter[2*MAX_PATH];
        _snwprintf_s(parameter, 2*MAX_PATH, L"register \"%s\" ", dllFilePath.c_str());
        sei.lpParameters = parameter;

        if (!ShellExecuteEx(&sei))
        {
            qCritical("failed to start installer, error is %d", GetLastError());
            return false;
        }

        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);

        // 创建标志文件
        if (flagFile.open(QFile::WriteOnly))
        {
            flagFile.close();
        }
    }

    m_enableVirtualCamera = true;
    RtmpManager::getInstance()->setRtmpFrameArriveCallback(this);
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

    // rgb24转成bgra后推给虚拟摄像头
    AVFrame *bgraFrame = av_frame_alloc();
    bgraFrame->width = frame->width;
    bgraFrame->height = frame->height;
    bgraFrame->format = AV_PIX_FMT_BGRA;
    av_frame_get_buffer(bgraFrame, 0);
    SwsContext *swsContext = sws_getContext(frame->width, frame->height, AV_PIX_FMT_RGB24,
                                                bgraFrame->width, bgraFrame->height, AV_PIX_FMT_BGRA,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height,
              bgraFrame->data, bgraFrame->linesize);

    uint8_t *bgraData = bgraFrame->data[0];
    int dataSize = frame->width * frame->height * 4; // 4 bytes per pixel for BGRA format
    scSendFrame(camera, bgraData, dataSize);

    av_frame_free(&bgraFrame);
    sws_freeContext(swsContext);
}
