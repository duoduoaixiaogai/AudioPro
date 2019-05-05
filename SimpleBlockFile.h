#ifndef SIMPLEBLOCKFILE_H
#define SIMPLEBLOCKFILE_H

#include "BlockFile.h"

#include <QFileInfo>

namespace RF {
    class SimpleBlockFile : public BlockFile {
    public:
        void SaveXML(XMLWriter &xmlFile) override;
        SimpleBlockFile(QFileInfo &&baseFileName,
                           samplePtr sampleData, size_t sampleLen,
                           sampleFormat format,
                           bool allowDeferredWrite = false,
                           bool bypassCache = false );
    };
}

#endif
