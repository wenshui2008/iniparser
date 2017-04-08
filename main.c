#include <stdio.h>
#include "iniparser.h"

int main(int argc,char * argv[])
{
	INIFILE * ini;
	const INI_ITEM * pItem;
	INISECTION * pSection;
	int nSectionCount;
	int i;

	ini = IniFile_Open("D:\\iAVCast\\conf\\test.ini",NULL);

	if (!ini) return -1;

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
	
	pSection = IniFile_GetSection(ini,"Video.Device");

	if (pSection) {
		LPCSTR pValue = IniSection_GetValue(pSection,"sDeviceName",NULL);
		if (pValue) {
			printf("value:%s\r\n",pValue);
		}

		pValue = IniSection_GetValue(pSection,"sInputSize",NULL);
		if (pValue) {
			printf("value:%s\r\n",pValue);
		}
	}

	pSection = IniFile_GetSection(ini,"General");

	if (pSection) {
		LPCSTR pValue = IniSection_GetValue(pSection,"sChannelName",NULL);
		if (pValue) {
			printf("value:%s\r\n",pValue);
		}

		pValue = IniSection_GetValue(pSection,"sWindowsUser",NULL);
		if (pValue) {
			printf("value:%s\r\n",pValue);
		}
	}

	IniFile_Close(ini);

	return 0;
}