#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include "moduleinterface.h"

#include <QMap>

namespace RF {

    struct ModuleInterfaceDeleter {
        void operator ()(ModuleInterface *pInterface) const;
    };

    using ModuleInterfaceHandle = std::unique_ptr<
        ModuleInterface, ModuleInterfaceDeleter
    >;

    typedef std::map<QString, ModuleInterfaceHandle> ModuleMap;
    // 使用QMap会出现错误，暂时不太清楚原因
    //typedef QMap<QString, ModuleInterfaceHandle> ModuleMap;

    class ModuleManager final : public ModuleManagerInterface {
    public:
        static ModuleManager& get();
        bool discoverProviders();

        ~ModuleManager();
        ComponentInterface *createProviderInstance(const PluginID & provider, const QString & path);
        ComponentInterface *CreateInstance(const PluginID & provider, const QString & path);
    private:
        ModuleManager() = default;

        void initializeBuiltins();
    private:
        static std::unique_ptr<ModuleManager> mInstance;

        ModuleMap mDynModules;
    };
}

#endif // MODULEMANAGER_H
