#include "effect.h"

namespace RF {

    bool Effect::LoadFactoryDefaults() {
        return true;
    }

    EffectType Effect::getType() {
        return  EffectTypeNone;
    }

    ComponentInterfaceSymbol Effect::getFamilyId() {
        return ComponentInterfaceSymbol(QString(""));
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
        return QString("");
    }

    ComponentInterfaceSymbol Effect::getSymbol() {
        return ComponentInterfaceSymbol(QString(""));
    }

    ComponentInterfaceSymbol Effect::getVendor() {
        return ComponentInterfaceSymbol(QString(""));
    }

    QString Effect::getVersion() {
        return QString("");
    }

    QString Effect::getDescription() {
        return QString("");
    }
}
