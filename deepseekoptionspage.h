#pragma once

#include <coreplugin/icore.h>
#include <coreplugin/dialogs/ioptionspage.h>

#include <utils/id.h>
#include <utils/filepath.h>

#include <extensionsystem/iplugin.h>
#include <utils/layoutbuilder.h>
#include <utils/icon.h> // Necesario para Utils::Icon

#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QPushButton>
#include <QComboBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPointer>
#include <QSettings>
#include <QMessageBox>
#include "deepseeksettings.h"

namespace DeepSeek {
namespace Internal {

class DeepSeekOptionsPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeepSeekOptionsPageWidget(QWidget *parent = nullptr);
    QString apiKey() const;
    QString apiUrl() const;
    QString model() const;
    QString systemPrompt() const;
    double temperature() const;
    int maxTokens() const;

    // Setters para cargar configuraciones
    void setApiKey(const QString &key);
    void setApiUrl(const QString &url);
    void setModel(const QString &model);
    void setSystemPrompt(const QString &prompt);
    void setTemperature(double temp);
    void setMaxTokens(int tokens);

private slots:
    void onConnectButtonClicked();
    void onModelSelectionChanged(int index);
    void handleNetworkReply();

private:
    void fetchModels();

    // Widgets
    QLineEdit *apiKeyEdit;
    QLineEdit *apiUrlEdit;
    QComboBox *modelComboBox;
    QPlainTextEdit *modelDescriptionEdit;
    QPlainTextEdit *systemPromptEdit;
    QDoubleSpinBox *temperatureSpinBox;
    QSpinBox *maxTokensSpinBox;
    QPushButton *connectButton;

    QNetworkAccessManager networkManager;
    QPointer<QNetworkReply> currentReply;
    QMap<QString, QString> modelDescriptions;

    QString getModelDescription(const QString &modelId);
};

// Configuration page for the DeepSeek plugin
class DeepSeekOptionsPage : public QObject, public Core::IOptionsPage
{
     Q_OBJECT
public:
     explicit DeepSeekOptionsPage(QObject *parent = nullptr);
    ~DeepSeekOptionsPage() override;

    QWidget *widget() override;
    void apply() override;
    void finish() override;

private:
    QPointer<DeepSeekOptionsPageWidget> m_widget;
    void loadSettings();
};

} // namespace Internal
} // namespace DeepSeek
