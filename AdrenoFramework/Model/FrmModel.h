//-----------------------------------------------------------------------------
// 
//  Model
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_MODEL_H
#define ADRENO_MODEL_H

#include "FrmJoint.h"
#include "FrmMaterial.h"
#include "FrmModelMesh.h"
#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Model
    {
        INT32     NumMaterials;
        Material* Materials;

        INT32     NumMeshes;
        Mesh*     Meshes;
        
        INT32     NumJoints;
        Joint*    Joints;

         Model();
        ~Model();

        void    ResizeMaterials ( int num_materials );
        void    ResizeMeshes    ( int num_meshes );
        void    ResizeJoints    ( int num_joints );

        private:
            void DeleteMaterials();
            void DeleteMeshes();
            void DeleteJoints();
    };
}

//-----------------------------------------------------------------------------

#endif