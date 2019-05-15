#ifndef OVERLAY_H
#define OVERLAY_H

#include <qglobal.h>

#include <utility>

QT_BEGIN_NAMESPACE
class QRect;
class QSize;
class QPaintDevice;
QT_END_MOC_NAMESPACE

namespace Renfeng {

    class OverlayPanel;

    class Overlay {
    public:
        virtual ~Overlay() = 0;
        std::pair<QRect, bool> getRectangle(QSize size);
        virtual std::pair<QRect, bool> doGetRectangle(QSize size) = 0;
        virtual void erase(QPaintDevice &dc, QPaintDevice &src);
        virtual void draw(OverlayPanel &panel, QPaintDevice &dc) = 0;
    };
}

#endif // OVERLAY_H
