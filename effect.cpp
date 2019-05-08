#include "effect.h"

namespace RF {

    Effect::Effect() {
        mClient = nullptr;
    }

    bool Effect::LoadFactoryDefaults() {
        return true;
    }

    EffectType Effect::getType() {
        return  EffectTypeNone;
    }

    ComponentInterfaceSymbol Effect::getFamilyId() {
        //        return ComponentInterfaceSymbol(QString(""));
        if (mClient)
        {
            return mClient->getFamilyId();
        }

        // Unusually, the internal and visible strings differ for the built-in
        // effect family.
        return { QString("Audacity"), QString("Built-in") };
    }

    bool Effect::isInteractive() {
        return true;
    }

    bool Effect::isDefault() {
        return true;
    }

    bool Effect::isLegacy() {
        return true;
    }

    bool Effect::supportsRealtime() {
        return true;
    }

    bool Effect::supportsAutomation() {
        return true;
    }

    QString Effect::getPath() {
        //        return QString("");
        if (mClient)
        {
            return mClient->getPath();
        }

        return BUILTIN_EFFECT_PREFIX + getSymbol().internal();
    }

    ComponentInterfaceSymbol Effect::getSymbol() {
        return ComponentInterfaceSymbol(QString(""));
    }

    ComponentInterfaceSymbol Effect::getVendor() {
        //        return ComponentInterfaceSymbol(QString(""));
        if (mClient)
        {
            return mClient->getVendor();
        }

        return QString("Audacity");
    }

    QString Effect::getVersion() {
        return QString("");
    }

    QString Effect::getDescription() {
        return QString("");
    }
}
