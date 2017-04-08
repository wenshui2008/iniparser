#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <fcntl.h>
#include "iniparser.h"

#ifdef _MSC_VER
#include <io.h>
#pragma warning(disable: 4996)
#endif

#ifndef O_BINARY
#define O_BINARY    0x8000 
#endif

#define MAX_INIFILE_SIZE		1024 * 1024 * 4
#define ALLOCSECTION_STEP_SIZE	32
#define ALLOCKEY_STEP_SIZE		32

typedef unsigned int	UINT;
typedef unsigned long   DWORD;
typedef int             BOOL;
typedef DWORD ULONG;
typedef char CHAR;
typedef unsigned char BYTE;
typedef char * LPSTR;
//typedef const LPSTR LPCSTR;
typedef BYTE * PBYTE;
typedef unsigned short USHORT;
typedef char * PCHAR;
typedef int INT;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifdef _MSC_VER
#define inline __inline
#define strnicmp _strnicmp
#else
#define strnicmp strncasecmp
#endif

#define IsDBCSLeadByte(c) (0)

typedef struct PROFILEKEY_s
{
	LPSTR	pszNameA;
	int  	nNameLength;
	LPSTR	pszValueA;
	int 	nValueLength;
	BYTE *	pszLineData;
	size_t	nLineLength;
}PROFILEKEY;

typedef struct CConfigFileA CConfigFileA;

typedef struct CProfileSection_s
{
	PROFILEKEY	*  pkeys;
	size_t		   nItemCount;
	size_t		   nItemCapacity;
	BYTE*		   pszLineData;
	size_t		   nLineLength;
	LPSTR		   pszNameA;
	size_t		   nNameLength;
	CConfigFileA * pConfigFile;
} CProfileSection;

typedef struct CConfigFileA 
{
	CProfileSection	*m_pSections;
	size_t			 m_nCountOfSection;
	size_t			 m_nSectionCapacity;

	BYTE*			 m_pFileDataMap;
	DWORD			 m_dwFileDataLength;
} CConfigFileA;

inline PROFILEKEY	* AppendKey(CProfileSection * pSection) {
	PROFILEKEY	*pNewkey;

	if (pSection->nItemCapacity == pSection->nItemCount) {
		size_t NewSize = pSection->nItemCapacity + ALLOCKEY_STEP_SIZE; 
		pNewkey = (PROFILEKEY	*) realloc(pSection->pkeys,NewSize * sizeof (PROFILEKEY));
		if (!pNewkey) return NULL;
		pSection->nItemCapacity = NewSize;
		pSection->pkeys = pNewkey;
		memset(&pSection->pkeys[pSection->nItemCount],0,ALLOCKEY_STEP_SIZE * sizeof (PROFILEKEY));
	}
	pNewkey = &pSection->pkeys[pSection->nItemCount];
	pSection->nItemCount ++;
	return pNewkey;
}

static
CProfileSection * FindSectionA(CConfigFileA * pIniFile,LPCSTR pszSection,int nNameLength)
{
	size_t i;

	for (i = 0; i < pIniFile->m_nCountOfSection; i++){
		CProfileSection * pSection = &pIniFile->m_pSections[i];
		if (pSection->nNameLength == (size_t)nNameLength && !strnicmp(pszSection,pSection->pszNameA,nNameLength)){
			return pSection;
		}
	}

	return NULL;
}

static
CProfileSection * AllocSectionA(CConfigFileA * pIniFile,LPCSTR pszSection,int nNameLength,BOOL bSearch)
{
	CProfileSection * pNewSection;
	if (bSearch) {
		pNewSection = FindSectionA(pIniFile,pszSection,nNameLength);
		if (pNewSection) return pNewSection;
	}

	if (pIniFile->m_nCountOfSection == pIniFile->m_nSectionCapacity) {
		CProfileSection * pEnd;
		size_t NewSize = pIniFile->m_nSectionCapacity + ALLOCSECTION_STEP_SIZE;
		pNewSection = (CProfileSection *)realloc(pIniFile->m_pSections,NewSize * sizeof (CProfileSection));
		if (!pNewSection) return NULL;
		pIniFile->m_pSections = pNewSection;
		pIniFile->m_nSectionCapacity = NewSize;
		pNewSection = &pIniFile->m_pSections[pIniFile->m_nCountOfSection];
		pEnd = &pIniFile->m_pSections[pIniFile->m_nSectionCapacity];
		memset(pNewSection,0,sizeof (CProfileSection) * ALLOCSECTION_STEP_SIZE);

		for (; pNewSection < pEnd; pNewSection++) {
			pNewSection->pConfigFile = pIniFile;
		}

	}
	pNewSection = &pIniFile->m_pSections[pIniFile->m_nCountOfSection];
	if (!pszSection) {
		pNewSection->pszNameA = "";
	}
	pNewSection->pConfigFile = pIniFile;
	pIniFile->m_nCountOfSection ++;

	return pNewSection;
}

//if pszSection is not NULL, the inifile parse only the section name by pszSection
static
BOOL IniParse(CConfigFileA * pIniFile,LPSTR pszIniData,size_t nNumOfChar,LPCSTR pszSection)
{
	PBYTE Src, BeginOfLine,EndOfLine, EqualSign, EndOfFile;
	PBYTE Name, EndOfName, Value, EndOfValue;
	CProfileSection * pCurSection = NULL;
	int	nOnlyOneSection = 0;
	int SectionLen = -1;
	if (pszSection) SectionLen = (int)strlen(pszSection);
#undef INI_TEXT
#define INI_TEXT(quote) quote

	Src = (PBYTE)pszIniData;
	EndOfFile = Src + nNumOfChar;

	while (Src < EndOfFile) {
		BeginOfLine = Src;
		//
		// Find first non-blank character on a line.  Skip blank lines
		//
		while (Src < EndOfFile && *Src <= INI_TEXT(' ')) {
			Src++;
		}

		if (Src >= EndOfFile) {
#if 0
			if (!pCurSection) pCurSection = AllocSectionA(pIniFile,NULL,0,FALSE);
			if (pCurSection) {
				PROFILEKEY *pKey = AppendKey(pCurSection);
				if (!pKey) break;
				pKey->pszLineData = BeginOfLine;
				pKey->nLineLength = Src - BeginOfLine;
				pKey->pszValueA = pKey->pszNameA = NULL;
			}
#endif
			break;
		}

		EndOfLine = Src;
		EqualSign = NULL;
		while (EndOfLine < EndOfFile) {
			if (EqualSign == NULL && *EndOfLine == INI_TEXT('=')) {
				EqualSign = ++EndOfLine;
			} else	if (*EndOfLine == INI_TEXT('\r') || *EndOfLine == INI_TEXT('\n')) {
				if (*EndOfLine == INI_TEXT('\r')) {
					EndOfLine++;
				}

				if (*EndOfLine == INI_TEXT('\n')) {
					EndOfLine++;
				}

				break;
			} else {
				EndOfLine++;
			}
		}

		if (*Src != INI_TEXT(';')) {
			if (*Src == INI_TEXT('[')) {
				USHORT NameLength;

				Name = Src + 1;
				while (Name < EndOfLine && *Name <= INI_TEXT(' ')) {
					Name++;
				}
				EndOfName = Name;
				while (EndOfName < EndOfLine) {
					if (*EndOfName == INI_TEXT(']')) {
						break;
					}
					if (IsDBCSLeadByte(*EndOfName)) {
						EndOfName++;
					}
					EndOfName++;
				}
				while (EndOfName > Name && EndOfName[ -1 ] <= INI_TEXT(' ')) {
					EndOfName--;
				}

				*EndOfName = '\0';
				NameLength = (USHORT)((PCHAR)EndOfName - (PCHAR)Name);
				pCurSection = AllocSectionA(pIniFile,(LPCSTR)Name,NameLength,TRUE);
				if (!pCurSection) break;
				pCurSection->pszLineData = BeginOfLine;
				pCurSection->nLineLength = EndOfLine - BeginOfLine;
				pCurSection->pszNameA = (LPSTR)Name;
				pCurSection->nNameLength = NameLength;


				if (nOnlyOneSection >= 1) {
					PROFILEKEY *pKey = AppendKey(pCurSection);
					if (!pKey) break;
					pKey->pszLineData = EndOfLine;
					pKey->nLineLength = EndOfFile - EndOfLine;
					pKey->pszValueA = pKey->pszNameA = NULL;
					break;
				}
				else if (SectionLen == NameLength && !memcmp((LPCSTR)Name,pszSection,NameLength)){
					nOnlyOneSection ++;
				}
			} else {
				PROFILEKEY *pKey;
				if (!pCurSection) pCurSection = AllocSectionA(pIniFile,NULL,0,FALSE);
				if (!pCurSection) break;
				pKey = AppendKey(pCurSection);
				if (!pKey) break;
				pKey->pszLineData = BeginOfLine;
				pKey->nLineLength = EndOfLine - BeginOfLine;

				if (EqualSign != NULL) {
					Name = Src;
					EndOfName = EqualSign - 1;
					while (EndOfName > Name && EndOfName[ -1 ] <= INI_TEXT(' ')) {
						EndOfName--;
					}
					*EndOfName = '\0';
					pKey->pszNameA = (LPSTR)Name;
					pKey->nNameLength = (USHORT)((PCHAR)EndOfName - (PCHAR)Name);
					Value = EqualSign;
				} else {
					Value = Src;
				}

				EndOfValue = EndOfLine;
				while (EndOfValue > Value && EndOfValue[ -1 ] <= INI_TEXT(' ')) {
					EndOfValue--;
				}

				while (*Value <= ' ' && Value < EndOfValue) {
					Value ++;
				}

				*EndOfValue = '\0';
				pKey->pszValueA = (LPSTR)Value;
				pKey->nValueLength = (USHORT)((PCHAR)EndOfValue - (PCHAR)Value);

				if ((Value[ 0 ] == '"' || Value[ 0 ] == '\'') &&
					pKey->nValueLength >= 2 &&
					(Value[ 0 ] == Value[ pKey->nValueLength - 1])
					) {
					pKey->pszValueA += 1;
					pKey->nValueLength -= 2;
					pKey->pszValueA[pKey->nValueLength] = '\0';
				}
			}
		}
		else {
			if (!pCurSection) pCurSection = AllocSectionA(pIniFile,NULL,0,FALSE);
			if (pCurSection) {
				PROFILEKEY *pKey = AppendKey(pCurSection);
				if (!pKey) break;
				pKey->pszLineData = BeginOfLine;
				pKey->nLineLength = EndOfLine - BeginOfLine;
				pKey->pszValueA = pKey->pszNameA = NULL;
			}
		}

		Src = EndOfLine;
	}

	return TRUE;
}

typedef struct CConfigFileA INIFILE;
typedef const struct CProfileSection_s INISECTION;

INIFILE * IniFile_Open(LPCSTR pszFile,LPCSTR pszSection)
{
	int fd;
	CConfigFileA * pIniFile;
	struct stat fs;
	int ret;
	BYTE *pByte;

	fd = open(pszFile,O_RDONLY|O_BINARY);
	if (fd == -1) {
		return NULL;
	}

	memset(&fs,0,sizeof(fs));
	
	if (fstat(fd,&fs) < 0 || fs.st_size > MAX_INIFILE_SIZE) {
		close(fd);
		return NULL;
	}

	pIniFile = (CConfigFileA *)malloc(sizeof (CConfigFileA) + fs.st_size + 2);
	if (!pIniFile) {
		close(fd);
		return NULL;
	}

	memset(pIniFile,0, sizeof (CConfigFileA));

	pIniFile->m_pFileDataMap = (BYTE*)(pIniFile + 1);
	pIniFile->m_dwFileDataLength = (DWORD)fs.st_size;

	ret = read(fd,pIniFile->m_pFileDataMap,(int)fs.st_size);
	if (ret != (int)fs.st_size) {
		close(fd);
		free(pIniFile);
		return NULL;
	}

	close(fd);

	pIniFile->m_pFileDataMap[fs.st_size] = '\0';
	pIniFile->m_pFileDataMap[fs.st_size + 1] = '\0';

	pByte = (BYTE*)pIniFile->m_pFileDataMap;
	if ( fs.st_size >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF ) {
		pIniFile->m_pFileDataMap += 3;
		pIniFile->m_dwFileDataLength -= 3;
	}

	if (!IniParse(pIniFile,(LPSTR)pIniFile->m_pFileDataMap,pIniFile->m_dwFileDataLength,pszSection)) {
		free(pIniFile);
		return NULL;
	}

	return pIniFile;
}

INIFILE * IniFile_LoadFromMemory(LPCSTR pszIniContent,int nSize)
{
	CConfigFileA * pIniFile;
	BYTE *pByte;

	if (nSize == -1) {
		nSize = (int)strlen(pszIniContent);
	}

	if (nSize > MAX_INIFILE_SIZE) {
		return NULL;
	}

	pIniFile = (CConfigFileA *)malloc(sizeof (CConfigFileA) + nSize + 2);
	if (!pIniFile) {
		return NULL;
	}

	memset(pIniFile,0, sizeof (CConfigFileA));

	pIniFile->m_pFileDataMap = (BYTE*)(pIniFile + 1);
	pIniFile->m_dwFileDataLength = (DWORD)nSize;

	memcpy(pIniFile->m_pFileDataMap,pszIniContent,nSize);

	pIniFile->m_pFileDataMap[nSize] = '\0';
	pIniFile->m_pFileDataMap[nSize + 1] = '\0';

	pByte = (BYTE*)pIniFile->m_pFileDataMap;
	if ( nSize >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF ) {
		pIniFile->m_pFileDataMap += 3;
		pIniFile->m_dwFileDataLength -= 3;
	}

	if (!IniParse(pIniFile,(LPSTR)pIniFile->m_pFileDataMap,pIniFile->m_dwFileDataLength,NULL)) {
		free(pIniFile);
		return NULL;
	}

	return pIniFile;
}

void IniFile_Close(INIFILE * pFile)
{
	if (pFile) {
		size_t i;

		for (i = 0; i < pFile->m_nCountOfSection; i++) {
			CProfileSection * pSection = &pFile->m_pSections[i];
			if (pSection->pkeys)
				free(pSection->pkeys);
		}
		if (pFile->m_pSections) 
			free(pFile->m_pSections);

		free(pFile);
	}
}


UINT IniFile_GetSectionCount(const INIFILE * pIniFile)
{
	return (UINT)pIniFile->m_nCountOfSection;
}

INISECTION * IniFile_GetSection(const INIFILE * pIniFile,LPCSTR pszSection)
{
	size_t NameLen,i;
	INISECTION * pSection;

	if (pszSection == NULL || *pszSection == '\0') {
		if (pIniFile->m_nCountOfSection > 0 && pIniFile->m_pSections->nNameLength == 0) 
			return pIniFile->m_pSections;
		return NULL;
	}
	NameLen = strlen(pszSection);
	for (i = 0; i < pIniFile->m_nCountOfSection; i++) {
		pSection = &pIniFile->m_pSections[i];
		if (pSection->nNameLength == (size_t)NameLen && !strnicmp(pszSection,pSection->pszNameA,NameLen)){
			return (INISECTION *)pSection;
		}
	}

	return NULL;
}

INISECTION * IniFile_GetSectionByIndex(const INIFILE * pIniFile,int iIndex)
{
	INISECTION * pSection;
	if (iIndex < 0 || iIndex >= (int)pIniFile->m_nCountOfSection) return NULL;
	pSection = &pIniFile->m_pSections[iIndex];
	return (INISECTION *)pSection;
}

LPCSTR IniFile_GetValue(const INIFILE * pIniFile,LPCSTR pszSection,LPCSTR pszKeyName,LPCSTR pszDefault)
{
	INISECTION * pSection = IniFile_GetSection(pIniFile,pszSection);
	if (!pSection)
		return pszDefault;

	return IniSection_GetValue(pSection,pszKeyName,pszDefault);
}

UINT IniSection_GetItemCount(INISECTION * pSection)
{
	return (UINT)pSection->nItemCount;
}

LPCSTR IniSection_GetName(INISECTION * pSection)
{
	return pSection->pszNameA;
}

LPCSTR IniSection_GetValue(INISECTION * pSection,LPCSTR pszKeyName,LPCSTR pszDefault)
{
	size_t i;
	size_t nKeyNameLen;

	if (!pszKeyName) return pSection->pszNameA;

	nKeyNameLen = strlen(pszKeyName);

	for (i = 0; i < pSection->nItemCount; i++) {
		PROFILEKEY *pKey = &pSection->pkeys[i];
		if (pKey->nNameLength == nKeyNameLen &&
			(*pszKeyName == *pKey->pszNameA) &&
			!memcmp(pszKeyName,pKey->pszNameA,nKeyNameLen)){
			return pKey->pszValueA;
		}
	}

	if (pszDefault) return pszDefault;

	return NULL;
}

const INI_ITEM * IniSection_GetItem(INISECTION * pSection,int iItem)
{
	if (iItem < 0 || iItem > (int)pSection->nItemCount) return NULL;

	return (const INI_ITEM *)&pSection->pkeys[iItem];
}

INT  IniSection_GetInt(INISECTION * pSection,LPCSTR pszKeyName,int nDefault)
{
	size_t i;
	size_t nKeyNameLen;

	if (!pszKeyName) return 0;

	nKeyNameLen = strlen(pszKeyName);

	for (i = 0; i < pSection->nItemCount; i++) {
		PROFILEKEY *pKey = &pSection->pkeys[i];
		if (pKey->nNameLength == nKeyNameLen &&
			(*pszKeyName == *pKey->pszNameA) &&
			!memcmp(pszKeyName,pKey->pszNameA,nKeyNameLen)){
				return atoi(pKey->pszValueA);
		}
	}
	
	return nDefault;
}





