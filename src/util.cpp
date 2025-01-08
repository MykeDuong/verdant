#include "util.h"
#include "parameters.h"
#include <cctype>
#include <string>
#include <sys/stat.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

static std::vector<size_t> findAllChar(const std::string& text, char ch) {
  std::vector<size_t> result;

  for (size_t i = 0; i < text.size(); i++) {
    if (text[i] == ch) {
      result.push_back(i);
    }
  }
  return result;
}
namespace Utility {
  std::string toLower(const std::string& str) {
    std::string lowerString;

    for (char c: str) {
      lowerString.push_back(std::tolower(c));
    }
    return lowerString;
  }
  std::string expandUser(std::string path) {
    if (not path.empty() and path[0] == '~') {
      assert(path.size() == 1 or path[1] == '/');  // or other error handling
      char const* home = getenv("HOME");
      if (home or (home = getenv("USERPROFILE"))) {
        path.replace(0, 1, home);
      }
      else {
        char const *hdrive = getenv("HOMEDRIVE"),
          *hpath = getenv("HOMEPATH");
        assert(hdrive);  // or other error handling
        assert(hpath);
        path.replace(0, 1, std::string(hdrive) + hpath);
      }
    }
    return path;
  }
  bool createDirectory(const std::string& userPath) {
    std::string path = userPath[0] == '~' ? expandUser(userPath) : userPath;
    assert(path[0] == '/');
    
    if (path[path.size() - 1] != '/') {
      path.push_back('/');
    }
    
    std::vector<size_t> slashes = findAllChar(path, '/');
    int status;
    for (size_t i = 0; i < slashes.size(); i++) {
      if (i == 0) {
        continue;
      }
      size_t slashIdx = slashes[i];
      std::string curPath = path.substr(0, slashIdx + 1);
#ifdef VERDANT_FLAG_DEBUG
      std::cout << "[DEBUG] Creating path " << curPath << std::endl;
#endif
      status = mkdir(curPath.c_str(), 0744);
      if ((status < 0) && (errno != EEXIST)) {
        return false;
      }
    }
    return true;
  }

  bool isDirectoryExist(const std::string& userPath) {
    std::string path = userPath[0] == '~' ? expandUser(userPath) : userPath;
    struct stat sb;

    if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
      return true;
    } else {
      return false;
    }
  }
  std::string getDatabasePath(const std::string& database) {
    std::string userPath = DATA_PATH + database + "/";
    return userPath[0] == '~' ? expandUser(userPath) : userPath;
  }

  bool isAlpha(const char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
  }
}
