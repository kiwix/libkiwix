/*
 * Copyright 2012 Emmanuel Engelhart <kelson@kiwix.org>
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

#ifndef KIWIX_NETWORKTOOLS_H
#define KIWIX_NETWORKTOOLS_H

#include <string>

namespace kiwix
{
// User-Agent in the format agreed for Kiwix software (kiwix/operations#797):
// "kiwix/<libkiwix version> (<component>)". libkiwix's own requests use the
// "libkiwix" component and kiwix-serve's in-browser code uses "serve".
std::string getUserAgent(const std::string& component = "libkiwix");

std::string download(const std::string& url);
}

#endif
