QT       += core gui network multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Utility/DownloadManager.cpp \
    Utility/DumpUtil.cpp \
    Utility/IcrCriticalSection.cpp \
    Utility/ImCharset.cpp \
    Utility/ImPath.cpp \
    Utility/LogBuffer.cpp \
    Utility/LogUtil.cpp \
    avatamanager.cpp \
    cameramanager.cpp \
    camerareadthread.cpp \
    ffmpegutil.cpp \
    filedownloader.cpp \
    httpclientbase.cpp \
    liveswapmanager.cpp \
    loginmanager.cpp \
    loginwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    rtmpmanager.cpp \
    rtmppullthread.cpp \
    settingmanager.cpp \
    uiutil.cpp

HEADERS += \
    Utility/DownloadManager.h \
    Utility/DumpUtil.h \
    Utility/IcrCriticalSection.h \
    Utility/ImCharset.h \
    Utility/ImPath.h \
    Utility/LogBuffer.h \
    Utility/LogMacro.h \
    Utility/LogUtil.h \
    avatamanager.h \
    cameramanager.h \
    camerareadthread.h \
    ffmpegutil.h \
    filedownloader.h \
    httpclientbase.h \
    liveswapmanager.h \
    loginmanager.h \
    loginwindow.h \
    mainwindow.h \
    rtmpmanager.h \
    rtmppullthread.h \
    settingmanager.h \
    uiutil.h

FORMS += \
    loginwindow.ui \
    mainwindow.ui

# ffmpeg
INCLUDEPATH += ../ffmpeg/include
LIBS += -L"$$_PRO_FILE_PWD_/../ffmpeg/lib" -lavdevice -lavformat -lavcodec -lavutil
