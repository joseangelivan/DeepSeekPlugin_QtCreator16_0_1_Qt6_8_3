// deepseeknavigationchat.h
#pragma once

#include <coreplugin/inavigationwidgetfactory.h>
#include <coreplugin/editormanager/editormanager.h>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QDialog>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QLineEdit;
class QPushButton;
class QListWidget;
QT_END_NAMESPACE

namespace DeepSeek {

class FileUtils
{
public:
    static bool writeFile(const QString &filePath, const QString &content, QString &errorMessage);
    static bool appendToFile(const QString &filePath, const QString &content, QString &errorMessage);
    static bool ensureDirectoryExists(const QString &filePath, QString &errorMessage);
};

class DeepSeekPreviewDialog : public QDialog
{
    Q_OBJECT
public:
    DeepSeekPreviewDialog(const QString &filePath, const QString &newContent, QWidget *parent = nullptr);
    bool accepted() const;

private:
    QTextEdit *m_previewEditor = nullptr;
    bool m_accepted = false;
};

class DeepSeekNavigationChat : public Core::INavigationWidgetFactory
{
    Q_OBJECT

public:
    DeepSeekNavigationChat();
    ~DeepSeekNavigationChat() override;

    Core::NavigationView createWidget() override;
    QString displayName() const override;
    int priority() const override;
    Utils::Id id() const override;

private slots:
    void onSendClicked();
    void handleApiReply(QNetworkReply *reply);

private:
    void sendApiRequest(const QString &endpoint, const QJsonObject &payload);
    void applyEditToCurrentFile(const QString &text);
    void applyEditToFile(const QString &filePath, const QString &content);
    void appendToChatHistory(const QString &sender, const QString &text);
    void sendSourceAnalysisCommand(const QString &command);
    void updateContextMetadata(QJsonObject &payload);
    void loadConversationHistory();
    void saveConversationHistory(const QString &message, const QString &response);
    void showPreviewDialog(const QString &filePath, const QString &newContent);

    QWidget *m_widget = nullptr;
    QLineEdit *m_inputLine = nullptr;
    QTextEdit *m_outputBox = nullptr;
    QPushButton *m_sendButton = nullptr;
    QListWidget *m_historyList = nullptr;

    QNetworkAccessManager *m_networkManager = nullptr;

    QString m_apiUrl = "https://api.deepseek.dev";
    QString m_apiKey = "YOUR_API_KEY";
    QString m_currentFile;
};

} // namespace DeepSeek
