#ifndef EXPORT_H
#define EXPORT_H

#include "progressdialog.h"
#include "SampleFormat.h"

#include <QString>
#include <QStringList>

#include <vector>

namespace Renfeng {

  class AudioProject;
  class MixerSpec;
  class Tags;
  class Mixer;
  class WaveTrack;
  class TimeTrack;
  using WaveTrackConstArray = std::vector < std::shared_ptr < const WaveTrack > >;

  class FormatInfo {
  public:
      FormatInfo() {}
      FormatInfo( const FormatInfo & ) = default;
      FormatInfo &operator = ( const FormatInfo & ) = default;
      ~FormatInfo() {}

      QString mFormat;
      QString mDescription;
      // QString mExtension;
      QStringList mExtensions;
      //      FileExtensions mExtensions;
      QString mMask;
      unsigned mMaxChannels;
      bool mCanMetaData;
  };

  class ExportPlugin {
  public:
      ExportPlugin();
      virtual ~ExportPlugin();

      int AddFormat();
      void SetFormat(const QString & format, int index);
      void SetCanMetaData(bool canmetadata, int index);
      void SetExtensions(QStringList extensions, int index);
      void SetDescription(const QString & description, int index);
      void AddExtension(const QString &extension,int index);
      void SetMaxChannels(unsigned maxchannels, unsigned index);

      virtual int GetFormatCount();
      virtual QString GetMask(int index);

      virtual QString GetDescription(int index);
      virtual QString GetExtension(int index = 0);
      virtual QStringList GetExtensions(int index = 0);
      virtual QString GetFormat(int index);

      virtual ProgressResult Export(AudioProject *project,
                             std::unique_ptr<ProgressDialog> &pDialog,
                             unsigned channels,
                             const QString &fName,
                             bool selectedOnly,
                             double t0,
                             double t1,
                             MixerSpec *mixerSpec = NULL,
                             const Tags *metadata = NULL,
                             int subformat = 0) = 0;
      virtual int SetNumExportChannels() { return -1; }
      virtual unsigned GetMaxChannels(int index);
  protected:
      std::unique_ptr<Mixer> CreateMixer(const WaveTrackConstArray &inputTracks,
               const TimeTrack* timeTrack,
               double startTime, double stopTime,
               unsigned numOutChannels, size_t outBufferSize, bool outInterleaved,
               double outRate, sampleFormat outFormat,
               bool highQuality = true, MixerSpec *mixerSpec = NULL);
  private:
      std::vector<FormatInfo> mFormatInfos;
  };

  using ExportPluginArray = std::vector < std::unique_ptr< ExportPlugin > > ;

  class Exporter {
  public:
      Exporter();
      virtual ~Exporter();
      void SetDefaultFormat( const QString & Format ){ mFormatName = Format;}
      void RegisterPlugin(std::unique_ptr<ExportPlugin> &&plugin);
      bool Process(AudioProject *project, bool selectedOnly,
                   double t0, double t1);
  private:
      bool ExamineTracks();
      bool GetFilename();
      bool ExportTracks();
      bool CheckMix();
  private:
      QString mFormatName;
      ExportPluginArray mPlugins;
      AudioProject *mProject;
      bool mSelectedOnly;
      double mT0;
      double mT1;
      int mNumSelected;
      unsigned mNumLeft;
      unsigned mNumRight;
      unsigned mNumMono;
      int mFormat;
      int mFilterIndex;
      int mSubFormat;
      QString mFilename;
      unsigned mChannels;
      std::unique_ptr<MixerSpec> mMixerSpec;
  };
}

#endif // EXPORT_H
