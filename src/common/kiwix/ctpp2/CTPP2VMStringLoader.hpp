/*
 * Copyright 2013 Renaud Gaudin <reg@kiwix.org>
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

#ifndef _CTPP2_VM_STRING_LOADER_HPP__
#define _CTPP2_VM_STRING_LOADER_HPP__ 1

#include "ctpp2/CTPP2VMLoader.hpp"
#include "ctpp2/CTPP2Util.hpp"
#include "ctpp2/CTPP2Exception.hpp"
#include "ctpp2/CTPP2VMExecutable.hpp"
#include "ctpp2/CTPP2VMInstruction.hpp"
#include "ctpp2/CTPP2VMMemoryCore.hpp"

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <resourceTools.h>
#include <iostream>
#include <string>

/**
  @file VMStringLoader.hpp
  @brief Load program core from file
*/

namespace CTPP // C++ Template Engine
{
// FWD
struct VMExecutable;

/**
  @class VMStringLoader CTPP2VMStringLoader.hpp <CTPP2VMStringLoader.hpp>
  @brief Load program core from file
*/
class CTPP2DECL VMStringLoader:
  public VMLoader
{
public:
	/**
	*/
	VMStringLoader(CCHAR_P rawContent, size_t rawContentSize);
	/**
	  @brief Get ready-to-run program
	*/
	const VMMemoryCore * GetCore() const;

	/**
	  @brief A destructor
	*/
	~VMStringLoader() throw();
private:
	/** Program core             */
	VMExecutable  * oCore;
	/** Ready-to-run program     */
	VMMemoryCore  * pVMMemoryCore;
};

} // namespace CTPP
#endif // _CTPP2_VM_STRING_LOADER_HPP__
// End.
