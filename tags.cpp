#include "tags.h"

namespace Renfeng {
  bool Tags::HasTag(const QString & name) const
  {
     QString key = name;
     key = key.toUpper();

     auto iter = mXref.find(key);
     return (iter != mXref.end());
  }

  QString Tags::GetTag(const QString & name) const
  {
     QString key = name;
     key = key.toUpper();

     auto iter = mXref.find(key);

     if (iter == mXref.end()) {
        return QString("");
     }

     auto iter2 = mMap.find(iter->second);
     if (iter2 == mMap.end()) {
//        wxASSERT(false);
        return QString("");
     }
     else
        return iter2->second;
  }
}
