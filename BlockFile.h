#ifndef BLOCKFILE_H
#define BLOCKFILE_H

#include "types.h"
#include "XMLWriter.h"

namespace RF {
     class BlockFile {
     public:
         virtual ~BlockFile();
         size_t GetLength() const { return mLen; }
         virtual size_t ReadData(samplePtr data, sampleFormat format,
                                 size_t start, size_t len, bool mayThrow = true)
               const = 0;
         virtual void SaveXML(XMLWriter &xmlFile) = 0;
     protected:
         size_t mLen;
     };
}

#endif
