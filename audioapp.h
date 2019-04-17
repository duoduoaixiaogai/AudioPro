#ifndef AUDIOAPP_H
#define AUDIOAPP_H

#include <QObject>

namespace RF {
    class AudioApp : public QObject {
    public:
       AudioApp() = default;
       ~AudioApp() = default;
       bool init();
    };
}

#endif // AUDIOAPP_H
