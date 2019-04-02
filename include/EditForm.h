/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef INCLUDE_EDIT_FORM_H_
#define INCLUDE_EDIT_FORM_H_

#include "Session.h"
#include "Form.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>

using namespace Wt;

class FormDialog;

class EditForm : public Wt::WContainerWidget {
 public:
  EditForm(Session& session);

  void update();

 private:
  Session& session_;
  std::unique_ptr<FormDialog> formDialog_;
};

class FormDialog : public WDialog {
 public:
  FormDialog(EditForm& owner, Session& session, Wt::Dbo::ptr<Form> form)
  : owner_(owner), session_(session), form_(form) {
    if(form_.get() == nullptr) {
      createNew = true;
      form_ = std::make_unique<Form>();
    }
    update();
  }

  void update();

  EditForm& owner_;
  Session& session_;
  Wt::Dbo::ptr<Form> form_;
  bool createNew{false};
  int nRows{0};
  WTable* table;
  std::vector<WLineEdit*> fieldTitles;
  std::vector<WLineEdit*> fieldDescriptions;

  void addRow(const std::string& title = "", const std::string& description = "");
};

#endif // INCLUDE_EDIT_FORM_H_
