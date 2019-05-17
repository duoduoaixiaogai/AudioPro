#include "export.h"
#include "exportpcm.h"
#include "project.h"
#include "WaveTrack.h"

#include <QMessageBox>
#include <QFileDialog>

namespace Renfeng {

    ExportPlugin::ExportPlugin()
    {
    }

    ExportPlugin::~ExportPlugin()
    {
    }

    int ExportPlugin::AddFormat()
    {
        FormatInfo nf;
        mFormatInfos.push_back(nf);
        return mFormatInfos.size();
    }

    void ExportPlugin::SetFormat(const QString & format, int index)
    {
        mFormatInfos[index].mFormat = format;
    }

    void ExportPlugin::SetCanMetaData(bool canmetadata, int index)
    {
        mFormatInfos[index].mCanMetaData = canmetadata;
    }

    void ExportPlugin::SetDescription(const QString & description, int index)
    {
        mFormatInfos[index].mDescription = description;
    }

    void ExportPlugin::AddExtension(const QString &extension,int index)
    {
        mFormatInfos[index].mExtensions.push_back(extension);
    }

    void ExportPlugin::SetMaxChannels(unsigned maxchannels, unsigned index)
    {
        mFormatInfos[index].mMaxChannels = maxchannels;
    }

    void ExportPlugin::SetExtensions(QStringList extensions, int index)
    {
        mFormatInfos[index].mExtensions = std::move(extensions);
    }

    int ExportPlugin::GetFormatCount()
    {
        return mFormatInfos.size();
    }

    Exporter::Exporter()
    {
        //      mMixerSpec = NULL;
        //      mBook = NULL;

        //      SetFileDialogTitle( _("Export Audio") );

        RegisterPlugin(New_ExportPCM());
        //      RegisterPlugin(New_ExportMP3());

        //#ifdef USE_LIBVORBIS
        //      RegisterPlugin(New_ExportOGG());
        //#endif
        //
        //#ifdef USE_LIBFLAC
        //      RegisterPlugin(New_ExportFLAC());
        //#endif
        //
        //#if USE_LIBTWOLAME
        //      RegisterPlugin(New_ExportMP2());
        //#endif
        //
        //      // Command line export not available on Windows and Mac platforms
        //      RegisterPlugin(New_ExportCL());
        //
        //#if defined(USE_FFMPEG)
        //      RegisterPlugin(New_ExportFFmpeg());
        //#endif
    }

    Exporter::~Exporter()
    {
    }

    void Exporter::RegisterPlugin(std::unique_ptr<ExportPlugin> &&ExportPlugin)
    {
        mPlugins.push_back(std::move(ExportPlugin));
    }

    bool Exporter::Process(AudioProject *project, bool selectedOnly,
                           double t0, double t1) {

        mProject = project;
        mSelectedOnly = selectedOnly;
        mT0 = t0;
        mT1 = t1;

        if (!ExamineTracks()) {
                return false;
            }

        if (!GetFilename()) {
                return false;
            }
    }

    bool Exporter::ExamineTracks()
    {
        mNumSelected = 0;
        mNumLeft = 0;
        mNumRight = 0;
        mNumMono = 0;

        double earliestBegin = mT1;
        double latestEnd = mT0;

        const TrackList *tracks = mProject->GetTracks();

        for (auto tr :
             tracks->Any< const WaveTrack >()
             + ( mSelectedOnly ? &Track::IsSelected : &Track::Any )
             - &WaveTrack::GetMute
             ) {
                mNumSelected++;

                if (tr->GetChannel() == Track::LeftChannel) {
                        mNumLeft++;
                    }
                else if (tr->GetChannel() == Track::RightChannel) {
                        mNumRight++;
                    }
                else if (tr->GetChannel() == Track::MonoChannel) {
                        // It's a mono channel, but it may be panned
                        float pan = tr->GetPan();

                        if (pan == -1.0)
                            mNumLeft++;
                        else if (pan == 1.0)
                            mNumRight++;
                        else if (pan == 0)
                            mNumMono++;
                        else {
                                // Panned partially off-center. Mix as stereo.
                                mNumLeft++;
                                mNumRight++;
                            }
                    }

                if (tr->GetOffset() < earliestBegin) {
                        earliestBegin = tr->GetOffset();
                    }

                if (tr->GetEndTime() > latestEnd) {
                        latestEnd = tr->GetEndTime();
                    }
            }

        if (mNumSelected == 0) {
                QString message;
                if(mSelectedOnly)
                    message = QString("All selected audio is muted.");
                else
                    message = QString("All audio is muted.");
                QMessageBox::information(nullptr, QString(""), QString("Unable to export"));
                return false;
            }

        if (mT0 < earliestBegin)
            mT0 = earliestBegin;

        if (mT1 > latestEnd)
            mT1 = latestEnd;

        return true;
    }

    bool Exporter::GetFilename()
    {
        mFormat = -1;
        QString maskString;
        QString defaultFormat = mFormatName;
        if( defaultFormat.isEmpty() )
            defaultFormat = QString("WAVFLT");

        mFilterIndex = 0;

        {
            int i = -1;
            for (const auto &pPlugin : mPlugins) {
                    ++i;
                    for (int j = 0; j < pPlugin->GetFormatCount(); j++)
                        {
                            maskString += pPlugin->GetMask(j) + QString(";;");
                            if (mPlugins[i]->GetFormat(j) == defaultFormat) {
                                    mFormat = i;
                                    mSubFormat = j;
                                }
                            if (mFormat == -1) mFilterIndex++;
                        }
                }
        }

        if (mFormat == -1)
            {
                mFormat = 0;
                mFilterIndex = 0;
                mSubFormat = 0;
            }

//        maskString.chop(1);
        QString defext = mPlugins[mFormat]->GetExtension(mSubFormat).toLower();

        mFilename = QFileDialog::getSaveFileName(nullptr, QString("Please save you file"), QString("D://"), maskString);

        return true;
    }

    QString ExportPlugin::GetMask(int index)
    {
        if (!mFormatInfos[index].mMask.isEmpty()) {
                return mFormatInfos[index].mMask;
            }

        QString mask = GetDescription(index) + QString("(");

        // Build the mask
        // const auto &ext = GetExtension(index);
        const auto &exts = GetExtensions(index);
        for (size_t i = 0; i < exts.size(); i++) {
                mask += QString("*.") + exts[i] + QString(")");
            }

        return mask;
    }

    QString ExportPlugin::GetDescription(int index)
    {
        return mFormatInfos[index].mDescription;
    }

    QString ExportPlugin::GetExtension(int index)
    {
        return mFormatInfos[index].mExtensions[0];
    }

    QStringList ExportPlugin::GetExtensions(int index)
    {
        return mFormatInfos[index].mExtensions;
    }

    QString ExportPlugin::GetFormat(int index)
    {
        return mFormatInfos[index].mFormat;
    }
}
