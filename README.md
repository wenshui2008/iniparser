# iniparser
iniparser is a simple, small, efficient, C INI configuration file parser that can be easily integrated into other programs.

# Examples

#### Load and parse an INI file.

        ini = IniFile_Open("test.ini",NULL);
        
        IniFile_Close(ini);
  
#### Or load an ini file from memory
  
        IniFile_LoadFromMemory(pszIniContent,-1);
  
#### Enumerates the ini file and print ini file

        nSectionCount = IniFile_GetSectionCount(ini);
	printf("Section Count:%d\r\n", nSectionCount);

	for (i = 0; i < nSectionCount; i++) {
		int iItem,nItemCount;
		pSection = IniFile_GetSectionByIndex(ini,i);
		nItemCount = IniSection_GetItemCount(pSection);
		printf("Section Name:%s,itemCount:%d\r\n", IniSection_GetName(pSection),nItemCount);

		for (iItem = 0; iItem < nItemCount; iItem ++) {
			pItem = IniSection_GetItem(pSection,iItem);
			printf("\t%s = %s\r\n",pItem->pKeyName,pItem->pValue);
		}

		printf("\r\n");
	}

#### Retrieve the key/value from ini file
first fetch the section
        
        pSection = IniFile_GetSection(ini,"Video.Device");
        
Then get the value from section by key name

        pValue = IniSection_GetValue(pSection,"sDeviceName",NULL);
        
# API -Very simple

        INIFILE * IniFile_Open(LPCSTR pszFile,LPCSTR pszSection);
        INIFILE * IniFile_LoadFromMemory(LPCSTR pszIniContent,int nSize);
        void IniFile_Close(INIFILE * pFile);
        UINT IniFile_GetSectionCount(const INIFILE * pIniFile);
        INISECTION * IniFile_GetSection(const INIFILE * pIniFile,LPCSTR pszSection);
        INISECTION * IniFile_GetSectionByIndex(const INIFILE * pIniFile,int iIndex);
        LPCSTR IniFile_GetValue(const INIFILE * pIniFile,LPCSTR pszSection,LPCSTR pszKeyName,LPCSTR pszDefault);

        UINT IniSection_GetItemCount(INISECTION * pSection);
        LPCSTR IniSection_GetName(INISECTION * pSection);
        LPCSTR IniSection_GetValue(INISECTION * pSection,LPCSTR pszKeyName,LPCSTR pszDefault);
        const INI_ITEM * IniSection_GetItem(INISECTION * pSection,int iItem);
        INT  IniSection_GetInt(INISECTION * pSection,LPCSTR pszKeyName,int nDefault);
        
# How to use
--------------------
There are 2 files in iniparser:

* iniparser.c
* iniparser.h

Only copy  the above two files to your preject directory,add to project.

And additionally a test file:

* main.c
Simply compile and run. build.sh is a linux build script.


