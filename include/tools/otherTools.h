/*
 * Copyright 2014 Emmanuel Engelhart <kelson@kiwix.org>
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

#ifndef KIWIX_OTHERTOOLS_H
#define KIWIX_OTHERTOOLS_H

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <pugixml.hpp>

namespace kiwix
{
void sleep(unsigned int milliseconds);
std::string nodeToString(pugi::xml_node node);
std::string converta2toa3(const std::string& a2code);
}

#endif
