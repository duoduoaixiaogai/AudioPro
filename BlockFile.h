#ifndef BLOCKFILE_H
#define BLOCKFILE_H

#include "types.h"
#include "XMLWriter.h"
#include "memoryx.h"

#include <QFileInfo>

namespace Renfeng {

    class AliasBlockFile;

    template< typename Result, typename... Args >
    inline std::shared_ptr< Result > make_blockfile (Args && ... args)
    {
        return std::make_shared< Result > ( std::forward< Args > ( args )... );
    }

    class SummaryInfo {
    public:
        SummaryInfo(size_t samples);

        int            fields; /* Usually 3 for Min, Max, RMS */
        sampleFormat   format;
        int            bytesPerFrame;
        size_t         frames64K;
        int            offset64K;
        size_t         frames256;
        int            offset256;
        size_t         totalSummaryBytes;
    };

    class BlockFile {
    public:
        struct MinMaxRMS { float min, max, RMS; };
        BlockFile(QFileInfo &&fileName, size_t samples);
        virtual ~BlockFile();
        size_t GetLength() const { return mLen; }
        virtual size_t ReadData(samplePtr data, sampleFormat format,
                                size_t start, size_t len, bool mayThrow = true)
        const = 0;
        virtual void SaveXML(XMLWriter &xmlFile) = 0;
        static unsigned long gBlockFileDestructionCount;
        virtual MinMaxRMS GetMinMaxRMS(bool mayThrow = true) const;
        virtual MinMaxRMS GetMinMaxRMS(size_t start, size_t len,
                                  bool mayThrow = true) const;
    protected:
        virtual void *CalcSummary(samplePtr buffer, size_t len,
                                     sampleFormat format,
                                     // This gets filled, if the caller needs to deallocate.  Else it is null.
                                     ArrayOf<char> &cleanup);
        void CalcSummaryFromBuffer(const float *fbuffer, size_t len,
                                      float *summary256, float *summary64K);
        static size_t CommonReadData(
              bool mayThrow,
              const QFileInfo &fileName, bool &mSilentLog,
              const AliasBlockFile *pAliasFile, sampleCount origin, unsigned channel,
              samplePtr data, sampleFormat format, size_t start, size_t len,
              const sampleFormat *pLegacyFormat = nullptr, size_t legacyLen = 0);
    protected:
        size_t mLen;
        SummaryInfo mSummaryInfo;
        float mMin, mMax, mRMS;
        QFileInfo mFileName;
        mutable bool mSilentLog;
    private:
        static ArrayOf<char> fullSummary;
    };

    using BlockFilePtr = std::shared_ptr<BlockFile>;

    class AliasBlockFile : public BlockFile {

    };
}

#endif
