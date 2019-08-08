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

#include <tools/pathTools.h>

#ifdef __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>
#elif _WIN32
#include <direct.h>
#include <windows.h>
#include "shlwapi.h"
#define getcwd _getcwd  // stupid MSFT "deprecation" warning
#endif

#include <map>

#ifdef _WIN32
const std::string SEPARATOR("\\");
#else
const std::string SEPARATOR("/");
#include <unistd.h>
#endif

#include <stdlib.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

bool isRelativePath(const string& path)
{
#ifdef _WIN32
  return path.empty() || path.substr(1, 2) == ":\\" ? false : true;
#else
  return path.empty() || path.substr(0, 1) == "/" ? false : true;
#endif
}

string computeRelativePath(const string path, const string absolutePath)
{
  std::vector<std::string> pathParts = kiwix::split(path, SEPARATOR);
  std::vector<std::string> absolutePathParts
      = kiwix::split(absolutePath, SEPARATOR);

  unsigned int commonCount = 0;
  while (commonCount < pathParts.size()
         && commonCount < absolutePathParts.size()
         && pathParts[commonCount] == absolutePathParts[commonCount]) {
      commonCount++;
  }

  string relativePath;
#ifdef _WIN32
  /* On Windows you have a token more because the root is represented
     by a letter */
  if (commonCount == 0) {
    relativePath = ".." + SEPARATOR;
  }
#endif

  for (unsigned int i = commonCount; i < pathParts.size(); i++) {
    relativePath += ".." + SEPARATOR;
  }
  for (unsigned int i = commonCount; i < absolutePathParts.size(); i++) {
    relativePath += absolutePathParts[i];
    relativePath += i + 1 < absolutePathParts.size() ? SEPARATOR : "";
  }
  return relativePath;
}

/* Warning: the relative path must be with slashes */
string computeAbsolutePath(const string path, const string relativePath)
{
  string absolutePath;

  if (path.empty()) {
    char* path = NULL;
    size_t size = 0;

#ifdef _WIN32
    path = _getcwd(path, size);
#else
    path = getcwd(path, size);
#endif

    absolutePath = string(path) + SEPARATOR;
  } else {
    absolutePath = path.substr(path.length() - 1, 1) == SEPARATOR
                       ? path
                       : path + SEPARATOR;
  }

#if _WIN32
  char* cRelativePath = _strdup(relativePath.c_str());
#else
  char* cRelativePath = strdup(relativePath.c_str());
#endif
  char* token = strtok(cRelativePath, "/");

  while (token != NULL) {
    if (string(token) == "..") {
      absolutePath = removeLastPathElement(absolutePath, true, false);
      token = strtok(NULL, "/");
    } else if (strcmp(token, ".") && strcmp(token, "")) {
      absolutePath += string(token);
      token = strtok(NULL, "/");
      if (token != NULL) {
        absolutePath += SEPARATOR;
      }
    } else {
      token = strtok(NULL, "/");
    }
  }

  return absolutePath;
}

string removeLastPathElement(const string path,
                             const bool removePreSeparator,
                             const bool removePostSeparator)
{
  string newPath = path;
  size_t offset = newPath.find_last_of(SEPARATOR);
  if (removePreSeparator &&
#ifndef _WIN32
      offset != newPath.find_first_of(SEPARATOR) &&
#endif
      offset == newPath.length() - 1) {
    newPath = newPath.substr(0, offset);
    offset = newPath.find_last_of(SEPARATOR);
  }
  newPath = removePostSeparator ? newPath.substr(0, offset)
                                : newPath.substr(0, offset + 1);
  return newPath;
}

string appendToDirectory(const string& directoryPath, const string& filename)
{
  string newPath = directoryPath + SEPARATOR + filename;
  return newPath;
}

string getLastPathElement(const string& path)
{
  return path.substr(path.find_last_of(SEPARATOR) + 1);
}

unsigned int getFileSize(const string& path)
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

string getFileSizeAsString(const string& path)
{
  ostringstream convert;
  convert << getFileSize(path);
  return convert.str();
}

string getFileContent(const string& path)
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

bool fileExists(const string& path)
{
#ifdef _WIN32
  return PathFileExists(path.c_str());
#else
  bool flag = false;
  fstream fin;
  fin.open(path.c_str(), ios::in);
  if (fin.is_open()) {
    flag = true;
  }
  fin.close();
  return flag;
#endif
}

bool makeDirectory(const string& path)
{
#ifdef _WIN32
  int status = _mkdir(path.c_str());
#else
  int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
  return status == 0;
}

string makeTmpDirectory()
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
  return string(ctmp);
#else
  char _template_array[] = {"/tmp/kiwix-lib_XXXXXX"};
  string dir = mkdtemp(_template_array);
  return dir;
#endif
}

/* Try to create a link and if does not work then make a copy */
bool copyFile(const string& sourcePath, const string& destPath)
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
  } catch (exception& e) {
    cerr << e.what() << endl;
    return false;
  }

  return true;
}

string getExecutablePath()
{
  char binRootPath[PATH_MAX];

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

bool writeTextFile(const string& path, const string& content)
{
  std::ofstream file;
  file.open(path.c_str());
  file << content;
  file.close();
  return true;
}

string getCurrentDirectory()
{
  char* a_cwd = getcwd(NULL, 0);
  string s_cwd(a_cwd);
  free(a_cwd);
  return s_cwd;
}

string getDataDirectory()
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

