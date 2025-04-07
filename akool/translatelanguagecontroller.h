#ifndef TRANSLATELANGUAGECONTROLLER_H
#define TRANSLATELANGUAGECONTROLLER_H

#include <QObject>
#include <QVector>
#include <QNetworkAccessManager>
#include "../Utility/httpclientbase.h"

class TranslateLanguage
{
public:
    // id
    QString m_tlId;

    // language name
    QString m_languageName;

    // language code
    QString m_languageCode;

    // 国旗图片url
    QString m_flagUrl;

    // 国旗在本地的图片文件路径
    QString m_localFlagImagePath;

    // 该语言默认的声音ID
    QString m_voiceId;

public:
    bool isValid() const
    {
        if (m_tlId.isEmpty() || m_languageCode.isEmpty())
        {
            return false;
        }

        return true;
    }

    bool isDownloaded() const
    {
        return !m_localFlagImagePath.isEmpty();
    }
};

class TranslateLanguageController : public HttpClientBase
{
    Q_OBJECT

public:
    enum {
        STEP_INIT = 1,
        STEP_GET_SOURCE_LANG_LIST = 2,
        STEP_GET_TARGET_LANG_LIST = 3,
        STEP_DOWNLOAD_FLAG = 4,
        STEP_FINISH = 5
    };

public:
    explicit TranslateLanguageController(QObject *parent = nullptr);

public:
    void run();

    // 获取Source语言列表
    QVector<TranslateLanguage> getSourceLanguages() { return m_sourceLanguages; }

    // 获取Target语言列表
    QVector<TranslateLanguage> getTargetLanguages() { return m_targetLanguages; }

signals:
    void runFinish();

protected:
    virtual void onHttpResponse(QNetworkReply *reply) override;

private:
    void getLanguageListFromServer(QString from);

    bool handleGetLanguageListResponse(QNetworkReply *reply);

    void downloadFlagImage();

private slots:
    void onMainTimer();

private:
    // 国旗图片存放路径，尾部有后划线
    QString m_flagImagePath;

    // Source语言列表
    QVector<TranslateLanguage> m_sourceLanguages;

    // Target语言列表
    QVector<TranslateLanguage> m_targetLanguages;

    // 当前步骤
    int m_currentStep = STEP_INIT;

    // 标志当前步骤是否正在进行
    bool m_currentStepOnGoing = false;
};

#endif // TRANSLATELANGUAGECONTROLLER_H
