#ifndef EFFECT_H
#define EFFECT_H

#include "effectinterface.h"

class QMainWindow;

namespace RF {
#define BUILTIN_EFFECT_PREFIX "Built-in Effect: "

    class TrackList;
    class TrackFactory;
    class SelectedRegion;

    class Effect : public EffectClientInterface,
            public EffectHostInterface {
    public:
        Effect();
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
        bool SetHost(EffectHostInterface *host) Q_DECL_OVERRIDE;
        virtual bool Startup(EffectClientInterface *client);
        unsigned GetAudioInCount() Q_DECL_OVERRIDE;
        unsigned GetAudioOutCount() Q_DECL_OVERRIDE;
        bool DoEffect(::QMainWindow *parent, double projectRate, TrackList *list,
                         TrackFactory *factory, SelectedRegion *selectedRegion,
                         bool shouldPrompt = true);
    private:
        EffectClientInterface *mClient;
        size_t mNumAudioIn;
        size_t mNumAudioOut;
    };
}

#endif // EFFECT_H
