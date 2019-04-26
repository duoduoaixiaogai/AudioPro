#ifndef DIRMANAGER_H
#define DIRMANAGER_H

#include "XMLTagHandler.h"

#include <unordered_map>

namespace RF {
    class DirManager final : public XMLTagHandler {
    public:

        DirManager();

        virtual ~DirManager();

    };
}

#endif
