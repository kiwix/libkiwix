/*
 * Copyright 2018-2020 Matthieu Gautier <mgautier@kymeria.fr>
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

namespace kiwix
{

Entry::Entry(zim::Entry entry)
  : entry(entry)
{
}

size_type Entry::getSize() const
{
  if (entry.isRedirect()) {
    return 0;
  } else {
    return entry.getItem().getSize();
  }
}

std::string Entry::getMimetype() const
{
  return entry.getItem(true).getMimetype();
}

bool Entry::isRedirect() const
{
  return entry.isRedirect();
}

Entry Entry::getRedirectEntry() const
{
  if ( !entry.isRedirect() ) {
    throw NoEntry();
  }

  return entry.getRedirectEntry();
}

Entry Entry::getFinalEntry() const
{
  int loopCounter = 42;
  auto final_entry = entry;
  while (final_entry.isRedirect() && loopCounter--) {
    final_entry = final_entry.getRedirectEntry();
  }
  // Prevent infinite loops.
  if (final_entry.isRedirect()) {
    throw NoEntry();
  }
  return final_entry;
}

}
