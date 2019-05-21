#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Renfeng {
  enum class ProgressResult : unsigned
  {
     Cancelled = 0, //<! User says that whatever is happening is undesirable and shouldn't have happened at all
     Success,       //<! User says nothing, everything works fine, continue doing whatever we're doing
     Failed,        //<! Something has gone wrong, we should stop and cancel everything we did
     Stopped        //<! Nothing is wrong, but user says we should stop now and leave things as they are now
  };

  class ProgressDialog : public QDialog {

  };
}

#endif // PROGRESSDIALOG_H
