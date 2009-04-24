/*
   Copyright (C) 2008 Erik Lax 

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   */

#ifndef _POFILE_HPP_
#define _POFILE_HPP_

#include "poitem.hpp"
#include "pocomment.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <pcrecpp.h>

using std::string;
using std::vector;
using std::pair;

class PoFile
{
	public:
		typedef enum { ALL, ONLY_MISSING, ONLY_MISSING_FUZZY, EXCLUDE_LANGUAGE } WriteType;
		typedef enum { DUMP_ALL, DUMP_LANGUAGE } DumpType;

		PoFile();
		PoFile(const string& file);
		void Open(const string& file);
		~PoFile();

		bool Import(const PoFile& file, const string& language, bool createIndex);
		bool Write(const string& file, const string& language = "", bool POD = true, WriteType type = ALL);

		const vector<PoEntry*>& List() const { return m_poList; }
		void Dump(DumpType type = DUMP_ALL);
	private:
		vector<PoEntry*> m_poList;
		string m_file;
};

#endif
