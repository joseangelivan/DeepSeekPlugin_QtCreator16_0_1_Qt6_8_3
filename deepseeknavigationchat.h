#pragma once
#include <coreplugin/inavigationwidgetfactory.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/session.h>
#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>
#include <utils/fileutils.h>
#include <utils/qtcassert.h> // Para verificaciones adicionales
#include <QString>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QDialog>
#include <QPointer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QJsonDocument>
#include <QDateTime>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QTimer>
#include <projectexplorer/project.h>
#include <projectexplorer/projectmanager.h>
#include "deepseeksettings.h"

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
    explicit DeepSeekPreviewDialog(const QString &filePath, const QString &newContent, QWidget *parent = nullptr);
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

protected:
    void handleGenericEditor(Core::IDocument *document, const QString &text);

private slots:
    void onSendClicked();
    void handleApiReply(QNetworkReply *reply);
    void onSettingsChanged();

private:
    // File operations
    void applyEditToCurrentFile(const QString &text);
    void applyEditToFile(const QString &filePath, const QString &content);
    void showPreviewDialog(const QString &filePath, const QString &newContent);

    // Chat operations
    void appendToChatHistory(const QString &sender, const QString &text);
    void sendSourceAnalysisCommand(const QString &command);
    void updateContextMetadata(QJsonObject &payload);

    // History management
    void loadConversationHistory();
    void saveConversationHistory(const QString &message, const QString &response);
    Utils::FilePath getHistoryFilePath() const;
    QJsonObject getCurrentContext() const;

    // API communication
    void sendApiRequest(const QString &endpoint, const QJsonObject &payload);

    // UI components
    QWidget *m_widget = nullptr;
    QLineEdit *m_inputLine = nullptr;
    QTextEdit *m_outputBox = nullptr;
    QPushButton *m_sendButton = nullptr;
    QListWidget *m_historyList = nullptr;

    // Network
    QNetworkAccessManager *m_networkManager = nullptr;
    QJsonArray m_conversationHistory;

    // Configuration - ahora con valores por defecto más seguros
    QString m_apiUrl = "";
    QString m_apiKey = "";
    QString m_model = "deepseek-chat";
    QString m_systemPrompt = "You are a helpful AI assistant";
    double m_temperature = 0.7;
    int m_maxTokens = 2048;
    QString m_currentFile;

signals:
    void settingsChanged();
};

} // namespace DeepSeek
