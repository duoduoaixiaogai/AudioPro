#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "XMLTagHandler.h"
#include "SampleFormat.h"

#include <vector>

namespace Renfeng {

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
        std::pair<float, float> GetMinMax(
              sampleCount start, sampleCount len, bool mayThrow) const;
        bool Get(samplePtr buffer, sampleFormat format,
                    sampleCount start, size_t len, bool mayThrow) const;
        void SetSamples(samplePtr buffer, sampleFormat format,
                    sampleCount start, sampleCount len);
        sampleCount GetBlockStart(sampleCount position) const;
        size_t GetBestBlockSize(sampleCount start) const;
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
        int FindBlock(sampleCount pos) const;
        bool Get(int b, samplePtr buffer, sampleFormat format,
              sampleCount start, size_t len, bool mayThrow) const;
        void CommitChangesIfConsistent
              (BlockArray &newBlock, sampleCount numSamples, const char *whereStr);
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
