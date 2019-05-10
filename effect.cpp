#include "effect.h"
#include "track.h"

namespace RF {

#define QUANTIZED_TIME(time, rate) (floor(((double)(time) * (rate)) + 0.5) / (rate))

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
        if (mClient)
        {
            return false;
        }

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

    bool Effect::SetHost(EffectHostInterface *host)
    {
        if (mClient)
        {
            return mClient->SetHost(host);
        }

        return true;
    }

    bool Effect::Startup(EffectClientInterface *client)
    {
        // Let destructor know we need to be shutdown
        mClient = client;

        // Set host so client startup can use our services
        if (!SetHost(this))
        {
            // Bail if the client startup fails
            mClient = nullptr;
            return false;
        }

        mNumAudioIn = GetAudioInCount();
        mNumAudioOut = GetAudioOutCount();

        return true;
    }

    unsigned Effect::GetAudioInCount()
    {
        if (mClient)
        {
            return mClient->GetAudioInCount();
        }

        return 0;
    }

    unsigned Effect::GetAudioOutCount()
    {
        if (mClient)
        {
            return mClient->GetAudioOutCount();
        }

        return 0;
    }

    bool Effect::DoEffect(::QMainWindow *parent,
                          double projectRate,
                          TrackList *list,
                          TrackFactory *factory,
                          SelectedRegion *selectedRegion,
                          bool shouldPrompt)
    {
        mOutputTracks.reset();

        mpSelectedRegion = selectedRegion;
        mFactory = factory;
        mProjectRate = projectRate;
        mTracks = list;

        CountWaveTracks();

        bool isSelection = false;

        mDuration = 0.0;

        mT0 = selectedRegion->t0();
        mT1 = selectedRegion->t1();

        if (mT1 > mT0)
        {
            // there is a selection: let's fit in there...
            // MJS: note that this is just for the TTC and is independent of the track rate
            // but we do need to make sure we have the right number of samples at the project rate
            double quantMT0 = QUANTIZED_TIME(mT0, mProjectRate);
            double quantMT1 = QUANTIZED_TIME(mT1, mProjectRate);
            mDuration = quantMT1 - quantMT0;
            isSelection = true;
            mT1 = mT0 + mDuration;
        }

        CountWaveTracks();

        if (!Init())
        {
            return false;
        }

        if (shouldPrompt && IsInteractive() && !PromptUser(parent))
           {
              return false;
           }

        return true;
    }

    void Effect::CountWaveTracks()
    {
        mNumTracks = mTracks->Selected< const WaveTrack >().size();
        mNumGroups = mTracks->SelectedLeaders< const WaveTrack >().size();
    }

    bool Effect::IsInteractive()
    {
       if (mClient)
       {
          return mClient->IsInteractive();
       }

       return true;
    }

    bool Effect::PromptUser(QMainWindow *parent)
    {
       return ShowInterface(parent, IsBatchProcessing());
    }

    bool Effect::IsBatchProcessing()
    {
       return mIsBatch;
    }

    bool Effect::ShowInterface(QMainWindow *parent, bool forceModal)
    {
       if (!IsInteractive())
       {
          return true;
       }

//       if (mUIDialog)
//       {
//          if ( mUIDialog->Close(true) )
//             mUIDialog = nullptr;
//          return false;
//       }

       if (mClient)
       {
          return mClient->ShowInterface(parent, forceModal);
       }

       // mUIDialog is null
       auto cleanup = valueRestorer( mUIDialog );

       mUIDialog = CreateUI(parent, this);
       if (!mUIDialog)
       {
          return false;
       }


       mUIDialog->Layout();
       mUIDialog->Fit();
       mUIDialog->SetMinSize(mUIDialog->GetSize());

       if( ScreenshotCommand::MayCapture( mUIDialog ) )
          return false;

       if( SupportsRealtime() && !forceModal )
       {
          mUIDialog->Show();
          cleanup.release();

          // Return false to bypass effect processing
          return false;
       }

       bool res = mUIDialog->ShowModal() != 0;

       return res;
    }
}
