#ifndef TRACK_H
#define TRACK_H

#include "commontrackpanelcell.h"
#include "types.h"

#include <QList>

#include <memory>

namespace RF {

    class Track;
    class DirManager;
    class ZoomInfo;
    class WaveTrack;
    class LabelTrack;
    class TimeTrack;
    class NoteTrack;

    using ListOfTracks = std::list<std::shared_ptr<Track> >;

    class Track : public CommonTrackPanelCell {

    };

    class TrackList final : public ListOfTracks {
        TrackList();
    public:
        virtual ~TrackList();

        static std::shared_ptr<TrackList> create();

    private:
        std::weak_ptr<TrackList> mSelf;
    };

    class TrackFactory {
    private:
        TrackFactory(const std::shared_ptr<DirManager> &dirManager,
                     const ZoomInfo *zoomInfo) :
            mDirManager(dirManager),
            mZoomInfo(zoomInfo) {}

        const std::shared_ptr<DirManager> mDirManager;
        const ZoomInfo *const mZoomInfo;
        friend class AudioProject;
        friend class BenchmarkDialog;

    public:
        std::unique_ptr<WaveTrack> duplicateWaveTrack(const WaveTrack &orig);
        std::unique_ptr<WaveTrack> newWaveTrack(sampleFormat format = (sampleFormat)0,
                                                double rate = 0);
        std::unique_ptr<LabelTrack> newLabelTrack();
        std::unique_ptr<TimeTrack> newTimeTrack();
        std::unique_ptr<NoteTrack> newNoteTrack();
    };

}

#endif // TRACK_H
