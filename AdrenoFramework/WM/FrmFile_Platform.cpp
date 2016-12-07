//--------------------------------------------------------------------------------------
// File: FrmFile_Win32.cpp
// Desc: Platform-specific file I/O support
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------

#include "FrmPlatform.h"
#include "FrmFile.h"

#include <stdio.h>


// Our FRM_FILE struct is just a stdio FILE on this platform

#ifdef WINCE
//-----------------------------------------------------------------------------
//! retrieve the full path filename 
//! take the application root name and append the provided filename
void CreateFullFileName(const char* pszFileName, char* pszFullFileName, unsigned int nFullFileNameBufSize)
//-----------------------------------------------------------------------------
{
    WCHAR wszFullModuleFileName[MAX_PATH];
    char  szRootDirName[MAX_PATH];

    if (strchr(pszFileName, ':') != NULL || pszFileName[0]=='\\')
    {
        //filename contains driver letter or starts at root, consider it full
        strncpy(pszFullFileName, pszFileName, nFullFileNameBufSize);
        return;
    }

    //get the root directory for this module
    GetModuleFileName((HMODULE)GetCurrentProcessId(), (LPWSTR)wszFullModuleFileName, MAX_PATH);
    //convert wide character sting to multibyte character string
    wcstombs(szRootDirName, wszFullModuleFileName, MAX_PATH);
    //remove the leading filename to obtain the root dir name
    *(strrchr(szRootDirName, '\\')) = '\0';
    //concatenate the rootdirname with the relative filename 
    strncpy(pszFullFileName, szRootDirName, nFullFileNameBufSize);
    strncat(pszFullFileName, "\\", nFullFileNameBufSize - strlen(pszFullFileName));
    strncat(pszFullFileName, pszFileName, nFullFileNameBufSize - strlen(pszFullFileName));
}
#endif // WINCE


//--------------------------------------------------------------------------------------
// Basic file functions
//--------------------------------------------------------------------------------------
BOOL FrmFile_Open( const CHAR* strFileName, UINT32 nDirection, FILE** ppFile )
{
	// Get WM directory
	char szWholePath[512];
	CreateFullFileName(strFileName, szWholePath, sizeof(szWholePath));

    CHAR* strMode;
    if( nDirection == FRM_FILE_READ )
        strMode = "rb";
    else if( nDirection == FRM_FILE_WRITE )
        strMode = "wb";
    else if( nDirection == FRM_FILE_READWRITE )
        strMode = "rb+";
    else
        return FALSE;

    FILE* file = fopen( szWholePath, strMode );
    (*ppFile) = (FILE*)file;
    return file ? TRUE : FALSE;
}

VOID FrmFile_Close( FILE* pFile )
{
    fclose( pFile );
}

UINT32 FrmFile_Read( FILE* pFile, VOID* ptr, UINT32 nSize )
{
    return fread( ptr, nSize, 1, pFile );
}

UINT32 FrmFile_Write( FILE* pFile, VOID* ptr, UINT32 nSize )
{
    return fwrite( ptr, nSize, 1, pFile );
}

UINT32 FrmFile_GetSize( FILE* pFile )
{
    UINT32 nCurrentPos = ftell( pFile );
    fseek( pFile, 0, SEEK_END );
    UINT32 nSize = ftell( pFile );
    fseek( pFile, nCurrentPos, SEEK_SET );
    return nSize;
}

