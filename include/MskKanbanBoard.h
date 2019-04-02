/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef INCLUDE_DEPLOY_TOOL_H_
#define INCLUDE_DEPLOY_TOOL_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WMenu.h>

#include "Session.h"

using namespace Wt;

namespace Wt {
  class WStackedWidget;
  class WAnchor;
} // namespace Wt

class Session;

class MskKanbanBoard : public WContainerWidget {
 public:
  MskKanbanBoard();

  void handleInternalPath(const std::string& internalPath);

 private:
  WStackedWidget* contentStack_{nullptr};

  Session session_;
};

#endif // INCLUDE_DEPLOY_TOOL_H_
