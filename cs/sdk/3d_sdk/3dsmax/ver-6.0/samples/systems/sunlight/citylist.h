
class CityList {
public:
	struct Entry {
		float latitude;
		float longitude;
		union {
			char* name;
			UINT  nameOff;
		};
	};

	CityList();
	~CityList();

	UINT Count() { return mNumCities; }
	Entry& operator[](UINT i) { return mpCityList[i]; }
	Entry* operator+(UINT i) { return mpCityList + i; }

	void init() { if (mpCityList == NULL) initializeList(); }

private:
	void initializeList();
	void moreNames(UINT size);
	static int byname(const void* p1, const void* p2);
	bool parseLine(FILE* fp, Entry& entry, UINT& namePos);

	Entry*		mpCityList;
	TCHAR*		mpCityNames;
	UINT		mNumCities;
	UINT		mCityNameSize;
};

extern CityList cityList;
