#include "deepseeksettings.h"
#include <coreplugin/icore.h>

namespace DeepSeek {

DeepSeekSettings::DeepSeekSettings(QObject *parent)
    : QObject(parent),
      m_isValid(false),
      m_temperature(0.7),
      m_maxTokens(2048)
{
    initDefaults();
    load();
}

void DeepSeekSettings::initDefaults()
{
    QMutexLocker locker(&m_dataMutex);
    m_apiUrl = QUrl("https://api.deepseek.com/v1");
    m_model = "Without Model";
    m_systemPrompt = "You are a helpful AI assistant";
}

// Implementación de getters (thread-safe)
QString DeepSeekSettings::apiKey() const {
    QMutexLocker locker(&m_dataMutex);
    return m_apiKey;
}

QUrl DeepSeekSettings::apiUrl() const {
    QMutexLocker locker(&m_dataMutex);
    return m_apiUrl;
}

void DeepSeekSettings::setModel(const QString &model)
{
    {
        QMutexLocker locker(&m_dataMutex);
        if (m_model == model)
            return;
        m_model = model;
    }
    emit modelChanged();
    emit settingsChanged();
}

void DeepSeekSettings::setSystemPrompt(const QString &systemPrompt)
{
    {
        QMutexLocker locker(&m_dataMutex);
        if (m_systemPrompt == systemPrompt)
            return;
        m_systemPrompt = systemPrompt;
    }
    emit systemPromptChanged();
    emit settingsChanged();
}

void DeepSeekSettings::setTemperature(double temperature)
{
    {
        QMutexLocker locker(&m_dataMutex);
        if (qFuzzyCompare(m_temperature, temperature))
            return;
        m_temperature = temperature;
    }
    emit temperatureChanged();
    emit settingsChanged();
}

void DeepSeekSettings::setMaxTokens(int maxTokens)
{
    {
        QMutexLocker locker(&m_dataMutex);
        if (m_maxTokens == maxTokens)
            return;
        m_maxTokens = maxTokens;
    }
    emit maxTokensChanged();
    emit settingsChanged();
}

QString DeepSeekSettings::model() const {
    QMutexLocker locker(&m_dataMutex);
    return m_model;
}

QString DeepSeekSettings::systemPrompt() const {
    QMutexLocker locker(&m_dataMutex);
    return m_systemPrompt;
}

double DeepSeekSettings::temperature() const {
    QMutexLocker locker(&m_dataMutex);
    return m_temperature;
}

int DeepSeekSettings::maxTokens() const {
    QMutexLocker locker(&m_dataMutex);
    return m_maxTokens;
}

// Implementación de setters (thread-safe)
void DeepSeekSettings::setApiKey(const QString &apiKey) {
    {
        QMutexLocker locker(&m_dataMutex);
        if (m_apiKey == apiKey) return;
        m_apiKey = apiKey;
    }
    validateSettings();
    emit apiKeyChanged();
    emit settingsChanged();
}

void DeepSeekSettings::setApiUrl(const QUrl &apiUrl) {
    {
        QMutexLocker locker(&m_dataMutex);
        if (m_apiUrl == apiUrl) return;
        m_apiUrl = apiUrl;
    }
    validateSettings();
    emit apiUrlChanged();
    emit settingsChanged();
}

// ... (implementaciones similares para otros setters)

bool DeepSeekSettings::isValid() const {
    QMutexLocker locker(&m_dataMutex);
    return m_isValid;
}

QString DeepSeekSettings::validationError() const {
    QMutexLocker locker(&m_dataMutex);
    return m_validationError;
}

void DeepSeekSettings::validateSettings() {
    QMutexLocker locker(&m_dataMutex);
    
    m_isValid = true;
    m_validationError.clear();

    if (m_apiKey.isEmpty()) {
        m_isValid = false;
        m_validationError = tr("API Key is required");
        return;
    }

    if (!m_apiUrl.isValid()) {
        m_isValid = false;
        m_validationError = tr("Invalid API URL");
        return;
    }

    if (m_model.isEmpty()) {
        m_isValid = false;
        m_validationError = tr("Model is required");
        return;
    }
}

void DeepSeekSettings::load() {
    auto *settings = Core::ICore::settings();
    settings->beginGroup("DeepSeek");

    setApiKey(settings->value("ApiKey").toString());
    setApiUrl(settings->value("ApiUrl", m_apiUrl.toString()).toString());
    setModel(settings->value("Model", m_model).toString());
    setSystemPrompt(settings->value("SystemPrompt", m_systemPrompt).toString());
    setTemperature(settings->value("Temperature", m_temperature).toDouble());
    setMaxTokens(settings->value("MaxTokens", m_maxTokens).toInt());

    settings->endGroup();
    validateSettings();
}

void DeepSeekSettings::save() {
    auto settings = Core::ICore::settings();
    settings->beginGroup("DeepSeek");

    {
        QMutexLocker locker(&m_dataMutex);
        settings->setValue("ApiKey", m_apiKey);
        settings->setValue("ApiUrl", m_apiUrl.toString());
        settings->setValue("Model", m_model);
        settings->setValue("SystemPrompt", m_systemPrompt);
        settings->setValue("Temperature", m_temperature);
        settings->setValue("MaxTokens", m_maxTokens);
    }

    settings->endGroup();
    settings->sync();
    validateSettings();
    emit settingsChanged();
}

} // namespace DeepSeek
