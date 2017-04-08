#ifndef _INIPARSER_H_INC_
#define _INIPARSER_H_INC_

typedef struct CConfigFileA INIFILE;
typedef const struct CProfileSection_s INISECTION;
typedef int INT;
typedef unsigned int UINT;
typedef const char * LPCSTR;

typedef struct INI_ITEM {
	LPCSTR pKeyName;
	int nKeySize;
	LPCSTR pValue;
	int nValueSize;
}INI_ITEM;

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

#endif //_INIPARSER_H_INC_

