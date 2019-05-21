#ifndef TAGS_H
#define TAGS_H

#include <QString>

#include <map>

namespace Renfeng {

  using TagMap = std::map< QString, QString >;

#define TAG_TITLE     QString("TITLE")
#define TAG_ARTIST   QString("ARTIST")
#define TAG_ALBUM    QString("ALBUM")
#define TAG_TRACK    QString("TRACKNUMBER")
#define TAG_YEAR     QString("YEAR")
#define TAG_GENRE    QString("GENRE")
#define TAG_COMMENTS QString("COMMENTS")
#define TAG_SOFTWARE QString("Software")
#define TAG_COPYRIGHT QString("Copyright")

  class Tags {
  public:
      bool HasTag(const QString & name) const;
      QString GetTag(const QString & name) const;
  private:
      TagMap mXref;
      TagMap mMap;
  };
}

#endif // TAGS_H
