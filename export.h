#ifndef EXPORT_H
#define EXPORT_H

#include <QString>
#include <QStringList>

#include <vector>

namespace Renfeng {

  class AudioProject;

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
  };
}

#endif // EXPORT_H
