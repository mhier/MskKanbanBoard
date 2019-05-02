#ifndef ISSUEUPDATER_H
#define ISSUEUPDATER_H

#include "Session.h"

#include <atomic>
#include <thread>
#include <curl/curl.h>

class IssueUpdater {
 public:
  IssueUpdater();

  ~IssueUpdater();

 private:
  void theThread();

  void update();
  void githubUpdate();
  void redmineUpdate();

  std::string wget(const std::string& url, bool isRedmine);
  std::thread updateThread;

  CURL* curlGithub;
  CURL* curlRedmine;
  CURLcode res;

  Dbo::Session session;

  std::atomic<bool> terminate;

  bool initialUpdateGithub = true;
  bool initialUpdateRedmine = true;
  std::string lastUpdateGithub{"1970-01-01"};
  std::string lastUpdateRedmine{"1970-01-01"};

 public:
  static std::atomic<std::time_t> lastUpdate;
};

#endif // ISSUEUPDATER_H
