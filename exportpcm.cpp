#include "exportpcm.h"
#include "export.h"
#include "FileFormats.h"
#include "mix.h"
#include "tags.h"
#include "progressdialog.h"
#include "project.h"
#include "FileFormats.h"

#include "sndfile.h"

#include <QMessageBox>

namespace Renfeng {

#define WXSIZEOF(array)   (sizeof(array)/sizeof(array[0]))

    struct
    {
        int format;
        const char *name;
        const char *desc;
    }

    static const kFormats[] =
    {
#if defined(__WXMAC__)
    { SF_FORMAT_AIFF | SF_FORMAT_PCM_16,   ("AIFF"),   ("AIFF (Apple) signed 16-bit PCM")    },
#endif
    { SF_FORMAT_WAV | SF_FORMAT_PCM_16,    ("WAV"),    ("WAV (Microsoft) signed 16-bit PCM") },
    { SF_FORMAT_WAV | SF_FORMAT_PCM_24,    ("WAV24"),  ("WAV (Microsoft) signed 24-bit PCM") },
    { SF_FORMAT_WAV | SF_FORMAT_FLOAT,     ("WAVFLT"), ("WAV (Microsoft) 32-bit float PCM")  },
    // { SF_FORMAT_WAV | SF_FORMAT_GSM610,    wxT("GSM610"), XO("GSM 6.10 WAV (mobile)")             },
};

    class ExportPCM final : public ExportPlugin
    {
    public:
        ExportPCM();

        ProgressResult Export(AudioProject *project,
                              std::unique_ptr<ProgressDialog> &pDialog,
                              unsigned channels,
                              const QString &fName,
                              bool selectedOnly,
                              double t0,
                              double t1,
                              MixerSpec *mixerSpec = NULL,
                              const Tags *metadata = NULL,
                              int subformat = 0) override;
    private:
        ArrayOf<char> AdjustString(const QString & wxStr, int sf_format);
        bool AddStrings(AudioProject *project, SNDFILE *sf, const Tags *tags, int sf_format);
        bool AddID3Chunk(QString fName, const Tags *tags, int sf_format);
    };

    ExportPCM::ExportPCM()
        :  ExportPlugin()
    {
        SF_INFO si;

        si.samplerate = 0;
        si.channels = 0;

        int format;

        for (size_t i = 0; i < WXSIZEOF(kFormats); i++)
            {
                format = AddFormat() - 1;
                si.format = kFormats[i].format;
                for (si.channels = 1; sf_format_check(&si); si.channels++)
                    ;
                QString ext = sf_header_extension(si.format);
                SetFormat(kFormats[i].name, format);
                SetCanMetaData(true, format);
                SetDescription(kFormats[i].desc, format);
                //                SetDescription(wxGetTranslation(kFormats[i].desc), format);
                AddExtension(ext, format);
                SetMaxChannels(si.channels - 1, format);
            }

        format = AddFormat() - 1;
        SetFormat("LIBSNDFILE", format);
        SetCanMetaData(true, format);
        SetDescription("Other uncompressed files", format);
        auto allext = sf_get_all_extensions();
        QString wavext = sf_header_extension(SF_FORMAT_WAV);

        SetExtensions(allext, format);
        SetMaxChannels(255, format);
    }

    std::unique_ptr<ExportPlugin> New_ExportPCM()
    {
        return std::make_unique<ExportPCM>();
    }

    ProgressResult ExportPCM::Export(AudioProject *project,
                                     std::unique_ptr<ProgressDialog> &pDialog,
                                     unsigned numChannels,
                                     const QString &fName,
                                     bool selectionOnly,
                                     double t0,
                                     double t1,
                                     MixerSpec *mixerSpec,
                                     const Tags *metadata,
                                     int subformat)
    {
        double       rate = project->GetRate();
        const TrackList   *tracks = project->GetTracks();
        int sf_format;

        if (subformat < 0 || static_cast<unsigned int>(subformat) >= WXSIZEOF(kFormats))
            {
                //              sf_format = ReadExportFormatPref();
            }
        else
            {
                sf_format = kFormats[subformat].format;
            }

        auto updateResult = ProgressResult::Success;

        {
            QFile f(fName);   // will be closed when it goes out of scope
            SFFile       sf; // wraps f

            QString     formatStr;
            SF_INFO      info;

            formatStr = SFCall<QString>(sf_header_name, sf_format & SF_FORMAT_TYPEMASK);

            info.samplerate = (unsigned int)(rate + 0.5);
            info.frames = (unsigned int)((t1 - t0)*rate + 0.5);
            info.channels = numChannels;
            info.format = sf_format;
            info.sections = 1;
            info.seekable = 0;

            if (!sf_format_check(&info))
                info.format = (info.format & SF_FORMAT_TYPEMASK);
            if (!sf_format_check(&info)) {
                    QMessageBox::warning(nullptr, QString(""), QString("Cannot export audio in this format."));
                    return ProgressResult::Cancelled;
                }

            if (f.open(QIODevice::WriteOnly)) {
                    sf.reset(SFCall<SNDFILE*>(sf_open_fd, f.handle(), SFM_WRITE, &info, false));
                    sf_command(sf.get(), SFC_SET_CLIPPING, NULL, sf_subtype_is_integer(sf_format)?SF_TRUE:SF_FALSE) ;
                }

            if (!sf) {
                    QMessageBox::warning(nullptr, QString(""), QString("Cannot export audio to %1").arg(fName));
                    return ProgressResult::Cancelled;
                }

            if (metadata == NULL)
                metadata = project->GetTags();

            sampleFormat format;
            if (sf_subtype_more_than_16_bits(info.format))
                format = floatSample;
            else
                format = int16Sample;
            float sampleCount = (float)(t1-t0)*rate*info.channels;
            float byteCount = sampleCount * ((format==int16Sample)?2:4);

            if( byteCount > 4.295e9)
                {
                    return ProgressResult::Failed;
                }

            size_t maxBlockLen = 44100 * 5;

            const WaveTrackConstArray waveTracks =
                    tracks->GetWaveTrackConstArray(selectionOnly, false);

            {
                auto mixer = CreateMixer(waveTracks,
                                         nullptr,
                                         t0, t1,
                                         info.channels, maxBlockLen, true,
                                         rate, format, true, mixerSpec);

                while (updateResult == ProgressResult::Success) {
                        sf_count_t samplesWritten;
                        size_t numSamples = mixer->Process(maxBlockLen);
                        if (numSamples == 0)
                            break;
                        samplePtr mixed = mixer->GetBuffer();

                        if (format == int16Sample)
                            samplesWritten = SFCall<sf_count_t>(sf_writef_short, sf.get(), (short *)mixed, numSamples);
                        else
                            samplesWritten = SFCall<sf_count_t>(sf_writef_float, sf.get(), (float *)mixed, numSamples);
                        if (static_cast<size_t>(samplesWritten) != numSamples) {
                                //                                                                  char buffer2[1000];
                                //                                                                  sf_error_str(sf.get(), buffer2, 1000);
                                //                                                                  AudacityMessageBox(wxString::Format(
                                //                                                                                                /* i18n-hint: %s will be the error message from libsndfile, which
                                //                                                                                                 * is usually something unhelpful (and untranslated) like "system
                                //                                                                                                 * error" */
                                //                                                                                                _("Error while writing %s file (disk full?).\nLibsndfile says \"%s\""),
                                //                                                                                                formatStr,
                                //                                                                                                wxString::FromAscii(buffer2)));
                                updateResult = ProgressResult::Cancelled;
                                break;
                            }
                        //                                                   updateResult = progress.Update(mixer->MixGetCurrentTime() - t0, t1 - t0);
                    }
            }

            if (updateResult == ProgressResult::Success ||
                      updateResult == ProgressResult::Stopped) {
                     if ((sf_format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV ||
                         (sf_format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAVEX) {
                        if (!AddStrings(project, sf.get(), metadata, sf_format)) {
                           // TODO: more precise message
//                           AudacityMessageBox(_("Unable to export"));
                           return ProgressResult::Cancelled;
                        }
                     }
                     if (0 != sf.close()) {
                        // TODO: more precise message
//                        AudacityMessageBox(_("Unable to export"));
                        return ProgressResult::Cancelled;
                     }
                  }
        }

        if (updateResult == ProgressResult::Success ||
               updateResult == ProgressResult::Stopped)
              if (((sf_format & SF_FORMAT_TYPEMASK) == SF_FORMAT_AIFF) ||
                  ((sf_format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV))
                 // Note: file has closed, and gets reopened and closed again here:
                 if (!AddID3Chunk(fName, metadata, sf_format) ) {
                    // TODO: more precise message
//                    AudacityMessageBox(_("Unable to export"));
                    return ProgressResult::Cancelled;
                 }

        return updateResult;
    }

    bool ExportPCM::AddStrings(AudioProject * project, SNDFILE *sf, const Tags *tags, int sf_format)
    {
       if (tags->HasTag(TAG_TITLE)) {
          auto ascii7Str = AdjustString(tags->GetTag(TAG_TITLE), sf_format);
          if (ascii7Str) {
             sf_set_string(sf, SF_STR_TITLE, ascii7Str.get());
          }
       }

       if (tags->HasTag(TAG_ALBUM)) {
          auto ascii7Str = AdjustString(tags->GetTag(TAG_ALBUM), sf_format);
          if (ascii7Str) {
             sf_set_string(sf, SF_STR_ALBUM, ascii7Str.get());
          }
       }

       if (tags->HasTag(TAG_ARTIST)) {
          auto ascii7Str = AdjustString(tags->GetTag(TAG_ARTIST), sf_format);
          if (ascii7Str) {
             sf_set_string(sf, SF_STR_ARTIST, ascii7Str.get());
          }
       }

       if (tags->HasTag(TAG_COMMENTS)) {
          auto ascii7Str = AdjustString(tags->GetTag(TAG_COMMENTS), sf_format);
          if (ascii7Str) {
             sf_set_string(sf, SF_STR_COMMENT, ascii7Str.get());
          }
       }

       if (tags->HasTag(TAG_YEAR)) {
          auto ascii7Str = AdjustString(tags->GetTag(TAG_YEAR), sf_format);
          if (ascii7Str) {
             sf_set_string(sf, SF_STR_DATE, ascii7Str.get());
          }
       }

       if (tags->HasTag(TAG_GENRE)) {
          auto ascii7Str = AdjustString(tags->GetTag(TAG_GENRE), sf_format);
          if (ascii7Str) {
             sf_set_string(sf, SF_STR_GENRE, ascii7Str.get());
          }
       }

       if (tags->HasTag(TAG_COPYRIGHT)) {
          auto ascii7Str = AdjustString(tags->GetTag(TAG_COPYRIGHT), sf_format);
          if (ascii7Str) {
             sf_set_string(sf, SF_STR_COPYRIGHT, ascii7Str.get());
          }
       }

       if (tags->HasTag(TAG_SOFTWARE)) {
          auto ascii7Str = AdjustString(tags->GetTag(TAG_SOFTWARE), sf_format);
          if (ascii7Str) {
             sf_set_string(sf, SF_STR_SOFTWARE, ascii7Str.get());
          }
       }

       if (tags->HasTag(TAG_TRACK)) {
          auto ascii7Str = AdjustString(tags->GetTag(TAG_TRACK), sf_format);
          if (ascii7Str) {
             sf_set_string(sf, SF_STR_TRACKNUMBER, ascii7Str.get());
          }
       }

       return true;
    }

    ArrayOf<char> ExportPCM::AdjustString(const QString & wxStr, int sf_format)
    {
       bool b_aiff = false;
       if ((sf_format & SF_FORMAT_TYPEMASK) == SF_FORMAT_AIFF)
             b_aiff = true;    // Apple AIFF file

       // We must convert the string to 7 bit ASCII
       size_t  sz = wxStr.length();
       if(sz == 0)
          return {};
       // Size for secure allocation in case of local wide char usage
       size_t  sr = (sz+4) * 2;

       ArrayOf<char> pDest{ sr, true };
       if (!pDest)
          return {};
       ArrayOf<char> pSrc{ sr, true };
       if (!pSrc)
          return {};

//       if(wxStr.mb_str(wxConvISO8859_1))
//          strncpy(pSrc.get(), wxStr.mb_str(wxConvISO8859_1), sz);
//       else if(wxStr.mb_str())
//          strncpy(pSrc.get(), wxStr.mb_str(), sz);
//       else
//          return {};

       char *pD = pDest.get();
       char *pS = pSrc.get();
       unsigned char c;

       // ISO Latin to 7 bit ascii conversion table (best approximation)
       static char aASCII7Table[256] = {
          0x00, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
          0x5f, 0x09, 0x0a, 0x5f, 0x0d, 0x5f, 0x5f, 0x5f,
          0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
          0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
          0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
          0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
          0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
          0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
          0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
          0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
          0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
          0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
          0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
          0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
          0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
          0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
          0x45, 0x20, 0x2c, 0x53, 0x22, 0x2e, 0x2b, 0x2b,
          0x5e, 0x25, 0x53, 0x28, 0x4f, 0x20, 0x5a, 0x20,
          0x20, 0x27, 0x27, 0x22, 0x22, 0x2e, 0x2d, 0x5f,
          0x22, 0x54, 0x73, 0x29, 0x6f, 0x20, 0x7a, 0x59,
          0x20, 0x21, 0x63, 0x4c, 0x6f, 0x59, 0x7c, 0x53,
          0x22, 0x43, 0x61, 0x22, 0x5f, 0x2d, 0x43, 0x2d,
          0x6f, 0x7e, 0x32, 0x33, 0x27, 0x75, 0x50, 0x27,
          0x2c, 0x31, 0x6f, 0x22, 0x5f, 0x5f, 0x5f, 0x3f,
          0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x43,
          0x45, 0x45, 0x45, 0x45, 0x49, 0x49, 0x49, 0x49,
          0x44, 0x4e, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x78,
          0x4f, 0x55, 0x55, 0x55, 0x55, 0x59, 0x70, 0x53,
          0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x63,
          0x65, 0x65, 0x65, 0x65, 0x69, 0x69, 0x69, 0x69,
          0x64, 0x6e, 0x6f, 0x6f, 0x6f, 0x6f, 0x6f, 0x2f,
          0x6f, 0x75, 0x75, 0x75, 0x75, 0x79, 0x70, 0x79
       };

       size_t i;
       for(i = 0; i < sr; i++) {
          c = (unsigned char) *pS++;
          *pD++ = aASCII7Table[c];
          if(c == 0)
             break;
       }
       *pD = '\0';

       if(b_aiff) {
          int len = (int)strlen(pDest.get());
          if((len % 2) != 0) {
             // In case of an odd length string, add a space char
             strcat(pDest.get(), " ");
          }
       }

       return pDest;
    }

    bool ExportPCM::AddID3Chunk(QString fName, const Tags *tags, int sf_format)
    {
    #ifdef USE_LIBID3TAG
       id3_tag_holder tp { id3_tag_new() };

       for (const auto &pair : tags->GetRange()) {
          const auto &n = pair.first;
          const auto &v = pair.second;
          const char *name = "TXXX";

          if (n.CmpNoCase(TAG_TITLE) == 0) {
             name = ID3_FRAME_TITLE;
          }
          else if (n.CmpNoCase(TAG_ARTIST) == 0) {
             name = ID3_FRAME_ARTIST;
          }
          else if (n.CmpNoCase(TAG_ALBUM) == 0) {
             name = ID3_FRAME_ALBUM;
          }
          else if (n.CmpNoCase(TAG_YEAR) == 0) {
             name = ID3_FRAME_YEAR;
          }
          else if (n.CmpNoCase(TAG_GENRE) == 0) {
             name = ID3_FRAME_GENRE;
          }
          else if (n.CmpNoCase(TAG_COMMENTS) == 0) {
             name = ID3_FRAME_COMMENT;
          }
          else if (n.CmpNoCase(TAG_TRACK) == 0) {
             name = ID3_FRAME_TRACK;
          }
          else if (n.CmpNoCase(wxT("composer")) == 0) {
             name = "TCOM";
          }

          struct id3_frame *frame = id3_frame_new(name);

          if (!n.IsAscii() || !v.IsAscii()) {
             id3_field_settextencoding(id3_frame_field(frame, 0), ID3_FIELD_TEXTENCODING_UTF_16);
          }
          else {
             id3_field_settextencoding(id3_frame_field(frame, 0), ID3_FIELD_TEXTENCODING_ISO_8859_1);
          }

          MallocString<id3_ucs4_t> ucs4{
             id3_utf8_ucs4duplicate((id3_utf8_t *) (const char *) v.mb_str(wxConvUTF8)) };

          if (strcmp(name, ID3_FRAME_COMMENT) == 0) {
             // A hack to get around iTunes not recognizing the comment.  The
             // language defaults to XXX and, since it's not a valid language,
             // iTunes just ignores the tag.  So, either set it to a valid language
             // (which one???) or just clear it.  Unfortunately, there's no supported
             // way of clearing the field, so do it directly.
             id3_field *f = id3_frame_field(frame, 1);
             memset(f->immediate.value, 0, sizeof(f->immediate.value));
             id3_field_setfullstring(id3_frame_field(frame, 3), ucs4.get());
          }
          else if (strcmp(name, "TXXX") == 0) {
             id3_field_setstring(id3_frame_field(frame, 2), ucs4.get());

             ucs4.reset(id3_utf8_ucs4duplicate((id3_utf8_t *) (const char *) n.mb_str(wxConvUTF8)));

             id3_field_setstring(id3_frame_field(frame, 1), ucs4.get());
          }
          else {
             auto addr = ucs4.get();
             id3_field_setstrings(id3_frame_field(frame, 1), 1, &addr);
          }

          id3_tag_attachframe(tp.get(), frame);
       }

       tp->options &= (~ID3_TAG_OPTION_COMPRESSION); // No compression

       // If this version of libid3tag supports it, use v2.3 ID3
       // tags instead of the newer, but less well supported, v2.4
       // that libid3tag uses by default.
    #ifdef ID3_TAG_HAS_TAG_OPTION_ID3V2_3
       tp->options |= ID3_TAG_OPTION_ID3V2_3;
    #endif

       id3_length_t len;

       len = id3_tag_render(tp.get(), 0);
       if (len == 0)
          return true;

       if ((len % 2) != 0) len++;   // Length must be even.
       ArrayOf<id3_byte_t> buffer { len, true };
       if (buffer == NULL)
          return false;

       // Zero all locations, for ending odd UTF16 content
       // correctly, i.e., two '\0's at the end.

       id3_tag_render(tp.get(), buffer.get());

       wxFFile f(fName, wxT("r+b"));
       if (f.IsOpened()) {
          wxUint32 sz;

          sz = (wxUint32) len;
          if (!f.SeekEnd(0))
             return false;
          if ((sf_format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV)
             {
                if (4 != f.Write("id3 ", 4))// Must be lower case for foobar2000.
                   return false ;
             }
          else {
             if (4 != f.Write("ID3 ", 4))
                return false;
             sz = wxUINT32_SWAP_ON_LE(sz);
          }
          if (4 != f.Write(&sz, 4))
             return false;

          if (len != f.Write(buffer.get(), len))
             return false;

          sz = (wxUint32) f.Tell() - 8;
          if ((sf_format & SF_FORMAT_TYPEMASK) == SF_FORMAT_AIFF)
             sz = wxUINT32_SWAP_ON_LE(sz);

          if (!f.Seek(4))
             return false;
          if (4 != f.Write(&sz, 4))
             return false;

          if (!f.Flush())
             return false;

          if (!f.Close())
             return false;
       }
       else
          return false;
    #endif
       return true;
    }
}
