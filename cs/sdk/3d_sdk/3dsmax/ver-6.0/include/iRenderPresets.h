/////////////////////////////////////////////////////////////////////////
//
//
//	Render Preset Interface
//
//	Created 5/14/2003	Tom Burke
//

#ifndef	I_RENDER_PRESETS_H
#define I_RENDER_PRESETS_H

#include "bitarray.h"

#define RENDER_PRESETS_CATEGORY_COUNT				64
#define RENDER_PRESETS_CUSTOM_CATEGORY_INDEX_BEGIN	32

// return values
//
#define RENDER_PRESETS_SUCCESS				0
#define RP_ERROR_OBSOLETE_FILE				1
#define RP_ERROR_INCOMPATABLE_FILE			2
#define RP_ERROR_CATEGORY_NOT_COMPATABLE	3
#define RP_ERROR_SAVING_FILE				4
#define RP_ERROR_LOADING_FILE				5
#define RP_ERROR_CANCEL						6
#define RP_ERROR_FILE_NOT_FOUND				7


class RenderPresetsContext {
public:
	RenderPresetsContext( int version, BitArray categories ) { mVersion = version; mCategories = categories; }
	BitArray GetCategories() { return mCategories; }
	int       GetVersion() { return mVersion; }

private:
	int      mVersion;
	BitArray mCategories;
};

class IRenderPresetsManager {
public:
	enum RendererSpecifier {
		kProduction = 0,
		kMaterial,
		kActiveShade
	};

	typedef enum {
		kUnspecified = -1,
		kLoadAll = 0,
		kLoadNonNodeRefMakers = 1,
		kLoadNone = 2,
	} NodeRefAction;

    // Save selected categories of render presets to the file
	virtual int      Save( RendererSpecifier rendSpecifier ) = 0;
	virtual int      Save( RendererSpecifier rendSpecifier, TCHAR * fileName ) = 0;
	virtual int      Save( RendererSpecifier rendSpecifier, TCHAR * fileName, BitArray saveCategories ) = 0;

    // Load selected categories of render presets from the file
	virtual int      Load( RendererSpecifier rendSpecifier ) = 0;
	virtual int      Load( RendererSpecifier rendSpecifier, TCHAR * fileName ) = 0;
	virtual int      Load( RendererSpecifier rendSpecifier, TCHAR * fileName, BitArray loadCategories, NodeRefAction nodeRefAction = kUnspecified ) = 0;

	// Check to see if file is compatable with current renderer
	virtual int      IsFileCompatible( RendererSpecifier rendSpecifier, TCHAR * fileName ) = 0;  

	// Convert between category index and category name for render preset files
	virtual TCHAR *  MapIndexToCategory( TCHAR * fileName, int catIndex  ) = 0;
	virtual int      MapCategoryToIndex( TCHAR * fileName, TCHAR* category ) = 0;

	// Convert between category index and category name for current renderers
	virtual TCHAR *  MapIndexToCategory( RendererSpecifier rendSpecifier, int catIndex  ) = 0;
	virtual int      MapCategoryToIndex( RendererSpecifier rendSpecifier, TCHAR* category ) = 0;

    // Retrieve the categories that were saved to the given file
	virtual BitArray LoadCategories( TCHAR * fileName ) = 0; // list of categories saved in the file
	virtual Tab<TCHAR *> LoadCategoryNames( TCHAR * fileName ) = 0;

	virtual BitArray SaveCategories( RendererSpecifier rendSpecifier ) = 0;
	virtual Tab<TCHAR *> SaveCategoryNames( RendererSpecifier rendSpecifier ) = 0;

	// Retrieve the current RenderPresetContext. 
	// This is provided so that renderers can tailor their 
	// save and load methods to accomodate the saving and loading of 
	// any combination of render preset categories
	virtual RenderPresetsContext * GetContext() = 0;
};

#endif // I_RENDER_PRESETS_H