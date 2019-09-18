/*
 * Copyright 2011-2014 Emmanuel Engelhart <kelson@kiwix.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "tools/pathTools.h"

#ifdef __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>
#elif _WIN32
#include <direct.h>
#include <windows.h>
#include "shlwapi.h"
#define getcwd _getcwd  // stupid MSFT "deprecation" warning
#endif

#include "tools/stringTools.h"

#include <string.h>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#ifdef _WIN32
  #define SEPARATOR "\\"
#else
  #define SEPARATOR "/"
#include <unistd.h>
#endif

#include <stdlib.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

bool isRelativePath(const std::string& path)
{
#ifdef _WIN32
  return path.empty() || path.substr(1, 2) == ":\\" ? false : true;
#else
  return path.empty() || path.substr(0, 1) == "/" ? false : true;
#endif
}

std::vector<std::string> normalizeParts(std::vector<std::string> parts, bool absolute)
{
  std::vector<std::string> ret;
#ifdef _WIN32
  //Special case if we have a drive directory not at first.
  //Starts from there.
  auto it = find_if(parts.rbegin(), parts.rend(),
                    [](const std::string& p) ->bool
                      { return p.length() == 2 && p[1] == ':'; });
  if (it != parts.rend()) {
    parts.erase(parts.begin(), it.base()-1);
  }
#endif

  for (auto& part: parts) {
    if (part == "..") {
      if (absolute) {
        // We try to remove as far as possible.
        if (ret.size() > 1) {
          ret.pop_back();
        }
      } else {
        // We remove only if we can remove it.
        // Else we add it.
        if (!ret.empty() && ret.back() != "..") {
          ret.pop_back();
        } else {
          ret.push_back("..");
        }
      }
      continue;
    }
    if (part == "") {
#ifndef _WIN32
      if (ret.empty()) {
        ret.push_back("");
      }
#endif
      continue;
    }
    if (part == ".") {
      continue;
    }
    ret.push_back(part);
  }
#ifndef _WIN32
  if (absolute && ret.size() == 1 && ret.back() == "") {
    ret.push_back("");
  }
#endif
  return ret;
}

std::string computeRelativePath(const std::string& path, const std::string& absolutePath)
{
  auto pathParts = normalizeParts(kiwix::split(path, SEPARATOR, false), false);
  auto absolutePathParts = kiwix::split(absolutePath, SEPARATOR, false);

  unsigned int commonCount = 0;
  while (commonCount < pathParts.size()
         && commonCount < absolutePathParts.size()
         && pathParts[commonCount] == absolutePathParts[commonCount]) {
      commonCount++;
  }

  std::vector<std::string> relativeParts;
  for (unsigned int i = commonCount; i < pathParts.size(); i++) {
    relativeParts.push_back("..");
  }
  for (unsigned int i = commonCount; i < absolutePathParts.size(); i++) {
    relativeParts.push_back(absolutePathParts[i]);
  }
  return kiwix::join(normalizeParts(relativeParts, false), SEPARATOR);
}

std::string computeAbsolutePath(const std::string& path, const std::string& relativePath)
{
  std::string absolutePath = path;
  if (path.empty()) {
    char* cpath;
#ifdef _WIN32
    cpath = _getcwd(NULL, 0);
#else
    cpath = getcwd(NULL, 0);
#endif
    absolutePath = cpath;
    free(cpath);
  }

  auto absoluteParts = normalizeParts(kiwix::split(absolutePath, SEPARATOR, false), true);
  auto relativeParts = kiwix::split(relativePath, SEPARATOR, false);

  absoluteParts.insert(absoluteParts.end(), relativeParts.begin(), relativeParts.end());
  return kiwix::join(normalizeParts(absoluteParts, true), SEPARATOR);
}

std::string removeLastPathElement(const std::string& path)
{
  auto parts = normalizeParts(kiwix::split(path, SEPARATOR, false), false);
  if (!parts.empty()) {
    parts.pop_back();
  }
  return kiwix::join(parts, SEPARATOR);
}

std::string appendToDirectory(const std::string& directoryPath, const std::string& filename)
{
  std::string newPath = directoryPath;
  if (!directoryPath.empty() && directoryPath.back() != SEPARATOR[0]) {
    newPath += SEPARATOR;
  }
  newPath += filename;
  return newPath;
}

std::string getLastPathElement(const std::string& path)
{
  auto parts = normalizeParts(kiwix::split(path, SEPARATOR), false);
  if (parts.empty()) {
    return "";
  }
  return parts.back();
}

unsigned int getFileSize(const std::string& path)
{
#ifdef _WIN32
  struct _stat filestatus;
  _stat(path.c_str(), &filestatus);
#else
  struct stat filestatus;
  stat(path.c_str(), &filestatus);
#endif

  return filestatus.st_size / 1024;
}

std::string getFileSizeAsString(const std::string& path)
{
  std::ostringstream convert;
  convert << getFileSize(path);
  return convert.str();
}

std::string getFileContent(const std::string& path)
{
  std::ifstream f(path, std::ios::in|std::ios::ate);
  std::string content;
  if (f.is_open()) {
    auto size = f.tellg();
    content.reserve(size);
    f.seekg(0, std::ios::beg);
    content.assign((std::istreambuf_iterator<char>(f)),
                    std::istreambuf_iterator<char>());
  }
  return content;
}

bool fileExists(const std::string& path)
{
#ifdef _WIN32
  return PathFileExists(path.c_str());
#else
  bool flag = false;
  std::fstream fin;
  fin.open(path.c_str(), std::ios::in);
  if (fin.is_open()) {
    flag = true;
  }
  fin.close();
  return flag;
#endif
}

bool makeDirectory(const std::string& path)
{
#ifdef _WIN32
  int status = _mkdir(path.c_str());
#else
  int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
  return status == 0;
}

std::string makeTmpDirectory()
{
#ifdef _WIN32
  char cbase[MAX_PATH];
  char ctmp[MAX_PATH];
  GetTempPath(MAX_PATH-14, cbase);
  // This create a file for us, ensure it is unique.
  // So we need to delete it and create the directory using the same name.
  GetTempFileName(cbase, "kiwix", 0, ctmp);
  DeleteFile(ctmp);
  _mkdir(ctmp);
  return std::string(ctmp);
#else
  char _template_array[] = {"/tmp/kiwix-lib_XXXXXX"};
  std::string dir = mkdtemp(_template_array);
  return dir;
#endif
}

/* Try to create a link and if does not work then make a copy */
bool copyFile(const std::string& sourcePath, const std::string& destPath)
{
  try {
#ifndef _WIN32
    if (link(sourcePath.c_str(), destPath.c_str()) != 0) {
#endif
      std::ifstream infile(sourcePath.c_str(), std::ios_base::binary);
      std::ofstream outfile(destPath.c_str(), std::ios_base::binary);
      outfile << infile.rdbuf();
#ifndef _WIN32
    }
#endif
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}

std::string getExecutablePath(bool realPathOnly)
{
  char binRootPath[PATH_MAX];

  if (!realPathOnly) {
    char* cAppImage = ::getenv("APPIMAGE");
    if (cAppImage) {
      char* cArgv0 = ::getenv("ARGV0");
      char* cOwd = ::getenv("OWD");
      if (cArgv0 && cOwd) {
        return appendToDirectory(cOwd, cArgv0);
      }
    }
  }

#ifdef _WIN32
  GetModuleFileName(NULL, binRootPath, PATH_MAX);
  return std::string(binRootPath);
#elif __APPLE__
  uint32_t max = (uint32_t)PATH_MAX;
  _NSGetExecutablePath(binRootPath, &max);
  return std::string(binRootPath);
#else
  ssize_t size = readlink("/proc/self/exe", binRootPath, PATH_MAX);
  if (size != -1) {
    return std::string(binRootPath, size);
  }
#endif

  return "";
}

bool writeTextFile(const std::string& path, const std::string& content)
{
  std::ofstream file;
  file.open(path.c_str());
  file << content;
  file.close();
  return true;
}

std::string getCurrentDirectory()
{
  char* a_cwd = getcwd(NULL, 0);
  std::string s_cwd(a_cwd);
  free(a_cwd);
  return s_cwd;
}

std::string getDataDirectory()
{
#ifdef _WIN32
  char* cDataDir = ::getenv("APPDATA");
#else
  char* cDataDir = ::getenv("KIWIX_DATA_DIR");
#endif
  std::string dataDir = cDataDir==nullptr ? "" : cDataDir;
  if (!dataDir.empty())
    return dataDir;
#ifdef _WIN32
  cDataDir = ::getenv("USERPROFILE");
  dataDir = cDataDir==nullptr ? getCurrentDirectory() : cDataDir;
#else
  cDataDir = ::getenv("XDG_DATA_HOME");
  dataDir = cDataDir==nullptr ? "" : cDataDir;
  if (dataDir.empty()) {
    cDataDir = ::getenv("HOME");
    dataDir = cDataDir==nullptr ? getCurrentDirectory() : cDataDir;
    dataDir = appendToDirectory(dataDir, ".local");
    dataDir = appendToDirectory(dataDir, "share");
  }
#endif
  return appendToDirectory(dataDir, "kiwix");
}

static std::map<std::string, std::string> extMimeTypes = {
 { "html", "text/html"},
 { "htm", "text/html"},
 { "png", "image/png"},
 { "tiff", "image/tiff"},
 { "tif", "image/tiff"},
 { "jpeg", "image/jpeg"},
 { "jpg", "image/jpeg"},
 { "gif", "image/gif"},
 { "svg", "image/svg+xml"},
 { "txt", "text/plain"},
 { "xml", "text/xml"},
 { "pdf", "application/pdf"},
 { "ogg", "application/ogg"},
 { "js", "application/javascript"},
 { "css", "text/css"},
 { "otf", "application/vnd.ms-opentype"},
 { "ttf", "application/font-ttf"},
 { "woff", "application/font-woff"},
 { "vtt", "text/vtt"}
};

/* Try to get the mimeType from the file extension */
std::string getMimeTypeForFile(const std::string& filename)
{
  std::string mimeType = "text/plain";
  auto pos = filename.find_last_of(".");

  if (pos != std::string::npos) {
    std::string extension = filename.substr(pos + 1);

    auto it = extMimeTypes.find(extension);
    if (it != extMimeTypes.end()) {
      mimeType = it->second;
    } else {
      it = extMimeTypes.find(kiwix::lcAll(extension));
      if (it != extMimeTypes.end()) {
        mimeType = it->second;
      }
    }
  }

  return mimeType;
}

