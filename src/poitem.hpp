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

#ifndef _POITEM_HPP_
#define _POITEM_HPP_

#include "poentry.hpp"

#include <string>
#include <map>

using std::string;
using std::map;

class PoItem : public PoEntry
{
	public:
		PoItem();
		const string& GetID() const;
		PoItem* AddStr(const string& str, const string& lang);
		PoItem* AddID(const string& str);
		PoItem* SetStr(const string& str, const string& lang);
		const map<string, string>& GetSTR() const;

		bool IsFuzzy(const string& lang) const;
		PoItem* SetFuzzy(const string& lang, bool fuzzy);
	private:
		string m_id;
		map<string, bool> m_fuzzy;
		map<string, string> m_str;
};

#endif
