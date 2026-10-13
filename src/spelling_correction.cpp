/*
 * Copyright (C) 2025 Veloman Yunkan
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

#include "spelling_correction.h"
#include "zim/archive.h"

#include <sstream>
#include <stdexcept>

#include <xapian.h>

namespace kiwix
{

namespace
{

std::vector<std::string> getAllTitles(const zim::Archive& a)
{
  std::vector<std::string> result;
  for (const auto& entry : a.iterByPath() ) {
     result.push_back(entry.getTitle());
  }
  return result;
}

void createXapianDB(std::string path, const zim::Archive& archive)
{
  const int flags = Xapian::DB_BACKEND_GLASS|Xapian::DB_CREATE;
  Xapian::WritableDatabase db(path, flags);
  for (const auto& t : getAllTitles(archive)) {
    db.add_spelling(t);
  }
  db.commit();
}

std::string spellingsDBPathForZIMArchive(std::filesystem::path cacheDirPath, const zim::Archive& a)
{
  // The version of spellings DB must be updated each time an important change
  // to the implementation is made that renders using the previous version
  // impossible or undesirable.
  const char SPELLINGS_DB_VERSION[] = "0.2";

  std::ostringstream filename;
  filename << a.getUuid() << ".spellingsdb.v" << SPELLINGS_DB_VERSION;
  return (cacheDirPath / filename.str()).string();
}

std::unique_ptr<Xapian::WritableDatabase> openOrCreateXapianDB(std::filesystem::path cacheDirPath, const zim::Archive& archive)
{
  const auto path = spellingsDBPathForZIMArchive(cacheDirPath, archive);
  try
  {
    {
      Xapian::Database checkIfDbAlreadyExists(path);
    }
    return std::make_unique<Xapian::WritableDatabase>(path);
  }
  catch (const Xapian::DatabaseOpeningError& )
  {
    createXapianDB(path, archive);
    return std::make_unique<Xapian::WritableDatabase>(path);
  }
}

} // unnamed namespace

SpellingsDB::SpellingsDB(const zim::Archive& archive, std::filesystem::path cacheDirPath)
  : impl_(openOrCreateXapianDB(cacheDirPath, archive))
{
}

SpellingsDB::~SpellingsDB()
{
}

std::vector<std::string> SpellingsDB::getSpellingCorrections(const std::string& word, uint32_t maxCount) const
{
  std::vector<std::string> result;
  while ( result.size() < maxCount ) {
    const auto term = impl_->get_spelling_suggestion(word, 3);
    if ( term.empty() )
      break;

    result.push_back(term);

    // temporarily remove this term so that another spellings could be obtained
    impl_->remove_spelling(term);
  }

  // restore temporarily removed terms
  for (const auto& t : result) {
    impl_->add_spelling(t);
  }

  return result;
}

} // namespace kiwix
