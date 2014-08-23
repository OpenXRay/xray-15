/**********************************************************************
 *<
	FILE: marketDefaultTest.cpp

	DESCRIPTION:	Tester code

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "marketDefaultTest.h"
#include "marketDefaults.h"
#include <io.h>


//--- MarketDefaultTest::Tester -------------------------------------------------------

static KeyDesc keys11[] = {
	{ _T("Key111"),  kInt,        false },
	{ _T("Key114"),  kInt,        true },
	{ _T("Key115"),  kFloat,      false },
	{ _T("Key118"),  kFloat,      true },
	{ _T("Key119"),  kIntArray,   false },
	{ _T("Key1112"), kIntArray,   true },
	{ _T("Key1113"), kFloatArray, false },
	{ _T("Key1116"), kFloatArray, true },
	{ _T("Key1117"), kString,     false },
	{ _T("Key1118"), kString,     true }
};

static KeyDesc keys12[] = {
	{ _T("Key111"),  kInt,        false },
	{ _T("Key114"),  kInt,        true },
	{ _T("Key115"),  kFloat,      false },
	{ _T("Key118"),  kFloat,      true },
	{ _T("Key119"),  kIntArray,   false },
	{ _T("Key1112"), kIntArray,   true },
	{ _T("Key1113"), kFloatArray, false },
	{ _T("Key1116"), kFloatArray, true },
	{ _T("Key1117"), kString,     false },
	{ _T("Key1118"), kString,     true }
};

static KeyDesc keys13[] = {
	{ _T("Key111"),  kInt,        false },
	{ _T("Key114"),  kInt,        true },
	{ _T("Key115"),  kFloat,      false },
	{ _T("Key118"),  kFloat,      true },
	{ _T("Key119"),  kIntArray,   false },
	{ _T("Key1112"), kIntArray,   true },
	{ _T("Key1113"), kFloatArray, false },
	{ _T("Key1116"), kFloatArray, true },
	{ _T("Key1117"), kString,     false },
	{ _T("Key1118"), kString,     true }
};

static KeyDesc keys15[] = {
	{ _T("Key111"),  kInt,        false },
	{ _T("Key114"),  kInt,        true },
	{ _T("Key115"),  kFloat,      false },
	{ _T("Key118"),  kFloat,      true },
	{ _T("Key119"),  kIntArray,   false },
	{ _T("Key1112"), kIntArray,   true },
	{ _T("Key1113"), kFloatArray, false },
	{ _T("Key1116"), kFloatArray, true },
	{ _T("Key1117"), kString,     false },
	{ _T("Key1118"), kString,     true }
};

static KeyDesc keys21[] = {
	{ _T("Key111"),  kInt,        false },
	{ _T("Key114"),  kInt,        true },
	{ _T("Key115"),  kFloat,      false },
	{ _T("Key118"),  kFloat,      true },
	{ _T("Key119"),  kIntArray,   false },
	{ _T("Key1112"), kIntArray,   true },
	{ _T("Key1113"), kFloatArray, false },
	{ _T("Key1116"), kFloatArray, true },
	{ _T("Key1117"), kString,     false },
	{ _T("Key1118"), kString,     true }
};

static KeyDesc keys22[] = {
	{ _T("Key111"),  kInt,        false },
	{ _T("Key114"),  kInt,        true },
	{ _T("Key115"),  kFloat,      false },
	{ _T("Key118"),  kFloat,      true },
	{ _T("Key119"),  kIntArray,   false },
	{ _T("Key1112"), kIntArray,   true },
	{ _T("Key1113"), kFloatArray, false },
	{ _T("Key1116"), kFloatArray, true },
	{ _T("Key1117"), kString,     false },
	{ _T("Key1118"), kString,     true }
};

static KeyDesc keys23[] = {
	{ _T("Key111"),  kInt,        false },
	{ _T("Key114"),  kInt,        true },
	{ _T("Key115"),  kFloat,      false },
	{ _T("Key118"),  kFloat,      true },
	{ _T("Key119"),  kIntArray,   false },
	{ _T("Key1112"), kIntArray,   true },
	{ _T("Key1113"), kFloatArray, false },
	{ _T("Key1116"), kFloatArray, true },
	{ _T("Key1117"), kString,     false },
	{ _T("Key1118"), kString,     true }
};

static KeyDesc keys25[] = {
	{ _T("Key111"),  kInt,        false },
	{ _T("Key114"),  kInt,        true },
	{ _T("Key115"),  kFloat,      false },
	{ _T("Key118"),  kFloat,      true },
	{ _T("Key119"),  kIntArray,   false },
	{ _T("Key1112"), kIntArray,   true },
	{ _T("Key1113"), kFloatArray, false },
	{ _T("Key1116"), kFloatArray, true },
	{ _T("Key1117"), kString,     false },
	{ _T("Key1118"), kString,     true }
};

static unsigned long classid12[3] = {0x453,  0x97654f4d, 0xdde456ed };
static unsigned long classid15[3] = {0xaaaa, 0x97654fd,  0xd4ef56ed };
static unsigned long classid16[3] = {0x4eee, 0x97654fd0, 0xd45bc6ed };
static unsigned long classid22[3] = {0x0,    0x976354fd, 0xd45634ed };
static unsigned long classid25[3] = {0x888,  0x976546fd, 0xd456e76d };
static unsigned long classid26[3] = {0x987,  0x976542fd, 0xd456edee };

static SectionDesc sections1[] = {
	{ _T("Section1"), keys11, sizeof(keys11) / sizeof(keys11[0]), false, false },
	{ classid12,      keys12, sizeof(keys12) / sizeof(keys12[0]), true, false },
	{ _T("Section3"), keys13, sizeof(keys13) / sizeof(keys13[0]), false, false },
	{ _T("Section4"), NULL,   0,                                  false, true },
	{ classid15,      keys15, sizeof(keys15) / sizeof(keys15[0]), true, false },
	{ classid16,      NULL,   0,                                  true, true },
};

static SectionDesc sections2[] = {
	{ _T("Test1"),    keys21, sizeof(keys21) / sizeof(keys21[0]), false, false },
	{ classid22,      keys22, sizeof(keys22) / sizeof(keys22[0]), true, false },
	{ _T("Test3"),    keys23, sizeof(keys23) / sizeof(keys23[0]), false, false },
	{ _T("Test4"),    NULL,   0,                                  false, true },
	{ classid25,      keys25, sizeof(keys25) / sizeof(keys25[0]), true, false },
	{ classid26,      NULL,   0,                                  true, true },
};

static FileDesc file1 = { sections1, sizeof(sections1) / sizeof(sections1[0]) };
static FileDesc file2 = { sections2, sizeof(sections2) / sizeof(sections2[0]) };
static bool file2Initialized = false;
static const int intValue = 4;
static const int floatTemplate = 44;
static const float floatValue = float(floatTemplate);
static float array3[3] = {22.0, 34.0, 67.0};
static float array4[4] = {22.0, 34.0, 67.0, 23.0};
static const int clarray[2] = {DMTL_CLASS_ID, 0};
static const int nullclarray[2] = {0, 0};
static const int standinclarray[2] = {STANDIN_CLASS_ID, 0};
static const int junkclarray[2] = {4, 5};
static const TCHAR* theString = _T("the string");

static const SClass_ID highLevelSClass = 0x33;
static const Class_ID highLevelClass(0x45, 0x56);

class CfgDefaultTestClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return FALSE; }
	void *			Create(BOOL loading = FALSE) { return NULL; }
	const TCHAR *	ClassName() { return _T("CfgDefaultTestClassDesc"); }
	SClass_ID		SuperClassID() { return highLevelSClass; }
	Class_ID		ClassID() { return highLevelClass; }
	const TCHAR* 	Category() { return _T("no category"); }

	const TCHAR*	InternalName() { return _T("CfgDefaultTestClassDesc"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

enum {
	kIntParam = 1,
	kFloatParam = 2,
	kPoint3Param = 3,
	kPoint4Param = 4,
	kStringParam = 5
};

class CfgDefaultTestParamBlock : public ParamBlockDesc2 {
public:
	CfgDefaultTestParamBlock(ClassDesc2& cd)
		: ParamBlockDesc2(1, _T("CfgDefaultTestParamBlock"), 0, &cd, 0, 
		
		kIntParam, _T("int"), TYPE_INT, 0, 0,
			p_configurable_default, 0, _T("testHighLevel"), NULL,
			end,

		kFloatParam, _T("float"), TYPE_FLOAT, 0, 0,
			p_configurable_default, 0.0f, _T("testHighLevel"), NULL,
			end,

		kPoint3Param, _T("point3"), TYPE_POINT3, 0, 0,
			p_configurable_default, Point3(0, 0, 0), _T("testHighLevel"), NULL,
			end,

		kPoint4Param, _T("point4"), TYPE_FRGBA, 0, 0,
			p_configurable_default, Point4(0, 0, 0, 0), _T("testHighLevel"), NULL,
			end,

		end) {}
};

// Get a random number from low to high.
double getRand(double low, double high)
{
	return low + double(rand()) * (high - low) / double(RAND_MAX);
}

// Get a random array count
short getArrayCount()
{
	double v = getRand(1.0, 15.0);
	return short(v);
}

// Get a random integer
int getIntValue()
{
	return int(getRand(-2349856.0, 9877865.0));
}

// Get a random float.
float getFloatValue()
{
	return float(getRand(-92745.0, 65467.0));
}

// Get a random string
TCHAR* getStringValue()
{
	int size = int(getRand(1.0, 15.0));

	TCHAR* val = new TCHAR[size + 1];
	val[size] = '\0';

	// last letter is lower case
	val[--size] = 'a' + int(getRand(0.0, 26.0));
	// first letter is upper case
	val[0] = 'A' + int(getRand(0.0, 26.0));
	// Other letter are random, 1/6 spaces, 1/6 upper case, 2/3 lower case
	while (--size > 0) {
		int r = int(getRand(-26.0, 5.0 * 26.0));
		if (r < 0)
			val[size] = ' ';
		else if (r < 26)
			val[size] = 'A' + r;
		else
			val[size] = 'a' + r % 26;
	}

	return val;
}

// Create random values for keys
static void initKeyValues(const KeyDesc* key)
{
	// Don't create values for missing keys
	if (key->mMissing)
		return;

	short arrayCount = 1;		// Default to 1 value
	switch (key->mType) {
		case kIntArray:
			// Get number of values in array
			arrayCount = getArrayCount();
			// Fall into...
		case kInt: {
			// Allocate memory for array
			key->mpValue = new int[arrayCount];
			key->mValueCount = arrayCount;
			// Add values to array
			while (arrayCount > 0) {
				--arrayCount;
				static_cast<int*>(key->mpValue)[arrayCount] = getIntValue();
			}
		} break;
		case kFloatArray:
			// Get number of values in array
			arrayCount = getArrayCount();
			// Fall into...
		case kFloat: {
			// Allocate memory for array
			key->mpValue = new float[arrayCount];
			key->mValueCount = arrayCount;
			// Add values to array
			while (arrayCount > 0) {
				--arrayCount;
				static_cast<float*>(key->mpValue)[arrayCount] = getFloatValue();
			}
		} break;
		case kString: {
			// Get the string
			key->mValueCount = 1;
			key->mpValue = getStringValue();
		}
	}
}

// Initialize values for a section
static void initSectionValues(const SectionDesc* section)
{
	if (section->mMissing)
		return;

	int count = section->mKeyCount;

	// Loop over the keys and initialize each one.
	while (count > 0) {
		--count;
		initKeyValues(section->mpKeys + count);
	}
}

// Initialize value for a file
static void initFileValues(const FileDesc* file)
{
	int count = file->mSectionCount;

	// Loop over the sections and initialize each one
	while (count > 0) {
		--count;
		initSectionValues(file->mpSections + count);
	}
}

// Delete the memory used by each key
static void deleteKeyValues(const KeyDesc* key)
{
	// We delete memory used by missing keys because
	// one of the test will delete keys and mark them
	// as missing.
	switch (key->mType) {
		case kIntArray:
		case kInt: {
			// Delete the memory
			delete[] static_cast<int*>(key->mpValue);
			key->mpValue = NULL;
			key->mValueCount = 0;
		} break;
		case kFloatArray:
		case kFloat: {
			// Delete the memory
			delete[] static_cast<float*>(key->mpValue);
			key->mpValue = NULL;
			key->mValueCount = 0;
		} break;
		case kString: {
			// Delete the memory
			delete[] static_cast<TCHAR*>(key->mpValue);
			key->mpValue = NULL;
			key->mValueCount = 0;
		}
	}
}

// Delete memory use by sections
static void deleteSectionValues(const SectionDesc* section)
{
	// We delete memory used by missing sections because
	// one of the tests deletes sections and marks it missing.
	int count = section->mKeyCount;

	// Delete the memory for each key
	while (count > 0) {
		--count;
		deleteKeyValues(section->mpKeys + count);
	}
}

// Delete memory used by files
static void deleteFileValues(const FileDesc* file)
{
	int count = file->mSectionCount;

	while (count > 0) {
		--count;
		deleteSectionValues(file->mpSections + count);
	}
}

// Randomly choose an integer text format.
static const char* getIntFormat(int& v)
{
	switch (int(getRand(0.0, 3.0))) {
	case 0:
		return "%d";
	case 1:
		return "0x%x";
	}
	v = -v;
	return "-0x%x";
}

// Randomly choose a float text format
static const char* getFloatFormat()
{
	switch (int(getRand(0.0, 3.0))) {
	case 0:
		return "%.10f";
	case 1:
		return "%.10g";
	}
	return "%.10e";
}

// Write key values to a file
static void createKeyValues(FILE* f, const KeyDesc* key)
{
	// If the key is missing, don't write it.
	if (key->mMissing)
		return;

	// Write the key name
	fprintf(f, "%s=", key->mpName);

	switch (key->mType) {
		case kIntArray:
		case kInt: {
			// Write the key values separated by a space
			const char* space = "";
			int i;
			for (i = 0; i < key->mValueCount; ++i) {
				int v = static_cast<int*>(key->mpValue)[i];
				const char* fmt = getIntFormat(v);
				fprintf(f, "%s", space);
				fprintf(f, fmt, v);
				space = " ";
			}
		} break;
		case kFloatArray:
		case kFloat: {
			// Write the key values separated by a space
			const char* space = "";
			int i;
			for (i = 0; i < key->mValueCount; ++i) {
				float v = static_cast<float*>(key->mpValue)[i];
				const char* fmt = getFloatFormat();
				fprintf(f, "%s", space);
				fprintf(f, fmt, v);
				space = " ";
			}
		} break;
		case kString: {
			// Write the key value
			TCHAR* v = static_cast<TCHAR*>(key->mpValue);
			fprintf(f, "%s", v);
		} break;
	}

	fprintf(f, "\r\n");
}

// Write a section to a file
static void createSectionValues(FILE* f, const SectionDesc* section)
{
	if (section->mMissing)
		return;

	// The section string is based on a super-class id and class id or a string
	if (section->mClassID) {
		fprintf(f, "[%lX %lX %lX]\r\n",
			static_cast<const long*>(section->mpName)[0],
			static_cast<const long*>(section->mpName)[1],
			static_cast<const long*>(section->mpName)[2]);
	}
	else
		fprintf(f, "[%s]\r\n", section->mpName);

	int count = section->mKeyCount;

	while (count > 0) {
		--count;
		createKeyValues(f, section->mpKeys + count);
	}

	fprintf(f, "\r\n");
}

// Load the contents of a file into memory. Return the memory
// in value and the length in len. The return value is 1 if the
// file was loaded, 0 if it was missing, -1 if it wasn't missing
// but couldn't be opened or was too big.
static int getFileContents(const TCHAR* file, char*& value, long& len)
{
	// Is the file missing
	bool f = _taccess(file, 0) == 0;
	value = NULL;
	len = 0;

	if (!f)
		return 0;	// Missing file return 0

	// Open the file.
	HANDLE h = CreateFile(file, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == NULL)
		return -1;	// File couldn't be opened

	// Get the file size. Limit file size to 1MB.
	DWORD high;
	DWORD size = GetFileSize(h, &high);
	if (high == 0 && size <= 1024 * 1024) {
		value = new char[size + 4];
		if (ReadFile(h, value, size + 4, &high, NULL) && high == size) {
			// File was read and the size is correct. 
			len = high;
			CloseHandle(h);
			return 1;
		}
		delete[] value;
		value = NULL;
	}

	// We had some problem.
	CloseHandle(h);
	return -1;
}

// compare the contents of two files in memory.
// If both are missing or both present and equal, that is
// OK, but if either had an error reading the file, then
// the compare fails.
static bool filesAreEqual(int found1, char* value1, long len1,
				   int found2, char* value2, long len2)
{
	return found1 >= 0 && found2 >= 0 && found1 == found2
		&& len1 == len2 && memcmp(value1, value2, len1) == 0;
}

// Check whether two files are equal. Load the file into
// memory and compare them.
static bool filesAreEqual(const TCHAR* file1, const TCHAR* file2)
{
	char* v1, * v2;
	long l1, l2;
	int f1, f2;
	f1 = getFileContents(file1, v1, l1);
	f2 = getFileContents(file2, v2, l2);
	bool e = filesAreEqual(f1, v1, l1, f2, v2, l2);
	delete[] v1;
	delete[] v2;
	return 1;
}

// Initialize the tester.
MarketDefaultTest::Tester::Tester(ErrorStatus& status)
	: mStatus(status),
	  mDefaultsDir(GetCOREInterface()->GetDir(APP_MARKETDEFAULTS_DIR))
{
	int len = mDefaultsDir.length() - 1;
	if (mDefaultsDir.last('\\') != len && mDefaultsDir.last('/') != len)
		mDefaultsDir += "\\";
}

// Delete memory for file1. Keep file2 around, because
// We can't delete a MarketDefaults object.
MarketDefaultTest::Tester::~Tester()
{
	deleteFileValues(&file1);
}

// Create a file and write values to it.
bool MarketDefaultTest::Tester::createFile(
	const TCHAR* path, const FileDesc* file)
{
	// Open the file.
	FILE* f = fopen(path, "w");
	if (f == NULL) {
		mStatus.message(kErrorMessage, "Unexpected Error: Cannot create %s\r\n", path);
		return false;
	}

	int count = file->mSectionCount;

	// Write the values for each section
	while (count > 0) {
		--count;
		createSectionValues(f, file->mpSections + count);
	}

	// Close the file
	fclose(f);
	return true;
}

// Get the path to a defaults file
TSTR MarketDefaultTest::Tester::getDefaultFilePath(const TCHAR* filename)
{
	return mDefaultsDir + filename;
}

// Get the path to a factory defaults file
TSTR MarketDefaultTest::Tester::getFactoryDefaultFilePath(const TCHAR* filename)
{
	return mDefaultsDir + "FactoryDefaults\\" + filename;
}

int MarketDefaultTest::Tester::testAll()
{
	mStatus.message(kInfoMessage, "Starting testAll\r\n");

	// Do all of the tests
	int errors = testFiles()
			   + testLowLevel()
			   + testHighLevel()
			   + testRanges()
			   + testCfgDefaults();

	mStatus.message(kInfoMessage, "Finished testAll - %d error(s)\r\n\r\n", errors);

	return errors;
}

int MarketDefaultTest::Tester::testFiles()
{
	mStatus.message(kInfoMessage, "Starting testFiles\r\n");

	// Initialize values for the files
	initFileValues(&file1);

	int errors = testFileCopy(_T("testFileCopy.ini"))
			   + testFileMissing(_T("testFileMissing.ini"))
			   + testFilePresent(_T("testFilePresent.ini"));

	mStatus.message(kInfoMessage, "Finished testFiles - %d errors\r\n\r\n", errors);

	return errors;
}

int MarketDefaultTest::Tester::testFileCopy(const TCHAR* file)
{
	mStatus.message(kInfoMessage, "Starting testFileCopy %s\r\n", file);

	// First create a file in the factory defaults
	TSTR path = getFactoryDefaultFilePath(file);
	if (!createFile(path, &file1))
		return 1;

	TSTR expected = getDefaultFilePath(file); 
	DeleteFile(expected);

	int errors = 0;

	// Now copy it to the defaults directory
	TSTR newPath = GetMarketDefaultsFileName(file);
	if (newPath != expected) {
		mStatus.message(kErrorMessage, _T("%s was copied to the wrong file\r\n")
									   _T("    Expected: %s\r\n")
									   _T("    Got: %s\r\n"),
			path.data(), expected.data(), newPath.data());
		++errors;
	}

	if (!filesAreEqual(path, newPath)) {
		mStatus.message(kErrorMessage, _T("%s was copied incorrectly\r\n"),
			newPath.data());
		++errors;
	}

	// Clean up
	DeleteFile(path);
	DeleteFile(expected);
	DeleteFile(newPath);

	mStatus.message(kInfoMessage, "Finished testFileCopy %s - %d error(s)\r\n\r\n", file, errors);

	return errors;
}

int MarketDefaultTest::Tester::testFileMissing(const TCHAR* file)
{
	mStatus.message(kInfoMessage, "Starting testFileMissing %s\r\n", file);

	// Now try again with a file that should exist
	TSTR path = getFactoryDefaultFilePath(file);
	DeleteFile(path);

	TSTR expected = getDefaultFilePath(file); 
	DeleteFile(expected);

	int errors = 0;

	// Now copy it to the defaults directory
	TSTR newPath = GetMarketDefaultsFileName(file);
	if (newPath != expected) {
		mStatus.message(kErrorMessage, _T("%s was copied to the wrong file\r\n")
									   _T("    Expected: %s\r\n")
									   _T("    Got: %s\r\n"),
			path.data(), expected.data(), newPath.data());
		++errors;
	}

	if (!filesAreEqual(path, newPath)) {
		mStatus.message(kErrorMessage, _T("%s was created incorrectly\r\n"),
			newPath.data());
		++errors;
	}

	// Clean up
	DeleteFile(path);
	DeleteFile(expected);
	DeleteFile(newPath);

	mStatus.message(kInfoMessage, "Finished testFileMissing %s - %d error(s)\r\n\r\n", file, errors);

	return errors;
}

int MarketDefaultTest::Tester::testFilePresent(const TCHAR* file)
{
	mStatus.message(kInfoMessage, "Starting testFilePresent %s\r\n", file);

	// Now try again with a file that should exist
	TSTR path = getFactoryDefaultFilePath(file);
	DeleteFile(path);

	TSTR expected = getDefaultFilePath(file); 
	DeleteFile(expected);

	if (!createFile(expected, &file1))
		return 1;

	char* valueSrc;
	long lengthSrc;
	int foundSrc = getFileContents(expected, valueSrc, lengthSrc);

	int errors = 0;

	// Now copy it to the defaults directory
	TSTR newPath = GetMarketDefaultsFileName(file);
	if (newPath != expected) {
		mStatus.message(kErrorMessage, _T("%s was copied to the wrong file\r\n")
									   _T("    Expected: %s\r\n")
									   _T("    Got: %s\r\n"),
			path.data(), expected.data(), newPath.data());
		++errors;
	}

	char* valueDst;
	long lengthDst;
	int foundDst = getFileContents(expected, valueDst, lengthDst);

	if (!filesAreEqual(foundSrc, valueSrc, lengthSrc,
			foundDst, valueDst, lengthDst)) {
		mStatus.message(kErrorMessage, _T("%s was changed by copy\r\n"),
			expected.data());
		++errors;
	}

	// Clean up
	delete[] valueSrc;
	delete[] valueDst;

	DeleteFile(path);
	DeleteFile(expected);
	DeleteFile(newPath);

	mStatus.message(kInfoMessage, "Finished testFilePresent %s - %d error(s)\r\n\r\n", file, errors);

	return errors;
}

// Test the low level interface
int MarketDefaultTest::Tester::testLowLevel()
{
	mStatus.message(kInfoMessage, "Starting testLowLevel\r\n");

	// If we haven't created the lowlevel test file do so now
	// We can only create it once for each MAX session, because
	// we can't delete the MarketDefaults object.
	if (!file2Initialized) {
		initFileValues(&file2);
		TSTR path = getDefaultFilePath(_T("testLowLevelRead.ini"));
		DeleteFile(path);
		if (!createFile(path, &file2))
			return 1;
		file2Initialized = true;
	}

	// Get the MarketDefaults object
	MarketDefaults* dflts = GetMarketDefaults(_T("testLowLevelRead"));
	if (dflts == NULL) {
		mStatus.message(kErrorMessage, _T("Unexpected Error: Market defaults is NULL\r\n"));
		return 1;
	}

	int errors = 0;

	// Verify the values in each section and key
	errors += verifyValues(dflts, &file2);

	// This one we can do multiple times, because we are using the API
	initFileValues(&file1);
	dflts = GetMarketDefaults(_T("testLowLevelWrite"));
	if (dflts == NULL) {
		mStatus.message(kErrorMessage, _T("Unexpected Error: Market defaults is NULL\r\n"));
		return 1;
	}

	// Write the value using the API
	errors += putFile(dflts, &file1);
	// Verify that they are correct
	errors += verifyValues(dflts, &file1);
	// Remove some of the values
	removeValues(dflts, &file1);
	// Verify that they are correct
	errors += verifyValues(dflts, &file1);

	// Cleanup
	deleteFileValues(&file1);
	DeleteFile(GetMarketDefaultsFileName(_T("testLowLevelWrite")));

	mStatus.message(kInfoMessage, "Finished testLowLevel - %d error(s)\r\n\r\n", errors);

	return errors;
}

int MarketDefaultTest::Tester::testHighLevel()
{
	mStatus.message(kInfoMessage, "Starting testHighLevel\r\n");

	int errors = 0;

	// This one we can do multiple times, because we are using the API
	MarketDefaults* dflts = GetMarketDefaults(_T("testHighLevel"));
	if (dflts == NULL) {
		mStatus.message(kErrorMessage, _T("Unexpected Error: Market defaults is NULL\r\n"));
		return 1;
	}

	// Create the values
	errors += createHighLevelValues(dflts);
	// Check the values
	errors += verifyHighLevelValues(dflts);

	// Cleanup
	DeleteFile(GetMarketDefaultsFileName(_T("testHighLevel")));

	mStatus.message(kInfoMessage, "Finished testHighLevel - %d error(s)\r\n\r\n", errors);

	return errors;
}

int MarketDefaultTest::Tester::testRanges()
{
	mStatus.message(kInfoMessage, "Starting testRanges\r\n");

	int errors = 0;

	// This one we can do multiple times, because we are using the API
	MarketDefaults* dflts = GetMarketDefaults(_T("testHighLevel"));
	if (dflts == NULL) {
		mStatus.message(kErrorMessage, _T("Unexpected Error: Market defaults is NULL\r\n"));
		return 1;
	}

	// Create the values
	errors += createHighLevelValues(dflts);
	// Check the values
	errors += verifyRanges(dflts);

	// Cleanup
	DeleteFile(GetMarketDefaultsFileName(_T("testHighLevel")));

	mStatus.message(kInfoMessage, "Finished testRanges - %d error(s)\r\n\r\n", errors);

	return errors;
}

int MarketDefaultTest::Tester::testCfgDefaults()
{
	mStatus.message(kInfoMessage, "Starting testCfgDefaults\r\n");

	int errors = 0;

	// This one we can do multiple times, because we are using the API
	MarketDefaults* dflts = GetMarketDefaults(_T("testHighLevel"));
	if (dflts == NULL) {
		mStatus.message(kErrorMessage, _T("Unexpected Error: Market defaults is NULL\r\n"));
		return 1;
	}

	CfgDefaultTestClassDesc cd;
	CfgDefaultTestParamBlock pd(cd);

#define CHECKVALUE(f, i, v, n) highLevelCheck(pd.paramdefs[i].def.f, v, _T(n))

	errors += CHECKVALUE(i, 0, intValue, "int"); 
	errors += CHECKVALUE(f, 1, floatValue, "float"); 
	errors += CHECKVALUE(p[0], 2, Point3(array3), "Point3"); 
	errors += CHECKVALUE(p4[0], 3, Point4(array4), "Point4"); 
#undef CHECKVALUE

	mStatus.message(kInfoMessage, "Finished testCfgDefaults - %d error(s)\r\n\r\n", errors);

	return errors;
}

int MarketDefaultTest::Tester::verifyValues(MarketDefaults* dflts, const FileDesc* file)
{
	int errors = 0;
	int i = 0;

	for (i = 0; i < file->mSectionCount; ++i) {
		errors += verifyValues(dflts, &file->mpSections[i]);
	}

	// Create the values
	errors += createHighLevelValues(dflts);

	return errors;
}

int MarketDefaultTest::Tester::verifyValues(MarketDefaults* dflts, const SectionDesc* section)
{
	bool exists;
	if (section->mClassID) {
		exists = dflts->SectionExists(
			static_cast<const long*>(section->mpName)[0],
			Class_ID(static_cast<const long*>(section->mpName)[1],
			static_cast<const long*>(section->mpName)[2]));
		dflts->SetSectionName(
			static_cast<const long*>(section->mpName)[0],
			Class_ID(static_cast<const long*>(section->mpName)[1],
			static_cast<const long*>(section->mpName)[2]));
	}
	else {
		exists = dflts->SectionExists(static_cast<const TCHAR*>(section->mpName));
		dflts->SetSectionName(static_cast<const TCHAR*>(section->mpName));
	}

	if (exists == section->mMissing) {
		mStatus.message(kErrorMessage,
			exists ? _T("Section [%s] exists but should be missing\r\n")
				   : _T("Section [%s] is missing but should exist\r\n"),
			dflts->GetSectionName());
		return 1;
	}

	if (section->mMissing)
		return 0;

	int errors = 0;

	int i = 0;

	for (i = 0; i < section->mKeyCount; ++i) {
		errors += verifyValues(dflts, &section->mpKeys[i]);
	}

	return errors;
}

template<class T>
int MarketDefaultTest::Tester::checkArray(MarketDefaults* dflts, const KeyDesc* key,
	T* val, int sizeAdjust,
	bool (MarketDefaults::*getFunc)(const TCHAR* key, int& ct, T* val, int size),
	const char* valType, const char* fmt)
{
	int size = key->mValueCount + sizeAdjust;
	if (size < 0)
		size = 0;

	int errors = 0;
	int ct;
	if (!(dflts->*getFunc)(key->mpName, ct, val, size)) {
		mStatus.message(kErrorMessage, _T("Key [%s]%s didn't return %valType array value\r\n"),
			dflts->GetSectionName(), key->mpName, valType);
		++errors;
	} else {
		if (ct != key->mValueCount) {
			mStatus.message(kErrorMessage, _T("Key [%s]%s cound didn't compare\r\n")
										   _T("    %d expected\r\n")
										   _T("    %d found\r\n"),
				dflts->GetSectionName(), key->mpName, key->mValueCount, ct);
			++errors;
		}
		if (size > ct)
			size = ct;
		const TCHAR* error = _T("Key [%s]%s didn't compare\r\n");
		for (ct = 0; ct < size; ++ct) {
			if (val[ct] != static_cast<T*>(key->mpValue)[ct]) {
				mStatus.message(kErrorMessage, error, dflts->GetSectionName(), key->mpName);
				error = "";
				mStatus.message(kErrorMessage, _T("    Index %d \""), ct);
				mStatus.message(kErrorMessage, fmt, static_cast<T*>(key->mpValue)[ct]);
				mStatus.message(kErrorMessage, _T("\" expected\r\n"));
				mStatus.message(kErrorMessage, _T("    Index %d \""), ct);
				mStatus.message(kErrorMessage, fmt, val[ct]);
				mStatus.message(kErrorMessage, _T("\" found\r\n"));
				++errors;
			}
		}
	}

	return errors;
}

int MarketDefaultTest::Tester::verifyValues(MarketDefaults* dflts, const KeyDesc* key)
{
	bool exists = dflts->KeyExists(key->mpName);

	if (exists == key->mMissing) {
		mStatus.message(kErrorMessage,
			exists ? _T("Key [%s]%s exists but should be missing\r\n")
				   : _T("Key [%s]%s is missing but should exist\r\n"),
			dflts->GetSectionName(), key->mpName);
		return 1;
	}

	if (key->mMissing)
		return 0;

	int errors = 0;

	switch (key->mType) {
		case kInt: {
			int val;
			if (!dflts->GetInt(key->mpName, val)) {
				mStatus.message(kErrorMessage, _T("Key [%s]%s didn't return an integer value\r\n"),
					dflts->GetSectionName(), key->mpName);
				++errors;
			} else if (val != static_cast<int*>(key->mpValue)[0]) {
				mStatus.message(kErrorMessage, _T("Key [%s]%s didn't compare\r\n")
											_T("    \"%d\" was expected\r\n")
											_T("    \"%d\" was found\r\n"),
					dflts->GetSectionName(), key->mpName, key->mpValue, val);
				++errors;
			}
		} break;

		case kIntArray: {
			int* val = new int[key->mValueCount + 10];
			errors += checkArray(dflts, key, val, 0, &MarketDefaults::GetIntArray,
				_T("an integer"), _T("%d"));
			errors += checkArray(dflts, key, val, -5, &MarketDefaults::GetIntArray,
				_T("an integer"), _T("%d"));
			errors += checkArray(dflts, key, val, +3, &MarketDefaults::GetIntArray,
				_T("an integer"), _T("%d"));
			delete[] val;
		} break;

		case kFloat: {
			float val;
			if (!dflts->GetFloat(key->mpName, val)) {
				mStatus.message(kErrorMessage, _T("Key [%s]%s didn't return a float value\r\n"),
					dflts->GetSectionName(), key->mpName);
				++errors;
			} else if (val != static_cast<float*>(key->mpValue)[0]) {
				mStatus.message(kErrorMessage, _T("Key [%s]%s didn't compare\r\n")
											_T("    \"%.10g\" was expected\r\n")
											_T("    \"%.10g\" was found\r\n"),
					dflts->GetSectionName(), key->mpName, key->mpValue, val);
				++errors;
			}
		} break;

		case kFloatArray: {
			float* val = new float[key->mValueCount + 10];
			errors += checkArray(dflts, key, val, 0, &MarketDefaults::GetFloatArray,
				_T("a float"), _T("%.10g"));
			errors += checkArray(dflts, key, val, -5, &MarketDefaults::GetFloatArray,
				_T("a float"), _T("%.10g"));
			errors += checkArray(dflts, key, val, +3, &MarketDefaults::GetFloatArray,
				_T("a float"), _T("%.10g"));
			delete[] val;
		} break;

		case kString: {
			TCHAR val[1024];
			if (!dflts->GetString(key->mpName, val, sizeof(val) / sizeof(val[0]))) {
				mStatus.message(kErrorMessage, _T("Key [%s]%s didn't return a string value\r\n"),
					key->mpName);
				++errors;
			} else if (_tcscmp(val, static_cast<TCHAR*>(key->mpValue)) != 0) {
				mStatus.message(kErrorMessage, _T("Key [%s]%s didn't compare\r\n")
											_T("    \"%s\" was expected\r\n")
											_T("    \"%s\" was found\r\n"),
					dflts->GetSectionName(), key->mpName, key->mpValue, val);
				++errors;
			}
		} break;
	}

	return errors;
}


// Write values to a file using the MarketDefaults API
int MarketDefaultTest::Tester::putFile(
	MarketDefaults* dflts, const FileDesc* file)
{
	int count = file->mSectionCount;

	// Write the values for each section
	int errors = 0;
	while (count > 0) {
		--count;
		errors += putSectionValues(dflts, file->mpSections + count);
	}

	// Return error count
	return errors;
}

// Write value for a section using the MarketDefaults API
int MarketDefaultTest::Tester::putSectionValues(MarketDefaults* dflts, const SectionDesc* section)
{
	// Don't write missing sections
	if (section->mMissing)
		return 0;

	// Set the section, based on a super-class id and class id or a string
	if (section->mClassID) {
		dflts->SetSectionName(static_cast<const long*>(section->mpName)[0],
			Class_ID(static_cast<const long*>(section->mpName)[1],
				static_cast<const long*>(section->mpName)[2]));
	}
	else
		dflts->SetSectionName(static_cast<const TCHAR*>(section->mpName));

	int count = section->mKeyCount;
	int errors = 0;

	// Write all of the values for the keys
	while (count > 0) {
		--count;
		errors += putKeyValues(dflts, section->mpKeys + count);
	}

	return errors;
}

// Write value for a key using the MarketDefault API
int MarketDefaultTest::Tester::putKeyValues(MarketDefaults* dflts, const KeyDesc* key)
{
	// Don't write missing strings
	if (key->mMissing)
		return 0;

	int errors = 0;

	// Write the value based on the type
	switch (key->mType) {
		case kInt:
			if (!dflts->PutInt(key->mpName, static_cast<int*>(key->mpValue)[0])) {
				mStatus.message(kErrorMessage, _T("Key [%s]:%s PutInt returned false\r\n"),
					dflts->GetSectionName(), key->mpName);
				++errors;
			}
			break;

		case kIntArray:
			if (!dflts->PutIntArray(key->mpName, key->mValueCount,
					static_cast<int*>(key->mpValue))) {
				mStatus.message(kErrorMessage, _T("Key [%s]:%s PutIntArray returned false\r\n"),
					dflts->GetSectionName(), key->mpName);
				++errors;
			}
			break;
		case kFloat:
			if (!dflts->PutFloat(key->mpName, static_cast<float*>(key->mpValue)[0])) {
				mStatus.message(kErrorMessage, _T("Key [%s]:%s PutFloat returned false\r\n"),
					dflts->GetSectionName(), key->mpName);
				++errors;
			}
			break;
		case kFloatArray:
			if (!dflts->PutFloatArray(key->mpName, key->mValueCount,
					static_cast<float*>(key->mpValue))) {
				mStatus.message(kErrorMessage, _T("Key [%s]:%s PutFloatArray returned false\r\n"),
					dflts->GetSectionName(), key->mpName);
				++errors;
			}
			break;
		case kString:
			if (!dflts->PutString(key->mpName, static_cast<TCHAR*>(key->mpValue))) {
				mStatus.message(kErrorMessage, _T("Key [%s]:%s PutString returned false\r\n"),
					dflts->GetSectionName(), key->mpName);
				++errors;
			}
			break;
	}

	return errors;
}


// Remove value and sections from defaults
void MarketDefaultTest::Tester::removeValues(
	MarketDefaults* dflts, const FileDesc* file)
{
	int count = file->mSectionCount;

	// Remove the values for each section
	while (count > 0) {
		--count;
		removeSectionValues(dflts, file->mpSections + count);
	}
}

// Remove a section or values in a section
void MarketDefaultTest::Tester::removeSectionValues(MarketDefaults* dflts, const SectionDesc* section)
{
	// Don't remove missing sections
	if (section->mMissing)
		return;

	// Set the section, based on a super-class id and class id or a string
	if (section->mClassID) {
		static remove = false;
		remove ^= true;
		if (remove) {
			dflts->DeleteSection(static_cast<const long*>(section->mpName)[0],
				Class_ID(static_cast<const long*>(section->mpName)[1],
					static_cast<const long*>(section->mpName)[2]));
			section->mMissing = true;
			return;
		}
		dflts->SetSectionName(static_cast<const long*>(section->mpName)[0],
			Class_ID(static_cast<const long*>(section->mpName)[1],
				static_cast<const long*>(section->mpName)[2]));
	}
	else {
		static remove = false;
		remove ^= true;
		if (remove) {
			dflts->DeleteSection(static_cast<const TCHAR*>(section->mpName));
			section->mMissing = true;
			return;
		}
		dflts->SetSectionName(static_cast<const TCHAR*>(section->mpName));
	}

	int count = section->mKeyCount;

	// Remove some of the values for the keys
	while (count > 0) {
		--count;
		removeKeyValues(dflts, section->mpKeys + count);
	}
}

// Remove value for a key using the MarketDefault API
void MarketDefaultTest::Tester::removeKeyValues(MarketDefaults* dflts, const KeyDesc* key)
{
	// Don't remove missing strings
	if (key->mMissing)
		return;

	static bool remove = false;

	remove ^= true;
	if (remove) {
		dflts->DeleteKey(key->mpName);
		key->mMissing = true;
	}
}

int MarketDefaultTest::Tester::highLevelPutError(bool ok, const TCHAR* type)
{
	if (ok)
		return 0;

	mStatus.message(kErrorMessage, _T("Error writing %s value\r\n"), type);
	return 1;
}

inline void MarketDefaultTest::Tester::format(int val)
{
	mStatus.message(kErrorMessage, "%d", val);
}	

inline void MarketDefaultTest::Tester::format(float val)
{
	mStatus.message(kErrorMessage, "%g", val);
}	

inline void MarketDefaultTest::Tester::format(const TSTR& val)
{
	mStatus.message(kErrorMessage, "%s", val.data());
}	

inline void MarketDefaultTest::Tester::format(const Point3& val)
{
	mStatus.message(kErrorMessage, "(%g, %g, %g)", val.x, val.y, val.z);
}	

inline void MarketDefaultTest::Tester::format(const Point4& val)
{
	mStatus.message(kErrorMessage, "(%g, %g, %g, %g)", val.x, val.y, val.z, val.w);
}	

inline void MarketDefaultTest::Tester::format(Class_ID val)
{
	mStatus.message(kErrorMessage, "(%x, %x)", val.PartA(), val.PartB());
}	

template<class T>
int MarketDefaultTest::Tester::highLevelCheck(T got, T expected,
	const TCHAR* type)
{
	if (got == expected)
		return 0;

	mStatus.message(kErrorMessage, _T("Error reading %s\r\n"), type);
	mStatus.message(kErrorMessage, _T("    \""));
	format(expected);
	mStatus.message(kErrorMessage, _T("\" was expected\r\n    \""));
	format(got);
	mStatus.message(kErrorMessage, _T("\" was found\r\n"));
	return 1;
}

int MarketDefaultTest::Tester::createHighLevelValues(MarketDefaults* dflts)
{
	int errors = 0;

	dflts->SetSectionName(highLevelSClass, highLevelClass);
	errors += highLevelPutError(dflts->PutInt(_T("int"), intValue),
		_T("int"));
	errors += highLevelPutError(dflts->PutFloat(_T("float"), floatValue),
		_T("float"));
	errors += highLevelPutError(dflts->PutFloatArray(_T("point3"), 3, array3),
		_T("point3"));
	errors += highLevelPutError(dflts->PutFloatArray(_T("point4"), 4, array4),
		_T("point4"));
	errors += highLevelPutError(dflts->PutString(_T("string"), theString),
		_T("string"));
	errors += highLevelPutError(dflts->PutString(_T("nullstring"), _T("")),
		_T("nullstring"));
	errors += highLevelPutError(dflts->PutIntArray(_T("classid"), 2, clarray),
		_T("classid"));
	errors += highLevelPutError(dflts->PutIntArray(_T("nullclassid"), 2, nullclarray),
		_T("nullclassid"));
	errors += highLevelPutError(dflts->PutIntArray(_T("standinclassid"), 2, standinclarray),
		_T("standinclassid"));
	errors += highLevelPutError(dflts->PutIntArray(_T("junkclassid"), 2, junkclarray),
		_T("junkclassid"));

	return errors;
}

int MarketDefaultTest::Tester::verifyHighLevelValues(MarketDefaults* dflts)
{
	int errors = 0;

#define CHECKVALUE(t, n, d, v) highLevelCheck(dflts->Get##t(highLevelSClass, highLevelClass,\
							_T(n), d), v, _T(n))

	errors += CHECKVALUE(Int, "int", 0, intValue);
	errors += CHECKVALUE(Float, "float", 0.0f, floatValue);
	errors += CHECKVALUE(Point3, "point3",  Point3(0,0,0), Point3(array3));
	errors += CHECKVALUE(Point4, "point4", Point4(0,0,0,0), Point4(array4));
	errors += CHECKVALUE(String, "string", _T(""), TSTR(theString));
	errors += CHECKVALUE(ClassID, "classid", Class_ID(0,0), Class_ID(clarray[0], clarray[1]));

	errors += verifyMtl(dflts->CreateMtl(highLevelSClass, highLevelClass, _T("classid"),
		Class_ID(0, 0)), Class_ID(clarray[0], clarray[1]), _T("classid"));

#undef CHECKVALUE

	return errors;
}

int MarketDefaultTest::Tester::verifyMtl(Mtl* m, Class_ID id, const TCHAR* name)
{
	int errors = 0;

	if (m == NULL) {
		mStatus.message(kErrorMessage, "Error reading %s\r\n", name);
		mStatus.message(kErrorMessage, "Could not create material\r\n");
		++errors;
	}
	else {
		if (m->SuperClassID() != MATERIAL_CLASS_ID) {
			mStatus.message(kErrorMessage, "Error reading %s\r\n", name);
			mStatus.message(kErrorMessage, "Expected material, got super-class %x\r\n",
				m->SuperClassID());
			++errors;
		}
		else if (m->ClassID() != id) {
			mStatus.message(kErrorMessage, "Error reading %s\r\n", name);
			mStatus.message(kErrorMessage, "Expected class id %x %x, got class %x %x\r\n",
				id.PartA(), id.PartB(),
				m->ClassID().PartA(), m->ClassID().PartB());
			++errors;
		}
		m->DeleteThis();
	}

	return errors;
}

int MarketDefaultTest::Tester::verifyRanges(MarketDefaults* dflts)
{
	int errors = 0;

#define CHECKRANGE(t, n, d, l, h, f, v) highLevelCheck(dflts->Get##t(highLevelSClass, highLevelClass,\
							_T(#n), d, MarketDefaults::Range<n, l, h>::f), v,\
							_T(#t) _T("Range<") _T(", ") _T(#l) _T(", ") _T(#h) _T(">::") _T(#f))
#define CHECKVALUE(t, n, d, r, v) highLevelCheck(dflts->Get##t(highLevelSClass, highLevelClass,\
							_T(n), d, MarketDefaults::r), v, _T(n))

	errors += CHECKRANGE(Int, int, 0, 0, 8, CheckII, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, 8, CheckEI, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, 8, CheckIE, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, 8, CheckEE, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, 8, Correct, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, 8, CorrectHigh, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, 8, CorrectLow, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, 3, CheckII, 0);
	errors += CHECKRANGE(Int, int, 0, 0, 3, CheckEI, 0);
	errors += CHECKRANGE(Int, int, 0, 0, 3, CheckIE, 0);
	errors += CHECKRANGE(Int, int, 0, 0, 3, CheckEE, 0);
	errors += CHECKRANGE(Int, int, 0, 0, 3, Correct, 3);
	errors += CHECKRANGE(Int, int, 0, 0, 3, CorrectHigh, 3);
	errors += CHECKRANGE(Int, int, 0, 0, 3, CorrectLow, 0);
	errors += CHECKRANGE(Int, int, 0, 6, 8, CheckII, 0);
	errors += CHECKRANGE(Int, int, 0, 6, 8, CheckEI, 0);
	errors += CHECKRANGE(Int, int, 0, 6, 8, CheckIE, 0);
	errors += CHECKRANGE(Int, int, 0, 6, 8, CheckEE, 0);
	errors += CHECKRANGE(Int, int, 0, 6, 8, Correct, 6);
	errors += CHECKRANGE(Int, int, 0, 6, 8, CorrectHigh, 0);
	errors += CHECKRANGE(Int, int, 0, 6, 8, CorrectLow, 6);
	errors += CHECKRANGE(Int, int, 0, 0, intValue, CheckII, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, intValue, CheckEI, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, intValue, CheckIE, 0);
	errors += CHECKRANGE(Int, int, 0, 0, intValue, CheckEE, 0);
	errors += CHECKRANGE(Int, int, 0, 0, intValue, Correct, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, intValue, CorrectHigh, intValue);
	errors += CHECKRANGE(Int, int, 0, 0, intValue, CorrectLow, intValue);
	errors += CHECKRANGE(Int, int, 0, intValue, 8, CheckII, intValue);
	errors += CHECKRANGE(Int, int, 0, intValue, 8, CheckEI, 0);
	errors += CHECKRANGE(Int, int, 0, intValue, 8, CheckIE, intValue);
	errors += CHECKRANGE(Int, int, 0, intValue, 8, CheckEE, 0);
	errors += CHECKRANGE(Int, int, 0, intValue, 8, Correct, intValue);
	errors += CHECKRANGE(Int, int, 0, intValue, 8, CorrectHigh, intValue);
	errors += CHECKRANGE(Int, int, 0, intValue, 8, CorrectLow, intValue);

	errors += CHECKRANGE(Float, float, 0, 00, 88, CheckII, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, 88, CheckEI, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, 88, CheckIE, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, 88, CheckEE, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, 88, Correct, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, 88, CorrectHigh, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, 88, CorrectLow, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, 38, CheckII, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 00, 38, CheckEI, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 00, 38, CheckIE, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 00, 38, CheckEE, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 00, 38, Correct, 38.0f);
	errors += CHECKRANGE(Float, float, 0, 00, 38, CorrectHigh, 38.0f);
	errors += CHECKRANGE(Float, float, 0, 00, 38, CorrectLow, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 60, 88, CheckII, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 60, 88, CheckEI, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 60, 88, CheckIE, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 60, 88, CheckEE, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 60, 88, Correct, 60.0f);
	errors += CHECKRANGE(Float, float, 0, 60, 88, CorrectHigh, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 60, 88, CorrectLow, 60.0f);
	errors += CHECKRANGE(Float, float, 0, 00, floatTemplate, CheckII, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, floatTemplate, CheckEI, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, floatTemplate, CheckIE, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 00, floatTemplate, CheckEE, 00.0f);
	errors += CHECKRANGE(Float, float, 0, 00, floatTemplate, Correct, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, floatTemplate, CorrectHigh, floatValue);
	errors += CHECKRANGE(Float, float, 0, 00, floatTemplate, CorrectLow, floatValue);
	errors += CHECKRANGE(Float, float, 0, floatTemplate, 88, CheckII, floatValue);
	errors += CHECKRANGE(Float, float, 0, floatTemplate, 88, CheckEI, 00.0f);
	errors += CHECKRANGE(Float, float, 0, floatTemplate, 88, CheckIE, floatValue);
	errors += CHECKRANGE(Float, float, 0, floatTemplate, 88, CheckEE, 00.0f);
	errors += CHECKRANGE(Float, float, 0, floatTemplate, 88, Correct, floatValue);
	errors += CHECKRANGE(Float, float, 0, floatTemplate, 88, CorrectHigh, floatValue);
	errors += CHECKRANGE(Float, float, 0, floatTemplate, 88, CorrectLow, floatValue);
	errors += CHECKVALUE(String, "string", _T("NULL"), CheckNULL, TSTR(_T("the string")));
	errors += CHECKVALUE(String, "nullstring", _T("NULL"), CheckNULL, TSTR(_T("NULL")));
	errors += CHECKVALUE(ClassID, "classid", Class_ID(MULTI_CLASS_ID,0), CheckNULL, Class_ID(clarray[0], clarray[1]));
	errors += CHECKVALUE(ClassID, "nullclassid", Class_ID(MULTI_CLASS_ID,0), CheckNULL, Class_ID(MULTI_CLASS_ID, 0));
	errors += CHECKVALUE(ClassID, "standinclassid", Class_ID(MULTI_CLASS_ID,0), CheckNULL, Class_ID(MULTI_CLASS_ID, 0));
	errors += CHECKVALUE(ClassID, "junkclassid", Class_ID(MULTI_CLASS_ID,0), CheckNULL, Class_ID(junkclarray[0], junkclarray[1]));

	errors += verifyMtl(dflts->CreateMtl(highLevelSClass, highLevelClass, _T("classid"),
		Class_ID(MULTI_CLASS_ID, 0), MarketDefaults::CheckNULL), Class_ID(clarray[0], clarray[1]),
		_T("classid"));
	errors += verifyMtl(dflts->CreateMtl(highLevelSClass, highLevelClass, _T("nullclassid"),
		Class_ID(MULTI_CLASS_ID, 0), MarketDefaults::CheckNULL), Class_ID(MULTI_CLASS_ID, 0),
		_T("nullclassid"));
	errors += verifyMtl(dflts->CreateMtl(highLevelSClass, highLevelClass, _T("standinclassid"),
		Class_ID(MULTI_CLASS_ID, 0), MarketDefaults::CheckNULL), Class_ID(MULTI_CLASS_ID, 0),
		_T("standinclassid"));
	errors += verifyMtl(dflts->CreateMtl(highLevelSClass, highLevelClass, _T("junkclassid"),
		Class_ID(MULTI_CLASS_ID, 0), MarketDefaults::CheckNULL), Class_ID(MULTI_CLASS_ID, 0),
		_T("junkclassid"));

#undef CHECKVALUE
#undef CHECKRANGE

	return errors;
}
