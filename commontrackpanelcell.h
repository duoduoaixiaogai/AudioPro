﻿#ifndef COMMONTRACKPANELCELL_H
#define COMMONTRACKPANELCELL_H

#include "trackpanelcell.h"

namespace Renfeng {
    class Track;

    class CommonTrackPanelCell : public TrackPanelCell
    {
    public:
        CommonTrackPanelCell()
            : mVertScrollRemainder(0.0)
        {}

        virtual ~CommonTrackPanelCell() = 0;

    private:
        double mVertScrollRemainder;
    };
}

#endif
