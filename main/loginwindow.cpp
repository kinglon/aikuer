#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "loginmanager.h"
#include "uiutil.h"

LoginWindow::LoginWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    initCtrls();
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::initCtrls()
{
    connect(ui->loginPushButton, &QPushButton::clicked, [this]() {
            onLoginBtnClicked();
        });
    connect(LoginManager::getInstance(), &LoginManager::onLoginResult, this, &LoginWindow::onLoginResult);
}

void LoginWindow::onLoginBtnClicked()
{
    QString name = ui->nameLineEdit->text();
    if (name.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"用户名不能为空"));
        return;
    }

    QString password = ui->passwordLineEdit->text();
    if (password.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"密码不能为空"));
        return;
    }

    ui->loginPushButton->setEnabled(false);
    LoginManager::getInstance()->login(name, password);
}

void LoginWindow::onLoginResult(bool success, QString errorMessage)
{
    ui->loginPushButton->setEnabled(true);
    if (!success)
    {
        UiUtil::showTip(errorMessage);
        return;
    }

    accept();
    close();
}
