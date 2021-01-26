/*
 * Copyright 2018 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef KIWIX_DOWNLOADER_H
#define KIWIX_DOWNLOADER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

namespace kiwix
{

class Aria2;
struct DownloadedFile {
  DownloadedFile()
   : success(false) {}
  bool success;
  std::string path;
};

class AriaError : public std::runtime_error {
 public:
  AriaError(const std::string& message) : std::runtime_error(message) {}
};


class Download {
 public:
  typedef enum { K_ACTIVE, K_WAITING, K_PAUSED, K_ERROR, K_COMPLETE, K_REMOVED, K_UNKNOWN } StatusResult;

  Download() :
    m_status(K_UNKNOWN) {}
  Download(std::shared_ptr<Aria2> p_aria, std::string did)
    : mp_aria(p_aria),
      m_status(K_UNKNOWN),
      m_did(did) {};
  void updateStatus(bool follow=false);
  void pauseDownload();
  void resumeDownload();
  void cancelDownload();
  StatusResult getStatus()          { return m_status; }
  std::string  getDid()             { return m_did; }
  std::string  getFollowedBy()      { return m_followedBy; }
  uint64_t     getTotalLength()     { return m_totalLength; }
  uint64_t     getCompletedLength() { return m_completedLength; }
  uint64_t     getDownloadSpeed()   { return m_downloadSpeed; }
  uint64_t     getVerifiedLength()  { return m_verifiedLength; }
  std::string  getPath()            { return m_path; }
  std::vector<std::string>&  getUris()             { return m_uris; }

 protected:
  std::shared_ptr<Aria2> mp_aria;
  StatusResult m_status;
  std::string m_did = "";
  std::string m_followedBy = "";
  uint64_t m_totalLength;
  uint64_t m_completedLength;
  uint64_t m_downloadSpeed;
  uint64_t m_verifiedLength;
  std::vector<std::string> m_uris;
  std::string m_path;
};

/**
 * A tool to download things.
 *
 */
class Downloader
{
 public:
  Downloader();
  virtual ~Downloader();

  void close();

  Download* startDownload(const std::string& uri, const std::vector<std::pair<std::string, std::string>>& options = {});
  Download* getDownload(const std::string& did);

  size_t getNbDownload() { return m_knownDownloads.size(); }
  std::vector<std::string> getDownloadIds();

 private:
  std::map<std::string, std::unique_ptr<Download>> m_knownDownloads;
  std::shared_ptr<Aria2> mp_aria;
};
}

#endif
