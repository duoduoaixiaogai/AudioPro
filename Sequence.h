﻿#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "XMLTagHandler.h"
#include "SampleFormat.h"

#include <vector>

namespace RF {

    class DirManager;

    class BlockFile;
    using BlockFilePtr = std::shared_ptr<BlockFile>;

    class SeqBlock {
    public:
        BlockFilePtr f;
        ///the sample in the global wavetrack that this block starts at.
        sampleCount start;

        SeqBlock()
            : f{}, start(0)
        {}

        SeqBlock(const BlockFilePtr &f_, sampleCount start_)
            : f(f_), start(start_)
        {}

        // Construct a SeqBlock with changed start, same file
        SeqBlock Plus(sampleCount delta) const
        {
            return SeqBlock(f, start + delta);
        }
    };

    class BlockArray : public std::vector<SeqBlock> {};

    class Sequence final : public XMLTagHandler {
    public:
        size_t GetMaxBlockSize() const;
        size_t GetIdealAppendLen() const;
        sampleFormat GetSampleFormat() const;
        void Append(samplePtr buffer, sampleFormat format, size_t len,
                    XMLWriter* blockFileLog=NULL);

        Sequence(const std::shared_ptr<DirManager> &projDirManager, sampleFormat format);
        size_t GetIdealBlockSize() const;
        sampleCount GetNumSamples() const { return mNumSamples; }
    private:
        static bool Read(samplePtr buffer, sampleFormat format,
                         const SeqBlock &b,
                         size_t blockRelativeStart, size_t len, bool mayThrow);
        void AppendBlocksIfConsistent
        (BlockArray &additionalBlocks, bool replaceLast,
         sampleCount numSamples, const QString whereStr);
        static void ConsistencyCheck
        (const BlockArray &block, size_t maxSamples, size_t from,
         sampleCount numSamples, const QString whereStr,
         bool mayThrow = true);
    private:

        static size_t    sMaxDiskBlockSize;

        std::shared_ptr<DirManager> mDirManager;
        sampleFormat  mSampleFormat;
        size_t   mMinSamples;
        size_t   mMaxSamples;

        BlockArray    mBlock;
        sampleCount   mNumSamples{ 0 };
    };
}

#endif
