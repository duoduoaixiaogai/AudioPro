#ifndef PANELWRAPPER_H
#define PANELWRAPPER_H

#include <QFrame>

namespace RF {
    template<typename Base>
    class TraversalWrapper : public Base {
    public:
        template<typename... Args>
        TraversalWrapper(Args&&... args)
            : Base(std::forward<Args>(args)...) {}
    };

    class FrameWrapper : public TraversalWrapper<QFrame> {
    public:
        FrameWrapper() {}
        FrameWrapper(QWidget *parent = nullptr)
            : TraversalWrapper<QFrame>(parent)
        {}
        bool create(QWidget *parent = nullptr) {

        }
    };
}

#endif // PANELWRAPPER_H
