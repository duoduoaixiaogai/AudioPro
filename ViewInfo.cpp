﻿#include "ViewInfo.h"

namespace Renfeng {
    ZoomInfo::ZoomInfo(double start, double pixelsPerSecond) {

    }

    ZoomInfo::~ZoomInfo() {

    }

    ViewInfo::ViewInfo(double start, double screenDuration, double pixelsPerSecond)
        : ZoomInfo(start, pixelsPerSecond)
        , selectedRegion() {

    }
}
