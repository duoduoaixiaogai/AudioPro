#ifndef VIEWINFO_H
#define VIEWINFO_H

#include <vector>

namespace RF {

    class Track;

    class ZoomInfo
    {
    public:
        ZoomInfo(double start, double pixelsPerSecond);
        ~ZoomInfo();
    };

}

#endif
