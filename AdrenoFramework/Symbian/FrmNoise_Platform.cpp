//--------------------------------------------------------------------------------------
// File: FrmNoise.h
// Desc: 
//
// Author:      QUALCOMM, Advanced Content Group - Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmNoise.h"
#include "FrmMath.h"

#include <ngi/libc/stdlib.h> // For rand()


#define RAND FLOAT32( ( rand() % ( B + B ) ) - B )

#define setup(i, b0, b1, r0, r1)\
	INT32 b0 = INT32(i + N) & BM;\
	INT32 b1 = (b0 + 1) & BM;\
	FLOAT32 r0 = (i + N) - INT32(i + N);\
	FLOAT32 r1 = r0 - 1;

CFrmPerlin::CFrmPerlin()
{
    for( INT32 i = 0; i < B; i++ )
    {
        m_p[i] = i;

        m_g1[i] = RAND / B;
        m_g2[i] = FrmVector2Normalize(FRMVECTOR2(RAND, RAND) / B);
        m_g3[i] = FrmVector3Normalize(FRMVECTOR3(RAND, RAND, RAND) / B);
	}

    for( INT32 i = 0; i < B; i++ )
    {
        INT32 r = rand() % B;
        INT32 tmp = m_p[i];
        m_p[i] = m_p[r];
        m_p[r] = tmp;
    }

    for( INT32 i = 0; i < B + 2; i++ )
    {
        m_p[B + i]  = m_p[i];
        m_g1[B + i] = m_g1[i];
        m_g2[B + i] = m_g2[i];
        m_g3[B + i] = m_g3[i];
	}
}


FLOAT32 CFrmPerlin::Noise1( const FLOAT32 x )
{
    setup(x, bx0, bx1, rx0, rx1);
	FLOAT32 sx = FrmSCurve(rx0);

	return FrmLerp(sx, rx0 * m_g1[m_p[bx0]], rx1 * m_g1[m_p[bx1]]);
}


FLOAT32 CFrmPerlin::Noise2( const FLOAT32 x, const FLOAT32 y )
{
	setup(x, bx0, bx1, rx0, rx1);
	setup(y, by0, by1, ry0, ry1);

	INT32 b00 = m_p[m_p[bx0] + by0];
	INT32 b10 = m_p[m_p[bx1] + by0];
	INT32 b01 = m_p[m_p[bx0] + by1];
	INT32 b11 = m_p[m_p[bx1] + by1];

	FLOAT32 sx = FrmSCurve(rx0);
	FLOAT32 sy = FrmSCurve(ry0);

	FLOAT32 a = FrmLerp(sx, FrmVector2Dot(m_g2[b00], FRMVECTOR2(rx0, ry0)), FrmVector2Dot(m_g2[b10], FRMVECTOR2(rx1, ry0)));
	FLOAT32 b = FrmLerp(sx, FrmVector2Dot(m_g2[b01], FRMVECTOR2(rx0, ry1)), FrmVector2Dot(m_g2[b11], FRMVECTOR2(rx1, ry1)));

	return FrmLerp(sy, a, b);
}


FLOAT32 CFrmPerlin::Noise3( const FLOAT32 x, const FLOAT32 y, const FLOAT32 z )
{
	setup(x, bx0, bx1, rx0, rx1);
	setup(y, by0, by1, ry0, ry1);
	setup(z, bz0, bz1, rz0, rz1);

	INT32 b00 = m_p[m_p[bx0] + by0];
	INT32 b10 = m_p[m_p[bx1] + by0];
	INT32 b01 = m_p[m_p[bx0] + by1];
	INT32 b11 = m_p[m_p[bx1] + by1];

	FLOAT32 sx = FrmSCurve(rx0);
	FLOAT32 sy = FrmSCurve(ry0);
	FLOAT32 sz = FrmSCurve(rz0);

	FLOAT32 a0 = FrmLerp(sx, FrmVector3Dot(m_g3[b00 + bz0], FRMVECTOR3(rx0, ry0, rz0)), FrmVector3Dot(m_g3[b10 + bz0], FRMVECTOR3(rx1, ry0, rz0)));
	FLOAT32 b0 = FrmLerp(sx, FrmVector3Dot(m_g3[b01 + bz0], FRMVECTOR3(rx0, ry1, rz0)), FrmVector3Dot(m_g3[b11 + bz0], FRMVECTOR3(rx1, ry1, rz0)));
	FLOAT32 c0 = FrmLerp(sy, a0, b0);

	FLOAT32 a1 = FrmLerp(sx, FrmVector3Dot(m_g3[b00 + bz1], FRMVECTOR3(rx0, ry0, rz1)), FrmVector3Dot(m_g3[b10 + bz1], FRMVECTOR3(rx1, ry0, rz1)));
	FLOAT32 b1 = FrmLerp(sx, FrmVector3Dot(m_g3[b01 + bz1], FRMVECTOR3(rx0, ry1, rz1)), FrmVector3Dot(m_g3[b11 + bz1], FRMVECTOR3(rx1, ry1, rz1)));
	FLOAT32 c1 = FrmLerp(sy, a1, b1);

	return FrmLerp(sz, c0, c1);
}


FLOAT32 CFrmPerlin::Turbulence1( const FLOAT32 x, FLOAT32 fFreq )
{
	FLOAT32 t = 0.0f;

	do
    {
		t += Noise1(fFreq * x) / fFreq;
		fFreq *= 0.5f;
	} while (fFreq >= 1.0f);
	
	return t;
}


FLOAT32 CFrmPerlin::Turbulence2( const FLOAT32 x, const FLOAT32 y, FLOAT32 fFreq )
{
	FLOAT32 t = 0.0f;

	do
    {
		t += Noise2(fFreq * x, fFreq * y) / fFreq;
		fFreq *= 0.5f;
	} while (fFreq >= 1.0f);
	
	return t;
}


FLOAT32 CFrmPerlin::Turbulence3( const FLOAT32 x, const FLOAT32 y, const FLOAT32 z, FLOAT32 fFreq )
{
	FLOAT32 t = 0.0f;

	do
    {
		t += Noise3(fFreq * x, fFreq * y, fFreq * z) / fFreq;
		fFreq *= 0.5f;
	} while (fFreq >= 1.0f);
	
	return t;
}


FLOAT32 CFrmPerlin::TileableNoise1(const FLOAT32 x, const FLOAT32 w)
{
	return( Noise1( x )     * ( w - x ) +
			Noise1( x - w ) *      x ) / w;
}

FLOAT32 CFrmPerlin::TileableNoise2(const FLOAT32 x, const FLOAT32 y, const FLOAT32 w, const FLOAT32 h)
{
	return ( Noise2( x,     y )     * ( w - x )* ( h - y ) +
			 Noise2( x - w, y )     *       x  * ( h - y ) +
			 Noise2( x,     y - h ) * ( w - x )*      y  +
			 Noise2( x - w, y - h ) *       x  *      y ) / ( w * h );
}

FLOAT32 CFrmPerlin::TileableNoise3(const FLOAT32 x, const FLOAT32 y, const FLOAT32 z, const FLOAT32 w, const FLOAT32 h, const FLOAT32 d)
{
	return ( Noise3( x,     y,     z )     * ( w - x ) * ( h - y ) * ( d - z ) +
			 Noise3( x - w, y,     z )     *        x  * ( h - y ) * ( d - z ) +
			 Noise3( x,     y - h, z )     * ( w - x ) *       y   * ( d - z ) +
			 Noise3( x - w, y - h, z )     *        x  *       y   * ( d - z ) + 
			 Noise3( x,     y,     z - d ) * ( w - x ) * ( h - y ) *       z  +
			 Noise3( x - w, y,     z - d ) *        x  * ( h - y ) *       z  +
			 Noise3( x,     y - h, z - d ) * ( w - x ) *       y   *       z  +
			 Noise3( x - w, y - h, z - d ) *        x  *       y   *       z ) / ( w * h * d );
}

FLOAT32 CFrmPerlin::TileableTurbulence1(const FLOAT32 x, const FLOAT32 w, FLOAT32 freq)
{
	FLOAT32 t = 0.0f;

	do
    {
		t += TileableNoise1(freq * x, w * freq) / freq;
		freq *= 0.5f;
	} while (freq >= 1.0f);
	
	return t;
}

FLOAT32 CFrmPerlin::TileableTurbulence2(const FLOAT32 x, const FLOAT32 y, const FLOAT32 w, const FLOAT32 h, FLOAT32 freq)
{
	FLOAT32 t = 0.0f;

	do
    {
		t += TileableNoise2(freq * x, freq * y, w * freq, h * freq) / freq;
		freq *= 0.5f;
	} while (freq >= 1.0f);
	
	return t;
}

FLOAT32 CFrmPerlin::TileableTurbulence3(const FLOAT32 x, const FLOAT32 y, const FLOAT32 z, const FLOAT32 w, const FLOAT32 h, const FLOAT32 d, FLOAT32 freq)
{
	FLOAT32 t = 0.0f;

	do
    {
		t += TileableNoise3(freq * x, freq * y, freq * z, w * freq, h * freq, d * freq) / freq;
		freq *= 0.5f;
	} while (freq >= 1.0f);
	
	return t;
}

