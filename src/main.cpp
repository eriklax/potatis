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

#include "main.hpp"
#include "global.hpp"

string program_name = "";

void usage(FILE* fp, int status)
{
	fprintf(fp, 
			APPNAME " " VERSION " build on " __DATE__ " (c) " AUTHOR " <" EMAIL ">\n"
			"Usage: %s [OPTION...]\n"
			"\n"
			" -h,  --help                Show this help\n"
			" -d,  --database <file>     POD database file (required)\n"
			"\n"
			" -i,  --import <file>       PO file to import\n"
			" -l,  --language <sv_SE>    Set the current language\n"
			" -x,  --index               Add new msgid(s) from PO file\n"
			" -o,  --output <file>       Save changes to POD to file\n"
			"\n"
			" -e,  --export <file>       PO file to export\n"
			" -l,  --language <sv_SE>    Set the current language\n"
			" -m,  --missing             Only export missing msgid/msgstr\n"
			" -f,  --fuzzy               Export fuzzy strings (requires --missing)\n"
			"\n"
			" -r,  --remove              Remove --language from POD\n"
			" -l,  --language <sv_SE>    Set the current language\n"
			" -o,  --output <file>       Save changes to POD to file\n"
			"\n"
			" -s,  --statistics          Show POD statistics\n"
			" -C,  --list-language       Show language(s) in POD\n"
			"\n"
/*
			"Examples:\n"
			"\n"
			" # Import sv_SE.po as a msgid master to POD database\n"
			" %s --database=messages.pod --language=sv_SE \\\n"
			"        --import=sv_SE.po --index\n"
			"\n"
			" # Import no_NO.po as a msgid slave to POD database\n"
			" %s --database=messages.pod --language=no_NO \\\n"
			"        --import=no_NO.po\n"
			"\n"
			" # Generate no_NO-missing.po with missing translations\n"
			" %s --database=messages.pod --language=no_NO \\\n"
			"        --export=no_NO.po --missing\n"
			"\n"
			" # Import no_NO-missing.po to POD database\n"
			" %s --database=messages.pod --language=no_NO \\\n"
			"        --import=no_NO.po\n"
			"\n"
			" # Remove da_DK.po from POD database\n"
			" %s --database=messages.pod --language=da_DK \\\n"
			"        --remove\n"
			"\n"
			" # Export translations as PO to use with eg. msgfmt\n"
			" %s --database=messages.pod --language=sv_SE \\\n"
			"        --export=sv_SE.po\n"
			"\n"
*/
			"Report bugs to " URL "\n\n",
/*		program_name.c_str(),
		program_name.c_str(),
		program_name.c_str(),
		program_name.c_str(),
		program_name.c_str(),
		program_name.c_str(),*/
		program_name.c_str()
			);
	exit(status);
}

int main(int argc, char* argv[])
{
	program_name = string(argv[0]);
	enum {
		CMD_NONE,
		CMD_STATISTICS,
		CMD_LIST_LANGUAGE,
		CMD_IMPORT,
		CMD_EXPORT,
		CMD_REMOVE
	} cmdAction = CMD_NONE;

	string database;
	string language;
	string file_import;
	string file_export;
	string file_dump;

	bool cmd_dump = false;
	bool cmd_export_missing = false;
	bool cmd_import_indexes = false;
	bool cmd_export_fuzzy = false;

	if (argc < 2)
		usage(stderr, 2);

	static struct option longopts[] = { 
		{ "help",		no_argument,		0x0,		'h'	},
		{ "database",	required_argument,	0x0,		'd'	},
		{ "statistics",	no_argument,		0x0,		's'	},
		{ "remove",		no_argument,		0x0,		'r'	},
		{ "language",	required_argument,	0x0,		'l'	},
		{ "import",		required_argument,	0x0,		'i'	},
		{ "export",		required_argument,	0x0,		'e'	},
		{ "missing",	no_argument,		0x0,		'm'	},
		{ "fuzzy",		no_argument,		0x0,		'f'	},
		{ "index",		no_argument,		0x0,		'x'	},
		{ "output",		required_argument,	0x0,		'o'	},
		{ "list-language",no_argument,		0x0,		'C' },
		{ 0x0,			0,					0x0,		0	}   
	}; 
	opterr = 0;
	optind = 0;
	int ch;
	while ((ch = getopt_long(argc, argv, "d:hrl:o:i:xme:rsCf", longopts, 0x0)) != -1) {
		switch(ch)
		{
			case 'h':
				usage(stdout, 0);
				break;
			case 'l':
				language = optarg;
				break;
			case 'd':
				database = optarg;
				break;
			case 'o':
				cmd_dump = true;
				file_dump = optarg;
				break;
			case 'i':
				if (cmdAction != CMD_NONE)
				{
					fprintf(stderr, "Can not do multiple operation at the time\n");
					return 1;
				}
				cmdAction = CMD_IMPORT;
				file_import = optarg;
				break;
			case 'x':
				cmd_import_indexes = true;
				break;
			case 'f':
				cmd_export_fuzzy = true;
				break;
			case 'm':
				cmd_export_missing = true;
				break;
			case 'e':
				if (cmdAction != CMD_NONE)
				{
					fprintf(stderr, "Can not do multiple operation at the time\n");
					return 1;
				}
				cmdAction = CMD_EXPORT;
				file_export = optarg;
				break;
			case 'C':
				if (cmdAction != CMD_NONE)
				{
					fprintf(stderr, "Can not do multiple operation at the time\n");
					return 1;
				}
				cmdAction = CMD_LIST_LANGUAGE;
				break;
			case 's':
				if (cmdAction != CMD_NONE)
				{
					fprintf(stderr, "Can not do multiple operation at the time\n");
					return 1;
				}
				cmdAction = CMD_STATISTICS;
				break;
			case 'r':
				if (cmdAction != CMD_NONE)
				{
					fprintf(stderr, "Can not do multiple operation at the time\n");
					return 1;
				}
				cmdAction = CMD_REMOVE;
				break;
			case 0: break;
			default:
					usage(stderr, 2);
					break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0 || cmdAction == CMD_NONE)
		usage(stderr, 2);

	if (!cmd_dump)
		file_dump = database;

	if (database.empty())
	{
		fprintf(stderr,"--database missing\n");
		return 1;
	}

	PoFile datastore;

	try {
		datastore.Open(database);
	} catch(std::runtime_error& e) {
		if (cmdAction != CMD_IMPORT)
		{
			fprintf(stderr, "POD database is missing/corrupt: %s\n", e.what());
			return 1;
		}
	}

	switch(cmdAction)
	{
		case CMD_REMOVE:
			{
				if (language.empty())
				{
					fprintf(stderr,"--language missing\n");
					return 1;
				}
				try {
					datastore.Write(file_dump, language, true, PoFile::EXCLUDE_LANGUAGE);
				} catch(std::runtime_error& e) {
					fprintf(stderr, "POD Error: %s\n", e.what());
					return 1;
				}
			}
			break;
		case CMD_IMPORT:
			{
				if (language.empty())
				{
					fprintf(stderr,"--language missing\n");
					return 1;
				}

				if (file_import.empty())
				{
					fprintf(stderr,"no file to import\n");
					return 1;
				}

				try {
					datastore.Import(PoFile(file_import), language, cmd_import_indexes);
					datastore.Write(file_dump, "", true, PoFile::ALL);
				} catch(std::runtime_error& e) {
					fprintf(stderr, "POD Error: %s\n", e.what());
					return 1;
				}
			}
			break;
		case CMD_EXPORT:
			{
				if (language.empty())
				{
					fprintf(stderr,"--language missing\n");
					return 1;
				}

				if (file_export.empty())
				{
					fprintf(stderr,"no file to import\n");
					return 1;
				}

				try {
					datastore.Write(file_export, language, false, cmd_export_missing?
																	(
																	cmd_export_fuzzy?
																		PoFile::ONLY_MISSING_FUZZY:PoFile::ONLY_MISSING
																	):
																	PoFile::ALL);
				} catch(std::runtime_error& e) {
					fprintf(stderr, "POD Error: %s\n", e.what());
					return 1;
				}
			}
			break;
		case CMD_LIST_LANGUAGE:
				datastore.Dump(PoFile::DUMP_LANGUAGE);
			break;
		case CMD_STATISTICS:
				datastore.Dump();
			break;
		default:
			fprintf(stderr,"(not implemented)\n");
			return 1;
	}

	return 0;
}
