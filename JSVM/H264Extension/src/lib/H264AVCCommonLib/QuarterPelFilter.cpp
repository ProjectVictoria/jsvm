/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************

This software module was originally developed by

Heiko Schwarz    (Fraunhofer HHI),
Tobias Hinz      (Fraunhofer HHI),
Karsten Suehring (Fraunhofer HHI)

in the course of development of the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video
Coding) for reference purposes and its performance may not have been optimized.
This software module is an implementation of one or more tools as specified by
the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding).

Those intending to use this software module in products are advised that its
use may infringe existing patents. ISO/IEC have no liability for use of this
software module or modifications thereof.

Assurance that the originally developed software module can be used
(1) in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) once the
ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) has been adopted; and
(2) to develop the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding): 

To the extent that Fraunhofer HHI owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, Fraunhofer HHI will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

Fraunhofer HHI retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards. 

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005. 

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The Fraunhofer HHI hereby donate this source code to the ITU, with the following
understanding:
    1. Fraunhofer HHI retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. Fraunhofer HHI retain full patent rights (if any exist) in the technical
       content of techniques and algorithms herein.
    3. The ITU shall make this code available to anyone, free of license or
       royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from 
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/




#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"

H264AVC_NAMESPACE_BEGIN


QuarterPelFilter::QuarterPelFilter():
m_bClip ( true )
{
  uninit();
}

QuarterPelFilter::~QuarterPelFilter()
{
}

ErrVal QuarterPelFilter::create( QuarterPelFilter*& rpcQuarterPelFilter )
{
  QuarterPelFilter* pcQuarterPelFilter;

  pcQuarterPelFilter = new QuarterPelFilter;

  rpcQuarterPelFilter = pcQuarterPelFilter;

  ROT( NULL == rpcQuarterPelFilter );

  return Err::m_nOK;
}


ErrVal QuarterPelFilter::destroy()
{
  delete this;

  return Err::m_nOK;
}


ErrVal QuarterPelFilter::init()
{
  m_bClip = true;

  m_afpFilterBlockFunc[0] = QuarterPelFilter::xFilter1;
  m_afpFilterBlockFunc[1] = QuarterPelFilter::xFilter2;
  m_afpFilterBlockFunc[2] = QuarterPelFilter::xFilter3;
  m_afpFilterBlockFunc[3] = QuarterPelFilter::xFilter4;

  m_afpXFilterBlockFunc[0] = QuarterPelFilter::xXFilter1;
  m_afpXFilterBlockFunc[1] = QuarterPelFilter::xXFilter2;
  m_afpXFilterBlockFunc[2] = QuarterPelFilter::xXFilter3;
  m_afpXFilterBlockFunc[3] = QuarterPelFilter::xXFilter4;
  return Err::m_nOK;
}

ErrVal QuarterPelFilter::uninit()
{
  m_afpFilterBlockFunc[0] = NULL;
  m_afpFilterBlockFunc[1] = NULL;
  m_afpFilterBlockFunc[2] = NULL;
  m_afpFilterBlockFunc[3] = NULL;

  m_afpXFilterBlockFunc[0] = NULL;
  m_afpXFilterBlockFunc[1] = NULL;
  m_afpXFilterBlockFunc[2] = NULL;
  m_afpXFilterBlockFunc[3] = NULL;
  return Err::m_nOK;
}


const Int g_aiTapCoeff[6] = { 1, -5,20,20,-5, 1};


Void QuarterPelFilter::predBlk( YuvMbBuffer* pcDesBuffer, YuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  Pel* pucDes     = pcDesBuffer->getYBlk( cIdx );
  Pel* pucSrc     = pcSrcBuffer->getYBlk( cIdx );
  Int iDesStride  = pcDesBuffer->getLStride();
  Int iSrcStride  = pcSrcBuffer->getLStride();
  Int iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;
  if( iDy == 0)
  {
    if( iDx == 0 )
    {
      iSizeX>>=2;
      for( Int y = 0; y < iSizeY; y++)
      {
        UInt* puiDes = (UInt*)pucDes;
        UInt* puiSrc = (UInt*)pucSrc;
        for( Int x = 0; x < iSizeX; x++ )
        {
          puiDes[x] = puiSrc[x];
        }
        pucDes += iDesStride;
        pucSrc += iSrcStride;
      }
      return;
    }


    if( iDx == 2 )
    {
      xPredDy0Dx2( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
      return;
    }

    // if( iDx == 1 || iDx == 3)
    xPredDy0Dx13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  if( iDx == 0)
  {
    if( iDy == 2 )
    {
      xPredDx0Dy2( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
      return;
    }

    // if( iDx == 1 || iDx == 3)
    xPredDx0Dy13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  if( iDx == 2)
  {
    if( iDy == 2 )
    {
      xPredDx2Dy2( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
      return;
    }

    // if( iDy == 1 || iDy == 3 )
    xPredDx2Dy13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }



  if( iDy == 2)
  {
    // if( iDx == 2 ) is already done
    // if( iDx == 1 || iDx == 3 )
    xPredDy2Dx13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  xPredElse( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
  return;
}



Void QuarterPelFilter::xPredDy0Dx2( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0];
      iTemp += pucSrc[x + 1];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1];
      iTemp -= pucSrc[x + 2];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2];
      iTemp += pucSrc[x + 3];
      pucDest[x] = gClip( (iTemp + 16) / 32 );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}

Void QuarterPelFilter::xPredDy0Dx13( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{

  iDx >>= 1;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0];
      iTemp += pucSrc[x + 1];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1];
      iTemp -= pucSrc[x + 2];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2];
      iTemp += pucSrc[x + 3];
      iTemp = gClip( (iTemp + 16) / 32 );
      pucDest[x] = (iTemp + pucSrc[ x + iDx] + 1) / 2;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx0Dy2( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0*iSrcStride];
      iTemp += pucSrc[x + 1*iSrcStride];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1*iSrcStride];
      iTemp -= pucSrc[x + 2*iSrcStride];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2*iSrcStride];
      iTemp += pucSrc[x + 3*iSrcStride];
      pucDest[x] = gClip( (iTemp + 16) / 32 );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}

Void QuarterPelFilter::xPredDx0Dy13( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  iDy = (iDy>>1) * iSrcStride;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0*iSrcStride];
      iTemp += pucSrc[x + 1*iSrcStride];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1*iSrcStride];
      iTemp -= pucSrc[x + 2*iSrcStride];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2*iSrcStride];
      iTemp += pucSrc[x + 3*iSrcStride];
      iTemp = gClip( (iTemp + 16) / 32 );
      pucDest[x] = (iTemp + pucSrc[ x + iDy] + 1)/2;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx2( Short*  psDest, Pel*  pucSrc, Int iSrcStride, UInt uiSizeY, UInt uiSizeX )
{
  pucSrc -= 2*iSrcStride;

  for( UInt y = 0; y < uiSizeY + 5; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Pel* puc = pucSrc + x;
      Int iTemp;
      iTemp  = puc[0];
      iTemp += puc[1];
      iTemp  = iTemp << 2;
      iTemp -= puc[-1];
      iTemp -= puc[2];
      iTemp += iTemp << 2;
      iTemp += puc[-2];
      iTemp += puc[3];
      psDest[x] = iTemp;

    }
    psDest += 16;
    pucSrc += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx2Dy2( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  Short asTemp[16*(16+6)];
  Short* psTemp = asTemp;
  xPredDx2( asTemp, pucSrc, iSrcStride, uiSizeY, uiSizeX );

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iIndex = x + 0x20;
      Int iTemp;
      iTemp  = psTemp[ 0x00 + iIndex];
      iTemp += psTemp[ 0x10 + iIndex];
      iTemp  = iTemp << 2;
      iTemp -= psTemp[-0x10 + iIndex];
      iTemp -= psTemp[ 0x20 + iIndex];
      iTemp += iTemp << 2;
      iTemp += psTemp[-0x20 + iIndex];
      iTemp += psTemp[ 0x30 + iIndex];

      pucDest[x] = gClip( (iTemp + 512) / 1024 );
    }
    psTemp  += 0x10;
    pucDest += iDestStride;
  }

}


Void QuarterPelFilter::xPredDx2Dy13( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  Short asTemp[16*(16+6)];
  Short* psTemp = asTemp;
  xPredDx2( asTemp, pucSrc, iSrcStride, uiSizeY, uiSizeX );

  iDy = (iDy == 1) ? 0 : 0x10;

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iIndex = x + 0x20;
      Int iTemp;
      iTemp  = psTemp[ 0x00 + iIndex];
      iTemp += psTemp[ 0x10 + iIndex];
      iTemp  = iTemp << 2;
      iTemp -= psTemp[-0x10 + iIndex];
      iTemp -= psTemp[ 0x20 + iIndex];
      iTemp += iTemp << 2;
      iTemp += psTemp[-0x20 + iIndex];
      iTemp += psTemp[ 0x30 + iIndex];
      iTemp = gClip( (iTemp + 512) / 1024 );
      pucDest[x] = (iTemp + gClip( (psTemp[iDy + iIndex] + 16) / 32 ) + 1) / 2;
    }
    psTemp  += 0x10;
    pucDest += iDestStride;
  }
}


Void QuarterPelFilter::xPredDy2Dx13( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  iDx = (iDx == 1) ? 2 : 3;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    Int aiTemp[6+16];
    for( UInt n = 0; n < 6+uiSizeX; n++)
    {
      Pel* puc = pucSrc + n - 2;
      aiTemp[n]  = puc[ 0*iSrcStride];
      aiTemp[n] += puc[ 1*iSrcStride];
      aiTemp[n]  = aiTemp[n] << 2;
      aiTemp[n] -= puc[-1*iSrcStride];
      aiTemp[n] -= puc[ 2*iSrcStride];
      aiTemp[n] += aiTemp[n] << 2;
      aiTemp[n] += puc[-2*iSrcStride];
      aiTemp[n] += puc[ 3*iSrcStride];
    }

    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp = 0;
      for( Int n = 0; n < 6; n++ )
      {
        iTemp += aiTemp[n+x]*g_aiTapCoeff[n];
      }
      iTemp = gClip( (iTemp + 512) / 1024 );
      pucDest[x] = (iTemp + gClip( (aiTemp[x+iDx] + 16) / 32 ) + 1) / 2;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}



Void QuarterPelFilter::xPredElse( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  Pel* pucSrcX = pucSrc;
  Pel* pucSrcY = pucSrc;

  pucSrcY += (iDx == 1) ? 0 : 1;
  pucSrcX += (iDy == 1) ? 0 : iSrcStride;

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTempX;
      iTempX  = pucSrcX[x - 0];
      iTempX += pucSrcX[x + 1];
      iTempX  = iTempX << 2;
      iTempX -= pucSrcX[x - 1];
      iTempX -= pucSrcX[x + 2];
      iTempX += iTempX << 2;
      iTempX += pucSrcX[x - 2];
      iTempX += pucSrcX[x + 3];
      iTempX = gClip( (iTempX + 16) / 32 );

      Int iTempY;
      iTempY  = pucSrcY[x - 0*iSrcStride];
      iTempY += pucSrcY[x + 1*iSrcStride];
      iTempY  = iTempY << 2;
      iTempY -= pucSrcY[x - 1*iSrcStride];
      iTempY -= pucSrcY[x + 2*iSrcStride];
      iTempY += iTempY << 2;
      iTempY += pucSrcY[x - 2*iSrcStride];
      iTempY += pucSrcY[x + 3*iSrcStride];
      iTempY = gClip( (iTempY + 16) / 32 );

      pucDest[x] = (iTempX + iTempY + 1) >> 1;
    }
    pucDest += iDestStride;
    pucSrcX += iSrcStride;
    pucSrcY += iSrcStride;
  }
}





ErrVal QuarterPelFilter::filterFrame( IntYuvPicBuffer *pcPelBuffer, IntYuvPicBuffer *pcHalfPelBuffer )
{
  ROT( NULL == pcPelBuffer );
  ROT( NULL == pcHalfPelBuffer );

  XPel*   pucSrc      = pcPelBuffer->getMbLumAddr ();
  Int     iHeight     = pcPelBuffer->getLHeight   ();
  Int     iWidth      = pcPelBuffer->getLWidth    ();
  Int     iStride     = pcPelBuffer->getLStride   ();
  Int     iMargin     = pcPelBuffer->getLXMargin  ();
  UInt    uiTmpXSize  = (iMargin + iWidth + iMargin) * 2;
  UInt    uiTmpYSize  = iMargin + iHeight + iMargin;
  Int     iMarginNew  = iMargin-4;
  Int     x, y;

  XXPel*  psTemp      = new XXPel[uiTmpXSize * uiTmpYSize];

  ROT( NULL == psTemp )

  XXPel*  ps          = &psTemp[ iMargin * uiTmpXSize + 2*iMargin ]; // fix provided by Shijun Sun

  for( y = 0; y < iHeight; y++ )
  {
    for( x = -iMarginNew; x < iWidth+iMarginNew; x++ )
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0];
      iTemp += pucSrc[x + 1];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1];
      iTemp -= pucSrc[x + 2];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2];
      iTemp += pucSrc[x + 3];
      ps[2*x]    = pucSrc[x]<<5;
      ps[2*x+1]  = iTemp;
    }

    ps     += uiTmpXSize;
    pucSrc += iStride;
  }

  // bot
  ps -= iMargin*2;               // fix provided by Shijun Sun
  for( y = 0; y < iMargin; y++ ) // fix provided by Shijun Sun
  {
    ::memcpy( &ps[y*uiTmpXSize], &ps[(y-1)*uiTmpXSize], uiTmpXSize*sizeof(XXPel) );
  }

  //top
  ps = &psTemp[ iMargin * uiTmpXSize ]; // fix provided by Shijun Sun
  for( y = 0; y < iMargin; y++ )        // fix provided by Shijun Sun
  {
    ::memcpy( &ps[-(y+1)*uiTmpXSize], &ps[-y*uiTmpXSize], uiTmpXSize*sizeof(XXPel) );
  }

  ps = &psTemp[ 4*uiTmpXSize + 2*iMargin ]; // fix provided by Shijun Sun
  iStride = uiTmpXSize;

  Int   iDesStrideHP  = pcHalfPelBuffer->getLStride();
  XPel* pucDesHP      = pcHalfPelBuffer->getMbLumAddr();
  pucDesHP -= (iMarginNew*iDesStrideHP)<<1;

  for( y = -iMarginNew; y < iHeight+iMarginNew; y++ )
  {
    for( x = -2*iMarginNew; x < 2*(iWidth+iMarginNew); x++ )
    {
      Int iTemp;
      iTemp  = ps[x - 0*iStride];
      iTemp += ps[x + 1*iStride];
      iTemp  = iTemp << 2;
      iTemp -= ps[x - 1*iStride];
      iTemp -= ps[x + 2*iStride];
      iTemp += iTemp << 2;
      iTemp += ps[x - 2*iStride];
      iTemp += ps[x + 3*iStride];

      pucDesHP[x]              = xClip( ( ps[x] + 16) / 32);
      pucDesHP[x+iDesStrideHP] = xClip( ( iTemp + 512) / 1024);
    }
    pucDesHP += iDesStrideHP<<1;
    ps     += iStride;
  }

  delete [] psTemp;
  psTemp = NULL;
  return Err::m_nOK;
}






Void QuarterPelFilter::xFilter1( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;
  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int a = pSrc[-1];
      Int b = pSrc[-iSrcStride];
      Int c = pSrc[+1];
      Int d = pSrc[iSrcStride];
      Int o = pSrc[0];

      pDes[0x000] = (a + b + 1) >> 1;
      pDes[0x100] = (b + o + 1) >> 1;
      pDes[0x200] = (b + c + 1) >> 1;
      pDes[0x300] = (a + o + 1) >> 1;
      pDes[0x500] = (c + o + 1) >> 1;
      pDes[0x600] = (a + d + 1) >> 1;
      pDes[0x700] = (d + o + 1) >> 1;
      pDes[0x800] = (d + c + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xXFilter1( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;
  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int a = pSrc[-1];
      Int b = pSrc[-iSrcStride];
      Int c = pSrc[+1];
      Int d = pSrc[iSrcStride];
      Int o = pSrc[0];

      pDes[0x000] = (a + b + 1) >> 1;
      pDes[0x100] = (b + o + 1) >> 1;
      pDes[0x200] = (b + c + 1) >> 1;
      pDes[0x300] = (a + o + 1) >> 1;
      pDes[0x500] = (c + o + 1) >> 1;
      pDes[0x600] = (a + d + 1) >> 1;
      pDes[0x700] = (d + o + 1) >> 1;
      pDes[0x800] = (d + c + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}


Void QuarterPelFilter::xFilter2( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int o = pSrc[0];

      pDes[0x000] = (o + pSrc[-iSrcStride-1] + 1) >> 1;
      pDes[0x100] = (o + pSrc[-iSrcStride  ] + 1) >> 1;
      pDes[0x200] = (o + pSrc[-iSrcStride+1] + 1) >> 1;
      pDes[0x300] = (o + pSrc[-1] + 1) >> 1;
      pDes[0x500] = (o + pSrc[+1] + 1) >> 1;
      pDes[0x600] = (o + pSrc[iSrcStride-1] + 1) >> 1;
      pDes[0x700] = (o + pSrc[iSrcStride  ] + 1) >> 1;
      pDes[0x800] = (o + pSrc[iSrcStride+1] + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xXFilter2( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int o = pSrc[0];

      pDes[0x000] = (o + pSrc[-iSrcStride-1] + 1) >> 1;
      pDes[0x100] = (o + pSrc[-iSrcStride  ] + 1) >> 1;
      pDes[0x200] = (o + pSrc[-iSrcStride+1] + 1) >> 1;
      pDes[0x300] = (o + pSrc[-1] + 1) >> 1;
      pDes[0x500] = (o + pSrc[+1] + 1) >> 1;
      pDes[0x600] = (o + pSrc[iSrcStride-1] + 1) >> 1;
      pDes[0x700] = (o + pSrc[iSrcStride  ] + 1) >> 1;
      pDes[0x800] = (o + pSrc[iSrcStride+1] + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xFilter3( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int o = pSrc[0];

      pDes[0x000] = (o + pSrc[-iSrcStride-1] + 1) >> 1;
      pDes[0x100] = (o + pSrc[-iSrcStride  ] + 1) >> 1;
      pDes[0x200] = (o + pSrc[-iSrcStride+1] + 1) >> 1;
      pDes[0x300] = (o + pSrc[-1] + 1) >> 1;
      pDes[0x500] = (o + pSrc[+1] + 1) >> 1;
      pDes[0x600] = (o + pSrc[iSrcStride-1] + 1) >> 1;
      pDes[0x700] = (o + pSrc[iSrcStride  ] + 1) >> 1;
      pDes[0x800] = (o + pSrc[iSrcStride+1] + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xXFilter3( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int o = pSrc[0];

      pDes[0x000] = (o + pSrc[-iSrcStride-1] + 1) >> 1;
      pDes[0x100] = (o + pSrc[-iSrcStride  ] + 1) >> 1;
      pDes[0x200] = (o + pSrc[-iSrcStride+1] + 1) >> 1;
      pDes[0x300] = (o + pSrc[-1] + 1) >> 1;
      pDes[0x500] = (o + pSrc[+1] + 1) >> 1;
      pDes[0x600] = (o + pSrc[iSrcStride-1] + 1) >> 1;
      pDes[0x700] = (o + pSrc[iSrcStride  ] + 1) >> 1;
      pDes[0x800] = (o + pSrc[iSrcStride+1] + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xFilter4( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int a = pSrc[-1];
      Int b = pSrc[-iSrcStride];
      Int c = pSrc[+1];
      Int d = pSrc[iSrcStride];
      Int o = pSrc[0];

      pDes[0x000] = (a + b + 1) >> 1;
      pDes[0x100] = (b + o + 1) >> 1;
      pDes[0x200] = (b + c + 1) >> 1;
      pDes[0x300] = (a + o + 1) >> 1;
      pDes[0x500] = (c + o + 1) >> 1;
      pDes[0x600] = (a + d + 1) >> 1;
      pDes[0x700] = (d + o + 1) >> 1;
      pDes[0x800] = (c + d + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}
Void QuarterPelFilter::xXFilter4( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int a = pSrc[-1];
      Int b = pSrc[-iSrcStride];
      Int c = pSrc[+1];
      Int d = pSrc[iSrcStride];
      Int o = pSrc[0];

      pDes[0x000] = (a + b + 1) >> 1;
      pDes[0x100] = (b + o + 1) >> 1;
      pDes[0x200] = (b + c + 1) >> 1;
      pDes[0x300] = (a + o + 1) >> 1;
      pDes[0x500] = (c + o + 1) >> 1;
      pDes[0x600] = (a + d + 1) >> 1;
      pDes[0x700] = (d + o + 1) >> 1;
      pDes[0x800] = (c + d + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}




Void QuarterPelFilter::predBlk( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  XPel* pucDes    = pcDesBuffer->getYBlk( cIdx );
  XPel* pucSrc    = pcSrcBuffer->getYBlk( cIdx );
  Int iDesStride  = pcDesBuffer->getLStride();
  Int iSrcStride  = pcSrcBuffer->getLStride();
  Int iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;
  if( iDy == 0)
  {
    if( iDx == 0 )
    {
      for( Int y = 0; y < iSizeY; y++)
      {
        for( Int x = 0; x < iSizeX; x++ )
        {
          pucDes[x] = pucSrc[x];
        }
        pucDes += iDesStride;
        pucSrc += iSrcStride;
      }
      return;
    }


    if( iDx == 2 )
    {
      xPredDy0Dx2( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
      return;
    }

    // if( iDx == 1 || iDx == 3)
    xPredDy0Dx13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  if( iDx == 0)
  {
    if( iDy == 2 )
    {
      xPredDx0Dy2( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
      return;
    }

    // if( iDx == 1 || iDx == 3)
    xPredDx0Dy13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  if( iDx == 2)
  {
    if( iDy == 2 )
    {
      xPredDx2Dy2( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
      return;
    }

    // if( iDy == 1 || iDy == 3 )
    xPredDx2Dy13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }



  if( iDy == 2)
  {
    // if( iDx == 2 ) is already done
    // if( iDx == 1 || iDx == 3 )
    xPredDy2Dx13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  xPredElse( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
  return;
}




Void QuarterPelFilter::predBlkAdapt( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX, 
                                      UShort* weight)
{
  XPel* pucDes    = pcDesBuffer->getYBlk( cIdx );
  XPel* pucSrc    = pcSrcBuffer->getYBlk( cIdx );
  Int iDesStride  = pcDesBuffer->getLStride();
  Int iSrcStride  = pcSrcBuffer->getLStride();
  Int iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;

  UShort usWeight;

  weightOnEnergy(&usWeight, pucSrc + (iDy+1)/4*iSrcStride + (iDx+1)/4, iSrcStride, iSizeY, iSizeX );
  *weight = (*weight) *usWeight;

  xPredAdapt( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX, *weight, 128 );

  return;
}


Void QuarterPelFilter::xPredDy0Dx2( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0];
      iTemp += pucSrc[x + 1];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1];
      iTemp -= pucSrc[x + 2];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2];
      iTemp += pucSrc[x + 3];
      pucDest[x] = xClip( (iTemp + 16) / 32 );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDy0Dx13( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{

  iDx >>= 1;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0];
      iTemp += pucSrc[x + 1];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1];
      iTemp -= pucSrc[x + 2];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2];
      iTemp += pucSrc[x + 3];
      iTemp = xClip( (iTemp + 16) / 32 );
      pucDest[x] = (iTemp + pucSrc[ x + iDx] + 1) / 2;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx0Dy2( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0*iSrcStride];
      iTemp += pucSrc[x + 1*iSrcStride];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1*iSrcStride];
      iTemp -= pucSrc[x + 2*iSrcStride];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2*iSrcStride];
      iTemp += pucSrc[x + 3*iSrcStride];
      pucDest[x] = xClip( (iTemp + 16) / 32 );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx0Dy13( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  iDy = (iDy>>1) * iSrcStride;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0*iSrcStride];
      iTemp += pucSrc[x + 1*iSrcStride];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1*iSrcStride];
      iTemp -= pucSrc[x + 2*iSrcStride];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2*iSrcStride];
      iTemp += pucSrc[x + 3*iSrcStride];
      iTemp = xClip( (iTemp + 16) / 32 );
      pucDest[x] = (iTemp + pucSrc[ x + iDy] + 1)/2;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}



Void QuarterPelFilter::xPredDx2( XXPel*  psDest, XPel*  pucSrc, Int iSrcStride, UInt uiSizeY, UInt uiSizeX )
{
  pucSrc -= 2*iSrcStride;

  for( UInt y = 0; y < uiSizeY + 5; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      XPel* puc = pucSrc + x;
      Int iTemp;
      iTemp  = puc[0];
      iTemp += puc[1];
      iTemp  = iTemp << 2;
      iTemp -= puc[-1];
      iTemp -= puc[2];
      iTemp += iTemp << 2;
      iTemp += puc[-2];
      iTemp += puc[3];
      psDest[x] = iTemp;

    }
    psDest += 16;
    pucSrc += iSrcStride;
  }
}



Void QuarterPelFilter::xPredDx2Dy2( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  XXPel   asTemp[16*(16+6)];
  XXPel*  psTemp = asTemp;
  xPredDx2( asTemp, pucSrc, iSrcStride, uiSizeY, uiSizeX );

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iIndex = x + 0x20;
      Int iTemp;
      iTemp  = psTemp[ 0x00 + iIndex];
      iTemp += psTemp[ 0x10 + iIndex];
      iTemp  = iTemp << 2;
      iTemp -= psTemp[-0x10 + iIndex];
      iTemp -= psTemp[ 0x20 + iIndex];
      iTemp += iTemp << 2;
      iTemp += psTemp[-0x20 + iIndex];
      iTemp += psTemp[ 0x30 + iIndex];

      pucDest[x] = xClip( (iTemp + 512) / 1024 );
    }
    psTemp  += 0x10;
    pucDest += iDestStride;
  }

}


Void QuarterPelFilter::xPredDx2Dy13( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  XXPel   asTemp[16*(16+6)];
  XXPel*  psTemp = asTemp;
  xPredDx2( asTemp, pucSrc, iSrcStride, uiSizeY, uiSizeX );

  iDy = (iDy == 1) ? 0 : 0x10;

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iIndex = x + 0x20;
      Int iTemp;
      iTemp  = psTemp[ 0x00 + iIndex];
      iTemp += psTemp[ 0x10 + iIndex];
      iTemp  = iTemp << 2;
      iTemp -= psTemp[-0x10 + iIndex];
      iTemp -= psTemp[ 0x20 + iIndex];
      iTemp += iTemp << 2;
      iTemp += psTemp[-0x20 + iIndex];
      iTemp += psTemp[ 0x30 + iIndex];
      iTemp = xClip( (iTemp + 512) / 1024 );
      pucDest[x] = (iTemp + xClip( (psTemp[iDy + iIndex] + 16) / 32 ) + 1) / 2;
    }
    psTemp  += 0x10;
    pucDest += iDestStride;
  }
}


Void QuarterPelFilter::xPredDy2Dx13( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  iDx = (iDx == 1) ? 2 : 3;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    Int aiTemp[6+16];
    for( UInt n = 0; n < 6+uiSizeX; n++)
    {
      XPel* puc = pucSrc + n - 2;
      aiTemp[n]  = puc[ 0*iSrcStride];
      aiTemp[n] += puc[ 1*iSrcStride];
      aiTemp[n]  = aiTemp[n] << 2;
      aiTemp[n] -= puc[-1*iSrcStride];
      aiTemp[n] -= puc[ 2*iSrcStride];
      aiTemp[n] += aiTemp[n] << 2;
      aiTemp[n] += puc[-2*iSrcStride];
      aiTemp[n] += puc[ 3*iSrcStride];
    }

    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp = 0;
      for( Int n = 0; n < 6; n++ )
      {
        iTemp += aiTemp[n+x]*g_aiTapCoeff[n];
      }
      iTemp = xClip( (iTemp + 512) / 1024 );
      pucDest[x] = (iTemp + xClip( (aiTemp[x+iDx] + 16) / 32 ) + 1) / 2;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}



Void QuarterPelFilter::xPredElse( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  XPel* pucSrcX = pucSrc;
  XPel* pucSrcY = pucSrc;

  pucSrcY += (iDx == 1) ? 0 : 1;
  pucSrcX += (iDy == 1) ? 0 : iSrcStride;

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTempX;
      iTempX  = pucSrcX[x - 0];
      iTempX += pucSrcX[x + 1];
      iTempX  = iTempX << 2;
      iTempX -= pucSrcX[x - 1];
      iTempX -= pucSrcX[x + 2];
      iTempX += iTempX << 2;
      iTempX += pucSrcX[x - 2];
      iTempX += pucSrcX[x + 3];
      iTempX = xClip( (iTempX + 16) / 32 );

      Int iTempY;
      iTempY  = pucSrcY[x - 0*iSrcStride];
      iTempY += pucSrcY[x + 1*iSrcStride];
      iTempY  = iTempY << 2;
      iTempY -= pucSrcY[x - 1*iSrcStride];
      iTempY -= pucSrcY[x + 2*iSrcStride];
      iTempY += iTempY << 2;
      iTempY += pucSrcY[x - 2*iSrcStride];
      iTempY += pucSrcY[x + 3*iSrcStride];
      iTempY = xClip( (iTempY + 16) / 32 );

      pucDest[x] = (iTempX + iTempY + 1) >> 1;
    }
    pucDest += iDestStride;
    pucSrcX += iSrcStride;
    pucSrcY += iSrcStride;
  }
}

Void QuarterPelFilter::xPredAdapt( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, 
                                    UInt uiSizeY, UInt uiSizeX, UShort weight, UShort wMax )
{
  int methodId;
  static int f4tap[4][4] = {
    { 0, 16,  0,  0}, 
    {-2, 14,  5, -1},
    {-2, 10, 10, -2},
    {-1,  5, 14, -2}
  };

  if(weight > wMax/2)
  {
    methodId = 2; 
  }
  else if(weight > 0)
  {
    methodId = 1; 
  }
  else
    return;


  // INTERPOLATION
  if(methodId==1)
  {
    // method 1: bi-linear for sub-pel samples
    for( UInt y = 0; y < uiSizeY; y++)
    {
      for( UInt x = 0; x < uiSizeX; x++)
      {
        Int iTemp;
        iTemp  = pucSrc[x - 0]*(4-iDx)*(4-iDy);
        iTemp += pucSrc[x + 1]*iDx*(4-iDy);
        iTemp += pucSrc[x + 1*iSrcStride]*(4-iDx)*iDy;
        iTemp += pucSrc[x + 1*iSrcStride + 1]*iDx*iDy;

        pucDest[x] = xClip( (iTemp + 8) >>4 );
      }
      pucDest += iDestStride;
      pucSrc += iSrcStride;
    }
  }
  else if(methodId == 2)
  {
    // method 2: 4 tap filter for sub-pel samples
    for( UInt y = 0; y < uiSizeY; y++)
    {
      for( UInt x = 0; x < uiSizeX; x++)
      {
        Int iTemp1[4], iTemp2;
        int i, j;

        for(i=0;i<4;i++)
        {
          iTemp1[i] = 0;
          for(j=0;j<4;j++)
            iTemp1[i] += pucSrc[x + (i-1)*iSrcStride + j-1] * f4tap[iDx][j];
        }

        iTemp2 = 0;
        for(j=0;j<4;j++)
          iTemp2 += iTemp1[j] * f4tap[iDy][j];

        pucDest[x] = xClip( (iTemp2 + 128) >> 8 );
      }
      pucDest += iDestStride;
      pucSrc += iSrcStride;
    }
  }
}

Void QuarterPelFilter::weightOnEnergy(UShort *usWeight, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX)
{
  Int iSSD = 0;
  int bitsShift = 8, i;

  for( UInt y = 0; y < iSizeY; y++)
  {
    for( UInt x = 0; x < iSizeX; x++)
    {
      Int iTemp;
      iTemp  = xClip( pucSrc[iSrcStride*y + x]);
      iSSD += iTemp*iTemp;
    }
  }

  for( i = (iSizeY/4)*(iSizeX/4); i > 1; i >>= 1 )
    bitsShift ++;

  iSSD                                = ( iSSD + (1 << (bitsShift-1)) ) >> bitsShift;
  *usWeight                           = (UShort) max( 0, min( 16, 20 - iSSD ) );
}

H264AVC_NAMESPACE_END
