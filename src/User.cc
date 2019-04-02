/*
 * FormalisedLog - Tool for creation of standardised log book entires at XFEL and similar DESY facilities
 */

#include "User.h"

#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(User);

using namespace Wt::Dbo;

User::User() {}

User::~User() {}
