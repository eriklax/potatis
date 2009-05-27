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

#include "pofile.hpp"
#include <iostream>
#include <fstream>

using std::ifstream;
using std::ofstream;

PoFile::PoFile()
{

}

PoFile::PoFile(const string& file)
{
	Open(file);
}

void PoFile::Open(const string& file)
{
	m_file = file;
	/*
	 * Pattern for parsing PO and POD files
	 */
	static pcrecpp::RE msgid("msgid\\s*\"(.*)\"\\s*");
	static pcrecpp::RE msgstr("msgstr\\s*(.*?)\\s*(~*)\"(.*)\"\\s*");
	static pcrecpp::RE msgstr2("\\s*\"(.*)\"\\s*");
	static pcrecpp::RE comment("\\s*#(.*)\\s*");

	/*
	 * Open file for reading
	 */
	ifstream fp(file.c_str(), ifstream::in);
	if (!fp)
	{
		throw std::runtime_error("Could not open PO/POD file for reading");
	}

	char buf[66535];

	enum { NONE, COMMENT, ID, ID_MORE, STR, STR_MORE, END } state = NONE, last_state = NONE;

	vector<string> pc;
	string pi;
	string ps;
	string last_language;
	string language;
	string fuzzy;

	int stat = 0;

	PoItem* tmpItem = 0x0;
	while (fp)
	{
		string collect;
		fp.getline(buf, sizeof(buf));
		if (!fp) {
			state = END;
		} else {
			if (comment.FullMatch(buf, &collect))
			{
				pc.push_back(collect);
				state = COMMENT;
			}
			if (msgid.FullMatch(buf, &collect))
			{
				if (state == COMMENT && pc.size() > 0)
				{
					m_poList.push_back(new PoComment(pc));
					pc.clear();
				}
				m_poList.push_back((tmpItem=new PoItem())->AddID(collect));
				stat++;
				state = ID;
			}
			if (msgstr.FullMatch(buf, &language, &fuzzy, &collect))
			{
				if (state != ID && state != ID_MORE && state != STR && state != STR_MORE)
				{
					delete tmpItem;
					throw std::runtime_error("Parse Error: msgstr not followed by msgid");
				}
				tmpItem->SetFuzzy(language, fuzzy=="~");
				tmpItem->AddStr(collect, language);
				state = STR;
			}
			if (msgstr2.FullMatch(buf, &collect))
			{
				if (state != ID && state != ID_MORE && state != STR && state != STR_MORE)
				{
					delete tmpItem;
					throw std::runtime_error("Parse Error: data not followed by msgid or msgstr");
				}
				if (state == ID || state == ID_MORE) {
					tmpItem->AddID(collect);	
					state = ID_MORE;
				}
				if (state == STR || state == STR_MORE) {
					tmpItem->AddStr(collect, language);	
					state = STR_MORE;
				}
			}
		}
		if (last_state == NONE) last_state = state;
		if ((state != NONE && last_state != state))
		{
			if (last_state == COMMENT)
			{
				m_poList.push_back(new PoComment(pc));
				pc.clear();
			}
			last_state = state;
		}
		if (state == END)
			break;
	}
	fp.close();
}

bool PoFile::Write(const string& file, const string& language, bool POD, WriteType type)
{
	int stat_msgstr = 0;
	int stat_msgid = 0;
	int stat_comments = 0;
	int stat_fuzzy = 0;

	ofstream fp(file.c_str(), ofstream::out);
	if (!fp)
	{
		throw std::runtime_error("Could not open PO/POD file for writing");
	}
	if (language != "" && type != EXCLUDE_LANGUAGE)
	{
		fp << "# "+ language +"\n";
		stat_comments++;
	}

	string lang = language;
	for(vector<PoEntry*>::iterator i = m_poList.begin(); i != m_poList.end(); i++)
	{
		switch((*i)->GetType())
		{
			case PoEntry::MSG_COMMENT:
				if (language == "" || type == EXCLUDE_LANGUAGE)
				{
					for(vector<string>::const_iterator x = static_cast<PoComment*>(*i)->GetComment().begin();
							x != static_cast<PoComment*>(*i)->GetComment().end();
							x++)
					{
						fp << "#" + *x + "\n";
						stat_comments++;
					}
				}
				break;
			case PoEntry::MSG_ITEM:
				bool nextID = false;
				vector<pair<string,string> > item;
				for(map<string, string>::const_iterator y = static_cast<PoItem*>(*i)->GetSTR().begin();
						y != static_cast<PoItem*>(*i)->GetSTR().end();
						y++)
				{
					if (language == "") lang = y->first;
					switch(type)
					{
						case ALL:
							// If lang matches! (see language == "") above...
							if (lang == y->first)
							{
								item.push_back(pair<string,string>(y->first, y->second));
							}
							break;
						case ONLY_MISSING_FUZZY:
							// Save initial "" definition or fuzzy
							if (lang == y->first && (static_cast<PoItem*>(*i)->GetID() == "" || static_cast<PoItem*>(*i)->IsFuzzy(lang)))
							{
								item.push_back(pair<string,string>(y->first, y->second));	
							} else {
								if (lang == y->first) { nextID = true; break; }
							}
							break;
						case ONLY_MISSING:
							// Save initial "" definition...
							if (lang == y->first && static_cast<PoItem*>(*i)->GetID() == "")
							{
								item.push_back(pair<string,string>(y->first, y->second));	
							} else {
								if (lang == y->first) { nextID = true; break; }
							}
							break;
						case EXCLUDE_LANGUAGE:
							if (lang != y->first)
							{
								item.push_back(pair<string,string>(y->first, y->second));	
							}
							break;
					}
				}
				if (nextID) break;
				if (POD == false)
				{
					if (static_cast<PoItem*>(*i)->IsFuzzy(language))
					{
						fp << "#, fuzzy\n";
						stat_fuzzy++;
					}
				}
				fp << "msgid \""  + static_cast<PoItem*>(*i)->GetID() +  "\"\n";
				stat_msgid++;

				// Let's translate it
				if (item.size() == 0 && POD == false)
				{
					fp << "msgstr \"\"\n\n";
					stat_msgstr++;
				} else {
					for(vector<pair<string,string> >::iterator m = item.begin(); m != item.end(); m++)
					{
						// We have the entry..
						if (POD)
						{
							if (static_cast<PoItem*>(*i)->IsFuzzy(m->first))
							{
								fp << "msgstr "  + m->first +  " ~";
							} else {
								fp << "msgstr "  + m->first +  " ";
							}
						} else {
							fp << "msgstr ";
						}
						stat_msgstr++;
						string out = m->second;
						if (out != "") {
							size_t offset = 0;
							while(offset <= out.size())
							{   
								string part;
								int next = out.substr(offset).find("\\n");
								if (next == string::npos)
								{   
									part = out.substr(offset);
								} else {
									part = out.substr(offset,next+2);   
								}   
								if (part.size() == 0) break;
								if (offset != 0)
								{
									fp << "\t";
								}
								fp << "\"" + part + "\"\n";
								offset += part.size();
							}
						} else fp << "\"\"\n";
					}
					fp << "\n";
				}
				break;
		}
	}
	fp.close();
	fprintf(stderr, "%s: saved: summary %d msgid/%d msgstr/%d fuzzy/%d #\n", file.c_str(),
				stat_msgid, stat_msgstr, stat_fuzzy, stat_comments);
}

bool PoFile::Import(const PoFile& file, const string& language, bool createIndex)
{
	int stat_msgid_merge = 0;
	int stat_msgid_create = 0;
	int stat_msgid_skipped = 0;
	int stat_msgid_fuzzy = 0;

	bool fuzzy = false;
	for(vector<PoEntry*>::const_iterator i = file.List().begin(); i != file.List().end(); i++)
	{
		switch((*i)->GetType())
		{
			case PoEntry::MSG_COMMENT:
				for(vector<string>::const_iterator x = static_cast<PoComment*>(*i)->GetComment().begin();
						x != static_cast<PoComment*>(*i)->GetComment().end();
						x++)
				{
					static pcrecpp::RE refuzzy(",\\s*fuzzy");
					if (refuzzy.PartialMatch(*x))
					{
						fuzzy = true;
					}
				}
			break;
			case PoEntry::MSG_ITEM:
				bool foundItem = false;
				for(vector<PoEntry*>::iterator y = m_poList.begin(); y != m_poList.end(); y++)
				{
					switch((*y)->GetType())
					{
						case PoEntry::MSG_ITEM:
							if (static_cast<PoItem*>(*i)->GetID() == static_cast<PoItem*>(*y)->GetID())
							{
								map<string, string> str = static_cast<PoItem*>(*i)->GetSTR();
								static_cast<PoItem*>(*y)->SetStr(str[""], language);
								if (fuzzy)
								{
									static_cast<PoItem*>(*y)->SetFuzzy(language, fuzzy);
									stat_msgid_fuzzy++;
								}
								foundItem = true;
								stat_msgid_merge++;
							}
							break;
					}
					if (foundItem == true)
						break;
				}
				if (!foundItem && createIndex)
				{
					map<string, string> str = static_cast<PoItem*>(*i)->GetSTR();
					PoItem* tmp = new PoItem();
					tmp->AddID(static_cast<PoItem*>(*i)->GetID());
					if (fuzzy)
					{
						tmp->SetFuzzy(language, fuzzy);
						stat_msgid_fuzzy++;
					}
					if (str.find("") != str.end())
					{
						tmp->SetStr(str[""], language);
					}
					m_poList.push_back(tmp);
					stat_msgid_create++;
				} else {
					if (!foundItem)
					{
						fprintf(stderr, "%s: import/%s: skipped msgid %s (use --index)\n", m_file.c_str(), language.c_str(),
								 static_cast<PoItem*>(*i)->GetID().c_str());
						stat_msgid_skipped++;
					}
				}
				fuzzy = false;
				break;
		}
	}
	fprintf(stderr, "%s: import/%s: summary =%d/~%d/+%d/!%d msgid\n", m_file.c_str(), language.c_str(),
		stat_msgid_merge, stat_msgid_fuzzy, stat_msgid_create, stat_msgid_skipped);
}

void PoFile::Dump(DumpType type)
{
	int stat_comments = 0;
	int stat_msgid = 0;
	int stat_msgstr = 0;
	map<string,int> stat_languages;
	map<string,int> stat_fuzzy;

	for(vector<PoEntry*>::iterator i = m_poList.begin(); i != m_poList.end(); i++)
	{
		switch((*i)->GetType())
		{
			case PoEntry::MSG_COMMENT:
				for(vector<string>::const_iterator x = static_cast<PoComment*>(*i)->GetComment().begin();
						x != static_cast<PoComment*>(*i)->GetComment().end();
						x++)
				{
					//printf("#%s\n", x->c_str());
					stat_comments++;
				}
				break;
			case PoEntry::MSG_ITEM:
				vector<pair<string,string> > item;
				//printf("msgid \"%s\"\n", static_cast<PoItem*>(*i)->GetID().c_str());
				stat_msgid++;
				//printf("msgstr %d (", static_cast<PoItem*>(*i)->GetSTR().size());
				for(map<string, string>::const_iterator y = static_cast<PoItem*>(*i)->GetSTR().begin();
						y != static_cast<PoItem*>(*i)->GetSTR().end();
						y++)
				{
					stat_languages[y->first]++;
					//if (y != static_cast<PoItem*>(*i)->GetSTR().begin()) printf(",");
					//printf("%s", y->first.c_str());
					stat_msgstr++;
					if (static_cast<PoItem*>(*i)->IsFuzzy(y->first)) stat_fuzzy[y->first]++;
				}
				//printf(")\n");
				break;
		}
	}
	if (type == DUMP_ALL)
	{
		// Present Collected Statitics
		fprintf(stderr, "%s: dump: summary %d msgid/%d msgstr/%d #\n", m_file.c_str(), stat_msgid, stat_msgstr, stat_comments);
		fprintf(stderr, "%s: dump: %d language(s) found\n", m_file.c_str(), stat_languages.size());
		for(map<string, int>::const_iterator i = stat_languages.begin(); i != stat_languages.end(); i++)
		{
			fprintf(stderr, "%s: dump/%s: %d msgid", m_file.c_str(), i->first.c_str(), i->second);
			if (i->second == stat_msgid)
			{
				fprintf(stderr, " OK");	
			} else {
				fprintf(stderr, " %.1f %%", ((float)i->second/stat_msgid)*100.0);
			}
			if (stat_fuzzy[i->first] > 0)
			{
				fprintf(stderr, " (%d fuzzy)", stat_fuzzy[i->first]);
			}
			fprintf(stderr, "\n");
		}
	}
	if (type == DUMP_LANGUAGE)
	{
		for(map<string, int>::const_iterator i = stat_languages.begin(); i != stat_languages.end(); i++)
		{
			fprintf(stdout, "%s\n", i->first.c_str());
		}
	}
	return;
}

PoFile::~PoFile()
{
	for(vector<PoEntry*>::const_iterator i = m_poList.begin(); i != m_poList.end(); i++)
	{
		delete *i;
	}
}
