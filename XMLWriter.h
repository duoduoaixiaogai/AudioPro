/**********************************************************************

  Audacity: A Digital Audio Editor

  XMLWriter.h

  Leland Lucius

**********************************************************************/
#ifndef XMLFILEWRITER_H
#define XMLFILEWRITER_H

#include <vector>
#include <QString>
#include <QStringList>
#include <QFile>
//#include <wx/arrstr.h>
//#include <wx/ffile.h>

//#include "../FileException.h"

///
/// XMLWriter
///
class /*AUDACITY_DLL_API*/ XMLWriter /* not final */ {

 public:

   XMLWriter();
   virtual ~XMLWriter();

//   virtual void StartTag(const QString &name);
//   virtual void EndTag(const QString &name);
//
//   virtual void WriteAttr(const QString &name, const QString &value);
//   virtual void WriteAttr(const QString &name, const QChar *value);
//
//   virtual void WriteAttr(const QString &name, int value);
//   virtual void WriteAttr(const QString &name, bool value);
//   virtual void WriteAttr(const QString &name, long value);
//   virtual void WriteAttr(const QString &name, long long value);
//   virtual void WriteAttr(const QString &name, size_t value);
//   virtual void WriteAttr(const QString &name, float value, int digits = -1);
//   virtual void WriteAttr(const QString &name, double value, int digits = -1);
//
//   virtual void WriteData(const QString &value);
//
//   virtual void WriteSubTree(const QString &value);
//
//   virtual void Write(const QString &data) = 0;
//
//   // Escape a string, replacing certain characters with their
//   // XML encoding, i.e. '<' becomes '&lt;'
//   QString XMLEsc(const QString & s);

 protected:

   bool mInTag;
   int mDepth;
   QStringList mTagstack;
   std::vector<int> mHasKids;

};

///
/// XMLFileWriter
///

/// This writes to a provisional file, and replaces the previously existing
/// contents by a file rename in Commit() only after all writes succeed.
/// The original contents may also be retained at a backup path name, as
/// directed by the optional constructor argument.
/// If it is destroyed before Commit(), then the provisional file is removed.
/// If the construction and all operations are inside a GuardedCall or event
/// handler, then the default delayed handler action in case of exceptions will
/// notify the user of problems.
class /*AUDACITY_DLL_API*/ XMLFileWriter final : private QFile, public XMLWriter {

 public:

   /// The caption is for message boxes to show in case of errors.
   /// Might throw.
   XMLFileWriter
      ( const QString &outputPath, const QString &caption,
        bool keepBackup = false );

   virtual ~XMLFileWriter();

   /// Close all tags and then close the file.
   /// Might throw.  If not, then create
   /// or modify the file at the output path.
   /// Composed of two steps, PreCommit() and PostCommit()
//   void Commit();
//
//   /// Does the part of Commit that might fail because of exhaustion of space
//   void PreCommit();
//
//   /// Does other parts of Commit that are not likely to fail for exhaustion
//   /// of space, but might for other reasons
//   void PostCommit();
//
//   /// Write to file. Might throw.
//   void Write(const QString &data) override;
//
//   QString GetBackupName() const { return mBackupName; }

 private:

//   void ThrowException(
//      const wxFileName &fileName, const QString &caption)
//   {
//      throw FileException{ FileException::Cause::Write, fileName, caption };
//   }

   /// Close file without automatically ending tags.
   /// Might throw.
   void CloseWithoutEndingTags(); // for auto-save files

   const QString mOutputPath;
   const QString mCaption;
   QString mBackupName;
   const bool mKeepBackup;

   QFile mBackupFile;

   bool mCommitted{ false };
};

///
/// XMLStringWriter
///
class XMLStringWriter final : public QString, public XMLWriter {

 public:

//   XMLStringWriter(size_t initialSize = 0);
//   virtual ~XMLStringWriter();
//
//   void Write(const QString &data) override;
//
//   QString Get();

 private:

};

#endif
