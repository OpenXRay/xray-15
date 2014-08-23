/****************************************************************************
 MAX File Finder
 Christer Janson
 September 20, 1998
 ShowPropDlg.cpp - Property dialog implementation
 ***************************************************************************/
#include "pch.h"

#include "app.h"
#include "resource.h"

INT_PTR CALLBACK	PropDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void VariantToString(App* pApp, PROPVARIANT* pProp, char* szString, int bufSize);
void TypeNameFromVariant(App* pApp, PROPVARIANT* pProp, char* szString, int bufSize);


HINSTANCE		hInstance;

void App::ShowProperties()
	{
	hInstance = GetInstance();

	if (!hPropDialog) {
		hPropDialog = CreateDialogParam(
			GetInstance(),
			MAKEINTRESOURCE(IDD_SHOW_PROPERTIES),
			GetHWnd(),
			(DLGPROC)PropDlgProc,
			(LPARAM)this);
		}
	else {
		SetActiveWindow(hPropDialog);
		GetProperties();
		}
	}

INT_PTR CALLBACK PropDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
	App* pApp = (App*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
						// WIN64 Cleanup: Martell

	switch (message) {
		case WM_INITDIALOG:
			{
			pApp = (App*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			// WIN64 Cleanup: Martell
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			//CenterWindow(hWnd, GetParent(hWnd));
			SetWindowPos(hWnd, HWND_TOP, pApp->regsettings.propdlgx, pApp->regsettings.propdlgy, 0, 0, 
				SWP_NOOWNERZORDER|SWP_NOSIZE);

			pApp->hPropDialog = hWnd;

			// Arrange the columns in the list view
			LV_COLUMN column;
			column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
			column.fmt = LVCFMT_LEFT;
			column.pszText = pApp->GetString(IDS_CUST_NAME);
			column.cx = 80;
			ListView_InsertColumn(GetDlgItem(hWnd, IDC_CUSTOM), 0, &column);
			column.pszText = pApp->GetString(IDS_CUST_VALUE);
			ListView_InsertColumn(GetDlgItem(hWnd, IDC_CUSTOM), 1, &column);
			column.pszText = pApp->GetString(IDS_CUST_TYPE);
			ListView_InsertColumn(GetDlgItem(hWnd, IDC_CUSTOM), 2, &column);

			pApp->GetProperties();
			}
			return 1;
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
			{
			RECT wr;
			GetWindowRect(hWnd, &wr);
			pApp->regsettings.propdlgx = wr.left;
			pApp->regsettings.propdlgy = wr.top;
			}
            pApp->hPropDialog = NULL;
            break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDC_PREV:
					{
					int cur = SendMessage(pApp->hListBox, LB_GETCURSEL, 0, 0);
					//int tot = SendMessage(pApp->hListBox, LB_GETCOUNT, 0, 0);
					if (cur > 0) {
						SendMessage(pApp->hListBox, LB_SETCURSEL, cur-1, 0);
						pApp->GetProperties();
						}
					}
					break;
				case IDC_NEXT:
					{
					int cur = SendMessage(pApp->hListBox, LB_GETCURSEL, 0, 0);
					int tot = SendMessage(pApp->hListBox, LB_GETCOUNT, 0, 0);
					if (cur < (tot-1)) {
						SendMessage(pApp->hListBox, LB_SETCURSEL, cur+1, 0);
						pApp->GetProperties();
						}
					}
					break;

				}
			return 1;
		}
	return 0;
	}

void App::GetProperties()
	{
	LPSTORAGE				pStorage = NULL;
	IPropertySetStorage*	pPropertySetStorage = NULL;
	IPropertyStorage*		pSummaryInfoStorage = NULL;
	IPropertyStorage*		pDocumentSummaryInfoStorage = NULL;
	IPropertyStorage*		pUserDefinedPropertyStorage = NULL;
	wchar_t					wfilename[_MAX_PATH];
	char					szBuf[256];
	char					filename[MAX_PATH];

	SendMessage(GetDlgItem(hPropDialog, IDC_TITLE), WM_SETTEXT, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hPropDialog, IDC_SUBJECT), WM_SETTEXT, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hPropDialog, IDC_AUTHOR), WM_SETTEXT, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hPropDialog, IDC_MANAGER), WM_SETTEXT, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hPropDialog, IDC_COMPANY), WM_SETTEXT, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hPropDialog, IDC_CATEGORY), WM_SETTEXT, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hPropDialog, IDC_KEYWORDS), WM_SETTEXT, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hPropDialog, IDC_COMMENTS), WM_SETTEXT, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hPropDialog, IDC_CONTENTS), LB_RESETCONTENT, 0, 0);
	ListView_DeleteAllItems(GetDlgItem(hPropDialog, IDC_CUSTOM));

	int idx = SendMessage(hListBox, LB_GETCURSEL, 0, 0);

	SendMessage(hListBox, LB_GETTEXT, idx, (LPARAM)filename);
	SetWindowText(hPropDialog, filename);

	MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, _MAX_PATH);
	HRESULT	res = StgOpenStorage(wfilename, (LPSTORAGE)0, STGM_DIRECT|STGM_READ|STGM_SHARE_EXCLUSIVE,	NULL,0,&pStorage);
	if (res!=S_OK) {
		return;
		}


	// Get the Storage interface
	if (S_OK != pStorage->QueryInterface(IID_IPropertySetStorage, (void**)&pPropertySetStorage)) {
		pStorage->Release();
		return;
		}

	// Get the SummaryInfo property set interface
	if (S_OK == pPropertySetStorage->Open(FMTID_SummaryInformation, STGM_READ|STGM_SHARE_EXCLUSIVE, &pSummaryInfoStorage)) {
		BOOL bFound = FALSE;

		PROPSPEC	PropSpec[5];
		PROPVARIANT	PropVar[5];

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = PID_TITLE;

		PropSpec[1].ulKind = PRSPEC_PROPID;
		PropSpec[1].propid = PID_SUBJECT;

		PropSpec[2].ulKind = PRSPEC_PROPID;
		PropSpec[2].propid = PID_AUTHOR;

		PropSpec[3].ulKind = PRSPEC_PROPID;
		PropSpec[3].propid = PID_KEYWORDS;

		PropSpec[4].ulKind = PRSPEC_PROPID;
		PropSpec[4].propid = PID_COMMENTS;

		HRESULT hr = pSummaryInfoStorage->ReadMultiple(5, PropSpec, PropVar);
		if (S_OK == hr) {
			if (PropVar[0].vt == VT_LPSTR) {
				SendMessage(GetDlgItem(hPropDialog, IDC_TITLE), WM_SETTEXT, 0, (LPARAM)PropVar[0].pszVal);
				}
			if (PropVar[1].vt == VT_LPSTR) {
				SendMessage(GetDlgItem(hPropDialog, IDC_SUBJECT), WM_SETTEXT, 0, (LPARAM)PropVar[1].pszVal);
				}
			if (PropVar[2].vt == VT_LPSTR) {
				SendMessage(GetDlgItem(hPropDialog, IDC_AUTHOR), WM_SETTEXT, 0, (LPARAM)PropVar[2].pszVal);
				}
			if (PropVar[3].vt == VT_LPSTR) {
				SendMessage(GetDlgItem(hPropDialog, IDC_KEYWORDS), WM_SETTEXT, 0, (LPARAM)PropVar[3].pszVal);
				}
			if (PropVar[4].vt == VT_LPSTR) {
				SendMessage(GetDlgItem(hPropDialog, IDC_COMMENTS), WM_SETTEXT, 0, (LPARAM)PropVar[4].pszVal);
				}
			}

		FreePropVariantArray(5, PropVar);
		pSummaryInfoStorage->Release();
		}


	// Get the DocumentSummaryInfo property set interface
	if (S_OK == pPropertySetStorage->Open(FMTID_DocSummaryInformation, STGM_READ|STGM_SHARE_EXCLUSIVE, &pDocumentSummaryInfoStorage)) {
		BOOL bFound = FALSE;

		PROPSPEC	PropSpec[5];
		PROPVARIANT	PropVar[5];

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = PID_MANAGER;

		PropSpec[1].ulKind = PRSPEC_PROPID;
		PropSpec[1].propid = PID_COMPANY;

		PropSpec[2].ulKind = PRSPEC_PROPID;
		PropSpec[2].propid = PID_CATEGORY;

		PropSpec[3].ulKind = PRSPEC_PROPID;
		PropSpec[3].propid = PID_HEADINGPAIR;

		PropSpec[4].ulKind = PRSPEC_PROPID;
		PropSpec[4].propid = PID_DOCPARTS;

		HRESULT hr = pDocumentSummaryInfoStorage->ReadMultiple(5, PropSpec, PropVar);
		if (S_OK == hr) {
			if (PropVar[0].vt == VT_LPSTR) {
				SendMessage(GetDlgItem(hPropDialog, IDC_MANAGER), WM_SETTEXT, 0, (LPARAM)PropVar[0].pszVal);
				}
			if (PropVar[1].vt == VT_LPSTR) {
				SendMessage(GetDlgItem(hPropDialog, IDC_COMPANY), WM_SETTEXT, 0, (LPARAM)PropVar[1].pszVal);
				}
			if (PropVar[2].vt == VT_LPSTR) {
				SendMessage(GetDlgItem(hPropDialog, IDC_CATEGORY), WM_SETTEXT, 0, (LPARAM)PropVar[2].pszVal);
				}
			if ((PropVar[3].vt == (VT_VARIANT | VT_VECTOR)) && (PropVar[4].vt == (VT_LPSTR | VT_VECTOR))) {
				CAPROPVARIANT*	pHeading = &PropVar[3].capropvar;
				CALPSTR*		pDocPart = &PropVar[4].calpstr;

				// Headings:
				// =========
				// 0  - General
				// 2  - Mesh Totals
				// 4  - Scene Totals
				// 6  - External Dependencies
				// 8  - Objects
				// 10 - Materials
				// 12 - Plug-Ins

				int nDocPart = 0;
				for (UINT i=0; i<pHeading->cElems; i+=2) {
					SendMessage(GetDlgItem(hPropDialog, IDC_CONTENTS), LB_ADDSTRING, 0, (LPARAM)pHeading->pElems[i].pszVal);
					for (int j=0; j<pHeading->pElems[i+1].lVal; j++) {
						sprintf(szBuf, "\t%s", pDocPart->pElems[nDocPart]);
						SendMessage(GetDlgItem(hPropDialog, IDC_CONTENTS), LB_ADDSTRING, 0, (LPARAM)szBuf);
						nDocPart++;
						}
					}

				}

			}

		FreePropVariantArray(5, PropVar);
		pDocumentSummaryInfoStorage->Release();
		}

	if (S_OK == pPropertySetStorage->Open(FMTID_UserDefinedProperties, STGM_READ|STGM_SHARE_EXCLUSIVE, &pUserDefinedPropertyStorage)) {
		int		numUserProps = 0;

		// First we need to count the properties
		IEnumSTATPROPSTG*	pIPropertyEnum;
		if (S_OK == pUserDefinedPropertyStorage->Enum(&pIPropertyEnum)) {
			STATPROPSTG property;
			while (pIPropertyEnum->Next(1, &property, NULL) == S_OK) {
				if (property.lpwstrName) {
					CoTaskMemFree(property.lpwstrName);
					property.lpwstrName = NULL;
					numUserProps++;
					}
				}

			PROPSPEC* pPropSpec = new PROPSPEC[numUserProps];
			PROPVARIANT* pPropVar = new PROPVARIANT[numUserProps];

			ZeroMemory(pPropVar, numUserProps*sizeof(PROPVARIANT));
			ZeroMemory(pPropSpec, numUserProps*sizeof(PROPSPEC));

			pIPropertyEnum->Reset();
			int idx = 0;
			while (pIPropertyEnum->Next(1, &property, NULL) == S_OK) {
				if (property.lpwstrName) {
					pPropSpec[idx].ulKind = PRSPEC_LPWSTR;
					pPropSpec[idx].lpwstr = (LPWSTR)CoTaskMemAlloc(sizeof(wchar_t)*(wcslen(property.lpwstrName)+1));
					wcscpy(pPropSpec[idx].lpwstr, property.lpwstrName);
					idx++;
					CoTaskMemFree(property.lpwstrName);
					property.lpwstrName = NULL;
					}
				}
			pIPropertyEnum->Release();

			ListView_DeleteAllItems(GetDlgItem(hPropDialog, IDC_CUSTOM));
			HRESULT hr = pUserDefinedPropertyStorage->ReadMultiple(idx, pPropSpec, pPropVar);
			if (S_OK == hr) {
				for (int i=0; i<idx; i++) {
					wcstombs(szBuf, pPropSpec[i].lpwstr, 255);
					LV_ITEM item;
					item.mask = LVIF_TEXT;
					item.iItem = i;
					item.iSubItem = 0;
					item.pszText = szBuf;
					item.cchTextMax = strlen(szBuf);
					ListView_InsertItem(GetDlgItem(hPropDialog, IDC_CUSTOM), &item);

					VariantToString(this, &pPropVar[i], szBuf, 255);
					item.iSubItem = 1;
					item.pszText = szBuf;
					item.cchTextMax = strlen(szBuf);
					ListView_SetItem(GetDlgItem(hPropDialog, IDC_CUSTOM), &item);

					TypeNameFromVariant(this, &pPropVar[i], szBuf, 255);
					item.iSubItem = 2;
					item.pszText = szBuf;
					item.cchTextMax = strlen(szBuf);
					ListView_SetItem(GetDlgItem(hPropDialog, IDC_CUSTOM), &item);
					}
				}

			for (int i=0; i<idx; i++) {
				CoTaskMemFree(pPropSpec[i].lpwstr);
				}

			FreePropVariantArray(numUserProps, pPropVar);

			delete [] pPropSpec;
			delete [] pPropVar;
			}

		pUserDefinedPropertyStorage->Release();
		}

	pPropertySetStorage->Release();
	pStorage->Release();
	}

void TypeNameFromVariant(App* pApp, PROPVARIANT* pProp, char* szString, int bufSize)
	{
	switch (pProp->vt) {
		case VT_LPWSTR:
		case VT_LPSTR:
			strcpy(szString, pApp->GetString(IDS_TYPE_TEXT));
			break;
		case VT_I4:
		case VT_R4:
		case VT_R8:
			strcpy(szString, pApp->GetString(IDS_TYPE_NUMBER));
			break;
		case VT_BOOL:
			strcpy(szString, pApp->GetString(IDS_TYPE_BOOL));
			break;
		case VT_FILETIME:
			strcpy(szString, pApp->GetString(IDS_TYPE_DATE));
			break;
		default:
			strcpy(szString, "");
			break;
		}
	}

void VariantToString(App* pApp, PROPVARIANT* pProp, char* szString, int bufSize)
	{
	switch (pProp->vt) {
		case VT_LPWSTR:
			wcstombs(szString, pProp->pwszVal, bufSize);
			break;
		case VT_LPSTR:
			strcpy(szString, pProp->pszVal);
			break;
		case VT_I4:
			sprintf(szString, "%ld", pProp->lVal);
			break;
		case VT_R4:
			sprintf(szString, "%f", pProp->fltVal);
			break;
		case VT_R8:
			sprintf(szString, "%lf", pProp->dblVal);
			break;
		case VT_BOOL:
			sprintf(szString, "%s", pProp->boolVal ? pApp->GetString(IDS_VAL_YES) : pApp->GetString(IDS_VAL_NO));
			break;
		case VT_FILETIME:
			SYSTEMTIME sysTime;
			FileTimeToSystemTime(&pProp->filetime, &sysTime);
			GetDateFormat(LOCALE_SYSTEM_DEFAULT,
						  DATE_SHORTDATE,
						  &sysTime,
						  NULL,
						  szString,
						  bufSize);
			break;
		default:
			strcpy(szString, "");
			break;
		}
	}
