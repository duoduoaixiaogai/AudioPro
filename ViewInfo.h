#ifndef VIEWINFO_H
#define VIEWINFO_H

#include "SelectedRegion.h"

#include <vector>

namespace Renfeng {

    class Track;

    class ZoomInfo
    {
    public:
        ZoomInfo(double start, double pixelsPerSecond);
        ~ZoomInfo();
        static double GetDefaultZoom()
           { return 44100.0 / 512.0; }
    };

    class ViewInfo final : public ZoomInfo
    {
    public:
       ViewInfo(double start, double screenDuration, double pixelsPerSecond);

       SelectedRegion selectedRegion;
    };

}

#endif
