/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#ifndef INCLUDE_WELCOME_H_
#define INCLUDE_WELCOME_H_

#include "Session.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

class Welcome : public WContainerWidget {
 public:
  Welcome(Session& session);

 private:
  Session& session_;
};

#endif // INCLUDE_WELCOME_H_
