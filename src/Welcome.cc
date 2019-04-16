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

Welcome::Welcome(Session& session) : session_(session) {
  clear();

  auto user = session_.user();

  dbo::Transaction transaction(session_.session_);
  Wt::Dbo::collection<Wt::Dbo::ptr<Issue>> issues;

  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned = 0")
               .where("isDesignChild = 0")
               .where("isReadyForImplementation = 0")
               .orderBy("lastChange")
               .resultList();
  auto selectedPanel = addWidget(std::make_unique<Wt::WPanel>());
  selectedPanel->setTitle(WString("Selected ({1})").arg(issues.size()));
  selectedPanel->addStyleClass("centered-example");
  selectedPanel->setCollapsible(true);
  auto selected = selectedPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, selected);

  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned != 0")
               .where("isDesign != 0")
               .orderBy("lastChange")
               .resultList();
  auto designProgressPanel = addWidget(std::make_unique<Wt::WPanel>());
  designProgressPanel->setTitle(WString("Design in progress ({1})").arg(issues.size()));
  designProgressPanel->addStyleClass("centered-example");
  designProgressPanel->setCollapsible(true);
  auto designProgess = designProgressPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, designProgess);

  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned = 0")
               .where("isDesignChild != 0")
               .where("isReadyForImplementation = 0")
               .orderBy("lastChange")
               .resultList();
  auto designDonePanel = addWidget(std::make_unique<Wt::WPanel>());
  designDonePanel->setTitle(WString("Childs of design tickets ({1})").arg(issues.size()));
  designDonePanel->addStyleClass("centered-example");
  designDonePanel->setCollapsible(true);
  auto designDone = designDonePanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, designDone);

  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned = 0")
               .where("isReadyForImplementation != 0")
               .orderBy("lastChange")
               .resultList();
  auto readyForImplPanel = addWidget(std::make_unique<Wt::WPanel>());
  readyForImplPanel->setTitle(WString("Ready for implementation ({1})").arg(issues.size()));
  readyForImplPanel->addStyleClass("centered-example");
  readyForImplPanel->setCollapsible(true);
  auto readyForImpl = readyForImplPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, readyForImpl);

  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned != 0")
               .where("isDesign = 0")
               .orderBy("lastChange")
               .resultList();
  auto implProgressPanel = addWidget(std::make_unique<Wt::WPanel>());
  implProgressPanel->setTitle(WString("Implementation in progress ({1})").arg(issues.size()));
  implProgressPanel->addStyleClass("centered-example");
  implProgressPanel->setCollapsible(true);
  auto implProgess = implProgressPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, implProgess);

  issues =
      session_.session_.find<Issue>().where("isOpen != 1").where("isRemoved != 1").orderBy("lastChange").resultList();
  auto donePanel = addWidget(std::make_unique<Wt::WPanel>());
  donePanel->setTitle(WString("Done ({1})").arg(issues.size()));
  donePanel->addStyleClass("centered-example");
  donePanel->setCollapsible(true);
  auto done = donePanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  showIssues(issues, done);

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
    //Wt::WLink link(issue->url);
    //link.setTarget(Wt::LinkTarget::NewWindow);
    //auto anchor = widget->addWidget(std::make_unique<Wt::WAnchor>(link));
    //auto panel = anchor->addWidget(std::make_unique<Wt::WPanel>());
    auto panel = widget->addWidget(std::make_unique<Wt::WPanel>());
    panel->setStyleClass("issue");
    if(issue->isDesign) panel->addStyleClass("design");
    if(issue->priority == Priority::urgent) panel->addStyleClass("urgent");
    auto container = std::make_unique<Wt::WContainerWidget>();

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
    auto w_age = container->addWidget(std::make_unique<WText>(WString("{1} ago").arg(age)));
    w_age->setStyleClass("age");

    container->clicked().connect([=] {
      issueDialog_ = std::make_unique<IssueDialog>(session_, issue);
      issueDialog_->show();
    });

    panel->setCentralWidget(std::move(container));
  }
}
