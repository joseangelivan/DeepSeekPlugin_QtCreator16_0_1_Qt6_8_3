// deepseeknavigationchat.cpp

#include "deepseeknavigationchat.h"

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
#include <coreplugin/editormanager/ieditor.h>
#include <utils/fileutils.h>

using namespace DeepSeek;

// =============================
// FileUtils
// =============================
bool FileUtils::writeFile(const QString &filePath, const QString &content, QString &errorMessage)
{
    if (!ensureDirectoryExists(filePath, errorMessage))
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        errorMessage = QObject::tr("Cannot write to file: %1").arg(file.errorString());
        return false;
    }
    QTextStream out(&file);
    out << content;
    return true;
}

bool FileUtils::appendToFile(const QString &filePath, const QString &content, QString &errorMessage)
{
    if (!ensureDirectoryExists(filePath, errorMessage))
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        errorMessage = QObject::tr("Cannot append to file: %1").arg(file.errorString());
        return false;
    }
    QTextStream out(&file);
    out << content;
    return true;
}

bool FileUtils::ensureDirectoryExists(const QString &filePath, QString &errorMessage)
{
    QFileInfo info(filePath);
    QDir dir;
    if (!dir.mkpath(info.absolutePath())) {
        errorMessage = QObject::tr("Failed to create directory: %1").arg(info.absolutePath());
        return false;
    }
    return true;
}

// =============================
// DeepSeekPreviewDialog
// =============================
DeepSeekPreviewDialog::DeepSeekPreviewDialog(const QString &filePath, const QString &newContent, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preview Changes - %1").arg(QFileInfo(filePath).fileName()));
    setModal(true);
    resize(600, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_previewEditor = new QTextEdit(this);
    m_previewEditor->setPlainText(newContent);
    layout->addWidget(m_previewEditor);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    QPushButton *acceptButton = new QPushButton(tr("Apply"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(acceptButton, &QPushButton::clicked, this, [this]() { m_accepted = true; accept(); });
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonsLayout->addWidget(acceptButton);
    buttonsLayout->addWidget(cancelButton);
    layout->addLayout(buttonsLayout);
}

bool DeepSeekPreviewDialog::accepted() const { return m_accepted; }

// =============================
// DeepSeekNavigationChat
// =============================
DeepSeekNavigationChat::DeepSeekNavigationChat()
{
    m_networkManager = new QNetworkAccessManager();
}

DeepSeekNavigationChat::~DeepSeekNavigationChat() {}

QString DeepSeekNavigationChat::displayName() const { return "DeepSeek Chat"; }
int DeepSeekNavigationChat::priority() const { return 100; }
Utils::Id DeepSeekNavigationChat::id() const { return "DeepSeek.Chat"; }

Core::NavigationView DeepSeekNavigationChat::createWidget()
{
    m_widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_widget);

    m_outputBox = new QTextEdit(m_widget);
    m_outputBox->setReadOnly(true);

    m_inputLine = new QLineEdit(m_widget);
    m_sendButton = new QPushButton(tr("Send"), m_widget);
    m_historyList = new QListWidget(m_widget);

    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(m_inputLine);
    inputLayout->addWidget(m_sendButton);

    layout->addWidget(new QLabel(tr("Chat History:")));
    layout->addWidget(m_historyList);
    layout->addWidget(new QLabel(tr("Assistant Output:")));
    layout->addWidget(m_outputBox);
    layout->addLayout(inputLayout);

    connect(m_sendButton, &QPushButton::clicked, this, &DeepSeekNavigationChat::onSendClicked);

    return {displayName(), m_widget};
}

void DeepSeekNavigationChat::onSendClicked()
{
    const QString message = m_inputLine->text().trimmed();
    if (message.isEmpty()) return;

    appendToChatHistory("You", message);
    QJsonObject payload;
    payload["message"] = message;
    updateContextMetadata(payload);

    sendApiRequest("/chat", payload);
    m_inputLine->clear();
}

void DeepSeekNavigationChat::sendApiRequest(const QString &endpoint, const QJsonObject &payload)
{
    QNetworkRequest request(QUrl(m_apiUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_apiKey.isEmpty())
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleApiReply(reply); });
}

void DeepSeekNavigationChat::handleApiReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        appendToChatHistory("Error", reply->errorString());
        return;
    }
    const QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    const QString content = obj["content"].toString();
    const QString action = obj["action"].toString();
    const QString filePath = obj["file"].toString();
    const QString newContent = obj["new_content"].toString();

    appendToChatHistory("DeepSeek", content);

    if (action == "replace_current") {
        applyEditToCurrentFile(newContent);
    } else if (action == "edit" && !filePath.isEmpty()) {
        showPreviewDialog(filePath, newContent);
    }
}

void DeepSeekNavigationChat::applyEditToCurrentFile(const QString &text)
{
    if (auto *editor = Core::EditorManager::currentEditor())
        editor->setText(text);
}

void DeepSeekNavigationChat::applyEditToFile(const QString &filePath, const QString &content)
{
    QString error;
    if (!FileUtils::writeFile(filePath, content, error)) {
        QMessageBox::critical(m_widget, tr("Write Error"), error);
    }
}

void DeepSeekNavigationChat::showPreviewDialog(const QString &filePath, const QString &newContent)
{
    DeepSeekPreviewDialog dlg(filePath, newContent, m_widget);
    if (dlg.exec() == QDialog::Accepted && dlg.accepted()) {
        applyEditToFile(filePath, newContent);
    }
}

void DeepSeekNavigationChat::appendToChatHistory(const QString &sender, const QString &text)
{
    m_outputBox->append(QString("<b>%1:</b><br>%2<br>").arg(sender, text.toHtmlEscaped()));
    m_historyList->addItem(QString("%1: %2").arg(sender, text));
    saveConversationHistory(sender, text);
}

void DeepSeekNavigationChat::updateContextMetadata(QJsonObject &payload)
{
    if (auto *editor = Core::EditorManager::currentEditor()) {
        payload["filename"] = editor->document()->filePath().toString();
        payload["selection"] = editor->selectedText();
    }
    // TODO: project_path if needed
}

void DeepSeekNavigationChat::sendSourceAnalysisCommand(const QString &command)
{
    QJsonObject payload;
    payload["command"] = command;
    updateContextMetadata(payload);
    sendApiRequest("/analyze", payload);
}

void DeepSeekNavigationChat::loadConversationHistory()
{
    // TODO: implement
}

void DeepSeekNavigationChat::saveConversationHistory(const QString &message, const QString &response)
{
    Q_UNUSED(message)
    Q_UNUSED(response)
    // TODO: implement persistent history if needed
}
