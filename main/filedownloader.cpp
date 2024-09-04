#include "filedownloader.h"

#define MAX_RETRY_COUNT 2

FileDownloader::FileDownloader(QObject *parent)
    : QObject{parent}
{
    connect(this, &FileDownloader::downloadFinish, this, &FileDownloader::onDownloadFinishSlot, Qt::QueuedConnection);
}

bool FileDownloader::run()
{    
    if (m_downloadUrl.isEmpty() || m_saveFilePath.isEmpty())
    {
        return false;
    }

    DeleteFile(m_saveFilePath.toStdWString().c_str());
    int taskId = CDownloadManager::GetInstance()->CreateDownloadTask(m_downloadUrl.toStdWString(),
                                                                     m_saveFilePath.toStdWString(),
                                                                     this);
    if (taskId == 0)
    {
        qCritical("failed to create the download task, url is %s", m_downloadUrl.toStdString().c_str());
        return false;
    }

    return true;
}

void FileDownloader::OnDownloadFinish(int taskId, bool isSuccess)
{
    (void)taskId;
    emit downloadFinish(isSuccess);
}

void FileDownloader::onDownloadFinishSlot(bool ok)
{
    if (ok)
    {
        emit runFinish(true);
    }
    else
    {
        if (m_retryCount >= MAX_RETRY_COUNT)
        {
            emit runFinish(false);
        }
        else
        {
            qInfo("retry to download file");
            m_retryCount++;
            int taskId = CDownloadManager::GetInstance()->CreateDownloadTask(m_downloadUrl.toStdWString(),
                                                                             m_saveFilePath.toStdWString(),
                                                                             this);
            if (taskId == 0)
            {
                qCritical("failed to create the download task, url is %s", m_downloadUrl.toStdString().c_str());
                emit runFinish(false);
            }
        }
    }
}
