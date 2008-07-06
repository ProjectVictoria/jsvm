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
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/YuvPicBuffer.h"


H264AVC_NAMESPACE_BEGIN


YuvMbBuffer::YuvMbBuffer()
: m_pPelCurrY( NULL )
, m_pPelCurrU( NULL )
, m_pPelCurrV( NULL )
{
  DO_DBG( ::memset( m_aucYuvBuffer, 0 , sizeof(m_aucYuvBuffer) ) );// TMM_INTERLACE
}


YuvMbBuffer::~YuvMbBuffer()
{
}

Void
YuvMbBuffer::setZero()
{
  ::memset( m_aucYuvBuffer, 0 , sizeof(m_aucYuvBuffer) );
}

Void YuvMbBuffer::loadIntraPredictors( YuvPicBuffer* pcSrcBuffer )
{
  Int y;

  XPel* pSrc = pcSrcBuffer->getMbLumAddr();
  XPel* pDes = getMbLumAddr();

  Int iSrcStride = pcSrcBuffer->getLStride();
  Int iDesStride = getLStride();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  ::memcpy( pDes, pSrc, sizeof(XPel)*21 );
  ::memcpy( pDes+iDesStride+17, pSrc+21, sizeof(XPel)*4 );

  for( y = 0; y < 16; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }

  pSrc = pcSrcBuffer->getMbCbAddr();
  pDes = getMbCbAddr();

  iSrcStride = pcSrcBuffer->getCStride();
  iDesStride = getCStride();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  ::memcpy( pDes, pSrc, sizeof(XPel)*9 );

  for( y = 0; y < 8; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }

  pSrc = pcSrcBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  ::memcpy( pDes, pSrc, sizeof(XPel)*9 );

  for( y = 0; y < 8; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }
}


Void YuvMbBuffer::loadBuffer( YuvPicBuffer* pcSrcBuffer )
{
  Int   y;
  XPel* pSrc;
  XPel* pDes;
  Int   iSrcStride;
  Int   iDesStride;

  pSrc = pcSrcBuffer->getMbLumAddr();
  pDes = getMbLumAddr();
  iDesStride = getLStride();
  iSrcStride = pcSrcBuffer->getLStride();

  for( y = 0; y < 16; y++ )
  {
    ::memcpy( pDes, pSrc, 16 * sizeof(XPel) );
    pDes += iDesStride;
    pSrc += iSrcStride;
  }

  pSrc = pcSrcBuffer->getMbCbAddr();
  pDes = getMbCbAddr();
  iDesStride = getCStride();
  iSrcStride = pcSrcBuffer->getCStride();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iDesStride;
    pSrc += iSrcStride;
  }

  pSrc = pcSrcBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
}


Void
YuvMbBuffer::add( YuvMbBuffer& rcIntYuvMbBuffer )
{
  Int   y, x;
  Int   iStride = getLStride  ();
  XPel* pSrc    = rcIntYuvMbBuffer.getMbLumAddr();
  XPel* pDes    = getMbLumAddr();

  for( y = 0; y < 16; y++ )
  {
    for( x = 0; x < 16; x++ )   pDes[x] += pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }

  iStride = getCStride  ();
  pSrc    = rcIntYuvMbBuffer.getMbCbAddr ();
  pDes    = getMbCbAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] += pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }

  pSrc    = rcIntYuvMbBuffer.getMbCrAddr ();
  pDes    = getMbCrAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] += pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }
}



Void
YuvMbBuffer::clip()
{
  Int   y, x;
  Int   iStride = getLStride  ();
  XPel* pDes    = getMbLumAddr();

  for( y = 0; y < 16; y++ )
  {
    for( x = 0; x < 16; x++ )   pDes[x] = gClip( pDes[x] );
    pDes += iStride;
  }

  iStride = getCStride  ();
  pDes    = getMbCbAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] = gClip( pDes[x] );
    pDes += iStride;
  }

  pDes    = getMbCrAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] = gClip( pDes[x] );
    pDes += iStride;
  }
}


Bool
YuvMbBuffer::isZero()
{
  Int   x, y;
  XPel* pPel    = getMbLumAddr();
  Int   iStride = getLStride  ();

  for( y = 0; y < 16; y++ )
  {
    for( x = 0; x < 16; x++ )
    {
      if( pPel[x] )
      {
        return false;
      }
    }
    pPel += iStride;
  }

  pPel    = getMbCbAddr ();
  iStride = getCStride  ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      if( pPel[x] )
      {
        return false;
      }
    }
    pPel += iStride;
  }

  pPel    = getMbCrAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      if( pPel[x] )
      {
        return false;
      }
    }
    pPel += iStride;
  }

  return true;
}

Void
YuvMbBuffer::subtract( YuvMbBuffer& rcIntYuvMbBuffer )
{
  Int   y, x;
  Int   iStride = getLStride  ();
  XPel* pSrc    = rcIntYuvMbBuffer.getMbLumAddr();
  XPel* pDes    = getMbLumAddr();

  for( y = 0; y < 16; y++ )
  {
    for( x = 0; x < 16; x++ )
      pDes[x] -= pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }

  iStride = getCStride  ();
  pSrc    = rcIntYuvMbBuffer.getMbCbAddr ();
  pDes    = getMbCbAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )
      pDes[x] -= pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }

  pSrc    = rcIntYuvMbBuffer.getMbCrAddr ();
  pDes    = getMbCrAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )
      pDes[x] -= pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }
}




Void YuvMbBuffer::loadChroma( YuvMbBuffer& rcSrcBuffer )
{
  const Int iStride = getCStride();
  XPel*     pDes    = getMbCbAddr();
  XPel*     pSrc    = rcSrcBuffer.getMbCbAddr();
  Int       y;

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }

  pDes = getMbCrAddr();
  pSrc = rcSrcBuffer.getMbCrAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }
}


Void YuvMbBuffer::loadLuma( YuvMbBuffer& rcSrcBuffer, LumaIdx c4x4Idx )
{
  const Int iStride = getLStride();
  XPel*     pDes    = getYBlk( c4x4Idx );
  XPel*     pSrc    = rcSrcBuffer.getYBlk( c4x4Idx );

  for( Int y = 0; y < 4; y++ )
  {
    ::memcpy( pDes, pSrc, 4 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }
}


Void YuvMbBuffer::loadLuma( YuvMbBuffer& rcSrcBuffer, B8x8Idx c8x8Idx )
{
  const Int iStride = getLStride();
  XPel*     pDes = getYBlk( c8x8Idx );
  XPel*     pSrc = rcSrcBuffer.getYBlk( c8x8Idx );

  for( Int y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }
}


Void YuvMbBuffer::loadLuma( YuvMbBuffer& rcSrcBuffer )
{
  const Int iStride = getLStride();
  XPel*     pDes = getMbLumAddr();
  XPel*     pSrc = rcSrcBuffer.getMbLumAddr();

  for( Int y = 0; y < 16; y++ )
  {
    ::memcpy( pDes, pSrc, 16 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }
}


Void YuvMbBuffer::setAllSamplesToZero()
{
  Int   y;
  XPel* pPel    = getMbLumAddr();
  Int   iStride = getLStride();

  for( y = 0; y < 16; y++ )
  {
    ::memset( pPel, 0x00, 16 * sizeof(XPel) );
    pPel += iStride;
  }

  pPel    = getMbCbAddr();
  iStride = getCStride();

  for( y = 0; y < 8; y++ )
  {
    ::memset( pPel, 0x00, 8 * sizeof(XPel) );
    pPel += iStride;
  }

  pPel    = getMbCrAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memset( pPel, 0x00, 8 * sizeof(XPel) );
    pPel += iStride;
  }
}


Void YuvMbBufferExtension::loadSurrounding( YuvPicBuffer* pcSrcBuffer )
{
  Int x, y;
  Int iDesStride = getLStride();
  Int iSrcStride = pcSrcBuffer->getLStride();
  XPel*     pSrc = pcSrcBuffer->getMbLumAddr();
  XPel*     pDes = getMbLumAddr();

  for( x = 0; x < 18; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < 16; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[16] = pSrc[16];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 18; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }

  iDesStride  = getCStride();
  iSrcStride  = pcSrcBuffer->getCStride();
  pSrc        = pcSrcBuffer->getMbCbAddr();
  pDes        = getMbCbAddr();

  for( x = 0; x < 10; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < 8; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[8]  = pSrc[8];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 10; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }

  pSrc        = pcSrcBuffer->getMbCrAddr();
  pDes        = getMbCrAddr();

  for( x = 0; x < 10; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < 8; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[8]  = pSrc[8];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 10; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }
}

//TMM_INTERLACE {
Void YuvMbBufferExtension::loadSurrounding_MbAff( YuvPicBuffer* pcSrcBuffer, UInt uiMask )
{
  Int   x, y;
  Bool  bTopIntra     = ( ( uiMask & 0x020 ) != 0 );
  Bool  bBotIntra     = ( ( uiMask & 0x040 ) != 0 );
  Int   iYSizeLuma    = ( bTopIntra || bBotIntra ? 8 : 16 );
  Int   iYSizeChroma  = iYSizeLuma >> 1;

  Int   iSrcStride    = pcSrcBuffer->getLStride();
  Int   iDesStride    = getLStride();
  XPel* pSrc          = pcSrcBuffer->getMbLumAddr() + ( bTopIntra ? iYSizeLuma * iSrcStride : 0 );
  XPel* pDes          = getMbLumAddr()              + ( bTopIntra ? iYSizeLuma * iDesStride : 0 );

  for( x = 0; x < 18; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < iYSizeLuma; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[16] = pSrc[16];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 18; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }

  iSrcStride  = pcSrcBuffer->getCStride();
  iDesStride  = getCStride();
  pSrc        = pcSrcBuffer->getMbCbAddr() + ( bTopIntra ? iYSizeChroma * iSrcStride : 0 );
  pDes        = getMbCbAddr()              + ( bTopIntra ? iYSizeChroma * iDesStride : 0 );
   
  for( x = 0; x < 10; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < iYSizeChroma; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[ 8] = pSrc[ 8];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 10; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }

  pSrc  = pcSrcBuffer->getMbCrAddr() + ( bTopIntra ? iYSizeChroma * iSrcStride : 0 );
  pDes  = getMbCrAddr()              + ( bTopIntra ? iYSizeChroma * iDesStride : 0 );

  for( x = 0; x < 10; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < iYSizeChroma; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[ 8] = pSrc[ 8];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
 for( x = 0; x < 10; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }
}

//TMM_INTERLACE }




Void YuvMbBufferExtension::xMerge( Int xDir, Int yDir, Int iSize, XPel* puc, Int iStride, Bool bCornerMbPresent, Bool bHalfYSize )
{
  XPel  pPelH[9];
  XPel  pPelV[9];
  Int   iXSize  = iSize;
  Int   iYSize  = ( bHalfYSize ? iSize >> 1 : iSize );
  Int   iAdd    = 1;
  Int   x, y, xo;

  if( yDir < 0 )	
  {
    puc    +=  iStride * ( iSize - 1 );
    iStride = -iStride; 
  }
  if( xDir < 0 )	
  {
    puc    += ( iSize - 1);
    iAdd    = -1;
  }

  for( x = 0; x <= iXSize; x++ )
  {
    pPelH[x] = puc[(x-1)*iAdd - iStride];
  }	
  for( y = 0; y <= iYSize; y++ )
  {
    pPelV[y] = puc[(y-1)*iStride - iAdd];
  }	

  if( ! bCornerMbPresent )
  {
    pPelV[0] = pPelH[0] = ( pPelH[1] + pPelV[1] + 1 ) >> 1;
  }

  for( y = 0; y < iYSize; y++, puc += iStride )
  {
    for( xo = 0, x = 0; x < iXSize; x++, xo += iAdd )
    {
      const Int iOffset = x-y;
      
      if( iOffset > 0 )
      {
        puc[xo] = ( pPelH[ iOffset-1] + 2*pPelH[ iOffset] + pPelH[ iOffset+1] + 2 ) >> 2;
      }
      else if( iOffset < 0 )
      {
        puc[xo] = ( pPelV[-iOffset-1] + 2*pPelV[-iOffset] + pPelV[-iOffset+1] + 2 ) >> 2;
      }
      else
      {
        puc[xo] = ( pPelH[1] + 2*pPelV[0] + pPelV[1] + 2 ) >> 2;
      }
    }
  }
}

Void YuvMbBufferExtension::mergeFromLeftAbove ( LumaIdx cIdx, Bool bCornerMbPresent, Bool bHalfYSize )
{
  xMerge(  1,  1, 8, getYBlk( cIdx ), getLStride(), bCornerMbPresent, bHalfYSize );
  xMerge(  1,  1, 4, getUBlk( cIdx ), getCStride(), bCornerMbPresent, bHalfYSize );
  xMerge(  1,  1, 4, getVBlk( cIdx ), getCStride(), bCornerMbPresent, bHalfYSize );
}

Void YuvMbBufferExtension::mergeFromRightBelow( LumaIdx cIdx, Bool bCornerMbPresent, Bool bHalfYSize )
{
  xMerge( -1, -1, 8, getYBlk( cIdx ), getLStride(), bCornerMbPresent, bHalfYSize );
  xMerge( -1, -1, 4, getUBlk( cIdx ), getCStride(), bCornerMbPresent, bHalfYSize );
  xMerge( -1, -1, 4, getVBlk( cIdx ), getCStride(), bCornerMbPresent, bHalfYSize );
}

Void YuvMbBufferExtension::mergeFromRightAbove( LumaIdx cIdx, Bool bCornerMbPresent, Bool bHalfYSize )
{
  xMerge( -1,  1, 8, getYBlk( cIdx ), getLStride(), bCornerMbPresent, bHalfYSize );
  xMerge( -1,  1, 4, getUBlk( cIdx ), getCStride(), bCornerMbPresent, bHalfYSize );
  xMerge( -1,  1, 4, getVBlk( cIdx ), getCStride(), bCornerMbPresent, bHalfYSize );
}

Void YuvMbBufferExtension::mergeFromLeftBelow ( LumaIdx cIdx, Bool bCornerMbPresent, Bool bHalfYSize )
{
  xMerge(  1, -1, 8, getYBlk( cIdx ), getLStride(), bCornerMbPresent, bHalfYSize );
  xMerge(  1, -1, 4, getUBlk( cIdx ), getCStride(), bCornerMbPresent, bHalfYSize );
  xMerge(  1, -1, 4, getVBlk( cIdx ), getCStride(), bCornerMbPresent, bHalfYSize );
}

Void YuvMbBufferExtension::copyFromBelow      ( LumaIdx cIdx, Bool bHalfYSize )
{
  Int   iYSize  = ( bHalfYSize ? 4 : 8 );
  XPel* pPel    = getYBlk( cIdx );
  Int   iStride = getLStride();
  Int   y;

  pPel += 8*iStride;
  for( y = 0; y < iYSize; y++ )
  {
    ::memcpy( pPel-iStride, pPel, 8 * sizeof(XPel) );
    pPel -= iStride;
  }

  pPel    = getUBlk( cIdx );
  iStride = getCStride();
  iYSize  = iYSize >> 1;

  pPel += 4*iStride;
  for( y = 0; y < iYSize; y++ )
  {
    ::memcpy( pPel-iStride, pPel, 4 * sizeof(XPel) );
    pPel -= iStride;
  }

  pPel    = getVBlk( cIdx );

  pPel += 4*iStride;
  for( y = 0; y < iYSize; y++ )
  {
    ::memcpy( pPel-iStride, pPel, 4 * sizeof(XPel) );
    pPel -= iStride;
  }
}

Void YuvMbBufferExtension::copyFromLeft       ( LumaIdx cIdx )
{
  XPel* pPel    = getYBlk( cIdx );
  Int   iStride = getLStride();
  Int   x, y;

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      pPel[x] = pPel[-1];
    }
    pPel += iStride;
  }

  pPel    = getUBlk( cIdx );
  iStride = getCStride();

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = pPel[-1];
    }
    pPel += iStride;
  }

  pPel    = getVBlk( cIdx );

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = pPel[-1];
    }
    pPel += iStride;
  }
}

Void YuvMbBufferExtension::copyFromAbove      ( LumaIdx cIdx, Bool bHalfYSize )
{
  Int   iYSize  = ( bHalfYSize ? 4 : 8 );
  XPel* pPel    = getYBlk( cIdx );
  Int   iStride = getLStride();
  Int   y;

  for( y = 0; y < iYSize; y++ )
  {
    ::memcpy( pPel, pPel-iStride, 8 * sizeof(XPel) );
    pPel += iStride;
  }

  pPel    = getUBlk( cIdx );
  iStride = getCStride();
  iYSize  = iYSize >> 1;

  for( y = 0; y < iYSize; y++ )
  {
    ::memcpy( pPel, pPel-iStride, 4 * sizeof(XPel) );
    pPel += iStride;
  }

  pPel    = getVBlk( cIdx );

  for( y = 0; y < iYSize; y++ )
  {
    ::memcpy( pPel, pPel-iStride, 4 * sizeof(XPel) );
    pPel += iStride;
  }
}

Void YuvMbBufferExtension::copyFromRight      ( LumaIdx cIdx )
{
  XPel* pPel    = getYBlk( cIdx );
  Int   iStride = getLStride();
  Int   y,x;

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      pPel[x] = pPel[8];
    }
    pPel += iStride;
  }

  pPel    = getUBlk( cIdx );
  iStride = getCStride();

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = pPel[4];
    }
    pPel += iStride;
  }

  pPel    = getVBlk( cIdx );

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = pPel[4];
    }
    pPel += iStride;
  }
}

Void YuvMbBufferExtension::xFill( LumaIdx cIdx, XPel cY, XPel cU, XPel cV, Bool bHalfYSize, Bool bLowerHalf )
{
  AOT( !bHalfYSize && bLowerHalf );
  Int   iYSize  = ( bHalfYSize ? 4 : 8 );
  Int   iStride = getLStride();
  XPel* pPel    = getYBlk( cIdx ) + ( bLowerHalf ? 4*iStride : 0 );
  Int   x, y;

  for( y = 0; y < iYSize; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      pPel[x] = cY;
    }
    pPel += iStride;
  }

  iYSize  = ( bHalfYSize ? 2 : 4 );
  iStride = getCStride();
  pPel    = getUBlk( cIdx ) + ( bLowerHalf ? 2*iStride : 0 );

  for( y = 0; y < iYSize; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = cU;
    }
    pPel += iStride;
  }

  pPel    = getVBlk( cIdx ) + ( bLowerHalf ? 2*iStride : 0 );

  for( y = 0; y < iYSize; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = cV;
    }
    pPel += iStride;
  }
}

Void YuvMbBufferExtension::copyFromLeftAbove  ( LumaIdx cIdx, Bool bHalfYSize )
{
  XPel cY = *(getYBlk( cIdx ) - 1 - getLStride());
  XPel cU = *(getUBlk( cIdx ) - 1 - getCStride());
  XPel cV = *(getVBlk( cIdx ) - 1 - getCStride());

  xFill( cIdx, cY, cU, cV, bHalfYSize, false );
}

Void YuvMbBufferExtension::copyFromRightAbove ( LumaIdx cIdx, Bool bHalfYSize )
{
  XPel cY = *(getYBlk( cIdx ) + 8 - getLStride());
  XPel cU = *(getUBlk( cIdx ) + 4 - getCStride());
  XPel cV = *(getVBlk( cIdx ) + 4 - getCStride());

  xFill( cIdx, cY, cU, cV, bHalfYSize, false );
}

Void YuvMbBufferExtension::copyFromLeftBelow  ( LumaIdx cIdx, Bool bHalfYSize )
{
  XPel cY = *(getYBlk( cIdx ) - 1 + 8 * getLStride());
  XPel cU = *(getUBlk( cIdx ) - 1 + 4 * getCStride());
  XPel cV = *(getVBlk( cIdx ) - 1 + 4 * getCStride());

  xFill( cIdx, cY, cU, cV, bHalfYSize, bHalfYSize );
}

Void YuvMbBufferExtension::copyFromRightBelow ( LumaIdx cIdx, Bool bHalfYSize )
{
  XPel cY = *(getYBlk( cIdx ) + 8 + 8 * getLStride());
  XPel cU = *(getUBlk( cIdx ) + 4 + 4 * getCStride());
  XPel cV = *(getVBlk( cIdx ) + 4 + 4 * getCStride());

  xFill( cIdx, cY, cU, cV, bHalfYSize, bHalfYSize );
}



H264AVC_NAMESPACE_END

