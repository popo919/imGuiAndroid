//--------------------------------------------------------------------------------------
// File: FrmFile_Symbian.cpp
// Desc: Platform-specific file I/O support
//
// Author:      QUALCOMM, Advanced Content Group - Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include <ngi/libc/stdio.h>
#include <ngi/libc/stdlib.h>
#include "FrmBasicTypes.h"
#include "FrmFile.h"


// Our FRM_FILE struct is just a stdio FILE on this platform
struct FRM_FILE : public FILE
{
};


//--------------------------------------------------------------------------------------
// Basic file functions
//--------------------------------------------------------------------------------------
BOOL FrmFile_Open( const CHAR* strFileName, UINT32 nDirection, FRM_FILE** ppFile )
{
    CHAR* strMode;
    if( nDirection == FRM_FILE_READ )
        strMode = "rb";
    else if( nDirection == FRM_FILE_WRITE )
        strMode = "wb";
    else if( nDirection == FRM_FILE_READWRITE )
        strMode = "rb+";
    else
        return FALSE;

    FILE* file = fopen( strFileName, strMode );
    (*ppFile) = (FRM_FILE*)file;
    return file ? TRUE : FALSE;
}

VOID FrmFile_Close( FRM_FILE* pFile )
{
    fclose( pFile );
}

UINT32 FrmFile_Read( FRM_FILE* pFile, VOID* ptr, UINT32 nSize )
{
    return fread( ptr, nSize, 1, pFile );
}

UINT32 FrmFile_Write( FRM_FILE* pFile, VOID* ptr, UINT32 nSize )
{
    return fwrite( ptr, nSize, 1, pFile );
}

UINT32 FrmFile_GetSize( FRM_FILE* pFile )
{
    UINT32 nCurrentPos = ftell( pFile );
    fseek( pFile, 0, SEEK_END );
    UINT32 nSize = ftell( pFile );
    fseek( pFile, nCurrentPos, SEEK_SET );
    return nSize;
}

