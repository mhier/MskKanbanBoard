/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef INCLUDE_WELCOME_H_
#define INCLUDE_WELCOME_H_

#include "Session.h"
#include "IssueDialog.h"
#include "KanbanLimitDialog.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WTimer.h>

using namespace Wt;

class Issue;

class Welcome : public WContainerWidget {
 public:
  Welcome(Session& session);
  void update();

  void showIssues(Wt::Dbo::collection<Wt::Dbo::ptr<Issue>>& issues, Wt::WContainerWidget* widget);

 private:
  Session& session_;
  std::unique_ptr<IssueDialog> issueDialog_;
  std::unique_ptr<KanbanLimitDialog> limitDialog_;
  Wt::WTimer updateTimer;

  Wt::WText* header;
  Wt::WPanel *selectedPanel, *readyForDesignPanel, *designProgressPanel, *designDonePanel, *readyForImplPanel,
      *implProgressPanel, *postponedPanel, *donePanel, *removedPanel;
  Wt::WContainerWidget *selected, *readyForDesign, *designProgess, *designDone, *readyForImpl, *implProgess, *postponed,
      *done, *removed;
};

#endif // INCLUDE_WELCOME_H_
