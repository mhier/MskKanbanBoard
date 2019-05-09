#ifndef KANBANLIMITDIALOG_H
#define KANBANLIMITDIALOG_H

#include "Session.h"
#include "KanbanLimit.h"

#include <Wt/WDialog.h>
#include <Wt/WDateEdit.h>
#include <Wt/WText.h>


class KanbanLimitDialog : public Wt::WDialog {
 public:
  KanbanLimitDialog(Session& session);

  Session& session_;

  WSpinBox *preselected, *readyForDesign, *designInProgress, *childsOfDesign, *readyForImplementation;
  WSpinBox *implementationInProgress, *postponed;
  Wt::Dbo::ptr<KanbanLimit> limit;

};
#endif // KANBANLIMITDIALOG_H
