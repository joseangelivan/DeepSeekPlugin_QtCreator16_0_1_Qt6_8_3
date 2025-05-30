#include "deepseekplugin_qtcreator16_0_1_qt6_8_3constants.h"
#include "deepseekplugin_qtcreator16_0_1_qt6_8_3tr.h"


// #include "ui_deepseekoptionspage.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icontext.h>
#include <coreplugin/icore.h>

#include <extensionsystem/pluginmanager.h> // << Necesario

#include <extensionsystem/iplugin.h>

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>

#include "deepseekoptionspage.h"
#include "deepseeknavigationchat.h"
#include "deepseeksettings.h"

using namespace Core;

namespace DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3::Internal {

class DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3Plugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE
                          "DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3.json")

public:
    DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3Plugin() = default;
    ~DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3Plugin() final
    {
        // Unregister objects from the plugin manager's object pool
        // Other cleanup, if needed.
    }

    void initialize() final
    {
        // Set up this plugin's factories, if needed.
        // Register objects in the plugin manager's object pool, if needed. (rare)
        // Load settings
        // Add actions to menus
        // Connect to other plugins' signals
        // In the initialize function, a plugin can be sure that the plugins it
        // depends on have passed their initialize() phase.

        // If you need access to command line arguments or to report errors, use the
        //    bool IPlugin::initialize(const QStringList &arguments, QString *errorString)
        // overload.

        // ActionContainer *menu = ActionManager::createMenu(Constants::MENU_ID);
        // menu->menu()->setTitle(Tr::tr("DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3"));
        // ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

        // ActionBuilder(this, Constants::ACTION_ID)
        //     .addToContainer(Constants::MENU_ID)
        //     .setText(Tr::tr("DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3 Action"))
        //     .setDefaultKeySequence(Tr::tr("Ctrl+Alt+Meta+A"))
        //     .addOnTriggered(this, &DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3Plugin::triggerAction);

        DeepSeek::DSS::inst();

        // Crear y registrar componentes
        m_optionsPage = new DeepSeek::Internal::DeepSeekOptionsPage(this);
        m_navigationChat = new DeepSeek::DeepSeekNavigationChat();

        ExtensionSystem::PluginManager::addObject(m_optionsPage);
        ExtensionSystem::PluginManager::addObject(m_navigationChat);
    }

    void extensionsInitialized() final
    {
        // Retrieve objects from the plugin manager's object pool, if needed. (rare)
        // In the extensionsInitialized function, a plugin can be sure that all
        // plugins that depend on it have passed their initialize() and
        // extensionsInitialized() phase.
    }

    ShutdownFlag aboutToShutdown() final
    {
        // Save settings
        // Disconnect from signals that are not needed during shutdown
        // Hide UI (if you add UI that is not in the main window directly)
        return SynchronousShutdown;
    }

private:
    void triggerAction()
    {
        QMessageBox::information(
            ICore::dialogParent(),
            Tr::tr("Action Triggered"),
            Tr::tr("This is an action from DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3."));
    }


    DeepSeek::Internal::DeepSeekOptionsPage *m_optionsPage = nullptr;
    DeepSeek::DeepSeekNavigationChat *m_navigationChat = nullptr;
};

} // namespace DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3::Internal

#include <deepseekplugin_qtcreator16_0_1_qt6_8_3.moc>
