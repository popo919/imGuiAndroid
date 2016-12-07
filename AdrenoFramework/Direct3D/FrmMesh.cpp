//--------------------------------------------------------------------------------------
// File: FrmMesh.cpp
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmMesh.h"
#include "FrmFile.h"


//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CFrmMesh::Load( const CHAR* strFileName )
{
    // Use internal structs with UINT32 in place of pointers and
    // then remap so that it works with 64-bit as well as 32-bit pointers
    // (32-bit and 64-bit builds of apps)
    struct FRM_MESH_FRAME_INTERNAL
    {
        CHAR               m_strName[32];

        FRMMATRIX4X4       m_matTransform;
        FRMMATRIX4X4       m_matBoneOffset2;
        FRMMATRIX4X4       m_matCombined;

        UINT32             m_pMesh;
        UINT32             m_pAnimationData;

        UINT32             m_pChild;
        UINT32             m_pNext;
    };

    struct FRM_MESH_INTERNAL
    {
        FRM_VERTEX_ELEMENT m_VertexLayout[8];
        UINT32             m_Reserved;

        UINT32             m_nNumVertices;
        UINT16             m_nVertexSize;
        UINT32             m_hVertexBuffer;

        UINT32             m_nNumIndices;
        UINT16             m_nIndexSize;
        UINT32             m_hIndexBuffer;

        UINT32             m_nNumBonesPerVertex;
        UINT32             m_nNumBoneMatrices;
        UINT32             m_nNumBones;
        UINT32             m_pBones;

        UINT32             m_nNumSubsets;
        UINT32             m_pSubsets;
    };

    struct FRM_MESH_BONE_INTERNAL
    {
        UINT32            m_pFrame;             // The mesh frame associated with this bone
        FRMMATRIX4X4      m_matBoneOffset;
    };

    struct FRM_MESH_SUBSET_INTERNAL
    {
        UINT32             m_nPrimType;
        UINT32             m_nNumIndices;
        UINT32             m_nIndexOffset;

        UINT32             m_pBoneIds;

        FRMVECTOR4         m_vDiffuseColor;
        FRMVECTOR4         m_vSpecularColor;
        FRMVECTOR3         m_vAmbientColor;
        FRMVECTOR3         m_vEmissiveColor;

        UINT32             m_nNumTextures;
        UINT32             m_pTextures;
    };

    struct FRM_MESH_TEXTURE_INTERNAL
    {
        CHAR               m_strTexture[32];
        UINT32             m_pTexture;
    };

    // Open the file
    FRM_FILE* file;
    if( FALSE == FrmFile_Open( strFileName, FRM_FILE_READ, &file ) )
        return FALSE;

    // Read in the mesh header
    FRM_MESH_FILE_HEADER header;
    FrmFile_Read( file, &header, sizeof(header) );
    if( header.nMagic != FRM_MESH_MAGIC_ID )
    {
        FrmFile_Close( file );
        return FALSE;
    }
    m_nNumFrames      = header.nNumFrames;
    m_nSystemDataSize = header.nSystemDataSize;
    m_nBufferDataSize = header.nBufferDataSize;

    // Read in the data
    m_pSystemData = new BYTE[m_nSystemDataSize];
    m_pBufferData = new BYTE[m_nBufferDataSize];
    FrmFile_Read( file, m_pSystemData, m_nSystemDataSize );
    FrmFile_Read( file, m_pBufferData, m_nBufferDataSize );

    // Done with the file
    FrmFile_Close( file );

    // Fix up pointers
    FRM_MESH_FRAME_INTERNAL *pFramesInternal = (FRM_MESH_FRAME_INTERNAL*)m_pSystemData;
    m_pFrames = new FRM_MESH_FRAME [ m_nNumFrames ];

    UINT32 meshFrmSize = sizeof(FRM_MESH_FRAME);
    UINT32 meshFrmSizeInternal = sizeof(FRM_MESH_FRAME_INTERNAL);
    UINT32 meshFrmSizeDiff = ( meshFrmSize - meshFrmSizeInternal );
    for( UINT32 i=0; i<m_nNumFrames; i++ )
    {
        FRM_MESH_FRAME *pFrame = &m_pFrames[i];
        FRM_MESH_FRAME_INTERNAL* pFrameInternal = &pFramesInternal[i];

        // Copy frame data
        ZeroMemory( pFrame, sizeof(FRM_MESH_FRAME) );
        memcpy( pFrame, pFrameInternal, offsetof( FRM_MESH_FRAME, m_matCombined ) );
        
        // Fix up child and next pointers
        if( pFrameInternal->m_pChild )
        {
            UINT32 numOffsetFrames = pFrameInternal->m_pChild / meshFrmSizeInternal;
            pFrame->m_pChild = (FRM_MESH_FRAME*)( &m_pFrames[numOffsetFrames] );
        }
        if( pFrameInternal->m_pNext )
        {
            UINT32 numOffsetFrames = pFrameInternal->m_pNext / meshFrmSizeInternal;
            pFrame->m_pNext = (FRM_MESH_FRAME*)( &m_pFrames[numOffsetFrames] );
        }

        if( pFrameInternal->m_pMesh )
        {
            FRM_MESH_INTERNAL *pMeshInternal = (FRM_MESH_INTERNAL*)( m_pSystemData + (UINT32)pFrameInternal->m_pMesh );

            pFrame->m_pMesh = new FRM_MESH;
            ZeroMemory( pFrame->m_pMesh, sizeof(FRM_MESH) );
            memcpy( pFrame->m_pMesh, pMeshInternal, offsetof( FRM_MESH_INTERNAL, m_nNumBones ) );  
            pFrame->m_pMesh->m_nNumSubsets = pMeshInternal->m_nNumSubsets;

            // Fix up mesh pointer
            FRM_MESH* pMesh = pFrame->m_pMesh;

            // Fix up bones pointer
            if( pMeshInternal->m_pBones )
            {
                FRM_MESH_BONE_INTERNAL *pBonesInternal = (FRM_MESH_BONE_INTERNAL*)( m_pSystemData + (UINT32)pMeshInternal->m_pBones );
                pMesh->m_pBones = new FRM_MESH_BONE[ pMesh->m_nNumBones ];

                for( UINT32 j=0; j<pMesh->m_nNumBones; j++ )
                {
                    pMesh->m_pBones[j].m_matBoneOffset = pBonesInternal->m_matBoneOffset;
                    UINT32 numOffsetFrames = pBonesInternal[j].m_pFrame / meshFrmSizeInternal;

                    pMesh->m_pBones[j].m_pFrame = (FRM_MESH_FRAME*)( &m_pFrames[numOffsetFrames] );
                }
            }

            // Fix up subsets pointer
            if( pMeshInternal->m_pSubsets )
            {
                FRM_MESH_SUBSET_INTERNAL *pSubsetsInternal = (FRM_MESH_SUBSET_INTERNAL*)( m_pSystemData + (UINT32)pMeshInternal->m_pSubsets );
                pMesh->m_pSubsets = new FRM_MESH_SUBSET[ pMesh->m_nNumSubsets ];                

                FRM_MESH_SUBSET* pSubsets = pMesh->m_pSubsets;

                for( UINT32 j=0; j<pMesh->m_nNumSubsets; j++ )
                {

                    pSubsets[j].m_nPrimType = pSubsetsInternal[j].m_nPrimType;
                    pSubsets[j].m_nNumIndices = pSubsetsInternal[j].m_nNumIndices;
                    pSubsets[j].m_nIndexOffset = pSubsetsInternal[j].m_nIndexOffset;
                    pSubsets[j].m_vDiffuseColor = pSubsetsInternal[j].m_vDiffuseColor;
                    pSubsets[j].m_vSpecularColor = pSubsetsInternal[j].m_vSpecularColor;
                    pSubsets[j].m_vAmbientColor = pSubsetsInternal[j].m_vAmbientColor;
                    pSubsets[j].m_vEmissiveColor = pSubsetsInternal[j].m_vEmissiveColor;
                    pSubsets[j].m_nNumTextures = pSubsetsInternal[j].m_nNumTextures;
                    pSubsets[j].m_pTextures = NULL;
                    pSubsets[j].m_pBoneIds = NULL;

                    // Fix up texture pointers
                    if( pSubsetsInternal[j].m_pTextures )
                    {
                        FRM_MESH_TEXTURE_INTERNAL* pTexturesInternal = (FRM_MESH_TEXTURE_INTERNAL*)( m_pSystemData + (UINT32)pSubsetsInternal[j].m_pTextures );                        
                        
                        pSubsets[j].m_pTextures = new FRM_MESH_TEXTURE[ pSubsets[j].m_nNumTextures ];
                        for ( UINT32 k = 0; k < pSubsets[j].m_nNumTextures; k++ )
                        {
                            memcpy( &pSubsets[j].m_pTextures[k].m_strTexture[0], &pTexturesInternal[k].m_strTexture[0], sizeof(pTexturesInternal[k].m_strTexture) );
                            pSubsets[j].m_pTextures[k].m_pTexture = NULL;
                        }                        
                    }

                    // Fix up bone subset pointers
                    if( pSubsetsInternal[j].m_pBoneIds )
                    {
                        UINT32* pBoneIdsInternal = (UINT32*)( m_pSystemData + (UINT32)pSubsetsInternal[j].m_pBoneIds );
                        pSubsets[j].m_pBoneIds = new UINT32[ pMesh->m_nNumBoneMatrices ];
                        memcpy( pSubsets[j].m_pBoneIds, pBoneIdsInternal, sizeof(UINT32) * pMesh->m_nNumBoneMatrices );
                    }
                }
            }
        }
    }

    return TRUE;
}
    

//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CFrmMesh::MakeDrawable( CFrmPackedResourceD3D* pResource )
{
    // Create vertex and index buffers
    for( UINT32 i=0; i<m_nNumFrames; i++ )
    {
        FRM_MESH* pMesh = m_pFrames[i].m_pMesh;
        if( pMesh )
        {
            // Create the vertex buffer
            BYTE* pVertexData = m_pBufferData + (UINT32)pMesh->m_hVertexBuffer;
            CFrmVertexBuffer* newVertexBuffer;
            if( FALSE == FrmCreateVertexBuffer( pMesh->m_nNumVertices, 
                                                pMesh->m_nVertexSize, 
                                                pVertexData, &newVertexBuffer ) )
                return FALSE;

            // Create the index buffer
            BYTE* pIndexData  = m_pBufferData + (UINT32)pMesh->m_hIndexBuffer;
            CFrmIndexBuffer* newIndexBuffer;
            if( FALSE == FrmCreateIndexBuffer( pMesh->m_nNumIndices, 
                                               pMesh->m_nIndexSize, 
                                               pIndexData, &newIndexBuffer ) )
            {
                delete newVertexBuffer;
                return FALSE;
            }

            // Store the new vertex/index buffers and handles to them
            pMesh->m_hVertexBuffer = (UINT32)m_vertexBuffers.size();
            pMesh->m_hIndexBuffer = (UINT32)m_indexBuffers.size();

            m_vertexBuffers.push_back(newVertexBuffer);
            m_indexBuffers.push_back(newIndexBuffer);

            // Create the textures
            if( pResource )
            {
                for( UINT32 j=0; j<pMesh->m_nNumSubsets; j++ )
                {
                    FRM_MESH_SUBSET* pSubset = &pMesh->m_pSubsets[j];
                    for( UINT32 k=0; k<pSubset->m_nNumTextures; k++ )
                    {
                        const CHAR* strTexture = pSubset->m_pTextures[k].m_strTexture;
                        CFrmTexture* pTexture  = pResource->GetTexture( strTexture );
                        pSubset->m_pTextures[k].m_pTexture = pTexture;
                    }
                }
            }
        }
    }
        
    // Done with the buffer data, so okay to delete it now
    delete[] m_pBufferData;
    m_pBufferData = NULL;

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmMesh::Destroy()
{
    for( UINT32 i=0; i<m_nNumFrames; i++ )
    {
        FRM_MESH* pMesh = m_pFrames[i].m_pMesh;
        if( pMesh )
        {
            // Release vertex and index buffers
            for (UINT32 i = 0; i < m_vertexBuffers.size(); i++)
            {
                delete m_vertexBuffers[i];
            }
            m_vertexBuffers.clear();

            for (UINT32 i = 0; i < m_indexBuffers.size(); i++)
            {
                delete m_indexBuffers[i];
            }
            m_indexBuffers.clear();

            // Release any textures
            for( UINT32 j=0; j<pMesh->m_nNumSubsets; j++ )
            {
                FRM_MESH_SUBSET* pSubset = &pMesh->m_pSubsets[j];
                for( UINT32 k=0; k<pSubset->m_nNumTextures; k++ )
                {
                    if( pSubset->m_pTextures[k].m_pTexture )
                        pSubset->m_pTextures[k].m_pTexture->Release();
                }

                if ( pSubset->m_pTextures ) delete [] pSubset->m_pTextures;
                pSubset->m_pTextures = NULL;

                if ( pSubset->m_pBoneIds )  delete [] pSubset->m_pBoneIds;
                pSubset->m_pBoneIds = NULL;
            }

            if ( pMesh->m_pBones )      delete [] pMesh->m_pBones;
            if ( pMesh->m_pSubsets )    delete [] pMesh->m_pSubsets;

            pMesh->m_pBones = NULL;
            pMesh->m_pSubsets = NULL;

        }
    }

    if( m_pFrames )     delete [] m_pFrames;
    if( m_pBufferData ) delete[] m_pBufferData;
    if( m_pSystemData ) delete[] m_pSystemData;

    m_pBufferData = NULL;
    m_pSystemData = NULL;
    m_pFrames     = NULL;
    m_nNumFrames  = 0;
}


//--------------------------------------------------------------------------------------
// Name: FrmReadAnimation
// Desc: 
//--------------------------------------------------------------------------------------
BOOL FrmReadAnimation( const CHAR* strFileName, FRM_ANIMATION_SET** ppAnimationSet )
{
    // Open the file for reading
    // Open the file
    FRM_FILE* file;
    if( FALSE == FrmFile_Open( strFileName, FRM_FILE_READ, &file ) )
        return FALSE;

    // Read the file header
    FRM_ANIMATION_FILE_HEADER Header;
    FrmFile_Read( file, &Header, sizeof(Header) );
    if( Header.m_nMagicID != FRM_ANIMATION_FILE_MAGIC_ID )
    {
        FrmFile_Close( file );
        return FALSE;
    }

    // Allocate memory and read the file
    BYTE* pData = new BYTE[Header.m_nDataSize];
    FrmFile_Read( file, pData, Header.m_nDataSize );
    FrmFile_Close( file );

    // Patch up the pointers to the animation key tables
    FRM_ANIMATION_SET* pAnimationSet = (FRM_ANIMATION_SET*)pData;

    pAnimationSet->m_pAnimations = (FRM_ANIMATION*)&pData[(UINT32)pAnimationSet->m_pAnimations];
    for( UINT32 i=0; i<pAnimationSet->m_nNumAnimations; i++ )
    {
        FRM_ANIMATION* pAnimation = &pAnimationSet->m_pAnimations[i];
        pAnimation->m_pKeyTimes  = (UINT32*)&pData[(UINT32)pAnimation->m_pKeyTimes];
        pAnimation->m_pKeyValues = (FRM_ANIMATION_KEY*)&pData[(UINT32)pAnimation->m_pKeyValues];
    }

    (*ppAnimationSet) = pAnimationSet;
    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: FrmGLPrimTypeToD3D()
// Desc: The MeshConverter uses GL enums to store primtypes.  This function converts
// from GL prim type to D3D_PRIMITIVE_TOPOLOGY
//--------------------------------------------------------------------------------------
D3D_PRIMITIVE_TOPOLOGY FrmGLPrimTypeToD3D( UINT32 nPrimType )
{
    switch ( nPrimType )
    {
    case 0x0000: // GL_POINTS
        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    case 0x0001: // GL_LINES
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case 0x0003: // GL_LINE_STRIP
        return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case 0x0004: // GL_TRIANGLES
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case 0x0005: // GL_TRIANGLE_STRIP
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    case 0x0002: // GL_LINE_LOOP    
    case 0x0006: // GL_TRIANGLE_FAN
    default:
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED; // Not supported    
    }
}

