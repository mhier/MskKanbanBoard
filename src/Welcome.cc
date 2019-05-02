/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#include "Welcome.h"

#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WDate.h>
#include <Wt/WPushButton.h>
#include <Wt/WPanel.h>
#include <Wt/WCheckBox.h>
#include <Wt/WApplication.h>
#include <Wt/WGroupBox.h>

#include "Issue.h"
#include "IssueUpdater.h"

Welcome::Welcome(Session& session) : session_(session) {
  update();
}

void Welcome::update() {
  clear();

  auto user = session_.user();

  dbo::Transaction transaction(session_.session_);
  Wt::Dbo::collection<Wt::Dbo::ptr<Issue>> issues;

  // Header
  auto infoAge = std::time(nullptr) - IssueUpdater::lastUpdate;
  auto updateIn = 62 - infoAge;
  auto header = addWidget(std::make_unique<Wt::WText>("Last update: " + std::to_string(infoAge) +
      " seconds ago. Auto-update in " + std::to_string(updateIn) + " seconds."));
  header->setStyleClass("lastUpdate");

  // Set refresh timer
  updateTimer.setSingleShot(true);
  updateTimer.setInterval(std::chrono::milliseconds(updateIn * 1000));
  updateTimer.timeout().connect([this] { update(); });
  updateTimer.start();

  // Selected
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned = 0")
               .where("isDesignChild = 0")
               .where("isReadyForImplementation = 0")
               .where("isPostponed = 0")
               .orderBy("lastChange")
               .resultList();
  auto selectedPanel = addWidget(std::make_unique<Wt::WPanel>());
  selectedPanel->setTitle(WString("Selected ({1})").arg(issues.size()));
  selectedPanel->addStyleClass("centered-example");
  selectedPanel->setCollapsible(true);
  auto selected = selectedPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, selected);

  // Design in progress
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned != 0")
               .where("isDesign != 0")
               .where("isPostponed = 0")
               .orderBy("lastChange")
               .resultList();
  auto designProgressPanel = addWidget(std::make_unique<Wt::WPanel>());
  designProgressPanel->setTitle(WString("Design in progress ({1})").arg(issues.size()));
  designProgressPanel->addStyleClass("centered-example");
  designProgressPanel->setCollapsible(true);
  auto designProgess = designProgressPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, designProgess);

  // Childs of design tickets
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned = 0")
               .where("isDesignChild != 0")
               .where("isReadyForImplementation = 0")
               .where("isPostponed = 0")
               .orderBy("lastChange")
               .resultList();
  auto designDonePanel = addWidget(std::make_unique<Wt::WPanel>());
  designDonePanel->setTitle(WString("Childs of design tickets ({1})").arg(issues.size()));
  designDonePanel->addStyleClass("centered-example");
  designDonePanel->setCollapsible(true);
  auto designDone = designDonePanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, designDone);

  // Ready for implementation
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned = 0")
               .where("isReadyForImplementation != 0")
               .where("isPostponed = 0")
               .orderBy("lastChange")
               .resultList();
  auto readyForImplPanel = addWidget(std::make_unique<Wt::WPanel>());
  readyForImplPanel->setTitle(WString("Ready for implementation ({1})").arg(issues.size()));
  readyForImplPanel->addStyleClass("centered-example");
  readyForImplPanel->setCollapsible(true);
  auto readyForImpl = readyForImplPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, readyForImpl);

  // Implementation in progress
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned != 0")
               .where("isDesign = 0")
               .where("isPostponed = 0")
               .orderBy("lastChange")
               .resultList();
  auto implProgressPanel = addWidget(std::make_unique<Wt::WPanel>());
  implProgressPanel->setTitle(WString("Implementation in progress ({1})").arg(issues.size()));
  implProgressPanel->addStyleClass("centered-example");
  implProgressPanel->setCollapsible(true);
  auto implProgess = implProgressPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, implProgess);

  // Postponed
  issues = session_.session_.find<Issue>()
               .where("isPostponed = 1")
               .where("isRemoved != 1")
               .orderBy("lastChange")
               .resultList();
  auto postponedPanel = addWidget(std::make_unique<Wt::WPanel>());
  postponedPanel->setTitle(WString("Postponed ({1})").arg(issues.size()));
  postponedPanel->addStyleClass("centered-example");
  postponedPanel->setCollapsible(true);
  auto postponed = postponedPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, postponed);

  // Done
  issues = session_.session_.find<Issue>()
               .where("isOpen != 1")
               .where("isPostponed = 0")
               .where("isRemoved != 1")
               .orderBy("lastChange")
               .resultList();
  auto donePanel = addWidget(std::make_unique<Wt::WPanel>());
  donePanel->setTitle(WString("Done ({1})").arg(issues.size()));
  donePanel->addStyleClass("centered-example");
  donePanel->setCollapsible(true);
  auto done = donePanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, done);

  // Removed
  issues = session_.session_.find<Issue>().where("isRemoved = 1").orderBy("lastChange DESC").resultList();
  auto removedPanel = addWidget(std::make_unique<Wt::WPanel>());
  removedPanel->setTitle(WString("Removed ({1})").arg(issues.size()));
  removedPanel->addStyleClass("centered-example");
  removedPanel->setCollapsible(true);
  removedPanel->collapse();
  auto removed = removedPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, removed);
}

/**********************************************************************************************************************/

void Welcome::showIssues(Wt::Dbo::collection<Wt::Dbo::ptr<Issue>>& issues, Wt::WContainerWidget* widget) {
  auto now =
      std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  for(auto issue : issues) {
    Wt::WContainerWidget* container;

    if(issue->isOpen) {
      Wt::WLink link = Wt::WLink(issue->url);
      link.setTarget(Wt::LinkTarget::NewWindow);
      auto anchor = widget->addWidget(std::make_unique<Wt::WAnchor>(link));
      container = anchor->addWidget(std::make_unique<Wt::WContainerWidget>());
    }
    else {
      container = widget->addWidget(std::make_unique<Wt::WContainerWidget>());
    }

    container->setStyleClass("issue");
    container->addStyleClass("panel");
    container->addStyleClass("panel-default");
    if(issue->isDesign) container->addStyleClass("design");
    if(issue->priority == Priority::urgent) container->addStyleClass("urgent");

    auto w_head = container->addWidget(std::make_unique<WText>(WString("{1} #{2}").arg(issue->project).arg(issue->id)));
    w_head->setStyleClass("head");

    auto w_title = container->addWidget(std::make_unique<WText>(WString("{1}").arg(issue->title)));
    w_title->setStyleClass("title");

    if(issue->assignee != "") {
      auto w_assignee = container->addWidget(std::make_unique<WText>(WString("{1}").arg(issue->assignee)));
      w_assignee->setStyleClass("assignee");
    }

    int n_age = now - issue->lastChange;
    std::string age;
    if(n_age <= 60) {
      age = std::to_string(n_age) + " seconds";
    }
    else if(n_age <= 3600) {
      age = std::to_string(n_age / 60) + " minutes";
    }
    else if(n_age <= 86400) {
      age = std::to_string(n_age / 3600) + " hours";
    }
    else if(n_age <= 14 * 86400) {
      age = std::to_string(n_age / 86400) + " days";
    }
    else {
      age = std::to_string(n_age / (7 * 86400)) + " weeks";
    }
    std::string tickets = std::to_string(issue->issuesDoneWhileInProgress.size());
    WString s_age;
    if(issue->isOpen && issue->isAssigned && !issue->isDesign && !issue->isPostponed) {
      s_age = WString("{1} ago / {2} tickets").arg(age).arg(tickets);
    }
    else {
      s_age = WString("{1} ago").arg(age);
    }
    auto w_age = container->addWidget(std::make_unique<WText>(s_age));
    w_age->setStyleClass("age");

    if(!issue->isOpen) {
      container->clicked().connect([=] {
        issueDialog_ = std::make_unique<IssueDialog>(session_, issue);
        issueDialog_->show();
      });
    }
  }
}
