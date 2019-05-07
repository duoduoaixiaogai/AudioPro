#include "SimpleBlockFile.h"
#include "SampleFormat.h"

namespace RF {
    SimpleBlockFile::SimpleBlockFile(QFileInfo &&baseFileName,
                                     samplePtr sampleData, size_t sampleLen,
                                     sampleFormat format,
                                     bool allowDeferredWrite,
                                     bool bypassCache):
        BlockFile {
            (baseFileName.setFile(baseFileName.absoluteFilePath() + QString(".au")), std::move(baseFileName)),
            sampleLen
}
    {
        mFormat = format;

        mCache.active = false;

        bool useCache = GetCache() && (!bypassCache);

        if (!(allowDeferredWrite && useCache) && !bypassCache)
        {
            bool bSuccess = WriteSimpleBlockFile(sampleData, sampleLen, format, NULL);
            //错误没有处理完毕
            //          if (!bSuccess)
            //             throw FileException{
            //                FileException::Cause::Write, GetFileName().name };
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

    bool SimpleBlockFile::GetCache()
    {
#ifdef DEPRECATED_AUDIO_CACHE
        // See http://bugzilla.audacityteam.org/show_bug.cgi?id=545.
        bool cacheBlockFiles = false;
        gPrefs->Read(wxT("/Directories/CacheBlockFiles"), &cacheBlockFiles);
        if (!cacheBlockFiles)
            return false;

        int lowMem = gPrefs->Read(wxT("/Directories/CacheLowMem"), 16l);
        if (lowMem < 16) {
            lowMem = 16;
        }
        lowMem <<= 20;
        return (GetFreeMemory() > lowMem);
#else
        return false;
#endif
    }

    bool SimpleBlockFile::WriteSimpleBlockFile(
            samplePtr sampleData,
            size_t sampleLen,
            sampleFormat format,
            void* summaryData)
    {
        QString name = mFileName.absoluteFilePath();
        QFile file(name);
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        auHeader header;

        // AU files can be either big or little endian.  Which it is is
        // determined implicitly by the byte-order of the magic 0x2e736e64
        // (.snd).  We want it to be native-endian, so we write the magic
        // to memory and then let it write that to a file in native
        // endianness
        header.magic = 0x2e736e64;

        // We store the summary data at the end of the header, so the data
        // offset is the length of the summary data plus the length of the header
        header.dataOffset = sizeof(auHeader) + mSummaryInfo.totalSummaryBytes;

        // dataSize is optional, and we opt out
        header.dataSize = 0xffffffff;

        switch(format) {
            case int16Sample:
                header.encoding = AU_SAMPLE_FORMAT_16;
                break;

            case int24Sample:
                header.encoding = AU_SAMPLE_FORMAT_24;
                break;

            case floatSample:
                header.encoding = AU_SAMPLE_FORMAT_FLOAT;
                break;
        }

        // TODO: don't fabricate
        header.sampleRate = 44100;

        // BlockFiles are always mono
        header.channels = 1;

        // Write the file
        ArrayOf<char> cleanup;
        if (!summaryData)
            summaryData = /*BlockFile::*/CalcSummary(sampleData, sampleLen, format, cleanup);
        //mchinen:allowing virtual override of calc summary for ODDecodeBlockFile.
        // PRL: cleanup fixes a possible memory leak!

        size_t nBytesToWrite = sizeof(header);
        size_t nBytesWritten = file.write(reinterpret_cast<const char*>(&header), nBytesToWrite);
        if (nBytesWritten != nBytesToWrite)
        {
            //          wxLogDebug(wxT("Wrote %lld bytes, expected %lld."), (long long) nBytesWritten, (long long) nBytesToWrite);
            return false;
        }

        nBytesToWrite = mSummaryInfo.totalSummaryBytes;
        nBytesWritten = file.write(static_cast<const char*>(summaryData), nBytesToWrite);
        if (nBytesWritten != nBytesToWrite)
        {
            //          wxLogDebug(wxT("Wrote %lld bytes, expected %lld."), (long long) nBytesWritten, (long long) nBytesToWrite);
            return false;
        }

        if( format == int24Sample )
        {
            // we can't write the buffer directly to disk, because 24-bit samples
            // on disk need to be packed, not padded to 32 bits like they are in
            // memory
            int *int24sampleData = (int*)sampleData;

            for( size_t i = 0; i < sampleLen; i++ )
            {
                nBytesToWrite = 3;
                nBytesWritten =
        #if wxBYTE_ORDER == wxBIG_ENDIAN
                        file.write((char*)&int24sampleData[i] + 1, nBytesToWrite);
#else
                        file.write((char*)&int24sampleData[i], nBytesToWrite);
#endif
                if (nBytesWritten != nBytesToWrite)
                {
                    //                wxLogDebug(wxT("Wrote %lld bytes, expected %lld."), (long long) nBytesWritten, (long long) nBytesToWrite);
                    return false;
                }
            }
        }
        else
        {
            // for all other sample formats we can write straight from the buffer
            // to disk
            nBytesToWrite = sampleLen * SAMPLE_SIZE(format);
            nBytesWritten = file.write(sampleData, nBytesToWrite);
            if (nBytesWritten != nBytesToWrite)
            {
                //             wxLogDebug(wxT("Wrote %lld bytes, expected %lld."), (long long) nBytesWritten, (long long) nBytesToWrite);
                return false;
            }
        }

        return true;
    }

    size_t SimpleBlockFile::ReadData(samplePtr data, sampleFormat format,
                                     size_t start, size_t len, bool mayThrow) const
    {
        if (mCache.active)
        {
            //wxLogDebug("SimpleBlockFile::ReadData(): Data are already in cache.");

            auto framesRead = std::min(len, std::max(start, mLen) - start);
            CopySamples(
                        (samplePtr)(mCache.sampleData.get() +
                                    start * SAMPLE_SIZE(mCache.format)),
                        mCache.format, data, format, framesRead);

            if ( framesRead < len ) {
                //             if (mayThrow)
                //                // Not the best exception class?
                //                throw FileException{ FileException::Cause::Read, mFileName };
                ClearSamples(data, format, framesRead, len - framesRead);
            }

            return framesRead;
        }
        else
            return CommonReadData( mayThrow,
                                   mFileName, mSilentLog, nullptr, 0, 0, data, format, start, len);
    }

    void SimpleBlockFile::SaveXML(XMLWriter &xmlFile) {

    }
}
