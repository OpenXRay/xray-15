/*===========================================================================*\
 | 
 |  FILE:	ConfigMgr.cpp
 |			Skeleton project and code for a Scene Exporter 
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 26-3-99
 | 
\*===========================================================================*/
/*===========================================================================*\
 |	Config manager from the Skeleton Exporter - just saves the radio button status
\*===========================================================================*/


#include "CVDExport.h"


// change the name when using this skeleton
#define		CFG_FILENAME		"cvd_export.cfg"
#define		CFG_VERSION			0x001


/*===========================================================================*\
 |  Get the config file's full name on the disk
\*===========================================================================*/

TSTR CVDExporter::GetConfigFilename()
{
	TSTR filename;
	
	filename += ip->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += CFG_FILENAME;

	return filename;
}

/*===========================================================================*\
 |  Load and save the values
\*===========================================================================*/

void CVDExporter::SaveExporterConfig()
{
	// Open the configuration file for writing
	TSTR filename = GetConfigFilename();
	FILE* cfgStream;

	cfgStream = fopen(filename, "wb");
	if (!cfgStream)
		return;

	// Write CFG version
	_putw(CFG_VERSION,				cfgStream);

	_putw(searchtype,				cfgStream);

	fclose(cfgStream);
}


BOOL CVDExporter::LoadExporterConfig()
{
	// Open the configuration file for reading
	TSTR filename = GetConfigFilename();

	// If the file doesn't exist yet, write out the defaults
	if(!DoesFileExist(filename)) SaveExporterConfig();


	FILE* cfgStream;

	cfgStream = fopen(filename, "rb");
	if (!cfgStream)
		return FALSE;

	// First item is a file version
	int fileVersion = _getw(cfgStream);

	if (fileVersion > CFG_VERSION) {
		// Unknown version
		fclose(cfgStream);
		return FALSE;
	}

	searchtype = _getw(cfgStream);

	fclose(cfgStream);

	return TRUE;

}
