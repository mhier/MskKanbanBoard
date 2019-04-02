#ifndef ISSUEUPDATER_H
#define ISSUEUPDATER_H

#include <thread>
#include <curl/curl.h>

class IssueUpdater {
 public:
  IssueUpdater() {
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl) {
      throw std::runtime_error("Could not initialise cURL");
    }
    updateThread = std::thread([this] { thread(); });
  }

  ~IssueUpdater() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

 private:
  void thread() {
    while(true) {
      update();
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1min);
    }
  }

  void update();

  std::string wget(const std::string& url);
  std::thread updateThread;

  CURL* curl;
  CURLcode res;
};

#endif // ISSUEUPDATER_H
