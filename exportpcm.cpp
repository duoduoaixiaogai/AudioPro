#include "exportpcm.h"
#include "export.h"
#include "FileFormats.h"

#include "sndfile.h"

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
}
