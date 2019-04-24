﻿/**********************************************************************

Audacity: A Digital Audio Editor

CommonTrackPanelCell.h

Paul Licameli split from TrackPanel.cpp

**********************************************************************/

#ifndef __AUDACITY_COMMON_TRACK_PANEL_CELL__
#define __AUDACITY_COMMON_TRACK_PANEL_CELL__

#include "TrackPanelCell.h"
#include "memoryx.h"
#include <stdlib.h>

namespace RF {
    class Track;

    class /*AUDACITY_DLL_API*/ CommonTrackPanelCell /* not final */
            : public TrackPanelCell
    {
    public:
        CommonTrackPanelCell()
            : mVertScrollRemainder(0.0)
        {}

        virtual ~CommonTrackPanelCell() = 0;

        // Default to the arrow cursor
        HitTestPreview DefaultPreview
        (const TrackPanelMouseState &, const AudacityProject *) override;

        virtual std::shared_ptr<Track> FindTrack() = 0;

    protected:
        unsigned HandleWheelRotation
        (const TrackPanelMouseEvent &event,
         AudacityProject *pProject) override;

    private:
        double mVertScrollRemainder;
    };
}

#endif
