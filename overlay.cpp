#include "overlay.h"

#include <QSize>
#include <QPixmap>
#include <QRect>

namespace RF {
    std::pair<QRect, bool> Overlay::getRectangle(QSize size) {
        auto result = doGetRectangle(size);
        return result;
    }

    void Overlay::erase(QPaintDevice &dc, QPaintDevice &src) {
        QRect rect(QPoint(0, 0), dynamic_cast<QPixmap&>(dc).size());
        rect.intersects(QRect(QPoint(0, 0), dynamic_cast<QPixmap&>(src).size()));
        auto smallRect(getRectangle(dynamic_cast<QPixmap&>(src).size()).first);
        rect.intersects(smallRect);
        if (!rect.isEmpty()) {
            dynamic_cast<QPixmap&>(dc) = dynamic_cast<QPixmap&>(src).copy(rect.x(), rect.y(), rect.width(),
                                             rect.height());
        }
    }
}
