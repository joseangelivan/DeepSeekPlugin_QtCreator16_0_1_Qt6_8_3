#pragma once

#include <QObject>
#include <QSettings>
#include <QUrl>
#include "singleton.h"

namespace DeepSeek {

class DeepSeekSettings : public QObject
{
    Q_OBJECT
    friend class Singleton<DeepSeekSettings>;

public:
    // Getters (const para thread-safety)
    QString apiKey() const;
    QUrl apiUrl() const;
    QString model() const;
    QString systemPrompt() const;
    double temperature() const;
    int maxTokens() const;

    // Setters con mutex interno
    void setApiKey(const QString &apiKey);
    void setApiUrl(const QUrl &apiUrl);
    void setModel(const QString &model);
    void setSystemPrompt(const QString &systemPrompt);
    void setTemperature(double temperature);
    void setMaxTokens(int maxTokens);

    // Validación (const para thread-safety)
    bool isValid() const;
    QString validationError() const;

    // Cargar/Guardar con mutex interno
    void load();
    void save();

signals:
    // Las señales son thread-safe en Qt
    void settingsChanged();
    void apiKeyChanged();
    void apiUrlChanged();
    void modelChanged();
    void systemPromptChanged();
    void temperatureChanged();
    void maxTokensChanged();

protected:
    explicit DeepSeekSettings(QObject *parent = nullptr);
    ~DeepSeekSettings() override = default;

private:
    void initDefaults();
    void validateSettings();

    // Datos protegidos por mutex
    mutable QMutex m_dataMutex;
    QString m_apiKey;
    QUrl m_apiUrl;
    QString m_model;
    QString m_systemPrompt;
    double m_temperature;
    int m_maxTokens;

    // Estado de validación
    bool m_isValid;
    QString m_validationError;
};

// Alias para el Singleton thread-safe
typedef Singleton<DeepSeekSettings> DSS;

} // namespace DeepSeek