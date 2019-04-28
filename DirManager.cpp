#include "DirManager.h"
#include "BlockFile.h"

namespace RF {
    BlockFilePtr DirManager::NewSimpleBlockFile(
            samplePtr sampleData, size_t sampleLen,
            sampleFormat format,
            bool allowDeferredWrite)
    {
        //       wxFileNameWrapper filePath{ MakeBlockFileName() };
        //       const wxString fileName{ filePath.GetName() };
        //
        //       auto newBlockFile = make_blockfile<SimpleBlockFile>
        //          (std::move(filePath), sampleData, sampleLen, format, allowDeferredWrite);
        //
        //       mBlockFileHash[fileName] = newBlockFile;
        //
        //       return newBlockFile;
        return nullptr;
    }

    void DirManager::BalanceInfoDel(const QString &file)
    {
        // do not use GetBalanceInfo(),
        // rather this function will be called from there.
        //       auto &balanceInfo = mBalanceInfo;
        //       auto &dirMidPool = balanceInfo.dirMidPool;
        //       auto &dirMidFull = balanceInfo.dirMidFull;
        //       auto &dirTopPool = balanceInfo.dirTopPool;
        //       auto &dirTopFull = balanceInfo.dirTopFull;
        //
        //       const wxChar *s=file;
        //       if(s[0]==wxT('e')){
        //          // this is one of the modern two-deep managed files
        //
        //          unsigned int topnum = (hexchar_to_int(s[1]) << 4) |
        //             hexchar_to_int(s[2]);
        //          unsigned int midnum = (hexchar_to_int(s[3]) << 4) |
        //             hexchar_to_int(s[4]);
        //          unsigned int midkey=topnum<<8|midnum;
        //
        //          // look for midkey in the mid pool
        //          if(dirMidFull.find(midkey) != dirMidFull.end()){
        //             // in the full pool
        //
        //             if(--dirMidFull[midkey]<256){
        //                // move out of full into available
        //                dirMidPool[midkey]=dirMidFull[midkey];
        //                dirMidFull.erase(midkey);
        //             }
        //          }else{
        //             if(--dirMidPool[midkey]<1){
        //                // erasing the key here is OK; we have provision to add it
        //                // back if its needed (unlike the dirTopPool hash)
        //                dirMidPool.erase(midkey);
        //
        //                // DELETE the actual directory
        //                wxString dir=(projFull != wxT("")? projFull: mytemp);
        //                dir += wxFILE_SEP_PATH;
        //                dir += file.Mid(0,3);
        //                dir += wxFILE_SEP_PATH;
        //                dir += wxT("d");
        //                dir += file.Mid(3,2);
        //                wxFileName::Rmdir(dir);
        //
        //                // also need to remove from toplevel
        //                if(dirTopFull.find(topnum) != dirTopFull.end()){
        //                   // in the full pool
        //                   if(--dirTopFull[topnum]<256){
        //                      // move out of full into available
        //                      dirTopPool[topnum]=dirTopFull[topnum];
        //                      dirTopFull.erase(topnum);
        //                   }
        //                }else{
        //                   if(--dirTopPool[topnum]<1){
        //                      // do *not* erase the hash entry from dirTopPool
        //                      // *do* DELETE the actual directory
        //                      dir=(projFull != wxT("")? projFull: mytemp);
        //                      dir += wxFILE_SEP_PATH;
        //                      dir += file.Mid(0,3);
        //                      wxFileName::Rmdir(dir);
        //                   }
        //                }
        //             }
        //          }
        //       }
    }

    auto DirManager::GetBalanceInfo() -> BalanceInfo &
    {
        // Before returning the map,
        // see whether any block files have disappeared,
        // and if so update

        //       auto count = BlockFile::gBlockFileDestructionCount;
        //       if ( mLastBlockFileDestructionCount != count ) {
        //          auto it = mBlockFileHash.begin(), end = mBlockFileHash.end();
        //          while (it != end)
        //          {
        //             BlockFilePtr ptr { it->second.lock() };
        //             if (!ptr) {
        //                auto name = it->first;
        //                mBlockFileHash.erase( it++ );
        //                BalanceInfoDel( name );
        //             }
        //             else
        //                ++it;
        //          }
        //       }
        //
        //       mLastBlockFileDestructionCount = count;
        //
        //       return mBalanceInfo;
        BalanceInfo b;
        return b;
    }

    QFileInfo DirManager::MakeBlockFileName()
    {
        //       auto &balanceInfo = GetBalanceInfo();
        //       auto &dirMidPool = balanceInfo.dirMidPool;
        //       auto &dirTopPool = balanceInfo.dirTopPool;
        //       auto &dirTopFull = balanceInfo.dirTopFull;
        //
        //       wxFileNameWrapper ret;
        //       wxString baseFileName;
        //
        //       unsigned int filenum,midnum,topnum,midkey;
        //
        //       while(1){
        //
        //          /* blockfiles are divided up into heirarchical directories.
        //             Each toplevel directory is represented by "e" + two unique
        //             hexadecimal digits, for a total possible number of 256
        //             toplevels.  Each toplevel contains up to 256 subdirs named
        //             "d" + two hex digits.  Each subdir contains 'a number' of
        //             files.  */
        //
        //          filenum=0;
        //          midnum=0;
        //          topnum=0;
        //
        //          // first action: if there is no available two-level directory in
        //          // the available pool, try to make one
        //
        //          if(dirMidPool.empty()){
        //
        //             // is there a toplevel directory with space for a NEW subdir?
        //
        //             if(!dirTopPool.empty()){
        //
        //                // there's still a toplevel with room for a subdir
        //
        //                DirHash::iterator iter = dirTopPool.begin();
        //                int newcount           = 0;
        //                topnum                 = iter->first;
        //
        //
        //                // search for unused midlevels; linear search adequate
        //                // add 32 NEW topnum/midnum dirs full of  prospective filenames to midpool
        //                for(midnum=0;midnum<256;midnum++){
        //                   midkey=(topnum<<8)+midnum;
        //                   if(BalanceMidAdd(topnum,midkey)){
        //                      newcount++;
        //                      if(newcount>=32)break;
        //                   }
        //                }
        //
        //                if(dirMidPool.empty()){
        //                   // all the midlevels in this toplevel are in use yet the
        //                   // toplevel claims some are free; this implies multiple
        //                   // internal logic faults, but simply giving up and going
        //                   // into an infinite loop isn't acceptible.  Just in case,
        //                   // for some reason, we get here, dynamite this toplevel so
        //                   // we don't just fail.
        //
        //                   // this is 'wrong', but the best we can do given that
        //                   // something else is also wrong.  It will contain the
        //                   // problem so we can keep going without worry.
        //                   dirTopPool.erase(topnum);
        //                   dirTopFull[topnum]=256;
        //                }
        //                continue;
        //             }
        //          }
        //
        //          if(dirMidPool.empty()){
        //             // still empty, thus an absurdly large project; all dirs are
        //             // full to 256/256/256; keep working, but fall back to 'big
        //             // filenames' and randomized placement
        //
        //             filenum = rand();
        //             midnum  = (int)(256.*rand()/(RAND_MAX+1.));
        //             topnum  = (int)(256.*rand()/(RAND_MAX+1.));
        //             midkey=(topnum<<8)+midnum;
        //
        //
        //          }else{
        //
        //             DirHash::iterator iter = dirMidPool.begin();
        //             midkey                 = iter->first;
        //
        //             // split the retrieved 16 bit directory key into two 8 bit numbers
        //             topnum = midkey >> 8;
        //             midnum = midkey & 0xff;
        //             filenum = (int)(4096.*rand()/(RAND_MAX+1.));
        //
        //          }
        //
        //          baseFileName.Printf(wxT("e%02x%02x%03x"),topnum,midnum,filenum);
        //
        //          if (!ContainsBlockFile(baseFileName)) {
        //             // not in the hash, good.
        //             if (!this->AssignFile(ret, baseFileName, true))
        //             {
        //                // this indicates an on-disk collision, likely due to an
        //                // orphan blockfile.  We should try again, but first
        //                // alert the balancing info there's a phantom file here;
        //                // if the directory is nearly full of orphans we neither
        //                // want performance to suffer nor potentially get into an
        //                // infinite loop if all possible filenames are taken by
        //                // orphans (unlikely but possible)
        //                BalanceFileAdd(midkey);
        //
        //             }else break;
        //          }
        //       }
        //       // FIXME: Might we get here without midkey having been set?
        //       //    Seemed like a possible problem in these changes in .aup directory hierarchy.
        //       BalanceFileAdd(midkey);
        //       return ret;
        return QFileInfo("C:\\niu.txt");
    }


}
