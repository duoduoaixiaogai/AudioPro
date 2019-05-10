#ifndef EFFECTINTERFACE_H
#define EFFECTINTERFACE_H

#include "moduleinterface.h"

class QMainWindow;

namespace RF {
    typedef enum EffectType : int {
        EffectTypeNone,
        EffectTypeHidden,
        EffectTypeGenerate,
        EffectTypeProcess,
        EffectTypeAnalyze,
        EffectTypeTool,
    } EffectType;

    class ConfigClientInterface {

    };

    class EffectHostInterface : public ConfigClientInterface
    {

    };

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
        virtual bool IsInteractive() = 0;
    };

    class EffectClientInterface  : public EffectDefinitionInterface {
    public:
        virtual ~EffectClientInterface() {}
        virtual bool SetHost(EffectHostInterface *host) = 0;
        virtual bool LoadFactoryDefaults() = 0;
        virtual unsigned GetAudioInCount() = 0;
        virtual unsigned GetAudioOutCount() = 0;
        virtual bool ShowInterface(QMainWindow *parent, bool forceModal = false) = 0;
    };
}

#endif // EFFECTINTERFACE_H
