#include "modulemanager.h"
#include "pluginmanager.h"
#include "moduleinterface.h"

#include <QVector>

namespace RF {

    std::unique_ptr<ModuleManager> ModuleManager::mInstance{};

    using BuiltinModuleList = QVector<ModuleMain>;

    BuiltinModuleList& builtinModuleList() {
        static BuiltinModuleList theList;
        return theList;
    }

    void registerBuiltinModule(ModuleMain moduleMain) {
        builtinModuleList().push_back(moduleMain);
        return;
    }

    ModuleManager::~ModuleManager() {

    }

    void ModuleManager::initializeBuiltins() {
        PluginManager& pm = PluginManager::get();

        for (auto moduleMain : builtinModuleList()) {
            ModuleInterfaceHandle module {
                moduleMain(this, nullptr), ModuleInterfaceDeleter{}
            };

           if (module->initialize()) {
               ModuleInterface *pInterface = module.get();
               const PluginID &id = pm.registerPlugin(pInterface);

               mDynModules[id] = std::move(module);

               pInterface->autoRegisterPlugins(pm);
           }
        }
    }

    ModuleManager& ModuleManager::get() {
        if (!mInstance) {
            mInstance.reset(new ModuleManager);
        }

        return *mInstance;
    }

    bool ModuleManager::discoverProviders() {
        initializeBuiltins();

        return true;
    }

    void ModuleInterfaceDeleter::operator()(ModuleInterface *pInterface) const {
        if (pInterface) {
            pInterface->terminate();

            std::unique_ptr<ModuleInterface> {pInterface};
        }
    }
}
