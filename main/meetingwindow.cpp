#include "meetingwindow.h"
#include "ui_meetingwindow.h"
#include <QTimer>
#include <QDateTime>
#include <QCloseEvent>
#include "uiutil.h"
#include "virtualcameramanager.h"

MeetingWindow::MeetingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MeetingWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initWindow();
}

MeetingWindow::~MeetingWindow()
{
    delete ui;
}

void MeetingWindow::initWindow()
{
    VirtualCameraManager::getInstance()->enableVirtualCamera(true);

    QTimer* updateImageTimer = new QTimer(this);
    updateImageTimer->setInterval(20);
    connect(updateImageTimer, &QTimer::timeout, this, &MeetingWindow::onUpdateImage);
    updateImageTimer->start();

    // 延迟1秒启动
    QTimer::singleShot(1000, [this](){
        m_meetingController = new MeetingController();
        m_meetingController->enableGenerateQImage();
        connect(m_meetingController, &MeetingController::printLog, this, &MeetingWindow::onPrintLog);
        connect(m_meetingController, &MeetingController::runFinish, [this]() {
            m_meetingController->deleteLater();
            m_meetingController = nullptr;
            if (m_needClose)
            {
                this->close();
            }
        });
        m_meetingController->run();
    });
}

void MeetingWindow::onPrintLog(QString content)
{
    static int lineCount = 0;
    if (lineCount >= 1000)
    {
        ui->logEdit->clear();
        lineCount = 0;
    }
    lineCount++;

    qInfo(content.toStdString().c_str());
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentTimeString = currentDateTime.toString("[MM-dd hh:mm:ss] ");
    QString line = currentTimeString + content;
    ui->logEdit->append(line);
}

void MeetingWindow::onUpdateImage()
{
    if (m_meetingController)
    {
        QImage* image = m_meetingController->popImage();
        if (image)
        {
            UiUtil::showImage(QPixmap::fromImage(*image), *ui->remoteImage);
            delete image;
        }
    }
}

void MeetingWindow::closeEvent(QCloseEvent *event)
{
    if (m_meetingController)
    {
        event->ignore();
        m_needClose = true;
        m_meetingController->requestStop();
    }
    else
    {
        event->accept();
    }
}
