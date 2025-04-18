﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initWindow();

private slots:
    void onAvataChanged();

    void onCreateLiveSwapResult(bool ok, QString errorMsg);

    void onRefreshCameraBtnClicked();

    void onEnableLiveSwapBtnClicked();

    void onStopLiveSwapBtnClicked();

    void onEnableVCameraCheckBoxClicked();

    void closeEvent(QCloseEvent *e);

    void onUpdateImage();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
