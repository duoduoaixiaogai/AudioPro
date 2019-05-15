#include "DirManager.h"
#include "SimpleBlockFile.h"

#include <QDir>

namespace Renfeng {

    QString DirManager::globaltemp;

    DirManager::DirManager()
    {
       mLastBlockFileDestructionCount = BlockFile::gBlockFileDestructionCount;

       // Seed the random number generator.
       // this need not be strictly uniform or random, but it should give
       // unclustered numbers
       srand(time(NULL));

       // Set up local temp subdir
       // Previously, Audacity just named project temp directories "project0",
       // "project1" and so on. But with the advent of recovery code, we need a
       // unique name even after a crash. So we create a random project index
       // and make sure it is not used already. This will not pose any performance
       // penalties as long as the number of open Audacity projects is much
       // lower than RAND_MAX.
       do {
          mytemp = globaltemp + QString("//") +
                    QString("project%1").arg(rand());
       } while (QDir(mytemp).exists());

//       numDirManagers++;
//
//       projPath = wxT("");
//       projName = wxT("");
//
//       mLoadingTarget = NULL;
//       mLoadingTargetIdx = 0;
//       mMaxSamples = ~size_t(0);

       // toplevel pool hash is fully populated to begin
       {
          // We can bypass the accessor function while initializing
          auto &balanceInfo = mBalanceInfo;
          auto &dirTopPool = balanceInfo.dirTopPool;
          for(int i = 0; i < 256; ++i)
             dirTopPool[i] = 0;
       }

       // Make sure there is plenty of space for temp files
//       wxLongLong freeSpace = 0;
//       if (wxGetDiskSpace(globaltemp, NULL, &freeSpace)) {
//          if (freeSpace < wxLongLong(wxLL(100 * 1048576))) {
//             ShowWarningDialog(NULL, wxT("DiskSpaceWarning"),
//                               _("There is very little free disk space left on this volume.\nPlease select another temporary directory in Preferences."));
//          }
//       }
    }

    DirManager::~DirManager()
    {
//       numDirManagers--;
//       if (numDirManagers == 0) {
//          CleanTempDir();
//          //::wxRmdir(temp);
//       } else if( projFull.IsEmpty() && !mytemp.IsEmpty()) {
//          CleanDir(mytemp, wxEmptyString, ".DS_Store", _("Cleaning project temporary files"), kCleanTopDirToo | kCleanDirsOnlyIfEmpty );
//       }
    }

    BlockFilePtr DirManager::NewSimpleBlockFile(
            samplePtr sampleData, size_t sampleLen,
            sampleFormat format,
            bool allowDeferredWrite)
    {
        QFileInfo filePath{ MakeBlockFileName() };
        const QString fileName{ filePath.baseName() };

        auto newBlockFile = make_blockfile<SimpleBlockFile>
                (std::move(filePath), sampleData, sampleLen, format, allowDeferredWrite);

        mBlockFileHash[fileName.toStdString()] = newBlockFile;

        return newBlockFile;
    }

    static inline unsigned int hexchar_to_int(unsigned int x)
    {
        if(x<48U)return 0;
        if(x<58U)return x-48U;
        if(x<65U)return 10U;
        if(x<71U)return x-55U;
        if(x<97U)return 10U;
        if(x<103U)return x-87U;
        return 15U;
    }

    void DirManager::BalanceInfoDel(const QString &file)
    {
        // do not use GetBalanceInfo(),
        // rather this function will be called from there.
        auto &balanceInfo = mBalanceInfo;
        auto &dirMidPool = balanceInfo.dirMidPool;
        auto &dirMidFull = balanceInfo.dirMidFull;
        auto &dirTopPool = balanceInfo.dirTopPool;
        auto &dirTopFull = balanceInfo.dirTopFull;

        const QChar *s = file.unicode();
        if(s[0]== QChar('e')){
            // this is one of the modern two-deep managed files

            unsigned int topnum = (hexchar_to_int(s[1].unicode()) << 4) |
                    hexchar_to_int(s[2].unicode());
            unsigned int midnum = (hexchar_to_int(s[3].unicode()) << 4) |
                    hexchar_to_int(s[4].unicode());
            unsigned int midkey=topnum<<8|midnum;

            // look for midkey in the mid pool
            if(dirMidFull.find(midkey) != dirMidFull.end()){
                // in the full pool

                if(--dirMidFull[midkey]<256){
                    // move out of full into available
                    dirMidPool[midkey]=dirMidFull[midkey];
                    dirMidFull.erase(midkey);
                }
            }else{
                if(--dirMidPool[midkey]<1){
                    // erasing the key here is OK; we have provision to add it
                    // back if its needed (unlike the dirTopPool hash)
                    dirMidPool.erase(midkey);

                    // DELETE the actual directory
                    QString dir=(projFull != QString("")? projFull: mytemp);
                    dir += QString('\\');
                    dir += file.mid(0,3);
                    dir += QString('\\');
                    dir += QString("d");
                    dir += file.mid(3,2);
                    deleteDirectory(dir);

                    // also need to remove from toplevel
                    if(dirTopFull.find(topnum) != dirTopFull.end()){
                        // in the full pool
                        if(--dirTopFull[topnum]<256){
                            // move out of full into available
                            dirTopPool[topnum]=dirTopFull[topnum];
                            dirTopFull.erase(topnum);
                        }
                    }else{
                        if(--dirTopPool[topnum]<1){
                            // do *not* erase the hash entry from dirTopPool
                            // *do* DELETE the actual directory
                            dir=(projFull != QString("")? projFull: mytemp);
                            dir += QString('\\');
                            dir += file.mid(0,3);
                            deleteDirectory(dir);
                        }
                    }
                }
            }
        }
    }

    auto DirManager::GetBalanceInfo() -> BalanceInfo &
    {
        // Before returning the map,
        // see whether any block files have disappeared,
        // and if so update

        auto count = BlockFile::gBlockFileDestructionCount;
        if ( mLastBlockFileDestructionCount != count ) {
            auto it = mBlockFileHash.begin(), end = mBlockFileHash.end();
            while (it != end)
            {
                BlockFilePtr ptr { it->second.lock() };
                if (!ptr) {
                    auto name = it->first;
                    mBlockFileHash.erase( it++ );
                    BalanceInfoDel( name.c_str() );
                }
                else
                    ++it;
            }
        }

        mLastBlockFileDestructionCount = count;

        return mBalanceInfo;
    }

    QFileInfo DirManager::MakeBlockFileName()
    {
        auto &balanceInfo = GetBalanceInfo();
        auto &dirMidPool = balanceInfo.dirMidPool;
        auto &dirTopPool = balanceInfo.dirTopPool;
        auto &dirTopFull = balanceInfo.dirTopFull;

        QFileInfo ret;
        QString baseFileName;

        unsigned int filenum,midnum,topnum,midkey;

        while(1){

            /* blockfiles are divided up into heirarchical directories.
                     Each toplevel directory is represented by "e" + two unique
                     hexadecimal digits, for a total possible number of 256
                     toplevels.  Each toplevel contains up to 256 subdirs named
                     "d" + two hex digits.  Each subdir contains 'a number' of
                     files.  */

            filenum=0;
            midnum=0;
            topnum=0;

            // first action: if there is no available two-level directory in
            // the available pool, try to make one

            if(dirMidPool.empty()){

                // is there a toplevel directory with space for a NEW subdir?

                if(!dirTopPool.empty()){

                    // there's still a toplevel with room for a subdir

                    DirHash::iterator iter = dirTopPool.begin();
                    int newcount           = 0;
                    topnum                 = iter->first;


                    // search for unused midlevels; linear search adequate
                    // add 32 NEW topnum/midnum dirs full of  prospective filenames to midpool
                    for(midnum=0;midnum<256;midnum++){
                        midkey=(topnum<<8)+midnum;
                        if(BalanceMidAdd(topnum,midkey)){
                            newcount++;
                            if(newcount>=32)break;
                        }
                    }

                    if(dirMidPool.empty()){
                        // all the midlevels in this toplevel are in use yet the
                        // toplevel claims some are free; this implies multiple
                        // internal logic faults, but simply giving up and going
                        // into an infinite loop isn't acceptible.  Just in case,
                        // for some reason, we get here, dynamite this toplevel so
                        // we don't just fail.

                        // this is 'wrong', but the best we can do given that
                        // something else is also wrong.  It will contain the
                        // problem so we can keep going without worry.
                        dirTopPool.erase(topnum);
                        dirTopFull[topnum]=256;
                    }
                    continue;
                }
            }

            if(dirMidPool.empty()){
                // still empty, thus an absurdly large project; all dirs are
                // full to 256/256/256; keep working, but fall back to 'big
                // filenames' and randomized placement

                filenum = rand();
                midnum  = (int)(256.*rand()/(RAND_MAX+1.));
                topnum  = (int)(256.*rand()/(RAND_MAX+1.));
                midkey=(topnum<<8)+midnum;


            }else{

                DirHash::iterator iter = dirMidPool.begin();
                midkey                 = iter->first;

                // split the retrieved 16 bit directory key into two 8 bit numbers
                topnum = midkey >> 8;
                midnum = midkey & 0xff;
                filenum = (int)(4096.*rand()/(RAND_MAX+1.));

            }

            baseFileName = QString("e%1%2%3").arg(topnum, 2, 10, QLatin1Char('0')).
                    arg(midnum, 2, 10, QLatin1Char('0')).arg(filenum, 3, 10, QLatin1Char('0'));

            if (!ContainsBlockFile(baseFileName)) {
                // not in the hash, good.
                if (!this->AssignFile(ret, baseFileName, true))
                {
                    // this indicates an on-disk collision, likely due to an
                    // orphan blockfile.  We should try again, but first
                    // alert the balancing info there's a phantom file here;
                    // if the directory is nearly full of orphans we neither
                    // want performance to suffer nor potentially get into an
                    // infinite loop if all possible filenames are taken by
                    // orphans (unlikely but possible)
                    BalanceFileAdd(midkey);

                }else break;
            }
        }
        // FIXME: Might we get here without midkey having been set?
        //    Seemed like a possible problem in these changes in .aup directory hierarchy.
        BalanceFileAdd(midkey);
        return ret;
    }

    int DirManager::BalanceMidAdd(int topnum, int midkey)
    {
        // enter the midlevel directory if it doesn't exist

        auto &balanceInfo = GetBalanceInfo();
        auto &dirMidPool = balanceInfo.dirMidPool;
        auto &dirMidFull = balanceInfo.dirMidFull;
        auto &dirTopPool = balanceInfo.dirTopPool;
        auto &dirTopFull = balanceInfo.dirTopFull;

        if(dirMidPool.find(midkey) == dirMidPool.end() &&
                dirMidFull.find(midkey) == dirMidFull.end()){
            dirMidPool[midkey]=0;

            // increment toplevel directory fill
            dirTopPool[topnum]++;
            if(dirTopPool[topnum]>=256){
                // this toplevel is now full; move it to the full hash
                dirTopPool.erase(topnum);
                dirTopFull[topnum]=256;
            }
            return 1;
        }
        return 0;
    }

    bool DirManager::AssignFile(QFileInfo &fileName,
                                const QString &value,
                                bool diskcheck)
    {
        QDir dir{ MakeBlockFilePath(value) };

        if(diskcheck){
            // verify that there's no possible collision on disk.  If there
            // is, log the problem and return FALSE so that MakeBlockFileName
            // can try again

            QDir checkit(dir.absolutePath());
//            if(!checkit.IsOpened()) return FALSE;

            // this code is only valid if 'value' has no extention; that
            // means, effectively, AssignFile may be called with 'diskcheck'
            // set to true only if called from MakeFileBlockName().

            QString filespec = QString("%1.*").arg(value);
//            filespec.Printf(wxT("%s.*"),value);
           QStringList files = checkit.entryList(QStringList(filespec));
            if(!files.isEmpty()){
                // collision with on-disk state!
//                wxString collision;
//                checkit.GetFirst(&collision,filespec);
//
//                wxLogWarning(_("Audacity found an orphan block file: %s. \nPlease consider saving and reloading the project to perform a complete project check."),
//                             collision);

                return false;
            }
        }
//        fileName.Assign(dir.GetFullPath(),value);
//        return fileName.IsOk();
        fileName.setFile(dir.absolutePath() + QString('/') + value);
        return true;
    }

    QDir DirManager::MakeBlockFilePath(const QString &value) {
        QDir dir(GetDataFilesDir());

        if(value.at(0)==QChar('d')){
            // this file is located in a subdirectory tree
            int location=value.indexOf(('b'));
            QString subdir=value.mid(0,location);
            dir.setPath(subdir);

            if(!dir.exists())
                dir.mkpath(dir.path());
        }

        if(value.at(0)==QChar('e')){
            // this file is located in a NEW style two-deep subdirectory tree
            QString topdir=value.mid(0,3);
            QString middir=QString("d");
            middir.append(value.mid(3,2));

            QString subPath = QString('/') + topdir + QString("/") + middir;
            dir.setPath(dir.absolutePath() + subPath);

            if(!dir.exists() && !dir.mkpath(dir.path()))
            { // need braces to avoid compiler warning about ambiguous else, see the macro
//                wxLogSysError(_("mkdir in DirManager::MakeBlockFilePath failed."));
                 // 暂时没有处理错误，只是加一条语句用于进入循环
                // test
                int a = 10;
            }
        }
        return dir;
    }

    bool DirManager::ContainsBlockFile(const QString &filepath) const
    {
        // check what the hash returns in case the blockfile is from a different project
        BlockHash::const_iterator it = mBlockFileHash.find(filepath.toStdString());
        return it != mBlockFileHash.end() &&
                BlockFilePtr{ it->second.lock() };
    }

    void DirManager::BalanceFileAdd(int midkey)
    {
        auto &balanceInfo = GetBalanceInfo();
        auto &dirMidPool = balanceInfo.dirMidPool;
        auto &dirMidFull = balanceInfo.dirMidFull;

        // increment the midlevel directory usage information
        if(dirMidPool.find(midkey) != dirMidPool.end()){
            dirMidPool[midkey]++;
            if(dirMidPool[midkey]>=256){
                // this middir is now full; move it to the full hash
                dirMidPool.erase(midkey);
                dirMidFull[midkey]=256;
            }
        }else{
            // this case only triggers in absurdly large projects; we still
            // need to track directory fill even if we're over 256/256/256
            dirMidPool[midkey]++;
        }
    }

    bool DirManager::deleteDirectory(const QString &path)
    {
        if (path.isEmpty())
        {
            return false;
        }

        QDir dir(path);
        if(!dir.exists())
        {
            return true;
        }

        dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        QFileInfoList fileList = dir.entryInfoList();
        foreach (QFileInfo fi, fileList)
        {
            if (fi.isFile())
            {
                fi.dir().remove(fi.fileName());
            }
            else
            {
                deleteDirectory(fi.absoluteFilePath());
            }
        }
        return dir.rmpath(dir.absolutePath());
    }

    QString DirManager::GetDataFilesDir() const
    {
        return projFull != QString("")? projFull: mytemp;
    }
}
