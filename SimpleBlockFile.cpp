#include "SimpleBlockFile.h"

namespace RF {
    SimpleBlockFile::SimpleBlockFile(QFileInfo &&baseFileName,
                                     samplePtr sampleData, size_t sampleLen,
                                     sampleFormat format,
                                     bool allowDeferredWrite,
                                     bool bypassCache)/*:
       BlockFile {
          (baseFileName.SetExt(wxT("au")), std::move(baseFileName)),
          sampleLen
       }*/
    {
       mFormat = format;

       mCache.active = false;

       bool useCache = GetCache() && (!bypassCache);

       if (!(allowDeferredWrite && useCache) && !bypassCache)
       {
          bool bSuccess = WriteSimpleBlockFile(sampleData, sampleLen, format, NULL);
          if (!bSuccess)
             throw FileException{
                FileException::Cause::Write, GetFileName().name };
       }

       if (useCache) {
          //wxLogDebug("SimpleBlockFile::SimpleBlockFile(): Caching block file data.");
          mCache.active = true;
          mCache.needWrite = true;
          mCache.format = format;
          const auto sampleDataSize = sampleLen * SAMPLE_SIZE(format);
          mCache.sampleData.reinit(sampleDataSize);
          memcpy(mCache.sampleData.get(), sampleData, sampleDataSize);
          ArrayOf<char> cleanup;
          void* summaryData = BlockFile::CalcSummary(sampleData, sampleLen,
                                                    format, cleanup);
          mCache.summaryData.reinit(mSummaryInfo.totalSummaryBytes);
          memcpy(mCache.summaryData.get(), summaryData,
                 mSummaryInfo.totalSummaryBytes);
        }
    }
}
