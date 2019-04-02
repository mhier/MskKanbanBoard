#include "IssueUpdater.h"
#include <iostream>

/**********************************************************************************************************************/

inline size_t curlWriteFunction(void* ptr, size_t, size_t nmemb, void* userdata) {
  std::string* buffer = static_cast<std::string*>(userdata);
  (*buffer) += std::string(static_cast<char*>(ptr), nmemb);
  return nmemb;
}

std::string IssueUpdater::wget(const std::string& url) {
  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "MskKanbanBoard");
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunction);
  res = curl_easy_perform(curl);
  if(res != CURLE_OK) {
    throw std::runtime_error("Reading from URL '" + url + "' failed: " + curl_easy_strerror(res));
  }
  return buffer;
}

/**********************************************************************************************************************/

void IssueUpdater::update() {
  auto jsonData = wget("https://api.github.com/search/issues?q=is%3Aopen+user%3AChimeraTK+label%3Aselected");
  std::cout << jsonData << std::endl;
}
