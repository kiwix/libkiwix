/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
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

#include "pathTools.h"

bool isRelativePath(const string &path) {
#ifdef _WIN32
  return path.empty() || path.substr(1, 2) == ":\\" ? false : true;
#else
  return path.empty() || path.substr(0, 1) == "/" ? false : true;
#endif
}

string computeAbsolutePath(const string path, const string relativePath) {
#ifdef _WIN32
  string separator = "\\";
#else
  string separator = "/";
#endif
  string absolutePath = path[path.length() - 1] == '/' ? path : path + "/";
  char *cRelativePath = strdup(relativePath.c_str());
  char *token = strtok(cRelativePath, "/");
  
  while (token != NULL) {
    if (string(token) == "..") {
      absolutePath = removeLastPathElement(absolutePath, true, false);
      token = strtok(NULL, "/");
    } else if (strcmp(token, ".") && strcmp(token, "")) {
      absolutePath += string(token);
      token = strtok(NULL, "/");
      if (token != NULL)
	absolutePath += separator;
    } else {
      token = strtok(NULL, "/");
    }
  }
  
  return absolutePath;
}

string removeLastPathElement(const string path, const bool removePreSeparator, const bool removePostSeparator) {
#ifdef _WIN32
  string separator = "\\";
#else
  string separator = "/";
#endif
  
  string newPath = path;
  size_t offset = newPath.find_last_of(separator);
  if (removePreSeparator && offset != newPath.find_first_of(separator) && offset == newPath.length()-1) {
    newPath = newPath.substr(0, offset);
    offset = newPath.find_last_of(separator);
  }
  newPath = removePostSeparator ? newPath.substr(0, offset) : newPath.substr(0, offset+1);
  return newPath;
}

string getLastPathElement(const string &path) {
#ifdef _WIN32
  string separator = "\\";
#else
  string separator = "/";
#endif

  return path.substr(path.find_last_of(separator) + 1);
}
  
unsigned int getFileSize(const string &path) {
  struct stat filestatus;
  stat(path.c_str(), &filestatus);
  return filestatus.st_size / 1024;
}

string getFileSizeAsString(const string &path) {
  char csize[42];
  sprintf(csize, "%u", getFileSize(path));
  return csize;
}

bool fileExists(const string &path) {
  bool flag = false;
  fstream fin;
  fin.open(path.c_str(), ios::in);
  if (fin.is_open()) {
    flag = true;
  }
  fin.close();
  return flag;
}

bool makeDirectory(const string &path) {
  int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  return status == 0;
}

bool copyFile(const string &sourcePath, const string &destPath) {
  try {
    std::ifstream infile(sourcePath.c_str(), std::ios_base::binary);
    std::ofstream outfile(destPath.c_str(), std::ios_base::binary);
    outfile << infile.rdbuf();
  } catch (exception &e) {
    cerr << e.what() << endl;
    return false;
  }
  
  return true;
}
