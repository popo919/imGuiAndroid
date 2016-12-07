//--------------------------------------------------------------------------------------
// File: FrmMath.inl
// Desc: 
//
// Author:      QUALCOMM, Advanced Content Group - Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include <ngi/libc/stdlib.h> // For rand()
#include <ngi/libc/math.h>
#include "ffloatmath.h"
#include "FrmBasicTypes.h"
#include "FrmMath.h"


FLOAT32 FrmFloor( FLOAT32 value ) { return (FLOAT32)floor( value ); }
FLOAT32 FrmRand()                 { return rand() * ( 1.0f / RAND_MAX ); }
FLOAT32 FrmSin( FLOAT32 deg )     { return (FLOAT32)sin( deg ); }
FLOAT32 FrmCos( FLOAT32 deg )     { return (FLOAT32)cos( deg ); }
FLOAT32 FrmTan( FLOAT32 deg )     { return (FLOAT32)tan( deg ); }
FLOAT32 FrmSqrt( FLOAT32 value )  { return (FLOAT32)sqrt( value ); }

    
//--------------------------------------------------------------------------------------
// Name: FrmLerp()
// Desc: 
//--------------------------------------------------------------------------------------
FLOAT32 FrmLerp( FLOAT32 t, FLOAT32 a, FLOAT32 b )
{
    return a + t * ( b - a );
}


//--------------------------------------------------------------------------------------
// Name: FrmSphrand()
// Desc: Returns a random point within a sphere centered at (0,0)
//--------------------------------------------------------------------------------------
FRMVECTOR3 FrmSphrand( FLOAT32 fRadius )
{
    // Calculate a random normalized vector
    FRMVECTOR3 vResult;
    vResult.x = 2.0f * FrmRand() - 1.0f;
    vResult.y = 2.0f * FrmRand() - 1.0f;
    vResult.z = 2.0f * FrmRand() - 1.0f;
    ffp_vec3_normalize( (FLOAT32*)&vResult );

    // Scale by a number between 0 and radius
    vResult *= fRadius * FrmRand();

    return vResult;
}


//--------------------------------------------------------------------------------------
// Name: ApplyVariance()
// Desc: Applies the specified amount of random variance to the specified value.
//--------------------------------------------------------------------------------------
FLOAT32 ApplyVariance( FLOAT32 fValue, FLOAT32 fVariance )
{
    return fValue - 0.5f * fVariance + FrmRand() * fVariance;
}


FLOAT32 FrmSCurve( const FLOAT32 t )
{
    return t * t * (3 - 2 * t);
}


FLOAT32 FrmVector2Dot( FRMVECTOR2& vVector0, FRMVECTOR2& vVector1 )
{
    return ( vVector0.x * vVector1.x +
             vVector0.y * vVector1.y );
}


FLOAT32 FrmVector2Length( FRMVECTOR2& vVector )
{
    return vVector.x * vVector.x + vVector.y * vVector.y;
}


FRMVECTOR2 FrmVector2Normalize( FRMVECTOR2& vVector )
{
    FLOAT32 fLength = FrmVector2Length( vVector );
    if( fLength != 0.0f )
        fLength = 1.0f / FrmSqrt( fLength );
    
    return vVector * fLength;
}


//--------------------------------------------------------------------------------------
// Name: FrmVector3Normalize()
// Desc: Normalizes a vector
//--------------------------------------------------------------------------------------
FRMVECTOR3 FrmVector3Normalize( FRMVECTOR3& vVector )
{
    FLOAT32 fLength = FrmVector3Length( vVector );
    if( fLength != 0.0f )
        fLength = 1.0f / FrmSqrt( fLength );
    
    return vVector * fLength;
}


FRMVECTOR3 FrmVector3Mul( FRMVECTOR3& vVector, FLOAT32 fScalar )
{
    FRMVECTOR3 vResult;
    vResult.x = vVector.x * fScalar;
    vResult.y = vVector.y * fScalar;
    vResult.z = vVector.z * fScalar;
    return vResult;
}


FRMVECTOR3 FrmVector3Add( FRMVECTOR3& vVector0, FRMVECTOR3& vVector1 )
{
    FRMVECTOR3 vResult;
    vResult.x = vVector0.x + vVector1.x;
    vResult.y = vVector0.y + vVector1.y;
    vResult.z = vVector0.z + vVector1.z;
    return vResult;
}


FRMVECTOR3 FrmVector3Cross( FRMVECTOR3& vVector0, FRMVECTOR3& vVector1 )
{
    FRMVECTOR3 vResult;
    vResult.x = vVector0.y * vVector1.z - vVector0.z * vVector1.y;
    vResult.y = vVector0.z * vVector1.x - vVector0.x * vVector1.z;
    vResult.z = vVector0.x * vVector1.y - vVector0.y * vVector1.x;
    return vResult;
}


FLOAT32 FrmVector3Dot( FRMVECTOR3& vVector0, FRMVECTOR3& vVector1 )
{
    return ( vVector0.x * vVector1.x +
             vVector0.y * vVector1.y +
             vVector0.z * vVector1.z );
}


FLOAT32 FrmVector3Length( FRMVECTOR3& vVector )
{
    return ( vVector.x * vVector.x +
             vVector.y * vVector.y +
             vVector.z * vVector.z );
}


FRMMATRIX4X4 FrmMatrixIdentity()
{
    FRMMATRIX4X4 matResult;

    matResult.m[0][0] = 1.0f;
    matResult.m[0][1] = 0.0f;
    matResult.m[0][2] = 0.0f;
    matResult.m[0][3] = 0.0f;

    matResult.m[1][0] = 0.0f;
    matResult.m[1][1] = 1.0f;
    matResult.m[1][2] = 0.0f;
    matResult.m[1][3] = 0.0f;

    matResult.m[2][0] = 0.0f;
    matResult.m[2][1] = 0.0f;
    matResult.m[2][2] = 1.0f;
    matResult.m[2][3] = 0.0f;

    matResult.m[0][3] = 0.0f;
    matResult.m[1][3] = 0.0f;
    matResult.m[2][3] = 0.0f;
    matResult.m[3][3] = 1.0f;

    return matResult;
}


FRMMATRIX4X4 FrmMatrixLookAtLH( FRMVECTOR3& vPosition, FRMVECTOR3& vLookAt, FRMVECTOR3& vUp )
{
    FRMVECTOR3 ZAxis = vLookAt - vPosition;
    FRMVECTOR3 XAxis = FrmVector3Cross( vUp, ZAxis );
    FRMVECTOR3 YAxis = FrmVector3Cross( ZAxis, XAxis );

    XAxis = FrmVector3Normalize( XAxis );
    YAxis = FrmVector3Normalize( YAxis );
    ZAxis = FrmVector3Normalize( ZAxis );

    FRMMATRIX4X4 matResult;

    matResult.m[0][0] = XAxis.x;
    matResult.m[0][1] = YAxis.x;
    matResult.m[0][2] = ZAxis.x;
    matResult.m[0][3] = 0.0f;

    matResult.m[1][0] = XAxis.y;
    matResult.m[1][1] = YAxis.y;
    matResult.m[1][2] = ZAxis.y;
    matResult.m[1][3] = 0.0f;

    matResult.m[2][0] = XAxis.z;
    matResult.m[2][1] = YAxis.z;
    matResult.m[2][2] = ZAxis.z;
    matResult.m[2][3] = 0.0f;

    matResult.m[3][0] = -FrmVector3Dot( XAxis, vPosition );
    matResult.m[3][1] = -FrmVector3Dot( YAxis, vPosition );
    matResult.m[3][2] = -FrmVector3Dot( ZAxis, vPosition );
    matResult.m[3][3] = 1.0f;

    return matResult;
}


FRMMATRIX4X4 FrmMatrixLookAtRH( FRMVECTOR3& vPosition, FRMVECTOR3& vLookAt, FRMVECTOR3& vUp )
{
    FRMVECTOR3 ZAxis = vLookAt - vPosition;
    FRMVECTOR3 XAxis = FrmVector3Cross( vUp, ZAxis );
    FRMVECTOR3 YAxis = FrmVector3Cross( ZAxis, XAxis );
    XAxis = FrmVector3Normalize( XAxis );
    YAxis = FrmVector3Normalize( YAxis );
    ZAxis = FrmVector3Normalize( ZAxis );

    FRMMATRIX4X4 matResult;

    matResult.m[0][0] = -XAxis.x;
    matResult.m[0][1] = +YAxis.x;
    matResult.m[0][2] = -ZAxis.x;
    matResult.m[0][3] = 0.0f;

    matResult.m[1][0] = -XAxis.y;
    matResult.m[1][1] = +YAxis.y;
    matResult.m[1][2] = -ZAxis.y;
    matResult.m[1][3] = 0.0f;

    matResult.m[2][0] = -XAxis.z;
    matResult.m[2][1] = +YAxis.z;
    matResult.m[2][2] = -ZAxis.z;
    matResult.m[2][3] = 0.0f;

    matResult.m[3][0] = +FrmVector3Dot( XAxis, vPosition );
    matResult.m[3][1] = -FrmVector3Dot( YAxis, vPosition );
    matResult.m[3][2] = +FrmVector3Dot( ZAxis, vPosition );
    matResult.m[3][3] = 1.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmVector3TransformNormal()
// Desc: Transforms a normal through a matrix
//--------------------------------------------------------------------------------------
FRMVECTOR3 FrmVector3TransformNormal( FRMVECTOR3& vSrcVector, FRMMATRIX4X4& matSrcMatrix )
{
    // Note: We MUST pre-initialize the dst vector, or the ffp function will not work!
    FRMVECTOR3 vDstVector = vSrcVector;
    ffp_vec3_xform4_normalize( (FLOAT32*)&matSrcMatrix, (FLOAT32*)&vDstVector,
                               (FLOAT32*)&vDstVector );
    return vDstVector;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixPerspectiveFovLH()
// Desc: Sets the matrix to a perspective projection
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixPerspectiveFovLH( FLOAT32 fFOVy, FLOAT32 fAspect, 
                                        FLOAT32 zNear, FLOAT32 zFar )
{
    FLOAT32 SinFov = FrmSin( 0.5f * fFOVy );
    FLOAT32 CosFov = FrmCos( 0.5f * fFOVy );
    FLOAT32 h = CosFov / SinFov;
    FLOAT32 w  = h / fAspect;

    FRMMATRIX4X4 matResult;

    matResult.m[0][0] = w;
    matResult.m[0][1] = 0.0f;
    matResult.m[0][2] = 0.0f;
    matResult.m[0][3] = 0.0f;

    matResult.m[1][0] = 0.0f;
    matResult.m[1][1] = h;
    matResult.m[1][2] = 0.0f;
    matResult.m[1][3] = 0.0f;

    matResult.m[2][0] = 0.0f;
    matResult.m[2][1] = 0.0f;
    matResult.m[2][2] = -( zFar+zNear ) / ( zFar-zNear );
    matResult.m[2][3] = -( 2.0f*zFar*zNear ) / ( zFar-zNear );

    matResult.m[3][0] = 0.0f;
    matResult.m[3][1] = 0.0f;
    matResult.m[3][2] = -1.0f;
    matResult.m[3][3] = 0.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixPerspectiveFovRH()
// Desc: Sets the matrix to a perspective projection
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixPerspectiveFovRH( FLOAT32 fFOVy, FLOAT32 fAspect,
                                        FLOAT32 zNear, FLOAT32 zFar )
{
    FLOAT32 SinFov = FrmSin( 0.5f * fFOVy );
    FLOAT32 CosFov = FrmCos( 0.5f * fFOVy );
    FLOAT32 h = CosFov / SinFov;
    FLOAT32 w  = h / fAspect;

    FRMMATRIX4X4 matResult;

    matResult.m[0][0] = w;
    matResult.m[0][1] = 0.0f;
    matResult.m[0][2] = 0.0f;
    matResult.m[0][3] = 0.0f;

    matResult.m[1][0] = 0.0f;
    matResult.m[1][1] = h;
    matResult.m[1][2] = 0.0f;
    matResult.m[1][3] = 0.0f;

    matResult.m[2][0] = 0.0f;
    matResult.m[2][1] = 0.0f;
    matResult.m[2][2] = -( zFar ) / ( zFar-zNear );
    matResult.m[2][3] = -1.0f;

    matResult.m[3][0] = 0.0f;
    matResult.m[3][1] = 0.0f;
    matResult.m[3][2] = -( zFar * zNear ) / ( zFar-zNear );
    matResult.m[3][3] = 0.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixNormal()
// Desc: Generates a matrix for transforming normals.
//--------------------------------------------------------------------------------------
FRMMATRIX3X3 FrmMatrixNormal( FRMMATRIX4X4& matSrcMatrix )
{
    // Compute the normal matrix, which is the inverse of the input matrix
    
    // Note that we assume that the input matrix is orthonormal, so that the inverse
    // of the matrix is really just a transpose

    FRMMATRIX3X3 matResult;

    matResult.m[0][0] = matSrcMatrix.m[0][0];
    matResult.m[0][1] = matSrcMatrix.m[1][0];
    matResult.m[0][2] = matSrcMatrix.m[2][0];
    
    matResult.m[1][0] = matSrcMatrix.m[0][1];
    matResult.m[1][1] = matSrcMatrix.m[1][1];
    matResult.m[1][2] = matSrcMatrix.m[2][1];
    
    matResult.m[2][0] = matSrcMatrix.m[0][2];
    matResult.m[2][1] = matSrcMatrix.m[1][2];
    matResult.m[2][2] = matSrcMatrix.m[2][2];

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixMultiply()
// Desc: Multiplies two matrices.
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixMultiply( FRMMATRIX4X4& matSrcMatrixA, FRMMATRIX4X4& matSrcMatrixB )
{
    FRMMATRIX4X4 DstMatrix;
#ifdef FRM_ROW_MAJOR
    ffp_matrix_mul( (FLOAT32*)&matSrcMatrixB, (FLOAT32*)&matSrcMatrixA, 
                    (FLOAT32*)&DstMatrix, FFP_MATRIX_TYPE_GENERAL, FFP_MATRIX_TYPE_GENERAL );
#else
    ffp_matrix_mul( (FLOAT32*)&matSrcMatrixA, (FLOAT32*)&matSrcMatrixB, 
                    (FLOAT32*)&DstMatrix, FFP_MATRIX_TYPE_GENERAL, FFP_MATRIX_TYPE_GENERAL );
#endif
    return DstMatrix;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixInverse()
// Desc: Computes the inverse of a matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixInverse( FRMMATRIX4X4& matSrcMatrix )
{
//    This doesn't work!!! Perhaps the matrix needs to be transposed first!
//    MATRIX4X4 DstMatrix = *pSrcMatrix;
//    ffp_matrix_inverse( (FLOAT32*)&DstMatrix, FFP_MATRIX_TYPE_GENERAL );
//    (*pDstMatrix) = DstMatrix;

    // The following code only works on orthonormal matrices
    FRMMATRIX4X4 DstMatrix;

    // Transpose rotation
    DstMatrix.m[0][0] = matSrcMatrix.m[0][0];  
    DstMatrix.m[0][1] = matSrcMatrix.m[1][0];  
    DstMatrix.m[0][2] = matSrcMatrix.m[2][0];
    DstMatrix.m[0][3] = 0.0f; 
    
    DstMatrix.m[1][0] = matSrcMatrix.m[0][1];
    DstMatrix.m[1][1] = matSrcMatrix.m[1][1];
    DstMatrix.m[1][2] = matSrcMatrix.m[2][1];
    DstMatrix.m[1][3] = 0.0f; 
    
    DstMatrix.m[2][0] = matSrcMatrix.m[0][2];
    DstMatrix.m[2][1] = matSrcMatrix.m[1][2];
    DstMatrix.m[2][2] = matSrcMatrix.m[2][2];
    DstMatrix.m[2][3] = 0.0f; 

    // Set translation as the negative dot product of the translation and rotation
    DstMatrix.m[0][0] = -(matSrcMatrix.m[3][0]*matSrcMatrix.m[0][0]) - (matSrcMatrix.m[3][1]*matSrcMatrix.m[0][1]) - (matSrcMatrix.m[3][2]*matSrcMatrix.m[0][2]);
    DstMatrix.m[1][1] = -(matSrcMatrix.m[3][0]*matSrcMatrix.m[1][0]) - (matSrcMatrix.m[3][1]*matSrcMatrix.m[1][1]) - (matSrcMatrix.m[3][2]*matSrcMatrix.m[1][2]);
    DstMatrix.m[2][2] = -(matSrcMatrix.m[3][0]*matSrcMatrix.m[2][0]) - (matSrcMatrix.m[3][1]*matSrcMatrix.m[2][1]) - (matSrcMatrix.m[3][2]*matSrcMatrix.m[2][2]);
    DstMatrix.m[3][3] = 1.0f;

    return DstMatrix;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixTranslate()
// Desc: Builds a translation matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixTranslate( FRMVECTOR3& vVector )
{
    return FrmMatrixTranslate( vVector.x, vVector.y, vVector.z );
}

FRMMATRIX4X4 FrmMatrixTranslate( FLOAT32 tx, FLOAT32 ty, FLOAT32 tz )
{
    FRMMATRIX4X4 matResult;

    matResult.m[0][0] = 1.0f;
    matResult.m[0][1] = 0.0f;
    matResult.m[0][2] = 0.0f;
    matResult.m[0][3] = 0.0f;

    matResult.m[1][0] = 0.0f;
    matResult.m[1][1] = 1.0f;
    matResult.m[1][2] = 0.0f;
    matResult.m[1][3] = 0.0f;

    matResult.m[2][0] = 0.0f;
    matResult.m[2][1] = 0.0f;
    matResult.m[2][2] = 1.0f;
    matResult.m[2][3] = 0.0f;

    matResult.m[0][3] = tx;
    matResult.m[1][3] = ty;
    matResult.m[2][3] = tz;
    matResult.m[3][3] = 1.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixScale()
// Desc: Builds a scaling matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixScale( FRMVECTOR3& vVector )
{
    return FrmMatrixScale( vVector.x, vVector.y, vVector.z );
}

FRMMATRIX4X4 FrmMatrixScale( FLOAT32 sx, FLOAT32 sy, FLOAT32 sz )
{
    FRMMATRIX4X4 matResult;

    matResult.m[0][0] = sx;
    matResult.m[0][1] = 0.0f;
    matResult.m[0][2] = 0.0f;
    matResult.m[0][3] = 0.0f;

    matResult.m[1][0] = 0.0f;
    matResult.m[1][1] = sy;
    matResult.m[1][2] = 0.0f;
    matResult.m[1][3] = 0.0f;

    matResult.m[2][0] = 0.0f;
    matResult.m[2][1] = 0.0f;
    matResult.m[2][2] = sz;
    matResult.m[2][3] = 0.0f;

    matResult.m[0][3] = 0.0f;
    matResult.m[1][3] = 0.0f;
    matResult.m[2][3] = 0.0f;
    matResult.m[3][3] = 1.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixRotate()
// Desc: Builds a rotation matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixRotate( FLOAT32 fAngle, FRMVECTOR3& vAxis )
{
    return FrmMatrixRotate( fAngle, vAxis.x, vAxis.y, vAxis.z );
}

FRMMATRIX4X4 FrmMatrixRotate( FLOAT32 fAngle, FLOAT32 rx, FLOAT32 ry, FLOAT32 rz )
{
    FRMMATRIX4X4 matResult;
    ffp_matrix_build_rotate( (FLOAT32*)&matResult, FrmDegrees(fAngle), rx, ry, rz );
    return matResult;
}
