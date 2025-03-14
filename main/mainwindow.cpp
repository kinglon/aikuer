﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "avatamanager.h"
#include "liveswapmanager.h"
#include "cameramanager.h"
#include "uiutil.h"
#include <QTimer>
#include <QPixmap>
#include "rtmpmanager.h"
#include "virtualcameramanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initWindow()
{
    connect(AvataManager::getInstance(), &AvataManager::avataChanged, this, &MainWindow::onAvataChanged);
    AvataManager::getInstance()->getAvataFromServer();
    connect(ui->refreshAvataButton, &QPushButton::clicked, []() {
            AvataManager::getInstance()->getAvataFromServer();
        });

    connect(ui->avataListWidget, &QListWidget::itemClicked, [&](QListWidgetItem *item){
            QString avatarId = item->data(Qt::UserRole).toString();
            AvataManager::getInstance()->setCurrentAvata(avatarId);
            LiveSwapManager::getInstance()->updateAvatar();
        });

    connect(LiveSwapManager::getInstance(), &LiveSwapManager::createLiveSwapResult, this, &MainWindow::onCreateLiveSwapResult);
    connect(ui->enableLiveSwapButton, &QPushButton::clicked, [this]() {
            onEnableLiveSwapBtnClicked();
        });
    connect(ui->stopLiveSwapButton, &QPushButton::clicked, [this]() {
            onStopLiveSwapBtnClicked();
        });

    connect(ui->refreshCameraButton, &QPushButton::clicked, [this]() {
            onRefreshCameraBtnClicked();
        });
    QTimer::singleShot(100, [this]() {
        onRefreshCameraBtnClicked();
    });

    QTimer* updateImageTimer = new QTimer(this);
    updateImageTimer->setInterval(20);
    connect(updateImageTimer, &QTimer::timeout, this, &MainWindow::onUpdateImage);
    updateImageTimer->start();

    connect(ui->enableVCameraCheckBox, &QPushButton::clicked, [this]() {
        onEnableVCameraCheckBoxClicked();
    });
}

void MainWindow::onAvataChanged()
{
    static QSize iconSize(150, 150);
    ui->avataListWidget->clear();
    ui->avataListWidget->setIconSize(iconSize);
    QVector<Avata>* avatars = AvataManager::getInstance()->getAvatas();
    for (const auto& avatar : *avatars)
    {
        if (avatar.m_localImagePath.isEmpty())
        {
            continue;
        }

        QListWidgetItem* item = new QListWidgetItem(ui->avataListWidget);
        QPixmap pixmap(avatar.m_localImagePath);
        QIcon icon(pixmap);
        item->setIcon(icon);
        item->setSizeHint(iconSize);
        item->setData(Qt::UserRole, avatar.m_avataId);
        ui->avataListWidget->addItem(item);
    }
}

void MainWindow::onCreateLiveSwapResult(bool ok, QString errorMsg)
{
    if (ok)
    {
        ui->enableLiveSwapButton->setEnabled(false);
        ui->stopLiveSwapButton->setEnabled(true);
        RtmpManager::getInstance()->startPush();
        RtmpManager::getInstance()->startPull();
    }
    else
    {
        ui->enableLiveSwapButton->setEnabled(true);
        ui->stopLiveSwapButton->setEnabled(false);
        UiUtil::showTip(errorMsg);
    }
}


void MainWindow::onRefreshCameraBtnClicked()
{
    ui->cameraComboBox->clear();
    CameraManager::getInstance()->refreshCameras();
    auto cameras = CameraManager::getInstance()->getCameras();
    if (cameras.size() == 0)
    {
        return;
    }

    for (auto camera : cameras)
    {
        ui->cameraComboBox->addItem(QString::fromUtf8(camera->long_name), camera);
    }
    ui->cameraComboBox->setCurrentIndex(0);
    CameraManager::getInstance()->setCurrentCamera(cameras[0]);
    CameraManager::getInstance()->stopReadCamera();
    CameraManager::getInstance()->startReadCamera();
}

void MainWindow::onEnableLiveSwapBtnClicked()
{
    if (!CameraManager::getInstance()->isOpen())
    {
        UiUtil::showTip(QString::fromWCharArray(L"摄像头未打开"));
        return;
    }

    ui->enableLiveSwapButton->setEnabled(false);
    LiveSwapManager::getInstance()->createLiveSwap();
}

void MainWindow::onStopLiveSwapBtnClicked()
{
    ui->enableLiveSwapButton->setEnabled(true);
    ui->stopLiveSwapButton->setEnabled(false);
    LiveSwapManager::getInstance()->closeLiveSwap();
    RtmpManager::getInstance()->stopPush();
    RtmpManager::getInstance()->stopPull();
}

void MainWindow::onEnableVCameraCheckBoxClicked()
{
    if (ui->enableVCameraCheckBox->isChecked())
    {
        if (!VirtualCameraManager::getInstance()->enableVirtualCamera(true))
        {
            ui->enableVCameraCheckBox->setChecked(false);
        }
    }
    else
    {
        VirtualCameraManager::getInstance()->enableVirtualCamera(false);
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    CameraManager::getInstance()->stopReadCamera();
    RtmpManager::getInstance()->stopPull();
}

void MainWindow::onUpdateImage()
{
    QImage* cameraImage = CameraManager::getInstance()->getCameraImage();
    if (cameraImage)
    {
        UiUtil::showImage(QPixmap::fromImage(*cameraImage), *ui->cameraImage);
        delete cameraImage;
    }

    QImage* rtmpPullImage = RtmpManager::getInstance()->getRtmpPullImage();
    if (rtmpPullImage)
    {
        UiUtil::showImage(QPixmap::fromImage(*rtmpPullImage), *ui->rtmpImage);
        delete rtmpPullImage;
    }
}
