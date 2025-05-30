#include "deepseekoptionspage.h"
#include <QSettings>

namespace DeepSeek {
namespace Internal {

DeepSeekOptionsPageWidget::DeepSeekOptionsPageWidget(QWidget *parent)
    : QWidget(parent){
    auto *layout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout;

    auto *apiKeyLabel = new QLabel(tr("API Key:"), this);
    apiKeyEdit = new QLineEdit(this);
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(apiKeyLabel, apiKeyEdit);

    auto *apiUrlLabel = new QLabel(tr("API URL:"), this);
    apiUrlEdit = new QLineEdit(this);
    formLayout->addRow(apiUrlLabel, apiUrlEdit);

    connectButton = new QPushButton(tr("Conectar y obtener modelos"), this);
    auto *connectLayout = new QHBoxLayout;
    connectLayout->addWidget(connectButton);
    connectLayout->addStretch();
    layout->addLayout(formLayout);
    layout->addLayout(connectLayout);

    auto *modelLabel = new QLabel(tr("Modelo:"), this);
    modelComboBox = new QComboBox(this);
    modelComboBox->setEditable(false);
    formLayout->addRow(modelLabel, modelComboBox);

    auto *modelDescLabel = new QLabel(tr("Descripción del modelo:"), this);
    modelDescriptionEdit = new QPlainTextEdit(this);
    modelDescriptionEdit->setReadOnly(true);
    modelDescriptionEdit->setMaximumHeight(80);
    formLayout->addRow(modelDescLabel, modelDescriptionEdit);

    auto *systemPromptLabel = new QLabel(tr("Prompt del Sistema:"), this);
    systemPromptLabel->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
    systemPromptEdit = new QPlainTextEdit(this);
    systemPromptEdit->setMinimumSize(QSize(0, 100));
    formLayout->addRow(systemPromptLabel, systemPromptEdit);

    auto *temperatureLabel = new QLabel(tr("Temperatura:"), this);
    temperatureSpinBox = new QDoubleSpinBox(this);
    temperatureSpinBox->setRange(0.01, 2.0);
    temperatureSpinBox->setSingleStep(0.1);
    temperatureSpinBox->setValue(0.7);
    formLayout->addRow(temperatureLabel, temperatureSpinBox);

    auto *maxTokensLabel = new QLabel(tr("Limite de Tokens:"), this);
    maxTokensSpinBox = new QSpinBox(this);
    maxTokensSpinBox->setRange(16, 32000);
    maxTokensSpinBox->setValue(2048);
    formLayout->addRow(maxTokensLabel, maxTokensSpinBox);

    layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    connect(connectButton, &QPushButton::clicked, this, &DeepSeekOptionsPageWidget::onConnectButtonClicked);
    connect(modelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeepSeekOptionsPageWidget::onModelSelectionChanged);

    modelComboBox->addItem("Without Model");
}

QString DeepSeekOptionsPageWidget::apiKey() const { return apiKeyEdit->text(); }
QString DeepSeekOptionsPageWidget::apiUrl() const { return apiUrlEdit->text(); }
QString DeepSeekOptionsPageWidget::model() const { return modelComboBox->currentText(); }
QString DeepSeekOptionsPageWidget::systemPrompt() const { return systemPromptEdit->toPlainText(); }
double DeepSeekOptionsPageWidget::temperature() const { return temperatureSpinBox->value(); }
int DeepSeekOptionsPageWidget::maxTokens() const { return maxTokensSpinBox->value(); }

// Setters para cargar configuraciones
void DeepSeekOptionsPageWidget::setApiKey(const QString &key) { apiKeyEdit->setText(key); }
void DeepSeekOptionsPageWidget::setApiUrl(const QString &url) { apiUrlEdit->setText(url); }
void DeepSeekOptionsPageWidget::setModel(const QString &model) {
    int index = modelComboBox->findText(model);
    if (index >= 0) {
        modelComboBox->setCurrentIndex(index);
    } else if (!model.isEmpty()) {
        modelComboBox->addItem(model);
        modelComboBox->setCurrentText(model);
    }
}
void DeepSeekOptionsPageWidget::setSystemPrompt(const QString &prompt) { systemPromptEdit->setPlainText(prompt); }
void DeepSeekOptionsPageWidget::setTemperature(double temp) { temperatureSpinBox->setValue(temp); }
void DeepSeekOptionsPageWidget::setMaxTokens(int tokens) { maxTokensSpinBox->setValue(tokens); }

// Slots para manejo de eventos
void DeepSeekOptionsPageWidget::onConnectButtonClicked(){
    connectButton->setEnabled(false);
    connectButton->setText(tr("Connecting..."));
    fetchModels();
}

void DeepSeekOptionsPageWidget::onModelSelectionChanged(int index){
    if (index < 0 || modelComboBox->count() <= 0) {
        modelDescriptionEdit->clear();
        return;
    }

    QString selectedModel = modelComboBox->currentText();
    modelDescriptionEdit->setPlainText(modelDescriptions.value(selectedModel, ""));
}

void DeepSeekOptionsPageWidget::fetchModels(){
    QString url = apiUrlEdit->text().trimmed();
    if (url.isEmpty()) { url = "https://api.deepseek.com"; }
    if (!url.endsWith("/")) { url += "/";  }
    if (!url.contains("/v1/")) { url += "v1/";  }

    QUrl apiUrl(url + "models");
    QNetworkRequest request(apiUrl);
    request.setRawHeader("Authorization", "Bearer " + apiKeyEdit->text().toUtf8());

    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
    }

    currentReply = networkManager.get(request);
    connect(currentReply, &QNetworkReply::finished, this, &DeepSeekOptionsPageWidget::handleNetworkReply);
}

QString DeepSeekOptionsPageWidget::getModelDescription(const QString &modelId){
    static QMap<QString, QString> descriptions = {
        {"deepseek-chat", "Modelo general para conversaciones y asistencia técnica."},
        {"deepseek-reasoner", "Optimizado para razonamiento lógico y matemático."}
    };
    return descriptions.value(modelId, "No description available.");
}

void DeepSeekOptionsPageWidget::handleNetworkReply(){
    connectButton->setEnabled(true);
    connectButton->setText(tr("Conectar y obtener modelos"));

    if (!currentReply){ return; }
    QString currentModel = modelComboBox->currentText();

    while (modelComboBox->count() > 1) {
        modelComboBox->removeItem(1);
    }
    modelDescriptions.clear();

    if (currentReply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(currentReply->readAll());
        if (doc.isObject() && doc.object().contains("data") && doc.object().value("data").isArray()) {
            QJsonArray models = doc.object().value("data").toArray();

            if (models.isEmpty()) {
                modelComboBox->setCurrentIndex(0); // "Without Model"
            } else {
                for (const auto &modelValue : models) {
                    if (modelValue.isObject()) {
                        QJsonObject modelObj = modelValue.toObject();
                        QString modelId = modelObj["id"].toString();
                        QString modelDesc = getModelDescription(modelId);
                        modelDescriptions[modelId] = modelDesc;

                        if (!modelId.isEmpty()) {
                            modelComboBox->addItem(modelId);
                        }
                    }
                }

                int index = modelComboBox->findText(currentModel);
                if (index >= 0) {
                    modelComboBox->setCurrentIndex(index);
                } else {
                    modelComboBox->setCurrentIndex(1); // Primer modelo válido
                }
            }
        } else {
            modelComboBox->setCurrentIndex(0); // "Without Model"
        }
    } else {
        // Error de red
        QMessageBox::warning(this, tr("Error de conexión"),
                             tr("No se pudo obtener el modelo: %1").arg(currentReply->errorString()));
        modelComboBox->setCurrentIndex(0); // "Without Model"
    }
    onModelSelectionChanged(modelComboBox->currentIndex());
    currentReply->deleteLater();
    currentReply = nullptr;
}


DeepSeekOptionsPage::DeepSeekOptionsPage( QObject *parent):
    Core::IOptionsPage(), QObject(parent), m_widget(nullptr){
    setId("DeepSeek.Settings");
    setDisplayName(tr("DeepSeek"));
    // Set default category
    setCategory(Utils::Id("Z.DeepSeek"));
    // Registrar la categoría con su icono
    Core::IOptionsPage::registerCategory("Z.DeepSeek", tr("DeepSeek"),
                                         Utils::FilePath(":/images/deepseek.png"));
}

DeepSeekOptionsPage::~DeepSeekOptionsPage(){}

void DeepSeekOptionsPage::loadSettings(){
    auto settings = DSS::inst();
    m_widget->setApiKey(settings->apiKey());
    m_widget->setApiUrl(settings->apiUrl().toString());
    m_widget->setModel(settings->model());
    m_widget->setSystemPrompt(settings->systemPrompt());
    m_widget->setTemperature(settings->temperature());
    m_widget->setMaxTokens(settings->maxTokens());
}

QWidget *DeepSeekOptionsPage::widget(){
    if (!m_widget) {
        m_widget = new DeepSeekOptionsPageWidget();
        loadSettings();
    }
    return qobject_cast<QWidget*>(m_widget);
}

void DeepSeekOptionsPage::apply(){
    if (!m_widget){ return; } // page was never shown
    auto settings = DSS::inst();
    // QMessageBox::information(nullptr, "Debug", "1");
    settings->setApiKey(m_widget->apiKey());
    settings->setApiUrl(QUrl(m_widget->apiUrl()));
    settings->setModel(m_widget->model());
    settings->setSystemPrompt(m_widget->systemPrompt());
    settings->setTemperature(m_widget->temperature());
    settings->setMaxTokens(m_widget->maxTokens());
    settings->save();
}

void DeepSeekOptionsPage::finish(){
    delete m_widget;
    m_widget = nullptr;
}


} // namespace Internal
} // namespace DeepSeek
