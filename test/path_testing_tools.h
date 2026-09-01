/*
* Copyright 2026 Hamazasp Avetisyan <hamik.avetisyan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef LIBKIWIX_PATH_TESTING_TOOLS_H
#define LIBKIWIX_PATH_TESTING_TOOLS_H

#ifdef _WIN32
# define S "\\"
# define AS "c:"
# define A_SAMBA "\\\\sambadir"
#else
# define S "/"
# define AS ""
#endif

#define P2(a, b) a S b
#define P3(a, b, c) P2(P2(a, b), c)
#define P4(a, b, c, d) P2(P3(a, b, c), d)
#define P5(a, b, c, d, e) P2(P4(a, b, c, d), e)
#define P6(a, b, c, d, e, f) P2(P5(a, b, c ,d, e), f)

#define A1(a) P2(AS,a)
#define A2(a, b) A1(P2(a, b))
#define A3(a, b, c) A1(P3(a, b, c))
#define A4(a, b, c, d) A1(P4(a, b, c, d))
#define A5(a, b, c, d, e) A1(P5(a, b, c, d, e))

#endif  // LIBKIWIX_PATH_TESTING_TOOLS_H
