#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>

WCHAR* MediaSearchPath();

//--------------------------------------------------------------------------------------
// Search a set of typical directories
//--------------------------------------------------------------------------------------
bool FindMediaSearchTypicalDirs(__in_ecount(cchSearch) WCHAR* strSearchPath,
	int cchSearch,
	__in LPCWSTR strLeaf,
	__in WCHAR* strExePath,
	__in WCHAR* strExeName);




//--------------------------------------------------------------------------------------
// Search parent directories starting at strStartAt, and appending strLeafName
// at each parent directory.  It stops at the root directory.
//--------------------------------------------------------------------------------------
bool FindMediaSearchParentDirs(__in_ecount(cchSearch) WCHAR* strSearchPath,
	int cchSearch,
	__in WCHAR* strStartAt,
	__in WCHAR* strLeafName);



HRESULT FindDXSDKMediaFileCch(__in_ecount(cchDest) WCHAR* strDestPath,
	int cchDest,
	__in LPCWSTR strFilename);



#endif // _COMMON_H_