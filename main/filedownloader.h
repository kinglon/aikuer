#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include "Utility/DownloadManager.h"

class FileDownloader : public QObject, public IDownloadCallback
{
    Q_OBJECT

public:
    explicit FileDownloader(QObject *parent = nullptr);

public:
    void setDownloadUrl(QString downloadUrl) { m_downloadUrl = downloadUrl; }

    void setSaveFilePath(QString saveFilePath) { m_saveFilePath = saveFilePath; }

    bool run();

public: // implement IDownloadCallback
    virtual void OnDownloadProgress(int taskId, int progress) override {(void)taskId; (void)progress; }

    virtual void OnDownloadFinish(int taskId, bool isSuccess) override;

signals:
    void runFinish(bool ok);

    // 内部使用
    void downloadFinish(bool ok);

private slots:
    void onDownloadFinishSlot(bool ok);

private:
    QString m_downloadUrl;

    QString m_saveFilePath;

    int m_retryCount = 0;
};

#endif // FILEDOWNLOADER_H
