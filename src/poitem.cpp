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

#include "poitem.hpp"

PoItem::PoItem() : PoEntry(MSG_ITEM)
{
}

PoItem* PoItem::SetStr(const string& str, const string& lang)
{
	m_str[lang] = str;
	return this;
}

PoItem* PoItem::AddID(const string& str)
{
	m_id += str;
	return this;
}

PoItem* PoItem::AddStr(const string& str, const string& lang)
{
	m_str[lang] += str;
	return this;
}

PoItem* PoItem::SetFuzzy(const string& lang, bool fuzzy)
{
	m_fuzzy[lang] = fuzzy;
	return this;
}

const string& PoItem::GetID() const
{
	return m_id;
}

const map<string, string>& PoItem::GetSTR() const
{
	return m_str;
}

bool PoItem::IsFuzzy(const string& lang) const
{
	map<string, bool>::const_iterator i = m_fuzzy.find(lang);
	if (i == m_fuzzy.end()) return false;
	return i->second;
}
