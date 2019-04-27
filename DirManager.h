#ifndef DIRMANAGER_H
#define DIRMANAGER_H

#include "types.h"

#include "XMLTagHandler.h"

#include <unordered_map>

namespace RF{

    class BlockFile;
    using BlockFilePtr = std::shared_ptr<BlockFile>;

    class DirManager final : public XMLTagHandler {
    public:

        DirManager();

        virtual ~DirManager();

        BlockFilePtr
              NewSimpleBlockFile(samplePtr sampleData,
                                         size_t sampleLen,
                                         sampleFormat format,
                                         bool allowDeferredWrite = false);

    };
}

#endif
