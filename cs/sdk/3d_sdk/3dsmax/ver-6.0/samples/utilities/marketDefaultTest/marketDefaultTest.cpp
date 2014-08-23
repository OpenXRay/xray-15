/**********************************************************************
 *<
	FILE: marketDefaultTest.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "marketDefaultTest.h"
#include "maxscrpt\maxscrpt.h"
#include <memory>

MarketDefaultTest theMarketDefaultTest;

class MarketDefaultTestClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &theMarketDefaultTest; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return MARKETDEFAULTTEST_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("MarketDefaultTest"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static MarketDefaultTestClassDesc MarketDefaultTestDesc;
ClassDesc2* GetMarketDefaultTestDesc() { return &MarketDefaultTestDesc; }


static const Interface_ID kTestInterfaceID(0x4cd63706, 0x4b531840);

//--- MarketDefaultTest -------------------------------------------------------
MarketDefaultTest::MarketDefaultTest()
	: MarketDefaultTestInterface(kTestInterfaceID, _T("marketDefaultsTestInterface"), 0, NULL, FP_CORE,
		kTestAll, _T("testAll"), 0, TYPE_INT, 0, 1,
			_T("outputStream"), 0, TYPE_VALUE,
				f_keyArgDefault, NULL,

		kTestFiles, _T("testFiles"), 0, TYPE_INT, 0, 1,
			_T("outputStream"), 0, TYPE_VALUE,
				f_keyArgDefault, NULL,

		kTestLowLevel, _T("testLowLevel"), 0, TYPE_INT, 0, 1,
			_T("outputStream"), 0, TYPE_VALUE,
				f_keyArgDefault, NULL,

		kTestHighLevel, _T("testHighLevel"), 0, TYPE_INT, 0, 1,
			_T("outputStream"), 0, TYPE_VALUE,
				f_keyArgDefault, NULL,

		kTestRanges, _T("testRanges"), 0, TYPE_INT, 0, 1,
			_T("outputStream"), 0, TYPE_VALUE,
				f_keyArgDefault, NULL,

		kTestCfgDefaults, _T("testConfigurableDefaults"), 0, TYPE_INT, 0, 1,
			_T("outputStream"), 0, TYPE_VALUE,
				f_keyArgDefault, NULL,

		properties,
			kGetOutputLevel, kSetOutputLevel, _T("outputLevel"), 0, TYPE_ENUM, kOutputLevel,

			enums,
			kOutputLevel, 4,
				_T("errors"), kErrorMessage,
				_T("warnings"), kWarningMessage,
				_T("information"), kInfoMessage,
				_T("debug"), kDebugMessage,
		end)
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
	mhStatus = NULL;
	mErrorLevel = kInfoMessage;
}

MarketDefaultTest::~MarketDefaultTest()
{

}

void MarketDefaultTest::BeginEditParams(Interface *ip,IUtil *iu)
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		MarketDefaultTestDlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void MarketDefaultTest::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void MarketDefaultTest::Init(HWND hWnd)
{
	CheckRadioButton(hWnd, IDC_SHOWALL, IDC_SHOWERRORS, IDC_SHOWALL);
}

void MarketDefaultTest::Destroy(HWND hWnd)
{
	if (mhStatus != NULL)
		DestroyWindow(mhStatus);
}

// This is the user mode test. It shows a dialog and puts the
// errors in to an edit control in the dialog.
void MarketDefaultTest::Test()
{
	if (mhStatus == NULL) {
		mhStatus = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_STATUS),
			GetCOREInterface()->GetMAXHWnd(), StatusDialogProc);
		if (mhStatus == NULL)
			return;
	}

	HWND edit = GetDlgItem(mhStatus, IDC_ERRORS);
	if (edit == NULL)
		return;

	int level = kInfoMessage;
	if (IsDlgButtonChecked(hPanel, IDC_SHOWERRORS))
		level = kErrorMessage;
	else if (IsDlgButtonChecked(hPanel, IDC_SHOWWARNINGS))
		level = kWarningMessage;

	test(NULL, &Tester::testAll, level);
}

int MarketDefaultTest::testAll(Value* stream)
{
	return test(stream, &Tester::testAll, mErrorLevel);
}

int MarketDefaultTest::testFiles(Value* stream)
{
	return test(stream, &Tester::testFiles, mErrorLevel);
}

int MarketDefaultTest::testLowLevel(Value* stream)
{
	return test(stream, &Tester::testLowLevel, mErrorLevel);
}

int MarketDefaultTest::testHighLevel(Value* stream)
{
	return test(stream, &Tester::testHighLevel, mErrorLevel);
}

int MarketDefaultTest::testRanges(Value* stream)
{
	return test(stream, &Tester::testRanges, mErrorLevel);
}

int MarketDefaultTest::testCfgDefaults(Value* stream)
{
	return test(stream, &Tester::testCfgDefaults, mErrorLevel);
}

int MarketDefaultTest::getOutputLevel()
{
	return mErrorLevel;
}

void MarketDefaultTest::setOutputLevel(int level)
{
	mErrorLevel = level;
}

int MarketDefaultTest::test(Value* stream, int (Tester::*what)(), int level)
{
	std::auto_ptr<ErrorStatus> status(NULL);
	int errors = 0;

	if (stream == NULL || !stream->_is_charstream()) {
		if (mhStatus == NULL) {
			mhStatus = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_STATUS),
				GetCOREInterface()->GetMAXHWnd(), StatusDialogProc);
			if (mhStatus == NULL)
				throw RuntimeError(_T("Cannot create error status dialog\r\n"));
		}

		HWND edit = GetDlgItem(mhStatus, IDC_ERRORS);
		if (edit == NULL) {
			DestroyWindow(mhStatus);
			throw RuntimeError(_T("Cannot create error status dialog\r\n"));
		}

		SendMessage(edit, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(""));
		UpdateWindow(edit);

		status = std::auto_ptr<ErrorStatus>(
			new ErrorStatusWindow(edit, level));
	}
	else {
		status = std::auto_ptr<ErrorStatus>(
			new ErrorStatusStream(static_cast<CharStream*>(stream), level));
	}

	theHold.Suspend();

	try {
		Tester tester(*status);
		try {
			errors = (tester.*what)();
		}
		catch (...) {
			status->message(kErrorMessage,
				_T("Unknown exception thrown.\r\n")
				_T("    Test terminated\r\n"));
			++errors;
		}
	}
	catch (...) {
		status->message(kErrorMessage,
			_T("Unknown exception thrown during initialization or cleanup.\r\n")
			_T("    Test terminated\r\n"));
		++errors;
	}

	theHold.Resume();
	return errors;
}

BOOL CALLBACK MarketDefaultTest::MarketDefaultTestDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theMarketDefaultTest.Init(hWnd);
			break;

		case WM_DESTROY:
			theMarketDefaultTest.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_TEST:
				theMarketDefaultTest.Test();
				break;
			}
			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theMarketDefaultTest.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


BOOL CALLBACK MarketDefaultTest::StatusDialogProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_GETMINMAXINFO:
			{
				LPMINMAXINFO p = reinterpret_cast<LPMINMAXINFO>(lParam);
				RECT dlg, msg;
				GetWindowRect(GetDlgItem(hWnd, IDC_ERRORS), &msg);
				GetWindowRect(hWnd, &dlg);

				int temp = dlg.right - msg.right					// Space on right
					+ msg.left - dlg.left;							// Space on left
				if (temp > p->ptMinTrackSize.x)
					p->ptMinTrackSize.x = temp;

				temp = dlg.bottom - msg.bottom						// Space on bottom
					+ msg.top - dlg.top;							// Space on top
				if (temp > p->ptMinTrackSize.y)
					p->ptMinTrackSize.y = temp;
			}
			return 1;
		
		case WM_WINDOWPOSCHANGING: {
			WINDOWPOS* pos = reinterpret_cast<WINDOWPOS*>(lParam);

			// Resize the dialog. If the size isn't changing return.
			if (pos->flags & SWP_NOSIZE)
				return true;

			RECT wdw;

			// Calculate the change in width and height
			GetWindowRect(hWnd, &wdw);
			int dx = pos->cx - (wdw.right - wdw.left);
			int dy = pos->cy - (wdw.bottom - wdw.top);

			HWND hEdit = GetDlgItem(hWnd, IDC_ERRORS);

			if (dx != 0 || dy != 0) {
				// Something changed. Let's calculate the size of the edit control.
				RECT client;
				GetWindowRect(hEdit, &client);
				SetWindowPos(hEdit, NULL, 0, 0,
					client.right - client.left + dx, 
					client.bottom - client.top + dy,
					SWP_NOMOVE | SWP_NOZORDER);
			}

			return true;
		} break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_INITDIALOG:
			break;

		case WM_DESTROY:
			theMarketDefaultTest.mhStatus = NULL;
			break;

		case WM_COMMAND:
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

//--- MarketDefaultTest::ErrorStatus -------------------------------------------------------
void MarketDefaultTest::ErrorStatus::message(int level, const char* fmt, ...)
{
	if (level <= mLevel) {
		va_list ap;
		va_start(ap, fmt);
		message(level, fmt, ap);
		va_end(ap);
	}
}

void MarketDefaultTest::ErrorStatus::message(int level, const TCHAR* fmt, va_list ap)
{
	if (level <= mLevel) {
		const int kBufSize = 8192;
		TCHAR buf[kBufSize + 1];
		_vsntprintf(buf, kBufSize, fmt, ap);
		buf[kBufSize] = '\0';
		puts(level, buf);
	}
}

void MarketDefaultTest::ErrorStatus::puts(int level, const TCHAR* msg)
{
	if (level <= mLevel)
		puts(msg);
}

//--- MarketDefaultTest::ErrorStatusWindow -------------------------------------------------------
void MarketDefaultTest::ErrorStatusWindow::puts(const TCHAR* msg)
{
	long len = SendMessage(mhwnd, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		len = 0;
	SendMessage(mhwnd, EM_SETSEL, len, len);
	SendMessage(mhwnd, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(msg));
	len += _tcslen(msg);
	SendMessage(mhwnd, EM_SETSEL, len, len);
	SendMessage(mhwnd, EM_SCROLLCARET, 0, 0);
	UpdateWindow(mhwnd);
}

//--- MarketDefaultTest::ErrorStatusStream -------------------------------------------------------
void MarketDefaultTest::ErrorStatusStream::puts(const TCHAR* msg)
{
	// Poor const usage
	mpStream->puts(const_cast<TCHAR*>(msg));
}
