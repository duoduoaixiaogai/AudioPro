#ifndef DIRMANAGER_H
#define DIRMANAGER_H

#include "types.h"

#include "XMLTagHandler.h"

#include <unordered_map>

#include <QFileInfo>

namespace Renfeng{

    class BlockFile;
    using BlockFilePtr = std::shared_ptr<BlockFile>;
    using DirHash = std::unordered_map<int, int>;
    using BlockHash = std::unordered_map< std::string, std::weak_ptr<BlockFile> >;

    class DirManager final : public XMLTagHandler {
    public:

        DirManager();

        virtual ~DirManager();

        BlockFilePtr
        NewSimpleBlockFile(samplePtr sampleData,
                           size_t sampleLen,
                           sampleFormat format,
                           bool allowDeferredWrite = false);
        bool AssignFile(QFileInfo &filename, const QString &value, bool check);
        bool ContainsBlockFile(const QString &filepath) const;
        QString GetDataFilesDir() const;
        static void SetTempDir(const QString &_temp) { globaltemp = _temp; }
    private:
        struct BalanceInfo
        {
            DirHash   dirTopPool;    // available toplevel dirs
            DirHash   dirTopFull;    // full toplevel dirs
            DirHash   dirMidPool;    // available two-level dirs
            DirHash   dirMidFull;    // full two-level dirs
        } mBalanceInfo;
        QString projFull;
        QString mytemp;
        static QString globaltemp;
        QFileInfo MakeBlockFileName();
        BalanceInfo &GetBalanceInfo();
        unsigned long mLastBlockFileDestructionCount { 0 };
        BlockHash mBlockFileHash;
        void BalanceInfoDel(const QString&);
        int BalanceMidAdd(int, int);
        QDir MakeBlockFilePath(const QString &value);
        void BalanceFileAdd(int);
        bool deleteDirectory(const QString &path);
    };
}

#endif
