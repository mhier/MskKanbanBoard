/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef INCLUDE_FORM_H_
#define INCLUDE_FORM_H_

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

class FormField;

class Form {
 public:
  Form() {}

  std::string identifier;
  std::string title;
  std::string description;
  std::string author;
  std::string severity;
  std::string keyword;
  std::string location;
  std::string logbook;
  std::string intro;
  Wt::Dbo::collection<Wt::Dbo::ptr<FormField>> fields;

  static constexpr size_t identifierLength{20};

  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::id(a, identifier, "identifier", identifierLength); // special database key, also see below
    Wt::Dbo::field(a, title, "title");
    Wt::Dbo::field(a, description, "description");
    Wt::Dbo::field(a, logbook, "logbook");
    Wt::Dbo::field(a, author, "author");
    Wt::Dbo::field(a, severity, "severity");
    Wt::Dbo::field(a, keyword, "keyword");
    Wt::Dbo::field(a, location, "location");
    Wt::Dbo::field(a, intro, "intro");
    Wt::Dbo::hasMany(a, fields, Wt::Dbo::ManyToOne, "fields_of_form");
  }
};

// special configuration for database key
namespace Wt { namespace Dbo {
  template<>
  struct dbo_traits<Form> : public dbo_default_traits {
    typedef std::string IdType;
    static IdType invalidId() { return std::string(); }
    static const char* surrogateIdField() { return 0; }
  };
}} // namespace Wt::Dbo

class FormField {
 public:
  FormField() {}

  std::string title;
  std::string description;
  std::string value;
  Wt::Dbo::ptr<Form> form;

  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::field(a, title, "title");
    Wt::Dbo::field(a, description, "description");
    Wt::Dbo::field(a, value, "value");
    Wt::Dbo::belongsTo(a, form, "fields_of_form");
  }
};

#endif /* INCLUDE_FORM_H_ */
