QT += quick winextras network

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../Utility/DownloadManager.cpp \
        ../Utility/DumpUtil.cpp \
        ../Utility/IcrCriticalSection.cpp \
        ../Utility/ImCharset.cpp \
        ../Utility/ImPath.cpp \
        ../Utility/LogBuffer.cpp \
        ../Utility/LogUtil.cpp \
        ../Utility/httpclientbase.cpp \
        avatarcontroller.cpp \
        echoremover.cpp \
        filedownloader.cpp \
        ipcworker.cpp \
        main.cpp \
        maincontroller.cpp \
        meetingcontroller.cpp \
        memoryimageprovider.cpp \
        settingmanager.cpp \
        statusmanager.cpp \
        translatelanguagecontroller.cpp \
        virtualcameramanager.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

#CONFIG += qmltypes
#QML_IMPORT_NAME = akoolqml
#QML_IMPORT_MAJOR_VERSION = 1

RC_ICONS = content/res/logo.ico

QMAKE_CXXFLAGS += -DQT_MESSAGELOGCONTEXT

# Enable PDB generation
QMAKE_CFLAGS_RELEASE += /Zi
QMAKE_CXXFLAGS_RELEASE += /Zi
QMAKE_LFLAGS_RELEASE += /DEBUG

HEADERS += \
    ../Utility/DownloadManager.h \
    ../Utility/DumpUtil.h \
    ../Utility/IcrCriticalSection.h \
    ../Utility/ImCharset.h \
    ../Utility/ImPath.h \
    ../Utility/LogBuffer.h \
    ../Utility/LogMacro.h \
    ../Utility/LogUtil.h \
    ../Utility/httpclientbase.h \
    avatarcontroller.h \
    echoremover.h \
    filedownloader.h \
    ipcworker.h \
    maincontroller.h \
    meetingcontroller.h \
    memoryimageprovider.h \
    settingmanager.h \
    statusmanager.h \
    translatelanguagecontroller.h \
    virtualcameramanager.h

# ffmpeg
INCLUDEPATH += ../ffmpeg2/include
LIBS += -L"$$_PRO_FILE_PWD_/../ffmpeg2" -lavdevice -lavformat -lavcodec -lavutil -lswscale -lswresample

# vcam
INCLUDEPATH += ../vcam/include
LIBS += -L"$$_PRO_FILE_PWD_/../vcam/lib" -ljericcam

# agora
INCLUDEPATH += ../agorasdk/high_level_api/include
LIBS += -L"$$_PRO_FILE_PWD_/../agorasdk" -lagora_rtc_sdk.dll

# windows
LIBS += -lwinmm
