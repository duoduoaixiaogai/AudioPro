#ifndef EFFECTINTERFACE_H
#define EFFECTINTERFACE_H

#include "moduleinterface.h"

namespace RF {
    typedef enum EffectType : int {
        EffectTypeNone,
        EffectTypeHidden,
        EffectTypeGenerate,
        EffectTypeProcess,
        EffectTypeAnalyze,
        EffectTypeTool,
    } EffectType;

    class EffectDefinitionInterface : public ComponentInterface {
    public:
        virtual ~EffectDefinitionInterface() {}
        virtual EffectType getType() = 0;
        virtual EffectType getClassification() {return getType();}
        virtual ComponentInterfaceSymbol getFamilyId() = 0;
        virtual bool isInteractive() = 0;
        virtual bool isDefault() = 0;
        virtual bool isLegacy() = 0;
        virtual bool supportsRealtime() = 0;
        virtual bool supportsAutomation() = 0;
    };
}

#endif // EFFECTINTERFACE_H
