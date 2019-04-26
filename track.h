#ifndef TRACK_H
#define TRACK_H

#include "commontrackpanelcell.h"
#include "SampleFormat.h"
#include "XMLTagHandler.h"
#include "DirManager.h"
#include "ViewInfo.h"
#include "WaveTrack.h"

namespace RF {

    using ListOfTracks = std::list< std::shared_ptr< Track > >;

    class Track : public CommonTrackPanelCell, public XMLTagHandler
    {
    public:
        Track(const std::shared_ptr<DirManager> &mDirManager);
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
