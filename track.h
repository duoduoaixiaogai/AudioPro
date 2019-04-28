#ifndef TRACK_H
#define TRACK_H

#include "commontrackpanelcell.h"
#include "SampleFormat.h"
#include "DirManager.h"
#include "ViewInfo.h"
//#include "WaveTrack.h"

#include <QSize>

namespace RF {

    class WaveTrack;

    using ListOfTracks = std::list< std::shared_ptr< Track > >;

    class Track : public CommonTrackPanelCell, public XMLTagHandler
    {
    public:
        Track(const std::shared_ptr<DirManager> &mDirManager);
        virtual ~ Track();
        mutable QSize vrulerSize;
        enum ChannelType
        {
            LeftChannel = 0,
            RightChannel = 1,
            MonoChannel = 2
        };
        enum : unsigned { DefaultHeight = 150 };
    protected:
        mutable std::shared_ptr<DirManager> mDirManager;
        double              mOffset;
        bool           mLinked;
        int            mY;
        int            mHeight;
        int            mIndex;
        bool           mMinimized;
        ChannelType         mChannel;
    private:
        bool           mSelected;
    };

    class AudioTrack : public Track
    {
    public:
        AudioTrack(const std::shared_ptr<DirManager> &projDirManager)
            : Track{ projDirManager } {}
    };

    class PlayableTrack : public AudioTrack
    {
    public:
        PlayableTrack(const std::shared_ptr<DirManager> &projDirManager)
            : AudioTrack{ projDirManager } {}
    };


    class TrackList final : /*public wxEvtHandler,*/ public ListOfTracks
    {
    };

    class TrackFactory
    {
    private:
        TrackFactory(const std::shared_ptr<DirManager> &dirManager, const ZoomInfo *zoomInfo):
            mDirManager(dirManager)
          , mZoomInfo(zoomInfo)
        {
        }

        const std::shared_ptr<DirManager> mDirManager;
        const ZoomInfo *const mZoomInfo;

    public:
        // These methods are defined in WaveTrack.cpp, NoteTrack.cpp,
        // LabelTrack.cpp, and TimeTrack.cpp respectively
        std::unique_ptr<WaveTrack> NewWaveTrack(sampleFormat format = (sampleFormat)0,
                                                double rate = 0);
    };
}

#endif
