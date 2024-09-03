#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private:
    void initCtrls();

private:
    void onLoginBtnClicked();

private slots:
    void onLoginResult(bool success, QString errorMessage);

private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
