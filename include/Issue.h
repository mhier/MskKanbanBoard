/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef INCLUDE_FORM_H_
#define INCLUDE_FORM_H_

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

class Issue;
class IssueReference {
  // Class for referencing issues in another issue. The Dbo layer does not support this directly...
 public:
  IssueReference() {}

  std::string identifier;
  Wt::Dbo::collection<Wt::Dbo::ptr<Issue>> issuesInProgressWhenDone;

  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::field(a, identifier, "identifier");
    Wt::Dbo::hasMany(a, issuesInProgressWhenDone, Wt::Dbo::ManyToMany, "issues_in_progress_while_done");
  }
};

enum class Priority { normal, urgent };

class Issue {
 public:
  Issue() {}

  std::string identifier;
  std::string project;
  int id;
  Priority priority;
  bool isOpen;
  bool isAssigned;
  bool isPostponed;
  bool isDesign;
  bool isDesignChild;
  bool isReadyForImplementation;
  bool isRemoved;
  std::string title;
  std::string assignee;
  std::string url;
  int lastChange;
  Wt::Dbo::collection<Wt::Dbo::ptr<IssueReference>> issuesDoneWhileInProgress;

  static constexpr size_t identifierLength{20};

  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::id(a, identifier, "identifier", identifierLength); // special database key, also see below
    Wt::Dbo::field(a, project, "project");
    Wt::Dbo::field(a, id, "id");
    Wt::Dbo::field(a, priority, "priority");
    Wt::Dbo::field(a, isOpen, "isOpen");
    Wt::Dbo::field(a, isAssigned, "isAssigned");
    Wt::Dbo::field(a, isPostponed, "isPostponed");
    Wt::Dbo::field(a, isDesign, "isDesign");
    Wt::Dbo::field(a, isDesignChild, "isDesignChild");
    Wt::Dbo::field(a, isReadyForImplementation, "isReadyForImplementation");
    Wt::Dbo::field(a, isRemoved, "isRemoved");
    Wt::Dbo::field(a, title, "title");
    Wt::Dbo::field(a, assignee, "assignee");
    Wt::Dbo::field(a, url, "url");
    Wt::Dbo::field(a, lastChange, "lastChange");
    Wt::Dbo::hasMany(a, issuesDoneWhileInProgress, Wt::Dbo::ManyToMany, "issues_done_while_in_progress");
  }
};

// special configuration for database key
namespace Wt { namespace Dbo {
  template<>
  struct dbo_traits<Issue> : public dbo_default_traits {
    typedef std::string IdType;
    static IdType invalidId() { return std::string(); }
    static const char* surrogateIdField() { return 0; }
  };
}} // namespace Wt::Dbo

#endif /* INCLUDE_FORM_H_ */
