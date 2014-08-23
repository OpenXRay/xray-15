#include <max.h>
#include "citylist.h"
#include <new>

CityList cityList;

CityList::CityList()
	: mpCityList(NULL), mpCityNames(NULL), mNumCities(0), mCityNameSize(0)
{
}

CityList::~CityList()
{
	delete[] mpCityList;
	delete[] mpCityNames;
}

void CityList::moreNames(UINT size)
{
	if (size > mCityNameSize) {
		TCHAR* p = static_cast<TCHAR*>(realloc(mpCityNames, size + 1024));
		if (p == NULL)
			throw std::bad_alloc();
		mpCityNames = p;
	}
}

int CityList::byname(const void* p1, const void* p2)
{
	const TCHAR* e1 = static_cast<const Entry*>(p1)->name;
	const TCHAR* e2 = static_cast<const Entry*>(p2)->name;
	if (e1[0] == '+')
		++e1;
	if (e2[0] == '+')
		++e2;
	return _tcscmp(e1, e2);
}

bool CityList::parseLine(FILE* fp, Entry& entry, UINT& namePos)
{
	TCHAR buf[512];

	if (fgets(buf, sizeof(buf) / sizeof(buf[0]) - 1, fp) != NULL) {
		TCHAR* p;
		TCHAR* first = NULL;
		TCHAR* last = NULL;

		for (p = buf; p[0] != '\0' && p[0] != ';'; p += _tclen(p)) {
			if (p[0] == ' ' || p[0] == '\t' || p[0] == '\r' || p[0] == '\n') {
				// Found white space.
				if (last == NULL)
					last = p;
			}
			else {
				if (first == NULL)
					first = p;
				last = NULL;
			}
		}

		// Strip trailing spaces if any, and any comments.
		if (last != NULL)
			last[0] = '\0';
		else
			p[0] = '\0';

		if (first != NULL) {
			int latitude, longitude, name;
			if (_stscanf(first, "%d %d %n", &latitude, &longitude, &name) == 2
					&& first[name] != '\0') {
				entry.latitude = float(latitude) / 10000.0f;
				entry.longitude = float(longitude) / 10000.0f;
				entry.nameOff = namePos;
				UINT nameEnd = namePos + _tcslen(first + name) + 1;
				moreNames(nameEnd);
				_tcscpy(mpCityNames + namePos, first + name);
				namePos = nameEnd;
				return true;
			}
		}
	}
	return false;
}

void CityList::initializeList()
{
	mNumCities = 0;
	delete[] mpCityList;
	mpCityList = NULL;
	delete[] mpCityNames;
	mpCityNames = NULL;
	mCityNameSize = 0;

	Interface* ip = GetCOREInterface();
	TSTR cityFile = ip->GetDir(APP_PLUGCFG_DIR);
	cityFile += "\\sitename.txt";

	// Open the city file.
	FILE* fp = fopen(cityFile.data(), "r");
	if (fp == NULL)
		return;			// No file, return with no cities

	// First count the cities in the file.
	UINT count = 0;
	UINT nameSize = 0;
	{
		Entry temp;
		while (!feof(fp)) {
			UINT namePos = 0;
			if (parseLine(fp, temp, namePos)) {
				++count;
				nameSize += namePos;
			}
		}
	}

	if (count <= 0)
		return;		// No Cities

	mpCityList = new Entry[count];
	mCityNameSize = nameSize;
	mpCityNames = static_cast<TCHAR*>(realloc(mpCityNames, mCityNameSize * sizeof(TCHAR)));
	UINT namePos = 0;

	fseek(fp, 0L, SEEK_SET);
	for (UINT i = 0; i < count && !feof(fp); ) {
		i += parseLine(fp, mpCityList[i], namePos);
	}
	fclose(fp);

	count = i;
	for (i = 0; i < count; ++i)
		mpCityList[i].name = mpCityNames + mpCityList[i].nameOff;

	if (count > 0) {
		qsort(mpCityList, count, sizeof(mpCityList[0]), byname);
		mNumCities = count;
	}
}
