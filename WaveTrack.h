#ifndef WAVETRACK_H
#define WAVETRACK_H

#include "track.h"
#include "SampleFormat.h"
#include "WaveClip.h"

#include <vector>

namespace RF {

    class WaveTrack final : public PlayableTrack {

    private:

        //
        // Constructor / Destructor / Duplicator
        //
        // Private since only factories are allowed to construct WaveTracks
        //

        WaveTrack(const std::shared_ptr<DirManager> &projDirManager,
                  sampleFormat format = (sampleFormat)0,
                  double rate = 0);
    };
}

#endif
