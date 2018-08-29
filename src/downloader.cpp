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

#include "downloader.h"
#include "common/pathTools.h"

#include <thread>
#include <chrono>

#include <iostream>

#include "aria2.h"
#include "xmlrpc.h"
#include "common/otherTools.h"
#include <pugixml.hpp>

namespace kiwix
{

/* Constructor */
Downloader::Downloader() :
  mp_aria(new Aria2())
{
}


/* Destructor */
Downloader::~Downloader()
{
}

pugi::xml_node find_member_in_struct(pugi::xml_node struct_node, std::string member_name) {
  for(auto member=struct_node.first_child(); member; member=member.next_sibling()) {
    std::string _member_name = member.child("name").text().get();
    if (_member_name == member_name) {
      return member.child("value");
    }
  }
  return pugi::xml_node();
}

DownloadedFile Downloader::download(const std::string& url) {
  DownloadedFile fileHandle;
  try {
    std::vector<std::string> uris = {url};
    std::vector<std::string> status_key = {"status", "files"};
    std::string gid;
    
    gid = mp_aria->addUri(uris);
    std::cerr << "gid is : " << gid << std::endl;
    pugi::xml_document ret;
    while(true) {
     auto strStatus = mp_aria->tellStatus(gid, status_key);
     MethodResponse response(strStatus);
     auto structNode = response.getParams().getParam(0).getValue().getStruct();
     auto status = structNode.getMember("status").getValue().getAsS();
     std::cerr << "Status is " << status << std::endl;
     if (status  == "complete") {
       fileHandle.success = true;
       auto filesMember = structNode.getMember("files");
       auto fileStruct = filesMember.getValue().getArray().getValue(0).getStruct();
       fileHandle.path = fileStruct.getMember("path").getValue().getAsS();
       std::cerr << "FilePath is " << fileHandle.path << std::endl;
     } else if (status == "error") {
       fileHandle.success = false;
     } else {
       // [TODO] Be wise here.
       std::this_thread::sleep_for(std::chrono::microseconds(100000));
       continue;
     }
     break;
    }
  } catch (...) { std::cerr << "waautena " << std::endl; };
  return fileHandle;
}

}
