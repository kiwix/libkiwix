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

#include "reader.h"
#include <time.h>

#include <zim/search.h>

namespace kiwix
{

Entry::Entry(zim::Article article)
  : article(article)
{
}

#define RETURN_IF_INVALID(WHAT) if(!good()) { return (WHAT); }

std::string Entry::getPath() const
{
  RETURN_IF_INVALID("");
  return article.getLongUrl();
}

std::string Entry::getTitle() const
{
  RETURN_IF_INVALID("");
  return article.getTitle();
}

std::string Entry::getContent() const
{
  RETURN_IF_INVALID("");
  return article.getData();
}

zim::Blob Entry::getBlob(offset_type offset) const
{
  RETURN_IF_INVALID(zim::Blob());
  return article.getData(offset);
}

zim::Blob Entry::getBlob(offset_type offset, size_type size) const
{
  RETURN_IF_INVALID(zim::Blob());
  return article.getData(offset, size);
}

std::pair<std::string, offset_type> Entry::getDirectAccessInfo() const
{
  RETURN_IF_INVALID(std::make_pair("", 0));
  return article.getDirectAccessInformation();
}

size_type Entry::getSize() const
{
  RETURN_IF_INVALID(0);
  return article.getArticleSize();
}

std::string Entry::getMimetype() const
{
  RETURN_IF_INVALID("");
  try {
    return article.getMimeType();
  } catch (exception& e) {
    return "application/octet-stream";
  }
}

bool Entry::isRedirect() const
{
  RETURN_IF_INVALID(false);
  return article.isRedirect();
}

bool Entry::isLinkTarget() const
{
  RETURN_IF_INVALID(false);
  return article.isLinktarget();
}

bool Entry::isDeleted() const
{
  RETURN_IF_INVALID(false);
  return article.isDeleted();
}

Entry Entry::getRedirectEntry() const
{
  RETURN_IF_INVALID(Entry());
  if ( !article.isRedirect() ) {
    throw NoEntry();
  }

  auto targeted_article = article.getRedirectArticle();
  if ( !targeted_article.good()) {
    throw NoEntry();
  }
  return targeted_article;
}

Entry Entry::getFinalEntry() const
{
  RETURN_IF_INVALID(Entry());
  if (final_article.good()) {
    return final_article;
  }

  int loopCounter = 42;
  final_article = article;
  while (final_article.isRedirect() && loopCounter--) {
    final_article = final_article.getRedirectArticle();
    if ( !final_article.good()) {
      throw NoEntry();
    }
  }

  return final_article;
}

}
