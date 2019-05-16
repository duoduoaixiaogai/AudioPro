#ifndef EFFECTINTERFACE_H
#define EFFECTINTERFACE_H

#include "moduleinterface.h"
#include "amplifydialog.h"

class QMainWindow;
class QDialog;

namespace Renfeng {
    typedef enum EffectType : int {
        EffectTypeNone,
        EffectTypeHidden,
        EffectTypeGenerate,
        EffectTypeProcess,
        EffectTypeAnalyze,
        EffectTypeTool,
    } EffectType;

    class EffectUIHostInterface
    {
    public:
        virtual ~EffectUIHostInterface() {}
    };

    class ConfigClientInterface {

    };

    class EffectUIClientInterface
    {
    public:
        virtual bool PopulateUI(QWidget* parent) = 0;
    };

    class EffectHostInterface : public ConfigClientInterface
    {
    public:
        virtual ~EffectHostInterface() {};
        virtual AmplifyDialog* CreateUI(QMainWindow *parent, EffectUIClientInterface *client) = 0;
    };

    class EffectDefinitionInterface : public ComponentInterface {
    public:
        virtual ~EffectDefinitionInterface() {}
        virtual EffectType GetType() = 0;
        virtual EffectType getClassification() {return GetType();}
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
      virtual void SetSampleRate(double rate) = 0;
      virtual size_t SetBlockSize(size_t maxBlockSize) = 0;
      virtual bool ProcessInitialize(sampleCount totalLen, ChannelNames chanMap = NULL) = 0;
      virtual bool ProcessFinalize() /* noexcept */ = 0;
      virtual sampleCount GetLatency() = 0;
      virtual size_t ProcessBlock(float **inBlock, float **outBlock, size_t blockLen) = 0;
    };
}

#endif // EFFECTINTERFACE_H
