#ifndef MEETINGWINDOW_H
#define MEETINGWINDOW_H

#include <QMainWindow>
#include "meetingcontroller.h"

namespace Ui {
class MeetingWindow;
}

class MeetingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MeetingWindow(QWidget *parent = nullptr);
    ~MeetingWindow();

private:
    void initWindow();

private slots:
    void onPrintLog(QString content);

    void onUpdateImage();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MeetingWindow *ui;

    MeetingController* m_meetingController = nullptr;

    bool m_needClose = false;
};

#endif // MEETINGWINDOW_H
