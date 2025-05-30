#include "deepseeknavigationchat.h"

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
DeepSeekNavigationChat::DeepSeekNavigationChat(): Core::INavigationWidgetFactory()
{
    m_networkManager = new QNetworkAccessManager(this);
    setDisplayName("DeepSeek Chat");
    setPriority(100);
    setId("DeepSeek.Chat");

    connect(DSS::inst(), &DeepSeekSettings::settingsChanged,
            this, &DeepSeekNavigationChat::onSettingsChanged);

    loadConversationHistory();
}

DeepSeekNavigationChat::~DeepSeekNavigationChat() {}

void DeepSeekNavigationChat::onSettingsChanged(){
    qDebug() << "DeepSeek settings changed";
}


Core::NavigationView DeepSeekNavigationChat::createWidget()
{
    m_widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_widget);

    m_outputBox = new QTextEdit(m_widget);
    m_outputBox->setReadOnly(true);
    m_outputBox->setAcceptRichText(true);

    m_inputLine = new QLineEdit(m_widget);
    m_sendButton = new QPushButton(tr("Send"), m_widget);
    m_historyList = new QListWidget(m_widget);
    m_historyList->setMaximumHeight(150);

    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(m_inputLine);
    inputLayout->addWidget(m_sendButton);

    layout->addWidget(new QLabel(tr("Chat History:")));
    layout->addWidget(m_historyList);
    layout->addWidget(new QLabel(tr("Assistant Output:")));
    layout->addWidget(m_outputBox);
    layout->addLayout(inputLayout);

    connect(m_sendButton, &QPushButton::clicked, this, &DeepSeekNavigationChat::onSendClicked);
    connect(m_inputLine, &QLineEdit::returnPressed, m_sendButton, &QPushButton::click);

    return {m_widget, {}};
}

void DeepSeekNavigationChat::onSendClicked(){
    const QString message = m_inputLine->text().trimmed();
    if (message.isEmpty()) return;

    auto settings = DSS::inst();
    if (!settings->isValid()) {
        appendToChatHistory("Error", settings->validationError());
        return;
    }
    appendToChatHistory("You", message);

    QJsonObject payload;
    payload["message"] = message;
    payload["model"] = settings->model();
    payload["system_prompt"] = settings->systemPrompt();
    payload["temperature"] = settings->temperature();
    payload["max_tokens"] = settings->maxTokens();

    updateContextMetadata(payload);

    sendApiRequest("/chat", payload);
    m_inputLine->clear();
}

void DeepSeekNavigationChat::sendApiRequest(const QString &endpoint, const QJsonObject &payload){
    auto settings = DSS::inst();

    QUrl apiUrl(settings->apiUrl());

    if (!apiUrl.path().endsWith("/v1")){ apiUrl.setPath("/v1"); }

    QString fullPath = apiUrl.path() + "/chat/completions";
    apiUrl.setPath(fullPath);

    if (!apiUrl.isValid()) {
        appendToChatHistory("Error", tr("Invalid API URL: %1").arg(apiUrl.toString()));
        return;
    }

    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!settings->apiKey().isEmpty()) {
        request.setRawHeader("Authorization",
                             QString("Bearer %1").arg(settings->apiKey()).toUtf8());
    }

    QJsonObject fullPayload;
    fullPayload["model"] = settings->model();

    QJsonArray messagesArray;
    if (!settings->systemPrompt().isEmpty()) {
        messagesArray.append(QJsonObject{
            {"role", "system"},
            {"content", settings->systemPrompt()}
        });
    }

    for (const auto &item : qAsConst(m_conversationHistory) ) {
        if (item.isObject()) {
            QJsonObject obj = item.toObject();
            messagesArray.append(QJsonObject{
                {"role", obj.contains("response") ? "assistant" : "user"},
                {"content", obj.contains("response") ? obj["response"].toString() : obj["message"].toString()}
            });
        }
    }

    messagesArray.append(QJsonObject{
        {"role", "user"},
        {"content", payload["message"].toString()}
    });

    fullPayload["messages"] = messagesArray;
    fullPayload["temperature"] = settings->temperature();
    fullPayload["max_tokens"] = settings->maxTokens();

    QNetworkReply *reply = m_networkManager->post(
        request,
        QJsonDocument(fullPayload).toJson()
        );

    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() { handleApiReply(reply); });

    QTimer::singleShot(30000, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });
}

void DeepSeekNavigationChat::handleApiReply(QNetworkReply *reply){
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg;

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            errorMsg = tr("Request timed out");
        } else {
            errorMsg = tr("HTTP %1: %2").arg(
                                            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
                           .arg(reply->errorString());

            // Leer respuesta del servidor para más detalles
            QByteArray response = reply->readAll();
            if (!response.isEmpty()) {
                errorMsg += "\n" + tr("Server response: %1").arg(QString::fromUtf8(response));
            }
        }

        appendToChatHistory("Error", errorMsg);
        return;
    }

    // Procesar respuesta exitosa
    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        appendToChatHistory("Error",
                            tr("Invalid JSON response: %1").arg(parseError.errorString()));
        return;
    }

    if (!doc.isObject()) {
        appendToChatHistory("Error", tr("Unexpected response format"));
        return;
    }

    QJsonObject responseObj = doc.object();

    // Procesar respuesta según formato esperado de DeepSeek API
    if (responseObj.contains("choices") && responseObj["choices"].isArray()) {
        QJsonArray choices = responseObj["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject firstChoice = choices.first().toObject();
            if (firstChoice.contains("message") &&
                firstChoice["message"].isObject()) {
                QJsonObject message = firstChoice["message"].toObject();
                QString content = message["content"].toString();
                appendToChatHistory("DeepSeek", content);
                saveConversationHistory(m_inputLine->text(), content);
                return;
            }
        }
    }

    // Si no coincide con el formato esperado, mostrar la respuesta completa
    appendToChatHistory("Debug", tr("Full API response: %1")
                                     .arg(QString::fromUtf8(responseData)));
}

void DeepSeekNavigationChat::applyEditToCurrentFile(const QString &text)
{
    if (Core::IEditor *editor = Core::EditorManager::currentEditor()) {
        if (Core::IDocument *document = editor->document()) {
            if (auto *textEditor = qobject_cast<TextEditor::TextEditorWidget*>(editor->widget())) {
                textEditor->setPlainText(text);
            } else {
                handleGenericEditor(document, text);
            }
        }
    }
}

void DeepSeekNavigationChat::handleGenericEditor(Core::IDocument *document, const QString &text)
{
    const Utils::FilePath path = document->filePath();
    if (path.isEmpty() || !path.isWritableFile()) {
        qWarning() << "Archivo no escribible:" << (path.isEmpty() ? "(vacío)" : path.toUserOutput());
        return;
    }

    Utils::FileSaver saver(path, QIODevice::Text);
    saver.write(text.toUtf8());
    if (!saver.finalize()) {
        qWarning() << "Error al guardar:" << saver.errorString();
        return;
    }

    document->reload(
        document->isModified() ? Core::IDocument::ReloadFlag::FlagReload
                               : Core::IDocument::ReloadFlag::FlagIgnore,
        Core::IDocument::ChangeType::TypeContents
        );
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
    QString formattedText = text.toHtmlEscaped().replace('\n', "<br>");
    m_outputBox->append(QString("<b>%1:</b><br>%2<br>").arg(sender, formattedText));

    QListWidgetItem *item = new QListWidgetItem(QString("%1: %2").arg(sender, text.left(50)));
    if (sender == "You") {
        item->setForeground(Qt::blue);
    } else if (sender == "DeepSeek") {
        item->setForeground(Qt::darkGreen);
    }
    m_historyList->addItem(item);
    m_historyList->scrollToBottom();
}

void DeepSeekNavigationChat::updateContextMetadata(QJsonObject &payload)
{
    payload["filename"] = "";
    payload["selection"] = "";
    payload["project"] = "";

    if (Core::IEditor *editor = Core::EditorManager::currentEditor()) {
        const Utils::FilePath filePath = editor->document()->filePath();
        payload["filename"] = filePath.toFSPathString();

        if (auto *textEditor = qobject_cast<TextEditor::TextEditorWidget*>(editor->widget())) {
            payload["selection"] = textEditor->selectedText();
        }

        if (auto *project = ProjectExplorer::ProjectManager::projectForFile(filePath)) {
            payload["project"] = project->displayName();

        }
    }
}

Utils::FilePath DeepSeekNavigationChat::getHistoryFilePath() const
{
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return Utils::FilePath::fromString(configDir + "/deepseek_conversation_history.json");
}

void DeepSeekNavigationChat::loadConversationHistory()
{
    const Utils::FilePath historyFile = getHistoryFilePath();
    if (!historyFile.exists()) {
        return;
    }

    Utils::FileReader reader;
    QString errorString;
    if (!reader.fetch(historyFile, &errorString)) {  // Usamos la versión con errorString
        qWarning() << "Failed to load history:" << errorString;
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(reader.data(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Invalid JSON in history file at offset" << parseError.offset
                   << ":" << parseError.errorString();
        return;
    }

    if (!doc.isArray()) {
        qWarning() << "History file should contain a JSON array";
        return;
    }

    m_conversationHistory = doc.array();
}

void DeepSeekNavigationChat::saveConversationHistory(const QString &message, const QString &response)
{
    QJsonObject entry;
    entry["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    entry["message"] = message;
    entry["response"] = response;
    entry["context"] = getCurrentContext();

    m_conversationHistory.append(entry);

    // Keep last 100 conversations
    const int maxHistory = 100;
    while (m_conversationHistory.size() > maxHistory) {
        m_conversationHistory.removeFirst();
    }

    // Save to file
    const Utils::FilePath historyFile = getHistoryFilePath();
    historyFile.parentDir().ensureWritableDir();

    Utils::FileSaver saver(getHistoryFilePath(), QIODevice::Text);
    saver.write(QJsonDocument(m_conversationHistory).toJson());
    if (!saver.finalize()) {
        qWarning() << "Failed to save history:" << saver.errorString();
    }
}

QJsonObject DeepSeekNavigationChat::getCurrentContext() const
{
    QJsonObject context;
    if (auto *editor = Core::EditorManager::currentEditor()) {
        const Utils::FilePath filePath = editor->document()->filePath();
        context["file"] = filePath.toFSPathString();

        if (auto *project = ProjectExplorer::ProjectManager::projectForFile(filePath)) {
            context["project"] = project->displayName();
        }
    }
    return context;
}

void DeepSeekNavigationChat::sendSourceAnalysisCommand(const QString &command)
{
    QJsonObject payload;
    payload["command"] = command;
    updateContextMetadata(payload);
    sendApiRequest("/analyze", payload);
}
