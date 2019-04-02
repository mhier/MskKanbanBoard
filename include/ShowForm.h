/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef INCLUDE_SHOW_FORM_H_
#define INCLUDE_SHOW_FORM_H_

#include "Session.h"
#include "Form.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>

#include "ElogBackend.h"

using namespace Wt;

class PreviewDialog;

class ShowForm : public WContainerWidget {
 public:
  ShowForm(Session& session, const std::string& identifier);

  Session& session_;
  Wt::Dbo::ptr<Form> form_;
  bool createNew{false};
  size_t nRows{0};
  WTable* table;
  std::vector<WLineEdit*> fieldValues;
  std::unique_ptr<PreviewDialog> previewDialog_;
};

class PreviewDialog : public WDialog {
 public:
  PreviewDialog(Session& session, ShowForm* owner);
  ElogBackend backend;
};

#endif // INCLUDE_SHOW_FORM_H_
