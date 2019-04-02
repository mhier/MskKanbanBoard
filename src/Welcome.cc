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

#include "Form.h"

Welcome::Welcome(Session& session) : session_(session) {
  clear();

  auto user = session_.user();

  dbo::Transaction transaction(session_.session_);

  addWidget(std::make_unique<WText>("<h2>Welcome!</h2>"));

  auto table = std::make_unique<WTable>();
  table->setHeaderCount(1);
  table->setWidth(WLength("100%"));
  table->addStyleClass("table form-inline table-hover");

  table->elementAt(0, 0)->addWidget(std::make_unique<WText>("#"));
  table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Logbook"));
  table->elementAt(0, 2)->addWidget(std::make_unique<WText>("Form title"));
  table->elementAt(0, 3)->addWidget(std::make_unique<WText>("Description"));

  auto forms = session_.session_.find<Form>().resultList();
  int row = 0;
  for(auto form : forms) {
    row++;

    table->elementAt(row, 0)->addWidget(std::make_unique<WText>(WString("{1}").arg(row)));
    table->elementAt(row, 1)->addWidget(std::make_unique<WText>(form->logbook));
    table->elementAt(row, 2)->addWidget(std::make_unique<WText>(form->title));
    table->elementAt(row, 3)->addWidget(std::make_unique<WText>(form->description));

    for(int i = 0; i < 3; ++i) {
      table->elementAt(row, i)->clicked().connect(
          this, [=] { WApplication::instance()->setInternalPath("/show/" + form->identifier, true); });
    }
  }

  addWidget(std::move(table));

  addWidget(std::make_unique<Wt::WAnchor>(
      Wt::WLink(Wt::LinkType::InternalPath, "/edit"), "Edit/Create Forms (KEEP OUT! EXPERTS ONLY!)"));
}
