#ifndef LOADEFFECTS_H
#define LOADEFFECTS_H

#include "moduleinterface.h"
#include "effect.h"

#include <memory>

#include <QStringList>

namespace RF {

   class PluginManagerInterface;
   class ModuleManager;

   class BuiltinEffectsModule final : public ModuleInterface {
   public:
       BuiltinEffectsModule(ModuleManagerInterface *moduleManager, const QString *path);
       virtual ~BuiltinEffectsModule();

       QString getPath() Q_DECL_OVERRIDE;
       ComponentInterfaceSymbol getSymbol() Q_DECL_OVERRIDE;
       ComponentInterfaceSymbol getVendor() Q_DECL_OVERRIDE;
       QString getVersion() Q_DECL_OVERRIDE;
       QString getDescription() Q_DECL_OVERRIDE;

       bool initialize() Q_DECL_OVERRIDE;
       void terminate() Q_DECL_OVERRIDE;

       QStringList fileExtensions() Q_DECL_OVERRIDE {return {};}
       QString installPath() Q_DECL_OVERRIDE {return {};}

       bool autoRegisterPlugins(PluginManagerInterface &pm) Q_DECL_OVERRIDE;
       QStringList findPluginPaths(PluginManagerInterface &pm) Q_DECL_OVERRIDE;
       unsigned discoverPluginsAtPath(const QString &path
                                      ,QString &errMsg
                                      ,const RegistrationCallback &callback) Q_DECL_OVERRIDE;
       bool isPluginValid(const QString &path, bool bFast) Q_DECL_OVERRIDE;
       ComponentInterface* createInstance(const QString &path) Q_DECL_OVERRIDE;
       void deleteInstance(ComponentInterface *instance) Q_DECL_OVERRIDE;
   private:

       std::unique_ptr<Effect> instantiate(const QString &path);
   private:
       ModuleManagerInterface *mModMan;
       QString mPath;

       QStringList mNames;
   };
}

#endif // LOADEFFECTS_H
