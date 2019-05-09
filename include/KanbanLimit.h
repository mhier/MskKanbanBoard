/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef KANBAN_LIMIT_H
#define KANBAN_LIMIT_H

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

class KanbanLimit {
 public:
  KanbanLimit() {}

  int preselected{0};
  int readyForDesign{0};
  int designInProgress{0};
  int childsOfDesign{0};
  int readyForImplementation{0};
  int implementationInProgress{0};
  int postponed{0};

  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::field(a, preselected, "preselected");
    Wt::Dbo::field(a, readyForDesign, "readyForDesign");
    Wt::Dbo::field(a, designInProgress, "designInProgress");
    Wt::Dbo::field(a, childsOfDesign, "childsOfDesign");
    Wt::Dbo::field(a, readyForImplementation, "readyForImplementation");
    Wt::Dbo::field(a, implementationInProgress, "implementationInProgress");
    Wt::Dbo::field(a, postponed, "postponed");
  }
};

#endif /* KANBAN_LIMIT_H */
