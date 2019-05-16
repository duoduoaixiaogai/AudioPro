#ifndef EFFECT_H
#define EFFECT_H

#include "effectinterface.h"
#include "SampleFormat.h"

#include <QDialog>

class QMainWindow;

namespace Renfeng {

  class Track;
  class WaveTrack;
#define BUILTIN_EFFECT_PREFIX "Built-in Effect: "

  inline double TrapDouble(double x, double min, double max)
  {
      if (x <= min)
          return min;

      if (x >= max)
          return max;

      return x;
  }

  class TrackList;
  class TrackFactory;
  class SelectedRegion;
  class EffectUIClientInterface;
  class Effect;

  class EffectUIHost final : public AmplifyDialog,
          public EffectUIHostInterface
  {
  public:
      EffectUIHost(QMainWindow *parent,
                   Effect *effect,
                   EffectUIClientInterface *client);
      bool Initialize();
  private:
      Effect *mEffect;
      EffectUIClientInterface *mClient;
  };

  class Effect : public EffectClientInterface,
          public EffectHostInterface,
          public EffectUIClientInterface,
          public QObject {
  public:
      Effect();
      virtual bool LoadFactoryDefaults();
//      virtual EffectType getType();
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
      AmplifyDialog* CreateUI(QMainWindow *parent, EffectUIClientInterface *client) override;
      bool PopulateUI(QWidget* parent) override;
      void SetSampleRate(double rate) override;
      size_t SetBlockSize(size_t maxBlockSize) override;
      bool ProcessInitialize(sampleCount totalLen, ChannelNames chanMap = NULL) override;
      bool ProcessFinalize() override;
      sampleCount GetLatency() override;
      EffectType GetType() override;
      size_t ProcessBlock(float **inBlock, float **outBlock, size_t blockLen) override;
  protected:
      virtual bool Init();
      const TrackList *inputTracks() const { return mTracks; }
      virtual bool PromptUser(QMainWindow *parent);
      virtual void PopulateOrExchange(QWidget *parent);
      virtual bool Process();
      virtual bool ProcessPass();
      void CopyInputTracks(bool allSyncLockSelected = false);
      void GetSamples(
            const WaveTrack *track, sampleCount *start, sampleCount *len);
      void ReplaceProcessedTracks(const bool bGoodResult);
  private:
      void CountWaveTracks();
      bool ProcessTrack(int count,
                           ChannelNames map,
                           WaveTrack *left,
                           WaveTrack *right,
                           sampleCount leftStart,
                           sampleCount rightStart,
                           sampleCount len,
                           FloatBuffers &inBuffer,
                           FloatBuffers &outBuffer,
                           ArrayOf< float * > &inBufPos,
                           ArrayOf< float *> &outBufPos);
  protected:
      std::shared_ptr<TrackList> mOutputTracks;
      SelectedRegion *mpSelectedRegion{};
      TrackFactory   *mFactory;
      double         mProjectRate;
      double         mT0;
      double         mT1;
      double         mF0;
      double         mF1;
      AmplifyDialog* mUIDialog;
      int            mPass;
      sampleCount    mSampleCnt;
      double         mSampleRate;
  private:
      EffectClientInterface *mClient;
      size_t mNumAudioIn;
      size_t mNumAudioOut;
      TrackList *mTracks;
      int mNumTracks;
      int mNumGroups;
      double mDuration;
      bool mIsBatch;
      std::vector<Track*> mIMap;
      std::vector<Track*> mOMap;
      size_t mBufferSize;
         size_t mBlockSize;
         unsigned mNumChannels;
  };

#define Param(name, type, key, def, min, max, scale) \
    static const QString KEY_ ## name = (key); \
    static const type DEF_ ## name = (def); \
    static const type MIN_ ## name = (min); \
    static const type MAX_ ## name = (max); \
    static const type SCL_ ## name = (scale);
}

#endif // EFFECT_H
