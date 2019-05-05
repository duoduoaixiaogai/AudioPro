#ifndef BLOCKFILE_H
#define BLOCKFILE_H

#include "types.h"
#include "XMLWriter.h"

namespace RF {

    template< typename Result, typename... Args >
    inline std::shared_ptr< Result > make_blockfile (Args && ... args)
    {
        return std::make_shared< Result > ( std::forward< Args > ( args )... );
    }

    class BlockFile {
    public:
        virtual ~BlockFile();
        size_t GetLength() const { return mLen; }
        virtual size_t ReadData(samplePtr data, sampleFormat format,
                                size_t start, size_t len, bool mayThrow = true)
        const = 0;
        virtual void SaveXML(XMLWriter &xmlFile) = 0;
        static unsigned long gBlockFileDestructionCount;
    protected:
        size_t mLen;
    };

    using BlockFilePtr = std::shared_ptr<BlockFile>;
}

#endif
