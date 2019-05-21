#include "loadeffects.h"
#include "moduleinterface.h"
#include "amplify.h"
#include "noisereduction.h"

namespace Renfeng {

#define EFFECT_LIST \
    EFFECT(AMPLIFY, EffectAmplify, ()) \
    EFFECT(NOISEREDUCTION, EffectNoiseReduction, ())

#define EFFECT(n, i, args) ENUM_ ## n,

    enum {
        EFFECT_LIST
    };

#undef EFFECT
#define EFFECT(n, i, args) results.push_back((n ## _PLUGIN_SYMBOL).internal());

    static const QStringList kEffectNames() {
        QStringList results;
        EFFECT_LIST;
        return results;
    }

#undef EFFECT
#define EFFECT(n, i, args) case ENUM_ ## n: return std::make_unique<i> args;

    DECLARE_MODULE_ENTRY(AudacityModule) {
        return new BuiltinEffectsModule(moduleManager, path);
    }

    DECLARE_BUILTIN_MODULE(BuiltinsEffectBuiltin)

    // BuiltinEffectsdModule
    BuiltinEffectsModule::BuiltinEffectsModule(Renfeng::ModuleManagerInterface *moduleManager,
                                               const QString *path) {
        mModMan = moduleManager;
        if (path) {
            mPath = *path;
        }
    }

    BuiltinEffectsModule::~BuiltinEffectsModule() {
        mPath.clear();
    }

    QString BuiltinEffectsModule::getPath() {
        return mPath;
    }

    ComponentInterfaceSymbol BuiltinEffectsModule::getSymbol() {
        return QString("Builtin Effects");
    }

    ComponentInterfaceSymbol BuiltinEffectsModule::getVendor() {
        return QString("The Audacity Team");
    }

    QString BuiltinEffectsModule::getVersion() {
        return QString("");
    }

    QString BuiltinEffectsModule::getDescription() {
        return QString("Provides builtin effects to Audacity");
    }

    bool BuiltinEffectsModule::initialize() {
        const auto &names = kEffectNames();
        for (const auto &name : names) {
            mNames.push_back(QString(BUILTIN_EFFECT_PREFIX) + name);
        }

        return true;
    }

    void BuiltinEffectsModule::terminate() {
        return;
    }

    bool BuiltinEffectsModule::autoRegisterPlugins(PluginManagerInterface &pm) {
        QString ignoredErrMsg;
        const auto &names = kEffectNames();
        for (const auto &name : names) {
            QString path(QString(BUILTIN_EFFECT_PREFIX) + name);

            if (!pm.isPluginRegistered(path)) {
                discoverPluginsAtPath(path, ignoredErrMsg,
                                      PluginManagerInterface::defaultRegistrationCallback);
            }
        }

        return false;
    }

    QStringList BuiltinEffectsModule::findPluginPaths(PluginManagerInterface &pm) {
        Q_UNUSED(pm)
        return mNames;
    }

    unsigned BuiltinEffectsModule::discoverPluginsAtPath(const QString &path,
                                                         QString &errMsg,
                                                         const RegistrationCallback &callback) {
       errMsg.clear();
       auto effect = instantiate(path);
       if (effect) {
           if (callback) {
               callback(this, effect.get());
           }
           return 1;
       }

       errMsg = QString("Unknow built-in effect name");
       return 0;
    }

    bool BuiltinEffectsModule::isPluginValid(const QString &path, bool bFast) {
        static_cast<void>(bFast);
        return mNames.contains(path);
    }

    ComponentInterface* BuiltinEffectsModule::createInstance(const QString &path) {
        return instantiate(path).release();
    }

    void BuiltinEffectsModule::deleteInstance(ComponentInterface *instance) {
        std::unique_ptr<Effect> {
            dynamic_cast<Effect*>(instance)
        };
    }

    std::unique_ptr<Effect> BuiltinEffectsModule::instantiate(const QString &path) {
        if (!path.startsWith(BUILTIN_EFFECT_PREFIX)) {
            return nullptr;
        }
        if (!mNames.contains(path)) {
            return nullptr;
        }

        switch (static_cast< QList<QString> >(mNames).indexOf(path)) {
            EFFECT_LIST;
        }

        return nullptr;
    }
}
