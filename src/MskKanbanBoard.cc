/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#include <iomanip>

#include <Wt/WApplication.h>
#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/RegistrationModel.h>
#include <Wt/WText.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WCalendar.h>
#include <Wt/WPushButton.h>
#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Auth/Dbo/UserDatabase.h>

#include "Session.h"
#include "MskKanbanBoard.h"
#include "EditForm.h"
#include "ShowForm.h"
#include "Welcome.h"
#include "Form.h"
#include "Version.h"

MskKanbanBoard::MskKanbanBoard() {
  addWidget(std::make_unique<WText>("<h1>MSK Kanban Board</h1>"));

  auto contentStack = std::make_unique<Wt::WStackedWidget>();
  contentStack->setHeight("100vH");
  contentStack->addStyleClass("contents");
  contentStack->setOverflow(Wt::Overflow::Auto);
  contentStack_ = addWidget(std::move(contentStack));

  std::stringstream sversion;
  sversion << std::setfill('0') << std::setw(2) << AppVersion::major << ".";
  sversion << std::setfill('0') << std::setw(2) << AppVersion::minor << ".";
  sversion << std::setfill('0') << std::setw(2) << AppVersion::patch;
  addWidget(std::make_unique<WText>("<hr/>MskKanbanBoard " + sversion.str()));

  WApplication::instance()->internalPathChanged().connect(this, &MskKanbanBoard::handleInternalPath);

  handleInternalPath(WApplication::instance()->internalPath());
}

void MskKanbanBoard::handleInternalPath(const std::string& internalPath) {
  Wt::Dbo::Transaction transaction(session_.session_);
  contentStack_->clear();
  if(internalPath == "/") {
    contentStack_->addWidget(std::make_unique<Welcome>(session_));
  }
  else if(internalPath == "/edit") {
    contentStack_->addWidget(std::make_unique<EditForm>(session_));
  }
  else if(internalPath.substr(0, 6) == "/show/") {
    contentStack_->addWidget(std::make_unique<ShowForm>(session_, internalPath.substr(6)));
  }
  else {
    WApplication::instance()->setInternalPath("/", true);
  }
}
