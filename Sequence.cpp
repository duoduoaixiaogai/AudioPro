﻿#include "Sequence.h"
#include "BlockFile.h"
#include "DirManager.h"
#include "SimpleBlockFile.h"

namespace RF {

    inline bool Overflows(double numSamples)
    {
        return numSamples > 9223372036854775807i64;
    }

    size_t Sequence::sMaxDiskBlockSize = 1048576;

    size_t Sequence::GetMaxBlockSize() const
    {
        return mMaxSamples;
    }

    Sequence::Sequence(const std::shared_ptr<DirManager> &projDirManager, sampleFormat format)
        : mDirManager(projDirManager)
        , mSampleFormat(format)
        , mMinSamples(sMaxDiskBlockSize / SAMPLE_SIZE(mSampleFormat) / 2)
        , mMaxSamples(mMinSamples * 2)
    {
    }

    size_t Sequence::GetIdealAppendLen() const
    {
        int numBlocks = mBlock.size();
        const auto max = GetMaxBlockSize();

        if (numBlocks == 0)
            return max;

        const auto lastBlockLen = mBlock.back().f->GetLength();
        if (lastBlockLen >= max)
            return max;
        else
            return max - lastBlockLen;
    }

    sampleFormat Sequence::GetSampleFormat() const
    {
        return mSampleFormat;
    }

    void Sequence::Append(samplePtr buffer, sampleFormat format,
                          size_t len, XMLWriter* blockFileLog /*=NULL*/)
    // STRONG-GUARANTEE
    {
        if (len == 0)
            return;

        // Quick check to make sure that it doesn't overflow
//        if (Overflows(mNumSamples.as_double() + ((double)len)))
//            THROW_INCONSISTENCY_EXCEPTION;

        BlockArray newBlock;
        sampleCount newNumSamples = mNumSamples;

        // If the last block is not full, we need to add samples to it
        int numBlocks = mBlock.size();
        SeqBlock *pLastBlock;
        decltype(pLastBlock->f->GetLength()) length;
        size_t bufferSize = mMaxSamples;
        SampleBuffer buffer2(bufferSize, mSampleFormat);
        bool replaceLast = false;
        if (numBlocks > 0 &&
                (length =
                 (pLastBlock = &mBlock.back())->f->GetLength()) < mMinSamples) {
            // Enlarge a sub-minimum block at the end
            const SeqBlock &lastBlock = *pLastBlock;
            const auto addLen = std::min(mMaxSamples - length, len);

            Read(buffer2.ptr(), mSampleFormat, lastBlock, 0, length, true);

            CopySamples(buffer,
                        format,
                        buffer2.ptr() + length * SAMPLE_SIZE(mSampleFormat),
                        mSampleFormat,
                        addLen);

            const auto newLastBlockLen = length + addLen;

            SeqBlock newLastBlock(
                        mDirManager->NewSimpleBlockFile(
                            buffer2.ptr(), newLastBlockLen, mSampleFormat,
                            blockFileLog != NULL
                    ),
                        lastBlock.start
                        );

            if (blockFileLog)
                // shouldn't throw, because XMLWriter is not XMLFileWriter
                static_cast< SimpleBlockFile * >( &*newLastBlock.f )
                    ->SaveXML( *blockFileLog );

            newBlock.push_back( newLastBlock );

            len -= addLen;
            newNumSamples += addLen;
            buffer += addLen * SAMPLE_SIZE(format);

            replaceLast = true;
        }
        // Append the rest as NEW blocks
        while (len) {
            const auto idealSamples = GetIdealBlockSize();
            const auto addedLen = std::min(idealSamples, len);
            BlockFilePtr pFile;
            if (format == mSampleFormat) {
                pFile = mDirManager->NewSimpleBlockFile(
                            buffer, addedLen, mSampleFormat, blockFileLog != NULL);
            }
            else {
                CopySamples(buffer, format, buffer2.ptr(), mSampleFormat, addedLen);
                pFile = mDirManager->NewSimpleBlockFile(
                            buffer2.ptr(), addedLen, mSampleFormat, blockFileLog != NULL);
            }

            if (blockFileLog)
                // shouldn't throw, because XMLWriter is not XMLFileWriter
                static_cast< SimpleBlockFile * >( &*pFile )->SaveXML( *blockFileLog );

            newBlock.push_back(SeqBlock(pFile, newNumSamples));

            buffer += addedLen * SAMPLE_SIZE(format);
            newNumSamples += addedLen;
            len -= addedLen;
        }

        AppendBlocksIfConsistent(newBlock, replaceLast,
                                 newNumSamples, "Append");

        // JKC: During generate we use Append again and again.
        // If generating a long sequence this test would give O(n^2)
        // performance - not good!
#ifdef VERY_SLOW_CHECKING
        ConsistencyCheck(wxT("Append"));
#endif
    }

    bool Sequence::Read(samplePtr buffer, sampleFormat format,
                        const SeqBlock &b, size_t blockRelativeStart, size_t len,
                        bool mayThrow)
    {
       const auto &f = b.f;

//       wxASSERT(blockRelativeStart + len <= f->GetLength());

       // Either throws, or of !mayThrow, tells how many were really read
       auto result = f->ReadData(buffer, format, blockRelativeStart, len, mayThrow);

       if (result != len)
       {
//          wxLogWarning(wxT("Expected to read %ld samples, got %ld samples."),
//                       len, result);
          return false;
       }

       return true;
    }

    size_t Sequence::GetIdealBlockSize() const
    {
       return mMaxSamples;
    }

    void Sequence::AppendBlocksIfConsistent
    (BlockArray &additionalBlocks, bool replaceLast,
     sampleCount numSamples, const QString whereStr)
    {
       // Any additional blocks are meant to be appended,
       // replacing the final block if there was one.

       if (additionalBlocks.empty())
          return;

       bool tmpValid = false;
       SeqBlock tmp;

       if ( replaceLast && ! mBlock.empty() ) {
          tmp = mBlock.back(), tmpValid = true;
          mBlock.pop_back();
       }

       auto prevSize = mBlock.size();

       bool consistent = false;
       auto cleanup = finally( [&] {
          if ( !consistent ) {
             mBlock.resize( prevSize );
             if ( tmpValid )
                mBlock.push_back( tmp );
          }
       } );

       std::copy( additionalBlocks.begin(), additionalBlocks.end(),
                  std::back_inserter( mBlock ) );

       // Check consistency only of the blocks that were added,
       // avoiding quadratic time for repeated checking of repeating appends
       ConsistencyCheck( mBlock, mMaxSamples, prevSize, numSamples, whereStr ); // may throw

       // now commit
       // use NOFAIL-GUARANTEE

       mNumSamples = numSamples;
       consistent = true;
    }

    void Sequence::ConsistencyCheck
       (const BlockArray &mBlock, size_t maxSamples, size_t from,
        sampleCount mNumSamples, const QString whereStr,
        bool mayThrow)
    {
//       bool bError = false;
//       // Construction of the exception at the appropriate line of the function
//       // gives a little more discrimination
//       InconsistencyException ex;
//
//       unsigned int numBlocks = mBlock.size();
//
//       unsigned int i;
//       sampleCount pos = from < numBlocks ? mBlock[from].start : mNumSamples;
//       if ( from == 0 && pos != 0 )
//          ex = CONSTRUCT_INCONSISTENCY_EXCEPTION, bError = true;
//
//       for (i = from; !bError && i < numBlocks; i++) {
//          const SeqBlock &seqBlock = mBlock[i];
//          if (pos != seqBlock.start)
//             ex = CONSTRUCT_INCONSISTENCY_EXCEPTION, bError = true;
//
//          if ( seqBlock.f ) {
//             const auto length = seqBlock.f->GetLength();
//             if (length > maxSamples)
//                ex = CONSTRUCT_INCONSISTENCY_EXCEPTION, bError = true;
//             pos += length;
//          }
//          else
//             ex = CONSTRUCT_INCONSISTENCY_EXCEPTION, bError = true;
//       }
//       if ( !bError && pos != mNumSamples )
//          ex = CONSTRUCT_INCONSISTENCY_EXCEPTION, bError = true;
//
//       if ( bError )
//       {
//          wxLogError(wxT("*** Consistency check failed at %d after %s. ***"),
//                     ex.GetLine(), whereStr);
//          wxString str;
//          DebugPrintf(mBlock, mNumSamples, &str);
//          wxLogError(wxT("%s"), str);
//          wxLogError(wxT("*** Please report this error to https://forum.audacityteam.org/. ***\n\n")
//                     wxT("Recommended course of action:\n")
//                     wxT("Undo the failed operation(s), then export or save your work and quit."));
//
//          //if (mayThrow)
//             //throw ex;
//          //else
//             wxASSERT(false);
//       }
    }
}