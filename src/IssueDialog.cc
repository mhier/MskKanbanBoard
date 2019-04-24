#include <Wt/WApplication.h>
#include <Wt/WPushButton.h>
#include <Wt/WMessageBox.h>
#include "IssueDialog.h"
#include <curl/curl.h>
#include <iomanip>

/**********************************************************************************************************************/

size_t webcurlWriteFunction(void* ptr, size_t, size_t nmemb, void* userdata) {
  std::string* buffer = static_cast<std::string*>(userdata);
  (*buffer) += std::string(static_cast<char*>(ptr), nmemb);
  return nmemb;
}

/**********************************************************************************************************************/

IssueDialog::IssueDialog(Session& session, Wt::Dbo::ptr<Issue> issue) : session_(session), issue_(issue) {
  titleBar()->clear();
  titleBar()->addWidget(
      std::make_unique<Wt::WText>("<h4>" + issue->project + " #" + std::to_string(issue->id) + "</h4>"));

  contents()->addWidget(std::make_unique<Wt::WText>("<h5>" + issue->title + "</h5>"));
  contents()->addWidget(std::make_unique<Wt::WText>(WString("<p>is open: {1}</p>").arg(std::to_string(issue->isOpen))));
  contents()->addWidget(
      std::make_unique<Wt::WText>(WString("<p>is assigned: {1}</p>").arg(std::to_string(issue->isAssigned))));
  contents()->addWidget(std::make_unique<Wt::WText>(WString("<p>assignee: {1}</p>").arg(issue->assignee)));
  contents()->addWidget(
      std::make_unique<Wt::WText>(WString("<p>is design: {1}</p>").arg(std::to_string(issue->isDesign))));
  contents()->addWidget(
      std::make_unique<Wt::WText>(WString("<p>is design child: {1}</p>").arg(std::to_string(issue->isDesignChild))));
  contents()->addWidget(std::make_unique<Wt::WText>(
      WString("<p>is ready for implementation: {1}</p>").arg(std::to_string(issue->isReadyForImplementation))));
  contents()->addWidget(
      std::make_unique<Wt::WText>(WString("<p>is postponed: {1}</p>").arg(std::to_string(issue->isPostponed))));
  contents()->addWidget(
      std::make_unique<Wt::WText>(WString("<p>is removed: {1}</p>").arg(std::to_string(issue->isRemoved))));

  if(!issue->isOpen) {
    auto remove = footer()->addWidget(std::make_unique<Wt::WPushButton>("Remove from board"));
    remove->clicked().connect(this, [this] {
      auto button = Wt::WMessageBox::show("Remove issue permanently from board?",
          "Are you sure you want to remove this issue from the board? Only the ticket master should do "
          "this, after checking this issue fulfills its definition of done.",
          Wt::StandardButton::Yes | Wt::StandardButton::No);
      if(button == Wt::StandardButton::Yes) {
        Wt::Dbo::Transaction transaction(session_.session_);
        issue_.modify()->isRemoved = true;
        this->hide();
        Wt::WApplication::instance()->setInternalPath("/", true);
      }
    });
  }

  auto open = footer()->addWidget(std::make_unique<Wt::WPushButton>("Open in tracker"));
  WLink link(issue->url);
  link.setTarget(Wt::LinkTarget::NewWindow);
  open->setLink(link);

  auto ok = footer()->addWidget(std::make_unique<Wt::WPushButton>("Close"));
  ok->setDefault(true);
  ok->clicked().connect(this, [=] { this->hide(); });
}

/**********************************************************************************************************************/
