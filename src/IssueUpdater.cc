#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>

#include "IssueUpdater.h"
#include <iostream>

#include "Issue.h"
#include "json11.hpp"

std::atomic<std::time_t> IssueUpdater::lastUpdate;

/**********************************************************************************************************************/

inline size_t curlWriteFunction(void* ptr, size_t, size_t nmemb, void* userdata) {
  std::string* buffer = static_cast<std::string*>(userdata);
  (*buffer) += std::string(static_cast<char*>(ptr), nmemb);
  return nmemb;
}

/**********************************************************************************************************************/

std::string IssueUpdater::wget(const std::string& url, bool isRedmine) {
  //std::cout << "wget: " << url << std::endl;
  std::string buffer;
  auto curl = curlGithub;
  if(isRedmine) curl = curlRedmine;
  //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "MskKanbanBoard");
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunction);
  if(isRedmine) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    curl_easy_setopt(curl, CURLOPT_USERPWD, "kanban_board:_@&rd$249J");
  }
  res = curl_easy_perform(curl);
  if(res != CURLE_OK) {
    throw std::runtime_error("Reading from URL '" + url + "' failed: " + curl_easy_strerror(res));
  }
  return buffer;
}

/**********************************************************************************************************************/

IssueUpdater::IssueUpdater() : updateThread([this] { theThread(); }) {
  terminate = false;
}

/**********************************************************************************************************************/

IssueUpdater::~IssueUpdater() {
  curl_easy_cleanup(curlGithub);
  curl_easy_cleanup(curlRedmine);
  curl_global_cleanup();
  terminate = true;
  updateThread.join();
}

/**********************************************************************************************************************/

void IssueUpdater::theThread() {
  curl_global_init(CURL_GLOBAL_ALL);
  curlGithub = curl_easy_init();
  if(!curlGithub) {
    throw std::runtime_error("Could not initialise cURL");
  }
  curlRedmine = curl_easy_init();
  if(!curlRedmine) {
    throw std::runtime_error("Could not initialise cURL");
  }

  getDatabaseSession(session);

  while(!terminate) {
    try {
      update();
    }
    catch(std::exception& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
    }
    size_t nsecs = 60;
    if(initialUpdateGithub || initialUpdateRedmine) nsecs = 5;
    using namespace std::chrono_literals;
    for(size_t i = 0; i < nsecs; ++i) {
      std::this_thread::sleep_for(1s);
      if(terminate) return;
    }
  }
}

/**********************************************************************************************************************/

void IssueUpdater::update() {
  Dbo::Transaction transaction(session);
  githubUpdate();
  redmineUpdate();
  lastUpdate = std::time(nullptr);
}

/**********************************************************************************************************************/

void IssueUpdater::githubUpdate() {
  auto jsonData = wget(
      "https://api.github.com/search/issues?q=user%3AChimeraTK+-label%3Alongterm+label%3Aselected+updated%3A%3E%3D" +
          lastUpdateGithub + "&sort=updated&order=asc",
      false);
  std::string err;
  auto parsed = json11::Json::parse(jsonData, err);
  if(parsed.is_null()) {
    std::cerr << "Error parsing github data: " << err << std::endl;
    initialUpdateGithub = false;
    return;
  }

  if(!parsed["items"].is_array()) {
    std::cerr << "Error parsing github data: 'items' is not an array." << std::endl;
    initialUpdateGithub = false;
    return;
  }

  for(auto& item : parsed["items"].array_items()) {
    std::string identifier = "gh:" + std::to_string(item["id"].int_value());
    auto issues = session.find<Issue>().where("identifier = ?").bind(identifier).limit(1).resultList();
    // check if issue does not yet exist in database
    if(issues.size() == 0) {
      // not existing: create it
      auto issue = Dbo::make_ptr<Issue>();
      issue.modify()->identifier = identifier;
      issue.modify()->id = item["number"].int_value();
      issue.modify()->url = item["html_url"].string_value();
      issue.modify()->title = item["title"].string_value();
      std::vector<std::string> urlsplit;
      boost::algorithm::split(urlsplit, issue->url, boost::algorithm::is_any_of("/"));
      issue.modify()->project = urlsplit[4];
      issue.modify()->isAssigned = !item["assignee"].is_null();
      if(issue.modify()->isAssigned) {
        issue.modify()->assignee = item["assignee"]["login"].string_value();
      }
      else {
        issue.modify()->assignee = "";
      }
      issue.modify()->isDesign = false;
      issue.modify()->isDesignChild = false;
      issue.modify()->isReadyForImplementation = false;
      issue.modify()->isPostponed = false;
      issue.modify()->isRemoved = false;
      issue.modify()->priority = Priority::normal;
      for(auto& label : item["labels"].array_items()) {
        if(label["name"] == "umbrella") issue.modify()->isDesign = true;
        if(label["name"] == "umbrellaChild") issue.modify()->isDesignChild = true;
        if(label["name"] == "postponed") issue.modify()->isPostponed = true;
        if(label["name"] == "readyForImplementation") issue.modify()->isReadyForImplementation = true;
        if(label["name"] == "urgent") issue.modify()->priority = Priority::urgent;
      }
      issue.modify()->isOpen = item["state"].string_value() != "closed";

      boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
      auto tstring = item["updated_at"].string_value();
      tstring[10] = ' ';
      tstring[19] = '.';
      tstring += "000";
      auto time = boost::posix_time::time_from_string(tstring);
      issue.modify()->lastChange = (time - epoch).total_seconds();

      session.add(issue);
    }
    else {
      // already existing: update it
      bool statusChange = false;
      auto issue = issues.front();
      if(issue->title != item["title"].string_value()) issue.modify()->title = item["title"].string_value();
      bool isAssigned = !item["assignee"].is_null();
      if(issue->isAssigned != isAssigned) {
        statusChange = true;
        issue.modify()->isAssigned = isAssigned;
      }
      std::string assignee;
      if(isAssigned) assignee = item["assignee"]["login"].string_value();
      if(issue->assignee != assignee) issue.modify()->assignee = assignee;
      bool isDesign = false;
      bool isDesignChild = false;
      bool isReadyForImplementation = false;
      bool isPostponed = false;
      Priority priority = Priority::normal;
      for(auto& label : item["labels"].array_items()) {
        if(label["name"] == "umbrella") isDesign = true;
        if(label["name"] == "umbrellaChild") isDesignChild = true;
        if(label["name"] == "postponed") isPostponed = true;
        if(label["name"] == "readyForImplementation") isReadyForImplementation = true;
        if(label["name"] == "urgent") priority = Priority::urgent;
      }
      if(issue->isDesign != isDesign) {
        issue.modify()->isDesign = isDesign;
        statusChange = true;
      }
      if(issue->isDesignChild != isDesignChild) {
        issue.modify()->isDesignChild = isDesignChild;
        statusChange = true;
      }
      if(issue->isPostponed != isPostponed) {
        issue.modify()->isPostponed = isPostponed;
        statusChange = true;
      }
      if(issue->isReadyForImplementation != isReadyForImplementation) {
        issue.modify()->isReadyForImplementation = isReadyForImplementation;
        statusChange = true;
      }
      if(issue->priority != priority) {
        issue.modify()->priority = priority;
      }
      bool isOpen = (item["state"].string_value() != "closed");
      if(issue->isOpen != isOpen) {
        issue.modify()->report_isOpen(isOpen, session);
        statusChange = true;
      }
      if(statusChange) {
        boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
        auto tstring = item["updated_at"].string_value();
        tstring[10] = ' ';
        tstring[19] = '.';
        tstring += "000";
        auto time = boost::posix_time::time_from_string(tstring);
        issue.modify()->lastChange = (time - epoch).total_seconds();
        issue.modify()->isRemoved = false;
      }
    }

    lastUpdateGithub = item["updated_at"].string_value().substr(0, 10); // first 10 characters are the date
  }

  if(parsed["total_count"].int_value() < 30) {
    if(initialUpdateGithub) {
      std::cout << "Initial update of github issues complete" << std::endl;
      initialUpdateGithub = false;
    }
  }
}

/**********************************************************************************************************************/

void IssueUpdater::redmineUpdate() {
  // status_id=* -> return both opend and closed issues
  // cf_15=1 -> custom field "selected"
  auto jsonData = wget("https://mskllrfredminesrv.desy.de/"
                       "issues.json?status_id=*&cf_15=1&cf_17=0&limit=100&sort=updated_on&updated_on=>=" +
          lastUpdateRedmine,
      true);
  std::string err;
  auto parsed = json11::Json::parse(jsonData, err);
  if(parsed.is_null()) {
    std::cerr << "Error parsing redmine data: " << err << std::endl;
    initialUpdateRedmine = false;
    return;
  }

  if(!parsed["issues"].is_array()) {
    std::cerr << "Error parsing redmine data: 'issues' is not an array." << std::endl;
    initialUpdateRedmine = false;
    return;
  }

  for(auto& item : parsed["issues"].array_items()) {
    std::string identifier = "rm:" + std::to_string(item["id"].int_value());
    auto issues = session.find<Issue>().where("identifier = ?").bind(identifier).limit(1).resultList();
    // check if issue does not yet exist in database
    if(issues.size() == 0) {
      // not existing: create it
      auto issue = Dbo::make_ptr<Issue>();
      issue.modify()->identifier = identifier;
      issue.modify()->id = item["id"].int_value();
      issue.modify()->url = "https://mskllrfredminesrv.desy.de/issues/" + std::to_string(issue.modify()->id);
      issue.modify()->title = item["subject"].string_value();
      issue.modify()->project = item["project"]["name"].string_value();
      issue.modify()->isAssigned = !item["assigned_to"].is_null();
      issue.modify()->priority = Priority::normal;
      if(item["priority"]["name"].string_value() == "Urgent") {
        issue.modify()->priority = Priority::urgent;
      }
      if(issue.modify()->isAssigned) {
        issue.modify()->assignee = item["assigned_to"]["name"].string_value();
      }
      else {
        issue.modify()->assignee = "";
      }
      for(auto& label : item["custom_fields"].array_items()) {
        if(label["name"] == "umbrella") issue.modify()->isDesign = std::stoi(label["value"].string_value());
        if(label["name"] == "umbrellaChild") issue.modify()->isDesignChild = std::stoi(label["value"].string_value());
        if(label["name"] == "postponed") issue.modify()->isPostponed = std::stoi(label["value"].string_value());
        if(label["name"] == "readyForImplementation") {
          issue.modify()->isReadyForImplementation = std::stoi(label["value"].string_value());
        }
      }
      issue.modify()->isOpen = (item["status"]["name"].string_value() != "Closed");

      boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
      auto tstring = item["updated_on"].string_value();
      tstring[10] = ' ';
      tstring[19] = '.';
      tstring += "000";
      auto time = boost::posix_time::time_from_string(tstring);
      issue.modify()->lastChange = (time - epoch).total_seconds();

      session.add(issue);
    }
    else {
      // already existing: update it
      bool statusChange = false;
      auto issue = issues.front();
      if(issue->title != item["subject"].string_value()) issue.modify()->title = item["subject"].string_value();
      bool isAssigned = !item["assigned_to"].is_null();
      Priority priority = Priority::normal;
      if(item["priority"]["name"].string_value() == "Urgent") {
        priority = Priority::urgent;
      }
      if(issue->priority != priority) {
        issue.modify()->priority = priority;
      }
      if(issue->isAssigned != isAssigned) {
        statusChange = true;
        issue.modify()->isAssigned = isAssigned;
      }
      std::string assignee;
      if(isAssigned) assignee = item["assigned_to"]["name"].string_value();
      if(issue->assignee != assignee) issue.modify()->assignee = assignee;
      bool isDesign = false;
      bool isDesignChild = false;
      bool isReadyForImplementation = false;
      bool isPostponed = false;
      for(auto& label : item["custom_fields"].array_items()) {
        if(label["name"] == "umbrella") isDesign = std::stoi(label["value"].string_value());
        if(label["name"] == "umbrellaChild") isDesignChild = std::stoi(label["value"].string_value());
        if(label["name"] == "postponed") isPostponed = std::stoi(label["value"].string_value());
        if(label["name"] == "readyForImplementation")
          isReadyForImplementation = std::stoi(label["value"].string_value());
      }
      if(issue->isDesign != isDesign) {
        issue.modify()->isDesign = isDesign;
        statusChange = true;
      }
      if(issue->isDesignChild != isDesignChild) {
        issue.modify()->isDesignChild = isDesignChild;
        statusChange = true;
      }
      if(issue->isPostponed != isPostponed) {
        issue.modify()->isPostponed = isPostponed;
        statusChange = true;
      }
      if(issue->isReadyForImplementation != isReadyForImplementation) {
        issue.modify()->isReadyForImplementation = isReadyForImplementation;
        statusChange = true;
      }
      bool isOpen = (item["status"]["name"].string_value() != "Closed");
      if(issue->isOpen != isOpen) {
        issue.modify()->report_isOpen(isOpen, session);
        statusChange = true;
      }
      if(statusChange) {
        boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
        auto tstring = item["updated_on"].string_value();
        tstring[10] = ' ';
        tstring[19] = '.';
        tstring += "000";
        auto time = boost::posix_time::time_from_string(tstring);
        issue.modify()->lastChange = (time - epoch).total_seconds();
      }
    }

    lastUpdateRedmine = item["updated_on"].string_value().substr(0, 10); // first 10 characters are the date
  }

  if(parsed["total_count"].int_value() < 100) {
    if(initialUpdateRedmine) {
      std::cout << "Initial update of redmine issues complete" << std::endl;
      initialUpdateRedmine = false;
    }
  }
}
