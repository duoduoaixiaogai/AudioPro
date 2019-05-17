#ifndef EXPORTPCM_H
#define EXPORTPCM_H

#include "memoryx.h"

namespace Renfeng {
   class ExportPlugin;

   std::unique_ptr<ExportPlugin> New_ExportPCM();
}

#endif // EXPORTPCM_H
