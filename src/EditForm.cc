/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#include "EditForm.h"
#include "Form.h"

#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTextArea.h>
#include <Wt/WApplication.h>
#include <Wt/WComboBox.h>
#include <Wt/WEnvironment.h>

class FormDialog;

EditForm::EditForm(Session& session) : session_(session) {
  update();
}

void EditForm::update() {
  clear();

  auto user = session_.user();

  dbo::Transaction transaction(session_.session_);

  addWidget(std::make_unique<WText>("<h2>Create and edit forms</h2>"));
  decorationStyle().setBorder({Wt::BorderStyle::Solid, {10}, {255, 0, 0}});

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
      table->elementAt(row, i)->clicked().connect(this, [=] {
        formDialog_ = std::make_unique<FormDialog>(*this, session_, form);
        formDialog_->show();
      });
    }
  }

  addWidget(std::move(table));

  auto newForm = addWidget(std::make_unique<Wt::WPushButton>("Create form..."));
  newForm->clicked().connect(this, [=] {
    formDialog_ = std::make_unique<FormDialog>(*this, session_, nullptr);
    formDialog_->show();
  });

  addWidget(std::make_unique<Wt::WText>(" "));

  auto toWelcomePage = addWidget(std::make_unique<Wt::WPushButton>("Go to front page"));
  toWelcomePage->clicked().connect(this, [=] { WApplication::instance()->setInternalPath("/", true); });
}

/**********************************************************************************************************************/

void FormDialog::update() {
  titleBar()->clear();
  contents()->clear();
  footer()->clear();
  contents()->addStyleClass("form-group");
  Dbo::Transaction transaction(session_.session_);

  titleBar()->addWidget(std::make_unique<WText>("Edit form"));

  auto layout = contents()->setLayout(std::make_unique<Wt::WVBoxLayout>());

  const WEnvironment& env = WApplication::instance()->environment();
  auto screenHeight = env.screenHeight();
  auto windowHeight = screenHeight > 800 ? screenHeight * 0.66 : screenHeight;

  contents()->setOverflow(Wt::Overflow::Scroll);
  setMinimumSize(WLength::Auto, WLength(windowHeight, LengthUnit::Pixel));
  setMaximumSize(WLength::Auto, WLength(windowHeight, LengthUnit::Pixel));

  auto logbook = layout->addWidget(std::make_unique<WComboBox>());
  logbook->setMinimumSize(WLength::Auto, WLength(2, LengthUnit::Pica));
  logbook->setMaximumSize(WLength::Auto, WLength(2, LengthUnit::Pica));
  int counter = 0;
  for(auto& l : logbooks) {
    logbook->addItem(l);
    if(form_->logbook == l) logbook->setCurrentIndex(counter);
    ++counter;
  }

  auto formIdentifier = layout->addWidget(std::make_unique<WLineEdit>());
  formIdentifier->setMinimumSize(WLength::Auto, WLength(2, LengthUnit::Pica));
  formIdentifier->setMaximumSize(WLength::Auto, WLength(2, LengthUnit::Pica));
  formIdentifier->setPlaceholderText("Identifier (cannot be changed later)");
  formIdentifier->setText(form_.get()->identifier);
  formIdentifier->setMaxLength(Form::identifierLength);
  if(!createNew) formIdentifier->setReadOnly(true);

  auto formTitle = layout->addWidget(std::make_unique<WLineEdit>());
  formTitle->setMinimumSize(WLength::Auto, WLength(2, LengthUnit::Pica));
  formTitle->setMaximumSize(WLength::Auto, WLength(2, LengthUnit::Pica));
  formTitle->setPlaceholderText("Title of the form");
  formTitle->setText(form_.get()->title);

  auto formDescription = layout->addWidget(std::make_unique<WTextArea>());
  formDescription->setMinimumSize(WLength::Auto, WLength(5, LengthUnit::Pica));
  formDescription->setMaximumSize(WLength::Auto, WLength(5, LengthUnit::Pica));
  formDescription->setPlaceholderText("Detailled description displayed on top");
  formDescription->setText(form_.get()->description);

  table = layout->addWidget(std::make_unique<WTable>());
  table->setHeaderCount(1);
  table->setWidth(WLength("100%"));
  table->addStyleClass("table form-inline table-hover");

  table->elementAt(0, 0)->addWidget(std::make_unique<WText>("Field title"));
  table->elementAt(0, 1)->addWidget(std::make_unique<WText>("Description"));
  for(auto field : form_.get()->fields) addRow(field->title, field->description);
  addRow();

  auto save = footer()->addWidget(std::make_unique<WPushButton>("Save"));
  save->clicked().connect([=] {
    Dbo::Transaction transact(session_.session_);
    if(createNew) {
      // check if identifier already exists
      auto n = session_.session_.find<Form>().where("identifier = ?").bind(formIdentifier->text()).resultList().size();
      if(n > 0) {
        formIdentifier->setFocus(true);
        return;
      }

      form_.modify()->identifier = formIdentifier->text().toUTF8();
    }
    form_.modify()->title = formTitle->text().toUTF8();
    form_.modify()->logbook = logbook->currentText().toUTF8();
    form_.modify()->description = formDescription->text().toUTF8();
    if(createNew) session_.session_.add(form_);
    form_.modify()->fields.clear();
    for(size_t i = 0; i < static_cast<size_t>(nRows); ++i) {
      if(fieldTitles[i]->text().toUTF8().size() == 0) continue;
      Wt::Dbo::ptr<FormField> field = std::make_unique<FormField>();
      field.modify()->form = form_;
      field.modify()->title = fieldTitles[i]->text().toUTF8();
      field.modify()->description = fieldDescriptions[i]->text().toUTF8();
      form_.modify()->fields.insert(field);
    }
    this->hide();
    owner_.update();
  });
  auto cancel = footer()->addWidget(std::make_unique<WPushButton>("Cancel"));
  cancel->clicked().connect([this] { this->hide(); });
}

void FormDialog::addRow(const std::string& titleValue, const std::string& descriptionValue) {
  ++nRows;
  auto title = table->elementAt(nRows, 0)->addWidget(std::make_unique<WLineEdit>());
  fieldTitles.push_back(title);
  title->setPlaceholderText("Field title");
  title->setText(titleValue);
  title->setWidth(WLength("100%"));
  int thisRow = nRows;
  title->changed().connect([=] {
    if(title->text().toUTF8().size() > 0 && thisRow == this->nRows) addRow();
  });
  auto description = table->elementAt(nRows, 1)->addWidget(std::make_unique<WLineEdit>());
  fieldDescriptions.push_back(description);
  description->setPlaceholderText("Description / help text");
  description->setText(descriptionValue);
  description->setWidth(WLength("100%"));
}
