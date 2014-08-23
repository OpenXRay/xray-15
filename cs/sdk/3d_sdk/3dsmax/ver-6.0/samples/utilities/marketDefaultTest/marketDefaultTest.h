/**********************************************************************
 *<
	FILE: marketDefaultTest.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef __MARKETDEFAULTTEST__H
#define __MARKETDEFAULTTEST__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "utilapi.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define MARKETDEFAULTTEST_CLASS_ID	Class_ID(0x5b508bde, 0x6307eca6)

enum {
	kErrorMessage = 0,
	kWarningMessage = 1,
	kInfoMessage = 2,
	kDebugMessage = 3,
	kAnyMessage = 3,
};

class MarketDefaultTestInterface : public FPStaticInterface {
public:
	DECLARE_DESCRIPTOR(MarketDefaultTestInterface)
};

class MarketDefaultTest : public UtilityObj,
						  public MarketDefaultTestInterface {
public:
	enum {
		kTestAll,
		kTestFiles,
		kTestLowLevel,
		kTestHighLevel,
		kTestRanges,
		kTestCfgDefaults,
		kGetOutputLevel,
		kSetOutputLevel,
		kGetOutputStream,
		kSetOutputStream,
		kOutputLevel
	};

	BEGIN_FUNCTION_MAP
		FN_1(kTestAll, TYPE_INT, testAll, TYPE_VALUE)
		FN_1(kTestFiles, TYPE_INT, testFiles, TYPE_VALUE)
		FN_1(kTestLowLevel, TYPE_INT, testLowLevel, TYPE_VALUE)
		FN_1(kTestHighLevel, TYPE_INT, testHighLevel, TYPE_VALUE)
		FN_1(kTestRanges, TYPE_INT, testRanges, TYPE_VALUE)
		FN_1(kTestCfgDefaults, TYPE_INT, testCfgDefaults, TYPE_VALUE)
		FN_0(kGetOutputLevel, TYPE_ENUM, getOutputLevel)
		VFN_1(kSetOutputLevel, setOutputLevel, TYPE_ENUM)
	END_FUNCTION_MAP

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);

	void Init(HWND hWnd);
	void Destroy(HWND hWnd);
	void Test();
	

	void DeleteThis() { }		
	//Constructor/Destructor

	MarketDefaultTest();		
	~MarketDefaultTest();		

private:
	class ErrorStatus;
	class ErrorStatusWindow;
	class ErrorStatusStream;
	class Tester;
	class PublishedFunctions;

	int test(Value* stream, int (Tester::*what)(), int level);

	int testAll(Value* stream);
	int testFiles(Value* stream);
	int testLowLevel(Value* stream);
	int testHighLevel(Value* stream);
	int testRanges(Value* stream);
	int testCfgDefaults(Value* stream);
	int getOutputLevel();
	void setOutputLevel(int level);

	static BOOL CALLBACK MarketDefaultTestDlgProc(
			HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK StatusDialogProc(
			HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND			hPanel;
	HWND			mhStatus;
	IUtil			*iu;
	Interface		*ip;
	int				mErrorLevel;
};

extern MarketDefaultTest theMarketDefaultTest;

// This structure describes a key in an ini file
struct KeyDesc {
	const TCHAR*		mpName;
	char				mType;
	mutable bool		mMissing;
	mutable short		mValueCount;
	mutable void*		mpValue;
};

// This structure describes a section
struct SectionDesc {
	const void*			mpName;
	const KeyDesc*		mpKeys;
	short				mKeyCount;
	bool				mClassID;
	mutable bool		mMissing;
};

// This structure describes a file
struct FileDesc {
	const SectionDesc*	mpSections;
	short				mSectionCount;
};



class MarketDefaultTest::ErrorStatus {
public:
	ErrorStatus(int level) : mLevel(level) {}

	void message(int level, const TCHAR* fmt, ...);
	void message(int level, const TCHAR* fmt, va_list ap);
	void puts(int level, const TCHAR* msg);

protected:
	virtual void puts(const TCHAR* msg) = 0;

private:
	int			mLevel;
};

class MarketDefaultTest::ErrorStatusWindow : public MarketDefaultTest::ErrorStatus {
public:
	ErrorStatusWindow(HWND wnd, int level) : ErrorStatus(level), mhwnd(wnd) {}
	using ErrorStatus::puts;

protected:
	virtual void puts(const TCHAR* msg);

private:
	HWND		mhwnd;
};

class CharStream;

class MarketDefaultTest::ErrorStatusStream : public MarketDefaultTest::ErrorStatus {
public:
	ErrorStatusStream(CharStream* stream, int level) : ErrorStatus(level), mpStream(stream) {}
	using ErrorStatus::puts;

protected:
	virtual void puts(const TCHAR* msg);

private:
	CharStream*		mpStream;
};

enum {
	kInt, kFloat, kIntArray, kFloatArray, kString
};

class MarketDefaults;

class MarketDefaultTest::Tester {
public:
	Tester(ErrorStatus& status);
	~Tester();

	// Run all tests
	int testAll();

	// Test the file access to the directory
	int testFiles();

	// Test low level ini file access
	int testLowLevel();

	// Test the high level ini file access
	int testHighLevel();

	// Test range checking
	int testRanges();

	// Test p_configurable default
	int testCfgDefaults();

	ErrorStatus& getStatus() const { return mStatus; }

protected:
	bool createFile(const TCHAR* path, const FileDesc* file);

	// Get the path to a defaults file
	TSTR getDefaultFilePath(const TCHAR* filename);

	// Get the path to a factory defaults file
	TSTR getFactoryDefaultFilePath(const TCHAR* filename);

	int testFileCopy(const TCHAR* file);
	int testFileMissing(const TCHAR* file);
	int testFilePresent(const TCHAR* file);

	int verifyValues(MarketDefaults* dflts, const FileDesc* file);
	int verifyValues(MarketDefaults* dflts, const SectionDesc* section);
	int verifyValues(MarketDefaults* dflts, const KeyDesc* key);

	template<class T>
	int checkArray(MarketDefaults* dflts, const KeyDesc* key,
		T* val, int sizeAdjust,
		bool (MarketDefaults::*getFunc)(const TCHAR* key, int& ct, T* val, int size),
		const char* valType, const char* fmt);

	int putFile(MarketDefaults* dflts, const FileDesc* file);
	int putSectionValues(MarketDefaults* dflts, const SectionDesc* section);
	int putKeyValues(MarketDefaults* dflts, const KeyDesc* key);

	void removeValues(MarketDefaults* dflts, const FileDesc* file);
	void removeSectionValues(MarketDefaults* dflts, const SectionDesc* section);
	void removeKeyValues(MarketDefaults* dflts, const KeyDesc* key);

	int createHighLevelValues(MarketDefaults* dflts);
	int verifyHighLevelValues(MarketDefaults* dflts);
	int verifyRanges(MarketDefaults* dflts);
	int verifyMtl(Mtl* m, Class_ID id, const TCHAR* name);
	int highLevelPutError(bool ok, const TCHAR* type);
	template<class T>
	int highLevelCheck(T got, T expected, const TCHAR* type);
	void format(int val);
	void format(float val);
	void format(const TSTR& val);
	void format(const Point3& val);
	void format(const Point4& val);
	void format(Class_ID val);

private:
	ErrorStatus&		mStatus;
	TSTR				mDefaultsDir;
};


#endif // __MARKETDEFAULTTEST__H
