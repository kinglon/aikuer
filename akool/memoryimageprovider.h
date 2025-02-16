#ifndef MEMORYIMAGEPROVIDER_H
#define MEMORYIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include "maincontroller.h"

class MemoryImageProvider : public QQuickImageProvider
{
public:
    MemoryImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

public:
    void setMainController(MainController* controller) { m_mainController = controller; }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    MainController* m_mainController = nullptr;

    QImage* m_lastImage = nullptr;
};

#endif // MEMORYIMAGEPROVIDER_H
