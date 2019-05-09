#ifndef ISSUEDIALOG_H
#define ISSUEDIALOG_H

#include "Session.h"
#include "Issue.h"

#include <Wt/WDialog.h>
#include <Wt/WDateEdit.h>
#include <Wt/WText.h>

using namespace Wt;

class IssueDialog : public WDialog {
 public:
  IssueDialog(Session& session, Wt::Dbo::ptr<Issue> issue);

  Session& session_;
  Wt::Dbo::ptr<Issue> issue_;
};
#endif // ISSUEDIALOG_H
