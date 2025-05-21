#include "deepseekoptionspage.h"
// #include "ui_deepseekoptionspage.h"
// #include "deepseeksettings.h"
// #include "deepseektr.h"



#include <QSettings>

namespace DeepSeek {
namespace Internal {


// DeepSeekOptionsPageWidget::DeepSeekOptionsPageWidget(QWidget *parent)
//     : QWidget(parent)
// {
//     // Crear los widgets
//     auto *layout = new QVBoxLayout(this);
//     auto *formLayout = new QFormLayout;

//     auto *apiKeyLabel = new QLabel(tr("API Key:"), this);
//     apiKeyEdit = new QLineEdit(this);
//     apiKeyEdit->setEchoMode(QLineEdit::Password);
//     formLayout->addRow(apiKeyLabel, apiKeyEdit);

//     auto *apiUrlLabel = new QLabel(tr("API URL:"), this);
//     apiUrlEdit = new QLineEdit(this);
//     formLayout->addRow(apiUrlLabel, apiUrlEdit);

//     auto *modelLabel = new QLabel(tr("Model:"), this);
//     modelEdit = new QLineEdit(this);
//     formLayout->addRow(modelLabel, modelEdit);

//     auto *systemPromptLabel = new QLabel(tr("System Prompt:"), this);
//     systemPromptLabel->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
//     systemPromptEdit = new QPlainTextEdit(this);
//     systemPromptEdit->setMinimumSize(QSize(0, 100));
//     formLayout->addRow(systemPromptLabel, systemPromptEdit);

//     auto *temperatureLabel = new QLabel(tr("Temperature:"), this);
//     temperatureSpinBox = new QDoubleSpinBox(this);
//     temperatureSpinBox->setRange(0.01, 2.0);
//     temperatureSpinBox->setSingleStep(0.1);
//     temperatureSpinBox->setValue(0.7);
//     formLayout->addRow(temperatureLabel, temperatureSpinBox);

//     auto *maxTokensLabel = new QLabel(tr("Max Tokens:"), this);
//     maxTokensSpinBox = new QSpinBox(this);
//     maxTokensSpinBox->setRange(16, 16384);
//     maxTokensSpinBox->setValue(2048);
//     formLayout->addRow(maxTokensLabel, maxTokensSpinBox);

//     layout->addLayout(formLayout);
//     layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
// }

// // Métodos para obtener valores si se requieren
// QString DeepSeekOptionsPageWidget::apiKey() const { return apiKeyEdit->text(); }
// QString DeepSeekOptionsPageWidget::apiUrl() const { return apiUrlEdit->text(); }
// QString DeepSeekOptionsPageWidget::model() const { return modelEdit->text(); }
// QString DeepSeekOptionsPageWidget::systemPrompt() const { return systemPromptEdit->toPlainText(); }
// double DeepSeekOptionsPageWidget::temperature() const { return temperatureSpinBox->value(); }
// int DeepSeekOptionsPageWidget::maxTokens() const { return maxTokensSpinBox->value(); }

// void DeepSeekOptionsPageWidget::setApiKey(const QString &key) { apiKeyEdit->setText(key); }
// void DeepSeekOptionsPageWidget::setApiUrl(const QString &url) { apiUrlEdit->setText(url); }
// void DeepSeekOptionsPageWidget::setModel(const QString &model) { modelEdit->setText(model); }
// void DeepSeekOptionsPageWidget::setSystemPrompt(const QString &prompt) { systemPromptEdit->setPlainText(prompt); }
// void DeepSeekOptionsPageWidget::setTemperature(double temp) { temperatureSpinBox->setValue(temp); }
// void DeepSeekOptionsPageWidget::setMaxTokens(int tokens) { maxTokensSpinBox->setValue(tokens); }



DeepSeekOptionsPageWidget::DeepSeekOptionsPageWidget(QWidget *parent)
    : QWidget(parent)
{
    // Crear los widgets
    auto *layout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout;

    // Campos para API Key y URL
    auto *apiKeyLabel = new QLabel(tr("API Key:"), this);
    apiKeyEdit = new QLineEdit(this);
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(apiKeyLabel, apiKeyEdit);

    auto *apiUrlLabel = new QLabel(tr("API URL:"), this);
    apiUrlEdit = new QLineEdit(this);
    formLayout->addRow(apiUrlLabel, apiUrlEdit);

    // Botón de conexión
    connectButton = new QPushButton(tr("Connect"), this);
    auto *connectLayout = new QHBoxLayout;
    connectLayout->addWidget(connectButton);
    connectLayout->addStretch();
    layout->addLayout(formLayout);
    layout->addLayout(connectLayout);

    // Combobox para modelos
    auto *modelLabel = new QLabel(tr("Model:"), this);
    modelComboBox = new QComboBox(this);
    modelComboBox->setEditable(false);
    formLayout->addRow(modelLabel, modelComboBox);

    // Campo para descripción del modelo
    auto *modelDescLabel = new QLabel(tr("Model Description:"), this);
    modelDescriptionEdit = new QPlainTextEdit(this);
    modelDescriptionEdit->setReadOnly(true);
    modelDescriptionEdit->setMaximumHeight(80);
    formLayout->addRow(modelDescLabel, modelDescriptionEdit);

    // Campos existentes para system prompt, temperatura y max tokens
    auto *systemPromptLabel = new QLabel(tr("System Prompt:"), this);
    systemPromptLabel->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
    systemPromptEdit = new QPlainTextEdit(this);
    systemPromptEdit->setMinimumSize(QSize(0, 100));
    formLayout->addRow(systemPromptLabel, systemPromptEdit);

    auto *temperatureLabel = new QLabel(tr("Temperature:"), this);
    temperatureSpinBox = new QDoubleSpinBox(this);
    temperatureSpinBox->setRange(0.01, 2.0);
    temperatureSpinBox->setSingleStep(0.1);
    temperatureSpinBox->setValue(0.7);
    formLayout->addRow(temperatureLabel, temperatureSpinBox);

    auto *maxTokensLabel = new QLabel(tr("Max Tokens:"), this);
    maxTokensSpinBox = new QSpinBox(this);
    maxTokensSpinBox->setRange(16, 16384);
    maxTokensSpinBox->setValue(2048);
    formLayout->addRow(maxTokensLabel, maxTokensSpinBox);

    // Añadir espacio final
    layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // Conectar señales
    connect(connectButton, &QPushButton::clicked, this, &DeepSeekOptionsPageWidget::onConnectButtonClicked);
    connect(modelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeepSeekOptionsPageWidget::onModelSelectionChanged);

    // Inicialización de valores
    modelComboBox->addItem("Without Model");
}

// Getters existentes
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
void DeepSeekOptionsPageWidget::onConnectButtonClicked()
{
    // Desactivar botón durante la operación
    connectButton->setEnabled(false);
    connectButton->setText(tr("Connecting..."));

    // Llamar a método para obtener modelos
    fetchModels();
}

void DeepSeekOptionsPageWidget::onModelSelectionChanged(int index)
{
    if (index < 0 || modelComboBox->count() <= 0) {
        modelDescriptionEdit->clear();
        return;
    }

    QString selectedModel = modelComboBox->currentText();
    modelDescriptionEdit->setPlainText(modelDescriptions.value(selectedModel, ""));
}

void DeepSeekOptionsPageWidget::fetchModels()
{
    QString url = apiUrlEdit->text().trimmed();
    if (url.isEmpty()) {
        url = "https://api.deepseek.com";
    }

    if (!url.endsWith("/")) {
        url += "/";
    }

    if (!url.contains("/v1/")) {
        url += "v1/";
    }

    QUrl apiUrl(url + "models");
    QNetworkRequest request(apiUrl);

    // Añadir encabezado de autenticación
    request.setRawHeader("Authorization", "Bearer " + apiKeyEdit->text().toUtf8());

    // Realizar la solicitud
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
    }

    currentReply = networkManager.get(request);
    connect(currentReply, &QNetworkReply::finished, this, &DeepSeekOptionsPageWidget::handleNetworkReply);
}

QString DeepSeekOptionsPageWidget::getModelDescription(const QString &modelId) {
    static QMap<QString, QString> descriptions = {
        {"deepseek-chat", "Modelo general para conversaciones y asistencia técnica."},
        {"deepseek-reasoner", "Optimizado para razonamiento lógico y matemático."}
    };
    return descriptions.value(modelId, "No description available.");
}

void DeepSeekOptionsPageWidget::handleNetworkReply()
{
    // Restablecer botón
    connectButton->setEnabled(true);
    connectButton->setText(tr("Connect"));

    if (!currentReply) {
        return;
    }

    // Guardar el modelo actual
    QString currentModel = modelComboBox->currentText();

    // Limpiar combobox excepto "Without Model"
    while (modelComboBox->count() > 1) {
        modelComboBox->removeItem(1);
    }

    // Limpiar descripciones
    modelDescriptions.clear();

    // Verificar respuesta
    if (currentReply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(currentReply->readAll());
        if (doc.isObject() && doc.object().contains("data") && doc.object().value("data").isArray()) {
            QJsonArray models = doc.object().value("data").toArray();

            if (models.isEmpty()) {
                // No hay modelos disponibles
                modelComboBox->setCurrentIndex(0); // "Without Model"
            } else {
                // Añadir modelos al combobox
                for (const auto &modelValue : models) {
                    if (modelValue.isObject()) {
                        QJsonObject modelObj = modelValue.toObject();
                        QString modelId = modelObj["id"].toString();
                        // QString modelDesc = modelObj["description"].toString();
                        QString modelDesc = getModelDescription(modelId);
                        modelDescriptions[modelId] = modelDesc;

                        if (!modelId.isEmpty()) {
                            modelComboBox->addItem(modelId);
                            // modelDescriptions[modelId] = modelDesc;
                            // QString modelDesc = getModelDescription(modelId);
                            // modelDescriptions[modelId] = modelDesc;
                        }
                    }
                }

                // Intentar seleccionar el modelo anteriormente seleccionado
                int index = modelComboBox->findText(currentModel);
                if (index >= 0) {
                    modelComboBox->setCurrentIndex(index);
                } else {
                    modelComboBox->setCurrentIndex(1); // Primer modelo válido
                }
            }
        } else {
            // Formato de respuesta inválido
            modelComboBox->setCurrentIndex(0); // "Without Model"
        }
    } else {
        // Error de red
        QMessageBox::warning(this, tr("Connection Error"),
                             tr("Failed to fetch models: %1").arg(currentReply->errorString()));
        modelComboBox->setCurrentIndex(0); // "Without Model"
    }

    // Actualizar descripción del modelo
    onModelSelectionChanged(modelComboBox->currentIndex());

    // Eliminar respuesta
    currentReply->deleteLater();
    currentReply = nullptr;
}


DeepSeekOptionsPage::DeepSeekOptionsPage(/*DeepSeekSettings *settings,*/ /*QObject *parent*/):
    Core::IOptionsPage(), // Changed from IOptionsPageProvider to IOptionsPage
    QObject(),
      // m_settings(settings),

      m_widget(nullptr)/*,
      ui(nullptr)*/
{

    // Registrar la categoría con su icono
    Core::IOptionsPage::registerCategory("Z.DeepSeek",
                                         tr("DeepSeek"),
                                         Utils::FilePath(":/images/deepseek.png"));

    setId("DeepSeek.Settings");
    setDisplayName(tr("DeepSeek"));




    // Set default category
    setCategory(Utils::Id("Z.DeepSeek"));

    // setCategoryIconPath(":/deepseekplugin/icon.png");

    // setDisplayCategory(QObject::tr("DeepSeekr"));




    // Store parent for safe deletions
    // setParent(parent);

    setId("DeepSeek.Settings");
    setDisplayName(tr("DeepSeek"));
    setCategory(Utils::Id("Z.DeepSeek"));

}

DeepSeekOptionsPage::~DeepSeekOptionsPage()
{
    // delete ui;
}

// Método para cargar configuraciones
void DeepSeekOptionsPage::loadSettings()
{
    if (!m_widget)
        return;

    auto settings = Core::ICore::settings();
    settings->beginGroup("DeepSeek");

    m_widget->setApiKey(settings->value("ApiKey", "").toString());
    m_widget->setApiUrl(settings->value("ApiUrl", "https://api.deepseek.com/v1").toString());
    m_widget->setModel(settings->value("Model", "deepseek-chat").toString());
    m_widget->setSystemPrompt(settings->value("SystemPrompt", "You are a helpful AI assistant").toString());
    m_widget->setTemperature(settings->value("Temperature", 0.7).toDouble());
    m_widget->setMaxTokens(settings->value("MaxTokens", 2048).toInt());

    settings->endGroup();
}


QWidget *DeepSeekOptionsPage::widget()
{
    if (!m_widget) {
         m_widget = new DeepSeekOptionsPageWidget;

        // Cargar configuraciones guardadas inmediatamente después de crear el widget
        loadSettings();
    }
    return qobject_cast<QWidget*>(m_widget);
}

void DeepSeekOptionsPage::apply()
{
    if (!m_widget) // page was never shown
        return;

    // m_settings->setApiKey(ui->apiKeyEdit->text());
    // m_settings->setApiUrl(ui->apiUrlEdit->text());
    // m_settings->setModel(ui->modelEdit->text());
    // m_settings->setSystemPrompt(ui->systemPromptEdit->toPlainText());
    // m_settings->setTemperature(ui->temperatureSpinBox->value());
    // m_settings->setMaxTokens(ui->maxTokensSpinBox->value());

    // // Use the global QSettings instance provided by Qt Creator
    // m_settings->saveSettings(Core::ICore::settings());

    auto settings = Core::ICore::settings();
    settings->beginGroup("DeepSeek");
    settings->setValue("ApiKey", m_widget->apiKey());
    settings->setValue("ApiUrl", m_widget->apiUrl());
    settings->setValue("Model", m_widget->model());
    settings->setValue("SystemPrompt", m_widget->systemPrompt());
    settings->setValue("Temperature", m_widget->temperature());
    settings->setValue("MaxTokens", m_widget->maxTokens());
    settings->endGroup();
}

void DeepSeekOptionsPage::finish()
{
    delete m_widget;
    m_widget = nullptr;
    // delete ui;
    // ui = nullptr;
}

// // Implementation of category setting methods - simplified to directly set fields
// void DeepSeekOptionsPage::setCategory(const Utils::Id &category)
// {
//     m_category = category;
// }

// void DeepSeekOptionsPage::setDisplayCategory(const QString &displayCategory)
// {
//     m_displayCategory = displayCategory;
// }

// void DeepSeekOptionsPage::setCategoryIconPath(const Utils::FilePath &iconPath)
// {
//     m_categoryIconPath = iconPath;
// }

} // namespace Internal
} // namespace DeepSeek
