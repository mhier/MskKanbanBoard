#ifndef ELOGBACKEND_H
#define ELOGBACKEND_H

#include <map>
#include <iostream>
#include <chrono>
#include <curl/curl.h>

inline size_t curlWriteFunction(void* ptr, size_t, size_t nmemb, void* userdata) {
  std::string* buffer = static_cast<std::string*>(userdata);
  (*buffer) += std::string(static_cast<char*>(ptr), nmemb);
  return nmemb;
}

struct ElogBackend {
  ElogBackend() {
    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();

    if(!curl) {
      throw std::runtime_error("Could not initialise cURL");
    }

    postData["author"] = "AutomatedEntry";
    postData["severity"] = "NONE";
    postData["date"] = "";
    postData["time"] = "";
    postData["keywords"] = "not_set";
    postData["location"] = "not_set";
    postData["title"] = "";
    postData["text"] = "";
    postData["category"] = "USERLOG";
    postData["backlink"] = "https://www.desy.de";
    postData["image"] = "";
    postData["experts"] = "";
    postData["email"] = "";
    postData["femail"] = "";
  }

  ~ElogBackend() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

  void prepareNew(const std::string& logbook) {
    char buff[100];

    // obtain date and compute week of the year
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);

    tm local_tm = *localtime(&tt);
    tm tm_jan1st = local_tm;
    tm_jan1st.tm_mon = 0;
    tm_jan1st.tm_mday = 1;
    tm_jan1st.tm_yday = 0;
    tm_jan1st.tm_hour = 0;
    tm_jan1st.tm_min = 0;
    tm_jan1st.tm_sec = 0;
    auto tt_jan1st = mktime(&tm_jan1st);
    tm_jan1st = *localtime(&tt_jan1st);

    int julian = local_tm.tm_yday;
    int dow = local_tm.tm_wday;
    int dowJan1 = tm_jan1st.tm_wday;
    int weekNum = ((julian + 6) / 7);
    if(dow <= dowJan1) ++weekNum;

    int year = local_tm.tm_year + 1900;

    // determine morning, evening or late shift
    std::string shiftName = "n";
    if(local_tm.tm_hour >= 7 && local_tm.tm_hour < 15) {
      shiftName = "M";
    }
    else if(local_tm.tm_hour >= 15 && local_tm.tm_hour < 23) {
      shiftName = "a";
    }

    // generate file name with date and time
    snprintf(buff, sizeof(buff), "%04d-%02d-%02dT%02d:%02d:%02d-00.xml", year, local_tm.tm_mon + 1, local_tm.tm_mday,
        local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
    postData["metainfo"] = buff;

    // generate URL with date, week number, shift etc.
    snprintf(buff, sizeof(buff), "%04d/%02d/%02d.%02d_%s", year, weekNum, local_tm.tm_mday, local_tm.tm_mon + 1,
        shiftName.c_str());
    url = baseUrl + "/" + logbook + "/data/" + buff + "/" + postData["metainfo"];

    // set date and time for the post
    snprintf(buff, sizeof(buff), "%02d/%02d/%04d", local_tm.tm_mon + 1, local_tm.tm_mday, year);
    postData["date"] = buff;
    snprintf(buff, sizeof(buff), "%02d:%02d:%02d", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
    postData["time"] = buff;
  }

  void setTitle(const std::string& title) { postData["title"] = title; }

  void setText(const std::string& text) { postData["text"] = text; }

  void setAuthor(const std::string& author) { postData["author"] = author; }

  std::string post() {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    curl_mime* mime;
    mime = curl_mime_init(curl);
    for(auto& p : postData) {
      curl_mimepart* part;
      part = curl_mime_addpart(mime);
      curl_mime_name(part, p.first.c_str());
      curl_mime_data(part, p.second.c_str(), CURL_ZERO_TERMINATED);
      if(p.first == "image") {
        curl_mime_filename(part, "");
      }
    }
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    std::string buffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunction);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      throw std::runtime_error(std::string("Posting to log book failed: ") + curl_easy_strerror(res));
    }
    curl_mime_free(mime);
    return buffer;
  }

 protected:
  std::map<std::string, std::string> postData;

  std::string url;
  std::string baseUrl{"https://ttfinfo.desy.de/elog/FileEdit?source="};
  // std::string baseUrl{"http://localhost:1234/elog/FileEdit?source="};

  CURL* curl;
  CURLcode res;
};

#endif // ELOGBACKEND_H
