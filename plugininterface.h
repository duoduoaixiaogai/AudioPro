#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include "types.h"
//#include "componentinterface.h"
//#include "moduleinterface.h"

namespace RF {

    class ComponentInterface;
    class ModuleInterface;
    class EffectDefinitionInterface;

    class PluginManagerInterface {
    public:
        static const PluginID& defaultRegistrationCallback(
                ModuleInterface *provider, ComponentInterface *ident);
        virtual bool isPluginRegistered(const QString &path) = 0;
        virtual const PluginID& registerPlugin(ModuleInterface *module) = 0;
        virtual const PluginID& registerPlugin(ModuleInterface *provider,
                                               EffectDefinitionInterface *effect,
                                               int type) = 0;
//        virtual void findFilesInPathList(const QString &pattern,
//                                         const QStringList &pathList,
//                                         QStringList &files,
//                                         bool directories = false) = 0;
    };
}

#endif // PLUGININTERFACE_H
