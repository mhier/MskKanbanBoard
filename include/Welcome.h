/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef INCLUDE_WELCOME_H_
#define INCLUDE_WELCOME_H_

#include "Session.h"
#include "IssueDialog.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

class Issue;

class Welcome : public WContainerWidget {
 public:
  Welcome(Session& session);

  void showIssues(Wt::Dbo::collection<Wt::Dbo::ptr<Issue>>& issues, Wt::WContainerWidget* widget);

 private:
  Session& session_;
  std::unique_ptr<IssueDialog> issueDialog_;
};

#endif // INCLUDE_WELCOME_H_
