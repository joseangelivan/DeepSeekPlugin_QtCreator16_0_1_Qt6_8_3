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

#include <QPointer>
#include <QSettings>

namespace DeepSeek {
namespace Internal {

// class DeepSeekSettings;



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

    // Nuevos setters para cargar configuraciones
    void setApiKey(const QString &key);
    void setApiUrl(const QString &url);
    void setModel(const QString &model);
    void setSystemPrompt(const QString &prompt);
    void setTemperature(double temp);
    void setMaxTokens(int tokens);

private:
    QLineEdit *apiKeyEdit;
    QLineEdit *apiUrlEdit;
    QLineEdit *modelEdit;
    QPlainTextEdit *systemPromptEdit;
    QDoubleSpinBox *temperatureSpinBox;
    QSpinBox *maxTokensSpinBox;
};



// namespace Ui {
// class DeepSeekOptionsPage;
// }

// Configuration page for the DeepSeek plugin
class DeepSeekOptionsPage : public QObject, public Core::IOptionsPage
{
     Q_OBJECT
public:
     explicit DeepSeekOptionsPage(/*QObject *parent = nullptr*/);
    ~DeepSeekOptionsPage() override;

    QWidget *widget() override;
    void apply() override;
    void finish() override;


    // Methods to handle category settings
    // void setCategory(const Utils::Id &category);
    // void setDisplayCategory(const QString &displayCategory);
    // void setCategoryIconPath(const Utils::FilePath &iconPath);

private:
    // DeepSeekSettings *m_settings;
    QPointer<DeepSeekOptionsPageWidget> m_widget;
    // Ui::DeepSeekOptionsPage *ui;
    // Utils::Id m_category;
    // QString m_displayCategory;
    Utils::FilePath m_categoryIconPath;

    // Método para cargar configuraciones
    void loadSettings();
};

} // namespace Internal
} // namespace DeepSeek
