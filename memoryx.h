#ifndef MEMORYX_H
#define MEMORYX_H

namespace RF {
    template<typename T>
    struct Destroyer {
        void operator () (T *p) const {
            if (p) {
                p->Destroy();
            }
        }
    };
}

#endif // MEMORYX_H
