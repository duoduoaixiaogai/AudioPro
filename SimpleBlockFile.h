#ifndef SIMPLEBLOCKFILE_H
#define SIMPLEBLOCKFILE_H

#include "BlockFile.h"

namespace RF {
    class SimpleBlockFile : public BlockFile {
    public:
        void SaveXML(XMLWriter &xmlFile) override;
    };
}

#endif
