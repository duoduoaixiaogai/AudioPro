#ifndef DIRMANAGER_H
#define DIRMANAGER_H

#include "types.h"

#include "XMLTagHandler.h"

#include <unordered_map>

#include <QFileInfo>

namespace RF{

    class BlockFile;
    using BlockFilePtr = std::shared_ptr<BlockFile>;
    using DirHash = std::unordered_map<int, int>;
    using BlockHash = std::unordered_map< QString, std::weak_ptr<BlockFile> >;

    class DirManager final : public XMLTagHandler {
    public:

        DirManager();

        virtual ~DirManager();

        BlockFilePtr
        NewSimpleBlockFile(samplePtr sampleData,
                           size_t sampleLen,
                           sampleFormat format,
                           bool allowDeferredWrite = false);
    private:
        struct BalanceInfo
        {
            DirHash   dirTopPool;    // available toplevel dirs
            DirHash   dirTopFull;    // full toplevel dirs
            DirHash   dirMidPool;    // available two-level dirs
            DirHash   dirMidFull;    // full two-level dirs
        } mBalanceInfo;
        QFileInfo MakeBlockFileName();
        BalanceInfo &GetBalanceInfo();
        unsigned long mLastBlockFileDestructionCount { 0 };
        BlockHash mBlockFileHash;
        void BalanceInfoDel(const QString&);
    };
}

#endif
