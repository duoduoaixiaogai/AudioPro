/**********************************************************************

  Audacity: A Digital Audio Editor

  XMLTagHandler.h

  Dominic Mazzoni
  Vaughan Johnson

  The XMLTagHandler class is an interface which should be implemented by
  classes which wish to be able to load and save themselves
  using XML files.

  The XMLValueChecker class implements static bool methods for checking
  input values from XML files.

**********************************************************************/
#ifndef XMLTAGHANDLER_H
#define XMLTAGHANDLER_H

//#include "../Audacity.h"
//#include <wx/string.h>
#include <stdio.h>
#include <QString>

#include "XMLWriter.h"
class XMLValueChecker
{
public:
   // "Good" means well-formed and for the file-related functions, names an existing file or folder.
   // These are used in HandleXMLTag and BuildFomXML methods to check the input for
   // security vulnerabilites, per the NGS report for UmixIt.
//   static bool IsGoodString(const QString & str);
//   // Labels are allowed to be very long.  At some future date we will format long labels nicely.
//   static bool IsGoodLongString(const QString & str);
//
//   static bool IsGoodFileName(const QString & strFileName, const QString & strDirName = QString(""));
//   static bool IsGoodFileString(const QString &str);
//   static bool IsGoodSubdirName(const QString & strSubdirName, const QString & strDirName = QString(""));
//   static bool IsGoodPathName(const QString & strPathName);
//   static bool IsGoodPathString(const QString &str);

   /** @brief Check that the supplied string can be converted to a long (32bit)
	* integer.
	*
	* Note that because QString::ToLong does additional testing, IsGoodInt doesn't
	* duplicate that testing, so use QString::ToLong after IsGoodInt, not just
	* atoi.
	* @param strInt The string to test
	* @return true if the string is convertable, false if not
	*/
//   static bool IsGoodInt(const QString & strInt);
   /** @brief Check that the supplied string can be converted to a 64bit
	* integer.
	*
	* Note that because QString::ToLongLong does additional testing, IsGoodInt64
	* doesn't duplicate that testing, so use QString::ToLongLong after IsGoodInt64
	* not just atoll.
	* @param strInt The string to test
	* @return true if the string is convertable, false if not
	*/
//   static bool IsGoodInt64(const QString & strInt);
//   static bool IsGoodIntForRange(const QString & strInt, const QString & strMAXABS);
//
//   static bool IsValidChannel(const int nValue);
//#ifdef USE_MIDI
//   static bool IsValidVisibleChannels(const int nValue);
//#endif
//   static bool IsValidSampleFormat(const int nValue); // true if nValue is one sampleFormat enum values
};


class /*AUDACITY_DLL_API*/ XMLTagHandler /* not final */ {
 public:
   XMLTagHandler(){};
   virtual ~XMLTagHandler(){};
   //
   // Methods to override
   //

   // This method will be called on your class if your class has
   // been registered to handle this particular tag.  Parse the
   // tag and the attribute-value pairs (null-terminated), and
   // return true on success, and false on failure.  If you return
   // false, you will not get any calls about children.
//   virtual bool HandleXMLTag(const QChar *tag, const QChar **attrs) = 0;

   // This method will be called when a closing tag is encountered.
   // It is optional to override this method.
//   virtual void HandleXMLEndTag(const QChar * tag) {}

   // This method will be called when element content has been
   // encountered.
   // It is optional to override this method.
//   virtual void HandleXMLContent(const QString & content) {}

   // If the XML document has children of your tag, this method
   // should be called.  Typically you should construct a NEW
   // object for the child, insert it into your own local data
   // structures, and then return it.  If you do not wish to
   // handle this child, return NULL and it will be ignored.
//   virtual XMLTagHandler *HandleXMLChild(const QChar *tag) = 0;

   // These functions recieve data from expat.  They do charset
   // conversion and then pass the data to the handlers above.
//   bool ReadXMLTag(const char *tag, const char **attrs);
//   void ReadXMLEndTag(const char *tag);
//   void ReadXMLContent(const char *s, int len);
//   XMLTagHandler *ReadXMLChild(const char *tag);
};

#endif // define XMLTAGHANDLER_H

