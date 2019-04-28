#ifndef EFFECT_H
#define EFFECT_H

#include "effectinterface.h"

namespace RF {
#define BUILTIN_EFFECT_PREFIX "Built-in Effect: "

    class Effect : public EffectClientInterface {
    public:
        virtual bool LoadFactoryDefaults();
        virtual EffectType getType();
        virtual ComponentInterfaceSymbol getFamilyId();
        virtual bool isInteractive();
        virtual bool isDefault();
        virtual bool isLegacy();
        virtual bool supportsRealtime();
        virtual bool supportsAutomation();
        virtual QString getPath();
        virtual ComponentInterfaceSymbol getSymbol();
        virtual ComponentInterfaceSymbol getVendor();
        virtual QString getVersion();
        virtual QString getDescription();
    };
}

#endif // EFFECT_H
