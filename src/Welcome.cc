/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#include "Welcome.h"

#include <cmath>

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
  // Header
  header = addWidget(std::make_unique<Wt::WText>(""));
  header->setStyleClass("lastUpdate");

  // Panels
  selectedPanel = addWidget(std::make_unique<Wt::WPanel>());
  selectedPanel->addStyleClass("centered-example");
  selectedPanel->setCollapsible(true);
  selectedPanel->collapse();
  selected = selectedPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  readyForDesignPanel = addWidget(std::make_unique<Wt::WPanel>());
  readyForDesignPanel->addStyleClass("centered-example");
  readyForDesignPanel->setCollapsible(true);
  readyForDesign = readyForDesignPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  designProgressPanel = addWidget(std::make_unique<Wt::WPanel>());
  designProgressPanel->addStyleClass("centered-example");
  designProgressPanel->setCollapsible(true);
  designProgess = designProgressPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  designDonePanel = addWidget(std::make_unique<Wt::WPanel>());
  designDonePanel->addStyleClass("centered-example");
  designDonePanel->setCollapsible(true);
  designDonePanel->collapse();
  designDone = designDonePanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  readyForImplPanel = addWidget(std::make_unique<Wt::WPanel>());
  readyForImplPanel->addStyleClass("centered-example");
  readyForImplPanel->setCollapsible(true);
  readyForImpl = readyForImplPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  implProgressPanel = addWidget(std::make_unique<Wt::WPanel>());
  implProgressPanel->addStyleClass("centered-example");
  implProgressPanel->setCollapsible(true);
  implProgess = implProgressPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  postponedPanel = addWidget(std::make_unique<Wt::WPanel>());
  postponedPanel->addStyleClass("centered-example");
  postponedPanel->setCollapsible(true);
  postponed = postponedPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  donePanel = addWidget(std::make_unique<Wt::WPanel>());
  donePanel->addStyleClass("centered-example");
  donePanel->setCollapsible(true);
  done = donePanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  removedPanel = addWidget(std::make_unique<Wt::WPanel>());
  removedPanel->addStyleClass("centered-example");
  removedPanel->setCollapsible(true);
  removedPanel->collapse();
  removed = removedPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());

  // Dialog to update the Kanban limits
  auto editLimitsAnchor = addWidget(std::make_unique<Wt::WAnchor>());
  editLimitsAnchor->addWidget(std::make_unique<Wt::WText>("Edit Kanban limits (KEEP OUT!)"));
  editLimitsAnchor->addStyleClass("keepOut");
  editLimitsAnchor->clicked().connect([&] {
    limitDialog_ = std::make_unique<KanbanLimitDialog>(session_);
    limitDialog_->show();
  });

  // initial update
  update();

  // update timer
  updateTimer.setInterval(std::chrono::milliseconds(10000));
  updateTimer.timeout().connect([this] { update(); });
  updateTimer.start();
}

void Welcome::update() {
  auto user = session_.user();

  dbo::Transaction transaction(session_.session_);
  Wt::Dbo::collection<Wt::Dbo::ptr<Issue>> issues;

  // obtain Kanban limits
  auto limit = session_.session_.find<KanbanLimit>().limit(1).resultValue();

  // Header
  std::time_t lastUpdate = IssueUpdater::lastUpdate;
  auto infoAge = std::time(nullptr) - lastUpdate;
  auto updateIn = 62 - infoAge;
  std::string sLastUpdate = std::ctime(&lastUpdate);
  header->setText("Last database update: " + sLastUpdate + " (" + std::to_string(infoAge) + " seconds ago).");

  // Set refresh timer
  updateTimer.setInterval(std::chrono::milliseconds(updateIn * 1000));
  updateTimer.start();

  // Pre-selected
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned = 0")
               .where("isDesignChild = 0")
               .where("isReadyForImplementation = 0")
               .where("isPostponed = 0")
               .where("isDesign = 0")
               .orderBy("lastChange")
               .resultList();
  if(limit->preselected > 0) {
    selectedPanel->setTitle(WString("Preselected ({1}/{2})").arg(issues.size()).arg(limit->preselected));
    if(issues.size() > size_t(limit->preselected)) {
      selectedPanel->titleBarWidget()->removeStyleClass("limitMatched");
      selectedPanel->titleBarWidget()->setStyleClass("limitExceeded");
    }
    else if(issues.size() == size_t(limit->preselected)) {
      selectedPanel->titleBarWidget()->removeStyleClass("limitExceeded");
      selectedPanel->titleBarWidget()->setStyleClass("limitMatched");
    }
  }
  else {
    selectedPanel->setTitle(WString("Preselected ({1})").arg(issues.size()));
  }
  showIssues(issues, selected);

  // Ready for design
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned = 0")
               .where("isDesignChild = 0")
               .where("isReadyForImplementation = 0")
               .where("isPostponed = 0")
               .where("isDesign = 1")
               .orderBy("lastChange")
               .resultList();
  if(limit->readyForDesign > 0) {
    readyForDesignPanel->setTitle(WString("Ready for design ({1}/{2})").arg(issues.size()).arg(limit->readyForDesign));
    if(issues.size() > size_t(limit->readyForDesign)) {
      readyForDesignPanel->titleBarWidget()->removeStyleClass("limitMatched");
      readyForDesignPanel->titleBarWidget()->addStyleClass("limitExceeded");
    }
    else if(issues.size() == size_t(limit->readyForDesign)) {
      readyForDesignPanel->titleBarWidget()->removeStyleClass("limitExceeded");
      readyForDesignPanel->titleBarWidget()->addStyleClass("limitMatched");
    }
  }
  else {
    readyForDesignPanel->setTitle(WString("Ready for design ({1})").arg(issues.size()));
  }
  showIssues(issues, readyForDesign);

  // Design in progress
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned != 0")
               .where("isDesign != 0")
               .where("isPostponed = 0")
               .orderBy("lastChange")
               .resultList();
  if(limit->designInProgress > 0) {
    designProgressPanel->setTitle(
        WString("Design in progress ({1}/{2})").arg(issues.size()).arg(limit->designInProgress));
    if(issues.size() > size_t(limit->designInProgress)) {
      designProgressPanel->titleBarWidget()->removeStyleClass("limitMatched");
      designProgressPanel->titleBarWidget()->addStyleClass("limitExceeded");
    }
    else if(issues.size() == size_t(limit->designInProgress)) {
      designProgressPanel->titleBarWidget()->removeStyleClass("limitExceeded");
      designProgressPanel->titleBarWidget()->addStyleClass("limitMatched");
    }
  }
  else {
    designProgressPanel->setTitle(WString("Design in progress ({1})").arg(issues.size()));
  }
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
  if(limit->childsOfDesign > 0) {
    designDonePanel->setTitle(
        WString("Childs of design tickets ({1}/{2})").arg(issues.size()).arg(limit->childsOfDesign));
    if(issues.size() > size_t(limit->childsOfDesign)) {
      designDonePanel->titleBarWidget()->removeStyleClass("limitMatched");
      designDonePanel->titleBarWidget()->addStyleClass("limitExceeded");
    }
    else if(issues.size() == size_t(limit->childsOfDesign)) {
      designDonePanel->titleBarWidget()->removeStyleClass("limitExceeded");
      designDonePanel->titleBarWidget()->addStyleClass("limitMatched");
    }
  }
  else {
    designDonePanel->setTitle(WString("Childs of design tickets ({1})").arg(issues.size()));
  }
  showIssues(issues, designDone);

  // Ready for implementation
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned = 0")
               .where("isReadyForImplementation != 0")
               .where("isPostponed = 0")
               .orderBy("lastChange")
               .resultList();
  if(limit->readyForImplementation > 0) {
    readyForImplPanel->setTitle(
        WString("Ready for implementation ({1}/{2})").arg(issues.size()).arg(limit->readyForImplementation));
    if(issues.size() > size_t(limit->readyForImplementation)) {
      readyForImplPanel->titleBarWidget()->removeStyleClass("limitMatched");
      readyForImplPanel->titleBarWidget()->addStyleClass("limitExceeded");
    }
    else if(issues.size() == size_t(limit->readyForImplementation)) {
      readyForImplPanel->titleBarWidget()->removeStyleClass("limitExceeded");
      readyForImplPanel->titleBarWidget()->addStyleClass("limitMatched");
    }
  }
  else {
    readyForImplPanel->setTitle(WString("Ready for implementation ({1})").arg(issues.size()));
  }
  showIssues(issues, readyForImpl);

  // Implementation in progress
  issues = session_.session_.find<Issue>()
               .where("isOpen = 1")
               .where("isAssigned != 0")
               .where("isDesign = 0")
               .where("isPostponed = 0")
               .orderBy("lastChange")
               .resultList();
  if(limit->implementationInProgress > 0) {
    implProgressPanel->setTitle(
        WString("Implementation in progress ({1}/{2})").arg(issues.size()).arg(limit->implementationInProgress));
    if(issues.size() > size_t(limit->implementationInProgress)) {
      implProgressPanel->titleBarWidget()->removeStyleClass("limitMatched");
      implProgressPanel->titleBarWidget()->addStyleClass("limitExceeded");
    }
    else if(issues.size() == size_t(limit->implementationInProgress)) {
      implProgressPanel->titleBarWidget()->removeStyleClass("limitExceeded");
      implProgressPanel->titleBarWidget()->addStyleClass("limitMatched");
    }
  }
  else {
    implProgressPanel->setTitle(WString("Implementation in progress ({1})").arg(issues.size()));
  }
  showIssues(issues, implProgess);

  // Postponed
  issues = session_.session_.find<Issue>()
               .where("isPostponed = 1")
               .where("isRemoved != 1")
               .orderBy("lastChange")
               .resultList();
  if(limit->postponed > 0) {
    postponedPanel->setTitle(WString("Postponed ({1}/{2})").arg(issues.size()).arg(limit->postponed));
    if(issues.size() > size_t(limit->postponed)) {
      postponedPanel->titleBarWidget()->removeStyleClass("limitMatched");
      postponedPanel->titleBarWidget()->addStyleClass("limitExceeded");
    }
    else if(issues.size() == size_t(limit->postponed)) {
      postponedPanel->titleBarWidget()->removeStyleClass("limitExceeded");
      postponedPanel->titleBarWidget()->addStyleClass("limitMatched");
    }
  }
  else {
    postponedPanel->setTitle(WString("Postponed ({1})").arg(issues.size()));
  }
  showIssues(issues, postponed);

  // Done
  issues = session_.session_.find<Issue>()
               .where("isOpen != 1")
               .where("isPostponed = 0")
               .where("isRemoved != 1")
               .orderBy("lastChange")
               .resultList();
  donePanel->setTitle(WString("Done ({1})").arg(issues.size()));
  showIssues(issues, done);

  // Removed
  issues = session_.session_.find<Issue>().where("isRemoved = 1").orderBy("lastChange DESC").resultList();
  removedPanel->setTitle(WString("Removed ({1})").arg(issues.size()));
  showIssues(issues, removed);
}

/**********************************************************************************************************************/

void Welcome::showIssues(Wt::Dbo::collection<Wt::Dbo::ptr<Issue>>& issues, Wt::WContainerWidget* widget) {
  widget->clear();
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
    if(issue->isReview) container->addStyleClass("review");
    if(issue->priority == Priority::urgent) container->addStyleClass("urgent");
    if(!issue->isOpen || issue->isPostponed) container->addStyleClass("inactive");

    auto w_head = container->addWidget(std::make_unique<WText>(WString("{1} #{2}").arg(issue->project).arg(issue->id)));
    w_head->setStyleClass("head");

    auto w_title = container->addWidget(std::make_unique<WText>(WString("{1}").arg(issue->title)));
    w_title->setStyleClass("title");

    if(issue->assignee != "") {
      auto w_assignee = container->addWidget(std::make_unique<WText>(WString("{1}").arg(issue->assignee)));
      w_assignee->setStyleClass("assignee");
    }

    if(issue->priority == Priority::urgent) {
      auto w_state = container->addWidget(std::make_unique<WText>("urgent "));
      w_state->setStyleClass("state");
    }
    if(issue->isDesign) {
      auto w_state = container->addWidget(std::make_unique<WText>("design "));
      w_state->setStyleClass("state");
    }
    if(issue->isReview && issue->assignee == "") {
      auto w_state = container->addWidget(std::make_unique<WText>("review required "));
      w_state->setStyleClass("state");
    }
    if(issue->isReview && issue->assignee != "") {
      auto w_state = container->addWidget(std::make_unique<WText>("in review "));
      w_state->setStyleClass("state");
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
    int days_age = std::min(int(std::floor(n_age / 86400.)), 10);
    w_age->addStyleClass("days" + std::to_string(days_age));

    if(!issue->isOpen) {
      container->clicked().connect([=] {
        issueDialog_ = std::make_unique<IssueDialog>(session_, issue);
        issueDialog_->show();
      });
    }
  }
}
