#ifndef MODULEINTERFACE_H
#define MODULEINTERFACE_H

#include "componentinterface.h"
#include "plugininterface.h"

#include <functional>

namespace Renfeng {

    class ModuleInterface : public ComponentInterface {
    public:
        virtual ~ModuleInterface() {}
        virtual bool initialize() = 0;
        virtual void terminate() = 0;
        virtual QStringList fileExtensions() = 0;
        virtual QString installPath() = 0;
        virtual bool autoRegisterPlugins(PluginManagerInterface &PluginManager) = 0;
        virtual QStringList findPluginPaths(PluginManagerInterface &pluginManager) = 0;
        using RegistrationCallback =
        std::function<
        const PluginID &(ModuleInterface *, ComponentInterface *)>;
        virtual unsigned discoverPluginsAtPath(
                const QString &path, QString &errMsg,
                const RegistrationCallback &callback)
        = 0;
        virtual bool isPluginValid(const QString &path, bool bFast) = 0;
        virtual ComponentInterface *createInstance(const QString &path) = 0;
        virtual void deleteInstance(ComponentInterface *instance) = 0;
    };

    class ModuleManagerInterface {
    public:
        //virtual void registerModule(ModuleInterface *module) = 0;
    };

#define MODULE_ENTRY AudacityModule
    typedef ModuleInterface *(*ModuleMain)(ModuleManagerInterface *moduleManager,
                                           const QString *path);
#define DECLARE_MODULE_ENTRY(name) \
    static ModuleInterface* name(ModuleManagerInterface *moduleManager, const QString *path)
#define DECLARE_BUILTIN_MODULE_BASE(name) \
    extern void registerBuiltinModule(ModuleMain rtn); \
    class name { \
    public: \
        name() {Register();} \
        void Register(); \
    }; \
    static name name ## _instance;

#define DECLARE_BUILTIN_MODULE(name) \
    DECLARE_BUILTIN_MODULE_BASE(name) \
    void name::Register() { \
        registerBuiltinModule(MODULE_ENTRY); \
    }
}

#endif // MODULEINTERFACE_H
