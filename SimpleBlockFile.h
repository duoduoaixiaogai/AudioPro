#ifndef SIMPLEBLOCKFILE_H
#define SIMPLEBLOCKFILE_H

#include "BlockFile.h"
#include "memoryx.h"

#include <QFileInfo>


namespace RF {

    struct SimpleBlockFileCache {
        bool active;
        bool needWrite;
        sampleFormat format;
        ArrayOf<char> sampleData, summaryData;

        SimpleBlockFileCache() {}
    };

    enum {
       AU_SAMPLE_FORMAT_16 = 3,
       AU_SAMPLE_FORMAT_24 = 4,
       AU_SAMPLE_FORMAT_FLOAT = 6,
    };

    typedef struct {
       unsigned int magic;      // magic number
       unsigned int dataOffset; // byte offset to start of audio data
       unsigned int dataSize;   // data length, in bytes (optional)
       unsigned int encoding;   // data encoding enumeration
       unsigned int sampleRate; // samples per second
       unsigned int channels;   // number of interleaved channels
    } auHeader;

    class SimpleBlockFile : public BlockFile {
    public:
        void SaveXML(XMLWriter &xmlFile) override;
        SimpleBlockFile(QFileInfo &&baseFileName,
                        samplePtr sampleData, size_t sampleLen,
                        sampleFormat format,
                        bool allowDeferredWrite = false,
                        bool bypassCache = false );
        size_t ReadData(samplePtr data, sampleFormat format,
                                size_t start, size_t len, bool mayThrow) const override;
    private:
        mutable sampleFormat mFormat;
    protected:
        SimpleBlockFileCache mCache;
        static bool GetCache();
        bool WriteSimpleBlockFile(samplePtr sampleData, size_t sampleLen,
                                     sampleFormat format, void* summaryData);
    };
}

#endif
