#ifndef EFFECT_H
#define EFFECT_H

#include "effectinterface.h"

class QMainWindow;
class QDialog;

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
        bool IsInteractive() override;
        virtual bool IsBatchProcessing();
        bool ShowInterface(QMainWindow *parent, bool forceModal = false) override;
    protected:
        virtual bool Init();
        const TrackList *inputTracks() const { return mTracks; }
        virtual bool PromptUser(QMainWindow *parent);
    private:
        void CountWaveTracks();
    protected:
        std::shared_ptr<TrackList> mOutputTracks;
        SelectedRegion *mpSelectedRegion{};
        TrackFactory   *mFactory;
        double         mProjectRate;
        double         mT0;
        double         mT1;
        double         mF0;
          double         mF1;
        QDialog       *mUIDialog;
    private:
        EffectClientInterface *mClient;
        size_t mNumAudioIn;
        size_t mNumAudioOut;
        TrackList *mTracks;
        int mNumTracks;
        int mNumGroups;
        double mDuration;
        bool mIsBatch;
    };
}

#endif // EFFECT_H
