potatis
=======
Potatis is a program that manages multiple translations in one single (messages.pod) "PO Database" file instead of multiple message.po file. This will help you manage multiple languages at the same time, since they are all together in one single file.

Purpose
=======
When you work with gettext() you often use English as the key language. If minor spelling changes/corrections are done in the key language the index of all .po files has to be changed even if the change was minor. (some may implement a Broken English to English translation file instead). This is no longer necessary. Another problem with multiple .po files is to keep them up to date with key indexes. Potatis will help you extract all the keys that needs to be translated for each language and import them back into your POD database. These problems may seem minor when working with one or two .po files, but in a larger scale these problems takes a lot of time and potentially targets for errors.

Example
=======
In this example we have two po files with missing translation for "World" in german. sv_SE.po

msgid "Hello"
msgstr "Hej"
msgid "World"
msgstr "Värld"

de_DE.po

#, fuzzy
msgid "Hello"
msgstr "Hallo"

The same information would be stored like this with potatis. messages.pod

msgid "Hello"
msgstr sv_SE "Hej"
msgstr de_DE ~"Hallo"
msgid "World"
msgstr sv_SE "Värld"

The POD file may be edited using a text editor. Using the --export and --missing command you may extract the missing keys to a file PO to be translated. de_DE-fixme.po

msgid "World"
msgstr ""

once translated it can be imported (--imported) into your POD file again. Using the command --extract without the --missing flag you may extract your languages to .po file which can be used with (msgfmt) to generate .mo files!

Use potatis with CMake
======================

# This is a sample CMake implementation of Potatis for "make install"..

# CHECK THE PATHS!

INSTALL(CODE "EXECUTE_PROCESS(COMMAND
  sh -c \"
	for i in `potatis --database ${CMAKE_CURRENT_SOURCE_DIR}/messages.pod --list-language`;
	do
		echo -- Generating ${CMAKE_INSTALL_ROOT}/$i/LC_MESSAGES/messages.mo
		mkdir -p ${CMAKE_INSTALL_ROOT}/$i/LC_MESSAGES
		potatis --database ${CMAKE_CURRENT_SOURCE_DIR}/messages.pod --export $i-tmp.po --language $i
		msgfmt -o ${CMAKE_INSTALL_ROOT}/$i/LC_MESSAGES/messages.mo $i-tmp.po
		rm $i-tmp.po
	done
	\"
)")
