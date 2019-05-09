#include <Wt/WApplication.h>
#include <Wt/WPushButton.h>
#include <Wt/WMessageBox.h>
#include <Wt/WTable.h>
#include <Wt/WSpinBox.h>
#include "KanbanLimitDialog.h"
#include <curl/curl.h>
#include <iomanip>

/**********************************************************************************************************************/

KanbanLimitDialog::KanbanLimitDialog(Session& session) : session_(session) {
  Wt::Dbo::Transaction transaction(session.session_);

  limit = session_.session_.find<KanbanLimit>().limit(1).resultValue();
  if(!limit) {
    limit.reset(std::make_unique<KanbanLimit>());
    session.session_.add(limit);
    std::cout << "**************************************************" << std::endl;
    std::cout << "**************************************************" << std::endl;
    std::cout << " Kanban limit initialised with all 0 " << std::endl;
    std::cout << "**************************************************" << std::endl;
    std::cout << "**************************************************" << std::endl;
  }

  titleBar()->clear();
  titleBar()->addWidget(std::make_unique<Wt::WText>("<h4>Edit Kanban limits</h4>"));

  contents()->addWidget(std::make_unique<Wt::WText>("<p>Please do not edit these limits without consent of the MSK "
                                                    "software group. A limit of 0 will disable the limit.</p>"));

  auto table = contents()->addWidget(std::make_unique<WTable>());
  table->setHeaderCount(1);
  table->setWidth(WLength("100%"));
  table->addStyleClass("table form-inline table-hover");

  table->elementAt(0, 0)->addWidget(std::make_unique<WText>("Group"));
  table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Limit"));

  table->elementAt(1, 0)->addWidget(std::make_unique<WText>("preselected"));
  preselected = table->elementAt(1, 1)->addWidget(std::make_unique<WSpinBox>());
  preselected->setValue(limit->preselected);

  table->elementAt(2, 0)->addWidget(std::make_unique<WText>("ready for design"));
  readyForDesign = table->elementAt(2, 1)->addWidget(std::make_unique<WSpinBox>());
  readyForDesign->setValue(limit->readyForDesign);

  table->elementAt(3, 0)->addWidget(std::make_unique<WText>("design in progress"));
  designInProgress = table->elementAt(3, 1)->addWidget(std::make_unique<WSpinBox>());
  designInProgress->setValue(limit->designInProgress);

  table->elementAt(4, 0)->addWidget(std::make_unique<WText>("childs of design tickets"));
  childsOfDesign = table->elementAt(4, 1)->addWidget(std::make_unique<WSpinBox>());
  childsOfDesign->setValue(limit->childsOfDesign);

  table->elementAt(5, 0)->addWidget(std::make_unique<WText>("ready for implementation"));
  readyForImplementation = table->elementAt(5, 1)->addWidget(std::make_unique<WSpinBox>());
  readyForImplementation->setValue(limit->readyForImplementation);

  table->elementAt(6, 0)->addWidget(std::make_unique<WText>("implementation in progress"));
  implementationInProgress = table->elementAt(6, 1)->addWidget(std::make_unique<WSpinBox>());
  implementationInProgress->setValue(limit->implementationInProgress);

  table->elementAt(7, 0)->addWidget(std::make_unique<WText>("postponed"));
  postponed = table->elementAt(7, 1)->addWidget(std::make_unique<WSpinBox>());
  postponed->setValue(limit->postponed);

  auto save = footer()->addWidget(std::make_unique<Wt::WPushButton>("Save"));
  save->clicked().connect(this, [this] {
    Wt::Dbo::Transaction trans(session_.session_);
    limit.modify()->preselected = preselected->value();
    limit.modify()->readyForDesign = readyForDesign->value();
    limit.modify()->designInProgress = designInProgress->value();
    limit.modify()->childsOfDesign = childsOfDesign->value();
    limit.modify()->readyForImplementation = readyForImplementation->value();
    limit.modify()->implementationInProgress = implementationInProgress->value();
    limit.modify()->postponed = postponed->value();
    hide();
  });

  auto cancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
  cancel->setDefault(true);
  cancel->clicked().connect(this, [this] { hide(); });
}

/**********************************************************************************************************************/
