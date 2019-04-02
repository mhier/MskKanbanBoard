/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef INCLUDE_SESSION_H_
#define INCLUDE_SESSION_H_

#include <vector>

#include <Wt/Auth/Login.h>

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include "User.h"

using namespace Wt;

typedef Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

// List of logbooks we can post to. This is part of the logbook URL
static const std::vector<std::string> logbooks{"TESTelog", "XFELelog"};

class Session {
 public:
  static void configureAuth();

  Session();
  ~Session();

  Auth::AbstractUserDatabase& users();
  Auth::Login& login() { return login_; }

  /*
   * These methods deal with the currently logged in user
   */
  std::string userName() const;

  Dbo::ptr<User> user() const;

  static const Auth::AuthService& auth();
  static const Auth::AbstractPasswordService& passwordAuth();

  mutable Dbo::Session session_;

  // add a new user
  void registerUser(std::string login, std::string email, Wt::WString password);

  // update a user (password will not be changed if empty)
  void updateUser(Wt::Dbo::ptr<User> user, std::string email, Wt::WString password);

  std::unique_ptr<UserDatabase> users_;
  Auth::Login login_;
};

#endif // INCLUDE_SESSION_H_
