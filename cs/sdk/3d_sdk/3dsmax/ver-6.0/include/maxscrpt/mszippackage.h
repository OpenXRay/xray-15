/*	
 *		MSZipPackage.h - MAXScript Zip Package file classes & utilities
 *
 *			Copyright © Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#ifndef _H_MSZIPPACKAGE
#define _H_MSZIPPACKAGE

#include "Parser.h"

class MSZipPackage;
class DotRunParser;
class MZPExtraction;
class TabMZPExtraction;

// MZPExtraction & TabMZPExtraction, for maintaing a table of
//   file extractions so far giving name in .zip and local extracted name
class MZPExtraction
{
public:
	TSTR zip_name;
	TSTR extracted_name;

	MZPExtraction(TCHAR* zip_name, TCHAR* extracted_name) :
		zip_name(zip_name), extracted_name(extracted_name) { }
};

class TabMZPExtraction : public Tab<MZPExtraction*>
{
public:
	~TabMZPExtraction();

	TCHAR* find(TCHAR* zip_name);
	void   add(TCHAR* zip_name, TCHAR* extracted_name);
};

// MSZipPackage: instances represent open .mzp package files
class MSZipPackage : public Value
{
public:
	enum clear_modes { CLEAR_NOW, CLEAR_ON_MAX_EXIT, CLEAR_ON_RESET, KEEP, };   // clear temp modes

	TSTR	file_name;			// zip package file name
	TSTR	package_name;		// parsed from mzp.run file...
	TSTR	description;
	int		version;
	TSTR	extract_dir;		// current extract dir
	TSTR	drop_file;			// primary drop file (if any in found)
	TabMZPExtraction extractions; // currently extracted files
	clear_modes clear_mode;
	WORD	flags;				// flag bits

	MSZipPackage(TCHAR* file_name) : file_name(file_name), flags(0), clear_mode(CLEAR_ON_MAX_EXIT) { tag = INTERNAL_CLASS_TAG; } 

	void	collect() { delete this; }

	// main zip package file in function
	ScripterExport static bool file_in_package(TCHAR* file_name, TSTR* extract_dir=NULL);
	static int WINAPI output_callback(char far *buf, unsigned long size);

	// package SearchFile
	ScripterExport static BOOL search_current_package(TCHAR* file_name, TCHAR* found_path);

	// extraction
	bool	extract_to(TCHAR* dir);
	TCHAR*	find_extraction(TCHAR* zip_name) { return extractions.find(zip_name); }
	void	add_extraction(TCHAR* zip_name, TCHAR* extracted_name);

	// directory & file manipulation
	ScripterExport static TSTR	expand_dir(TCHAR* dir);
	ScripterExport static TSTR	expand_file(TCHAR* file_name);
	TSTR	expand_dir_for_extraction(TCHAR* dir);
	TSTR	expand_file_for_extraction(TCHAR* file_name);

	// running
	void	run_all_scripts();
	bool	run_script(TCHAR* zip_name);
};

// MZP flags
#define MZP_DONT_RUN		0x0001		// just loading, don't run any scripts
#define MZP_HAVE_DROPFILE	0x0002		// 'drop' command encountered
#define MZP_HAVE_OPEN		0x0004		// 'open' command encountered

// interpret copy() function type bits...
#define MZP_COPY		0x00
#define MZP_MOVE		0x01
#define MZP_FILES		0x00
#define MZP_TREE		0x02
#define MZP_NOREPLACE	0x04

// parser specialization to parse & run mzp.run control file
class DotRunParser : public Parser
{
	MSZipPackage* mzp;

public:

	DotRunParser (MSZipPackage* mzp) : mzp(mzp) { }

	// parse & run mzp.run file
	bool	interpret(TCHAR* src);

	// parser production functions
	void	name();
	void	version();
	void	description();
	void	extract();
	void	run();
	void	drop();
	void	open();
	void	merge();
	void	xref();
	void	import();
	void	copy(BYTE type);
	void	clear();
	void	keep();

	// utils
	void	copy_files(TCHAR* from, TCHAR* to, BYTE type);
	ScripterExport static bool	create_dir(TCHAR* dir);
};

#endif
