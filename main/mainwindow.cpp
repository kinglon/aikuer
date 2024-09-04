#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "avatamanager.h"
#include "liveswapmanager.h"
#include "uiutil.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCtrls()
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
            ui->enableLiveSwapButton->setEnabled(false);
            LiveSwapManager::getInstance()->createLiveSwap();
        });
    connect(ui->stopLiveSwapButton, &QPushButton::clicked, [this]() {
            ui->enableLiveSwapButton->setEnabled(true);
            ui->stopLiveSwapButton->setEnabled(false);
            LiveSwapManager::getInstance()->closeLiveSwap();
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
    }
    else
    {
        ui->enableLiveSwapButton->setEnabled(true);
        ui->stopLiveSwapButton->setEnabled(false);
        UiUtil::showTip(errorMsg);
    }
}
