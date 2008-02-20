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
#include "H264AVCCommonLib/Tables.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/ControlMngIf.h"
#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/Frame.h"

#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN

const UChar LoopFilter::g_aucBetaTab[52]  =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  2,  2,  2,  3,
    3,  3,  3,  4,  4,  4,  6,  6,  7,  7,
    8,  8,  9,  9, 10, 10, 11, 11, 12, 12,
   13, 13, 14, 14, 15, 15, 16, 16, 17, 17,
   18, 18
};

const LoopFilter::AlphaClip LoopFilter::g_acAlphaClip[52] =
{
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },

  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  4, { 0, 0, 0, 0, 0} },
  {  4, { 0, 0, 0, 1, 1} },
  {  5, { 0, 0, 0, 1, 1} },
  {  6, { 0, 0, 0, 1, 1} },

  {  7, { 0, 0, 0, 1, 1} },
  {  8, { 0, 0, 1, 1, 1} },
  {  9, { 0, 0, 1, 1, 1} },
  { 10, { 0, 1, 1, 1, 1} },
  { 12, { 0, 1, 1, 1, 1} },
  { 13, { 0, 1, 1, 1, 1} },
  { 15, { 0, 1, 1, 1, 1} },
  { 17, { 0, 1, 1, 2, 2} },
  { 20, { 0, 1, 1, 2, 2} },
  { 22, { 0, 1, 1, 2, 2} },

  { 25, { 0, 1, 1, 2, 2} },
  { 28, { 0, 1, 2, 3, 3} },
  { 32, { 0, 1, 2, 3, 3} },
  { 36, { 0, 2, 2, 3, 3} },
  { 40, { 0, 2, 2, 4, 4} },
  { 45, { 0, 2, 3, 4, 4} },
  { 50, { 0, 2, 3, 4, 4} },
  { 56, { 0, 3, 3, 5, 5} },
  { 63, { 0, 3, 4, 6, 6} },
  { 71, { 0, 3, 4, 6, 6} },

  { 80, { 0, 4, 5, 7, 7} },
  { 90, { 0, 4, 5, 8, 8} },
 { 101, { 0, 4, 6, 9, 9} },
 { 113, { 0, 5, 7,10,10} },
 { 127, { 0, 6, 8,11,11} },
 { 144, { 0, 6, 8,13,13} },
 { 162, { 0, 7,10,14,14} },
 { 182, { 0, 8,11,16,16} },
 { 203, { 0, 9,12,18,18} },
 { 226, { 0,10,13,20,20} },

 { 255, { 0,11,15,23,23} },
 { 255, { 0,13,17,25,25} }
} ;



LoopFilter::LoopFilter() 
: m_pcControlMngIf        ( 0 )
, m_pcReconstructionBypass( 0 )
{
}

LoopFilter::~LoopFilter()
{
}

ErrVal
LoopFilter::create( LoopFilter*& rpcLoopFilter )
{
  rpcLoopFilter = new LoopFilter;
  ROF( rpcLoopFilter );
  return Err::m_nOK;
}

ErrVal
LoopFilter::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal
LoopFilter::init( ControlMngIf*         pcControlMngIf,
                  ReconstructionBypass* pcReconstructionBypass,
                  Bool                  bEncoder )
{
  ROF( pcControlMngIf );
  ROF( pcReconstructionBypass );

  m_pcControlMngIf          = pcControlMngIf;
  m_pcReconstructionBypass  = pcReconstructionBypass;
  m_bEncoder                = bEncoder;
  return Err::m_nOK;
}

ErrVal
LoopFilter::uninit()
{
  m_pcControlMngIf          = 0;
  m_pcReconstructionBypass  = 0;
  return Err::m_nOK;
}


ErrVal
LoopFilter::process( SliceHeader& rcSH,
                     Frame*       pcFrame,
                     Frame*       pcResidual,
                     MbDataCtrl*  pcMbDataCtrl,
                     Bool         bInterLayerFlag,
                     Bool         bSpatialScalabilityFlag )
{
  ROF ( m_pcControlMngIf );
  ROF ( pcFrame );
  ROF ( pcMbDataCtrl );

  RNOK( m_pcControlMngIf->initSliceForFiltering ( rcSH ) );
  RNOK( pcMbDataCtrl    ->initSlice             ( rcSH, POST_PROCESS, false, NULL ) );
  m_bVerMixedMode = false;
  m_bHorMixedMode = false;

  Frame* apcFrame   [4] = { NULL, NULL, NULL, NULL };
  Frame* apcResidual[4] = { NULL, NULL, NULL, NULL };
  RNOK( pcFrame->addFrameFieldBuffer() );
  apcFrame[ TOP_FIELD ] = pcFrame->getPic( TOP_FIELD );
  apcFrame[ BOT_FIELD ] = pcFrame->getPic( BOT_FIELD );
  apcFrame[ FRAME     ] = pcFrame->getPic( FRAME     );
  if( pcResidual )
  {
    RNOK( pcResidual->addFrameFieldBuffer() );
    apcResidual [ TOP_FIELD ] = pcResidual->getPic( TOP_FIELD );
    apcResidual [ BOT_FIELD ] = pcResidual->getPic( BOT_FIELD );
    apcResidual [ FRAME     ] = pcResidual->getPic( FRAME     );
  }


  //===== filtering =====
  const DBFilterParameter&  rcDFP       = ( bInterLayerFlag ? rcSH.getInterLayerDeblockingFilterParameter() : rcSH.getDeblockingFilterParameter() );
  Int                       iFilterIdc  = rcDFP.getDisableDeblockingFilterIdc();
  LFPass                    eNumPasses  = ( iFilterIdc == 3 || iFilterIdc == 6 ? TWO_PASSES : ONE_PASS ); 

  for( LFPass eLFPass = FIRST_PASS; eLFPass < eNumPasses; eLFPass = LFPass( eLFPass + 1 ) )
  {
    for( UInt uiMbAddress = 0; uiMbAddress < rcSH.getMbInPic(); uiMbAddress++ )
    {
      MbDataAccess* pcMbDataAccess = 0;
      UInt          uiMbY, uiMbX;
      rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress );

      RNOK( pcMbDataCtrl    ->initMb            (  pcMbDataAccess, uiMbY, uiMbX ) );
      RNOK( m_pcControlMngIf->initMbForFiltering( *pcMbDataAccess, uiMbY, uiMbX, rcSH.isMbaffFrame() ) );
      PicType       eMbPicType        = pcMbDataAccess->getMbPicType();
      YuvPicBuffer* pcFrameBuffer     = apcFrame[ eMbPicType ]->getFullPelYuvBuffer();
      YuvPicBuffer* pcResidualBuffer  = ( pcResidual ? apcResidual[ eMbPicType ]->getFullPelYuvBuffer() : 0 );

      RNOK( xFilterMb( *pcMbDataAccess, pcFrameBuffer, pcResidualBuffer, bInterLayerFlag, bSpatialScalabilityFlag, eLFPass ) );
    }
  }
  ROFRS ( bInterLayerFlag, Err::m_nOK );

  RNOK  ( xPadding( rcSH, pcMbDataCtrl, apcFrame ) );
  return Err::m_nOK;
}


ErrVal
LoopFilter::xPadding( SliceHeader&  rcSH,
                      MbDataCtrl*   pcMbDataCtrl,
                      Frame*        apcFrame[] )
{
  Bool bMbAff = rcSH.isMbaffFrame();

  for( UInt uiMbAddress = 0; uiMbAddress < rcSH.getMbInPic(); uiMbAddress++ )
  {
    MbDataAccess* pcMbDataAccess = 0;
    UInt          uiMbY, uiMbX;
    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress );

    RNOK( pcMbDataCtrl    ->initMb            (  pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcControlMngIf->initMbForFiltering( *pcMbDataAccess, uiMbY, uiMbX, bMbAff ) );
    UInt  uiMask = 0;

    if( bMbAff )
    {
      RNOK( pcMbDataCtrl->getBoundaryMask_MbAff( uiMbY, uiMbX, uiMask ) );
      if( uiMask )
      {
        PicType                 eMbPicType  = ( ( uiMbY % 2 ) ? BOT_FIELD : TOP_FIELD );
        YuvPicBuffer*           pcPicBuffer = apcFrame[ eMbPicType ]->getFullPelYuvBuffer();
        IntYuvMbBufferExtension cBuffer;
        cBuffer.setAllSamplesToZero   ();
        cBuffer.loadSurrounding_MbAff ( pcPicBuffer,  uiMask );
        RNOK( m_pcReconstructionBypass->padRecMb_MbAff( &cBuffer, uiMask ));
        pcPicBuffer->loadBuffer_MbAff ( &cBuffer,     uiMask );
      }
    }
    else
    {
      RNOK( pcMbDataCtrl->getBoundaryMask( uiMbY, uiMbX, uiMask ) );
      if( uiMask )
      {
        PicType                 eMbPicType  = pcMbDataAccess->getMbPicType();
        YuvPicBuffer*           pcPicBuffer = apcFrame[ eMbPicType ]->getFullPelYuvBuffer();
        IntYuvMbBufferExtension cBuffer;
        cBuffer.setAllSamplesToZero ();
        cBuffer.loadSurrounding     ( pcPicBuffer );
        RNOK( m_pcReconstructionBypass->padRecMb( &cBuffer, uiMask ));
        pcPicBuffer->loadBuffer     ( &cBuffer );
      }
    }
  }

  return Err::m_nOK;
}



ErrVal
LoopFilter::xFilterMb( MbDataAccess& rcMbDataAccess, YuvPicBuffer* pcYuvBuffer, YuvPicBuffer* pcResidual, Bool bInterLayerFlag, Bool bSpatialScalableFlag, LFPass eLFPass )
{
  RNOK( xRecalcCBP( rcMbDataAccess ) );

  const DBFilterParameter&  rcDFP       = ( bInterLayerFlag ? rcMbDataAccess.getSH().getInterLayerDeblockingFilterParameter() : rcMbDataAccess.getSH().getDeblockingFilterParameter() );
  Int                       iFilterIdc  = rcDFP.getDisableDeblockingFilterIdc();
  Bool                      b8x8        = rcMbDataAccess.getMbData().isTransformSize8x8();

  ROTRS( iFilterIdc == 1,                                           Err::m_nOK );
  ROTRS( bInterLayerFlag && ! rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  //===== check residual blocks and set "residual CBP" =====
  UInt uiCbp = 0;
  if( pcResidual )
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      uiCbp += ( ( pcResidual->isCurr4x4BlkNotZero( cIdx ) ? 1 : 0 ) << cIdx.b4x4() );
    }
  }
  rcMbDataAccess.getMbData().setMbCbpResidual( uiCbp );

  //===== set mode parameters =====
  Bool bFieldFlag   = rcMbDataAccess.getMbData().getFieldFlag();
  Bool bCurrFrame   = ( rcMbDataAccess.getMbPicType() == FRAME );
  m_bVerMixedMode   = ( bFieldFlag != rcMbDataAccess.getMbDataLeft().getFieldFlag() );
  if( bFieldFlag )
  {
    m_bHorMixedMode = ( bFieldFlag != rcMbDataAccess.getMbDataAboveAbove().getFieldFlag() );
  }
  else
  {
    m_bHorMixedMode = ( bFieldFlag != rcMbDataAccess.getMbDataAbove().getFieldFlag() );
  }
  
  //===== determine boundary filter strength =====
  m_bAddEdge        = true;
  if( m_bHorMixedMode && bCurrFrame )
  {
    for( B4x4Idx cIdx; cIdx.b4x4() < 4; cIdx++ )
    {
      m_aucBsHorTop[cIdx.x()] = xGetHorFilterStrength( rcMbDataAccess, cIdx, iFilterIdc, bInterLayerFlag, bSpatialScalableFlag, eLFPass );
    }
  }
  if( m_bVerMixedMode )
  {
    for( B4x4Idx cIdx; cIdx.b4x4() < 16; cIdx = B4x4Idx( cIdx + 4 ) )
    {
      m_aucBsVerBot[cIdx.y()] = xGetVerFilterStrength( rcMbDataAccess, cIdx, iFilterIdc, bInterLayerFlag, bSpatialScalableFlag, eLFPass );
    }
  }
  m_bAddEdge = false;
  for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    if( !b8x8 || ( ( cIdx.x() & 1 ) == 0 ) )
    {
      m_aaaucBs[VER][cIdx.x()][cIdx.y()]  = xGetVerFilterStrength( rcMbDataAccess, cIdx, iFilterIdc, bInterLayerFlag, bSpatialScalableFlag, eLFPass );
    }
    else
    {
      m_aaaucBs[VER][cIdx.x()][cIdx.y()]  = 0;
    }
    if( !b8x8 || ( ( cIdx.y() & 1 ) == 0 ) )
    {
      m_aaaucBs[HOR][cIdx.x()][cIdx.y()]  = xGetHorFilterStrength( rcMbDataAccess, cIdx, iFilterIdc, bInterLayerFlag, bSpatialScalableFlag, eLFPass );
    }
    else
    {
      m_aaaucBs[HOR][cIdx.x()][cIdx.y()]  = 0;
    }
  }

  //===== filtering =====
  m_bHorMixedMode = m_bHorMixedMode && bCurrFrame;
  RNOK  ( xLumaVerFiltering   ( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  RNOK  ( xLumaHorFiltering   ( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  ROTRS ( iFilterIdc > 3, Err::m_nOK );
	RNOK  ( xChromaVerFiltering ( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
	RNOK  ( xChromaHorFiltering ( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  return Err::m_nOK;
}


ErrVal
LoopFilter::xRecalcCBP( MbDataAccess &rcMbDataAccess )
{
  UInt uiCbp    = 0;
  UInt uiStart  = 0;
  UInt uiStop   = 16;
  if( rcMbDataAccess.getMbData().isTransformSize8x8() )
  {
    const UChar *pucScan = rcMbDataAccess.getMbData().getFieldFlag() ? g_aucFieldScan64 : g_aucFrameScan64;
    uiStart <<= 2;
    uiStop  <<= 2;
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      TCoeff *piCoeff = rcMbDataAccess.getMbTCoeffs().get8x8( cIdx );
      for( UInt ui = uiStart; ui < uiStop; ui++ )
      {
        if( m_bEncoder ? piCoeff[pucScan[ui]].getLevel() : piCoeff[pucScan[ui]].getCoeff() )
        {
          uiCbp |= 0x33 << cIdx.b8x8();
          break;
        }
      }
    }
  }
  else
  {
    const UChar *pucScan = rcMbDataAccess.getMbData().getFieldFlag() ? g_aucFieldScan : g_aucFrameScan;
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      TCoeff *piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );
      for( UInt ui = uiStart; ui < uiStop; ui++ )
      {
        if( m_bEncoder ? piCoeff[pucScan[ui]].getLevel() : piCoeff[pucScan[ui]].getCoeff() )
        {
          uiCbp |= 1<<cIdx.b4x4();
          break;
        }
      }
    }
    if( rcMbDataAccess.getMbData().isIntra16x16() )
    {
      uiCbp = uiCbp ? 0xFFFF : 0;
    }
  }
  rcMbDataAccess.getMbData().setAndConvertMbExtCbp( uiCbp );
  return Err::m_nOK;
}


UInt
LoopFilter::xGetHorFilterStrength ( const MbDataAccess& rcMbDataAccess,
                                    LumaIdx             cIdx,
                                    Int                 iFilterIdc,
                                    Bool                bInterLayerFlag,
                                    Bool                bSpatialScalableFlag,
                                    LFPass              eLFPass )
{
  Short sHorMvThr     = 4;
  Short sVerMvThr     = ( rcMbDataAccess.getMbPicType() == FRAME ? 4 : 2 );
  Bool  bFilterInside = xFilterInsideEdges( rcMbDataAccess, iFilterIdc, bInterLayerFlag, eLFPass );
  Bool  bFilterTop    = xFilterTopEdge    ( rcMbDataAccess, iFilterIdc, bInterLayerFlag, eLFPass );
  ROFRS( ( ! cIdx.y() && bFilterTop ) || ( cIdx.y() && bFilterInside ), 0 );


  //--------------------------
  //---   INTERNAL EDGES   ---
  //--------------------------
  if( cIdx.y() ) 
  {
    const MbData& rcMbDataCurr = rcMbDataAccess.getMbData();

    //===== check special condition for I_BL in spatial scalable coding =====
    if( bSpatialScalableFlag && rcMbDataCurr.isIntraBL() )
    {
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                           ), 1 ); // only coefficients of the current layer are counted
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 1 ); // only coefficients of the current layer are counted
      return 0;
    }

    //===== check for intra =====
    ROTRS( rcMbDataCurr.isIntra(), 3 );

    //===== check for transform coefficients and residual samples =====
    ROTRS( rcMbDataCurr.is4x4BlkResidual( cIdx                           ), 2 );
    ROTRS( rcMbDataCurr.is4x4BlkResidual( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 ); 
    ROTRS( rcMbDataCurr.is4x4BlkCoded   ( cIdx                           ), 2 );
    ROTRS( rcMbDataCurr.is4x4BlkCoded   ( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 );

    //===== check for motion vectors ====
    if( rcMbDataCurr.isInterPMb() )
    {
      return xCheckMvDataP( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
    }
    return   xCheckMvDataB( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
  }


  //-------------------------
  //---   EXTERNAL EDGE   ---
  //-------------------------
  const MbData& rcMbDataCurr  = rcMbDataAccess.getMbData();
  const MbData& rcMbDataAbove = xGetMbDataAbove( rcMbDataAccess ); 

  //===== check special condition for inter-layer deblocking =====
  if( bInterLayerFlag )
  {
    ROFRS( rcMbDataCurr .isIntra(),  0 ); // redundant - just for clarity
    ROFRS( rcMbDataAbove.isIntra(),  0 );
  }

  //===== check special condition for I_BL in spatial scalable coding =====
  if( bSpatialScalableFlag && ( rcMbDataCurr.isIntraBL() || rcMbDataAbove.isIntraBL() ) )
  {
    ROTRS( rcMbDataCurr .isIntraButnotIBL(),  4 );
    ROTRS( rcMbDataAbove.isIntraButnotIBL(),  4 );
    if( rcMbDataCurr.isIntraBL() && rcMbDataAbove.isIntraBL() )
    {
      ROTRS( rcMbDataCurr .is4x4BlkCoded( cIdx                            ), 1 ); // only coefficients of the current layer are counted
      ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 1 ); // only coefficients of the current layer are counted
      return 0;
    }
    ROTRS( ! rcMbDataCurr .isIntra() && rcMbDataCurr .is4x4BlkResidual( cIdx                            ), 2 ); 
    ROTRS( ! rcMbDataAbove.isIntra() && rcMbDataAbove.is4x4BlkResidual( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 ); 
    return 1;
  }

  //===== check for intra =====
  UInt   bIntraBs = ( ! m_bHorMixedMode && rcMbDataAccess.getMbPicType() == FRAME ? 4 : 3 );
  ROTRS( rcMbDataCurr .isIntra(), bIntraBs ); 
  ROTRS( rcMbDataAbove.isIntra(), bIntraBs ); 

  //===== check for transform coefficients and residual samples =====
  ROTRS( rcMbDataCurr. is4x4BlkResidual ( cIdx                            ), 2 );
  ROTRS( rcMbDataAbove.is4x4BlkResidual ( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 ); 
  ROTRS( rcMbDataCurr. is4x4BlkCoded    ( cIdx                            ), 2 );
  ROTRS( rcMbDataAbove.is4x4BlkCoded    ( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );

  //===== check for mixed mode =====
  ROTRS( m_bHorMixedMode, 1 );

  //===== check for motion vectors ====
  if( rcMbDataCurr.isInterPMb() && rcMbDataAbove.isInterPMb() )
  {
    return xCheckMvDataP( rcMbDataCurr, cIdx, rcMbDataAbove, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
  }
  return   xCheckMvDataB( rcMbDataCurr, cIdx, rcMbDataAbove, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
}


UInt
LoopFilter::xGetVerFilterStrength( const MbDataAccess&  rcMbDataAccess,
                                   LumaIdx              cIdx,
                                   Int                  iFilterIdc,
                                   Bool                 bInterLayerFlag,
                                   Bool                 bSpatialScalableFlag,
                                   LFPass               eLFPass )
{
  Short sHorMvThr     = 4;
  Short sVerMvThr     = ( rcMbDataAccess.getMbPicType() == FRAME ? 4 : 2 );
  Bool  bFilterInside = xFilterInsideEdges( rcMbDataAccess, iFilterIdc, bInterLayerFlag, eLFPass );
  Bool  bFilterLeft   = xFilterLeftEdge   ( rcMbDataAccess, iFilterIdc, bInterLayerFlag, eLFPass );
  ROFRS( ( ! cIdx.x() && bFilterLeft ) || ( cIdx.x() && bFilterInside ), 0 );


  //--------------------------
  //---   INTERNAL EDGES   ---
  //--------------------------
  if( cIdx.x() ) 
  {
    const MbData& rcMbDataCurr = rcMbDataAccess.getMbData();

    //===== check special condition for I_BL in spatial scalable coding =====
    if( bSpatialScalableFlag && rcMbDataCurr.isIntraBL() )
    {
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                          ), 1 ); // only coefficients of the current layer are counted
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 1 ); // only coefficients of the current layer are counted
      return 0;
    }

    //===== check for intra =====
    ROTRS( rcMbDataCurr.isIntra(), 3 );

    //===== check for transform coefficients and residual samples =====
    ROTRS( rcMbDataCurr.is4x4BlkResidual( cIdx                          ), 2 );
    ROTRS( rcMbDataCurr.is4x4BlkResidual( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 2 ); 
    ROTRS( rcMbDataCurr.is4x4BlkCoded   ( cIdx                          ), 2 );
    ROTRS( rcMbDataCurr.is4x4BlkCoded   ( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 2 );

    //===== check for motion vectors ====
    if( rcMbDataCurr.isInterPMb() )
    {
      return xCheckMvDataP( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
    }
    return   xCheckMvDataB( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
  }


  //-------------------------
  //---   EXTERNAL EDGE   ---
  //-------------------------
  B4x4Idx       cIdxLeft     = B4x4Idx( cIdx );
  const MbData& rcMbDataCurr = rcMbDataAccess.getMbData();
  const MbData& rcMbDataLeft = xGetMbDataLeft( rcMbDataAccess, cIdxLeft ); 

  //===== check special condition for inter-layer deblocking =====
  if( bInterLayerFlag )
  {
    ROFRS( rcMbDataCurr.isIntra(),  0 ); // redundant - just for clarity
    ROFRS( rcMbDataLeft.isIntra(),  0 );
  }

  //===== check special condition for I_BL in spatial scalable coding =====
  if( bSpatialScalableFlag && ( rcMbDataCurr.isIntraBL() || rcMbDataLeft.isIntraBL() ) )
  {
    ROTRS( rcMbDataCurr.isIntraButnotIBL(),  4 );
    ROTRS( rcMbDataLeft.isIntraButnotIBL(),  4 );
    if( rcMbDataCurr.isIntraBL() && rcMbDataLeft.isIntraBL() )
    {
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx     ), 1 ); // only coefficients of the current layer are counted
      ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdxLeft ), 1 ); // only coefficients of the current layer are counted
      return 0;
    }
    ROTRS( ! rcMbDataCurr.isIntra() && rcMbDataCurr.is4x4BlkResidual( cIdx     ), 2 ); 
    ROTRS( ! rcMbDataLeft.isIntra() && rcMbDataLeft.is4x4BlkResidual( cIdxLeft ), 2 ); 
    return 1;
  }

  //===== check for intra =====
  ROTRS( rcMbDataCurr.isIntra(), 4 ); 
  ROTRS( rcMbDataLeft.isIntra(), 4 ); 

  //===== check for transform coefficients and residual samples =====
  ROTRS( rcMbDataCurr.is4x4BlkResidual ( cIdx     ), 2 );
  ROTRS( rcMbDataLeft.is4x4BlkResidual ( cIdxLeft ), 2 ); 
  ROTRS( rcMbDataCurr.is4x4BlkCoded    ( cIdx     ), 2 );
  ROTRS( rcMbDataLeft.is4x4BlkCoded    ( cIdxLeft ), 2 );

  //===== check for mixed mode =====
  ROTRS( m_bVerMixedMode, 1 );

  //===== check for motion vectors ====
  if( rcMbDataCurr.isInterPMb() && rcMbDataLeft.isInterPMb() )
  {
    return xCheckMvDataP( rcMbDataCurr, cIdx, rcMbDataLeft, cIdxLeft, sHorMvThr, sVerMvThr );
  }
  return   xCheckMvDataB( rcMbDataCurr, cIdx, rcMbDataLeft, cIdxLeft, sHorMvThr, sVerMvThr );
}


const MbData&
LoopFilter::xGetMbDataLeft( const MbDataAccess& rcMbDataAccess, LumaIdx& rcIdx )
{
  if( ! m_bVerMixedMode)
  {
    rcIdx = ( rcIdx + LEFT_MB_LEFT_NEIGHBOUR );
    return rcMbDataAccess.getMbDataLeft();
  }
  if( rcMbDataAccess.getMbPicType() == FRAME )
  {
    rcIdx = (LumaIdx)B4x4Idx( rcMbDataAccess.isTopMb() ? ( rcIdx < 8 ? 3 : 7 ) : ( rcIdx < 8 ? 11 : 15 ) );
    ROTRS(  rcMbDataAccess.isTopMb() &&  m_bAddEdge,  rcMbDataAccess.getMbDataBelowLeft() );
    ROTRS( !rcMbDataAccess.isTopMb() && !m_bAddEdge,  rcMbDataAccess.getMbDataAboveLeft() );
    return                                            rcMbDataAccess.getMbDataLeft();
  }
  Bool bBotHalf = ( rcIdx > 7 );
  rcIdx         = (LumaIdx)B4x4Idx( ( ( rcIdx % 8) << 1 ) + ( m_bAddEdge ? 7 : 3 ) );
  ROTRS(  rcMbDataAccess.isTopMb() &&  bBotHalf,      rcMbDataAccess.getMbDataBelowLeft() );
  ROTRS( !rcMbDataAccess.isTopMb() && !bBotHalf,      rcMbDataAccess.getMbDataAboveLeft() );
  return                                              rcMbDataAccess.getMbDataLeft();
}


const MbData&
LoopFilter::xGetMbDataAbove( const MbDataAccess& rcMbDataAccess )
{
  ROTRS( ! m_bHorMixedMode && rcMbDataAccess.getMbData().getFieldFlag(),  rcMbDataAccess.getMbDataAboveAbove()  );
  ROTRS( ! m_bHorMixedMode,                                               rcMbDataAccess.getMbDataAbove()       );
  ROTRS( rcMbDataAccess.getMbPicType() == FRAME && m_bAddEdge,            rcMbDataAccess.getMbDataAboveAbove()  );
  ROTRS( rcMbDataAccess.getMbPicType() == FRAME,                          rcMbDataAccess.getMbDataAbove()       );
  ROTRS( rcMbDataAccess.isTopMb(),                                        rcMbDataAccess.getMbDataAbove()       );
  return                                                                  rcMbDataAccess.getMbDataAboveAbove();
}


Bool
LoopFilter::xFilterInsideEdges( const MbDataAccess& rcMbDataAccess, Int iFilterIdc, Bool bInterLayerFlag, LFPass eLFPass )
{
  ROTRS( iFilterIdc == 1,                                           false );
  ROTRS( iFilterIdc == 3 && eLFPass == SECOND_PASS,                 false );
  ROTRS( iFilterIdc == 6 && eLFPass == SECOND_PASS,                 false );
  ROTRS( bInterLayerFlag && ! rcMbDataAccess.getMbData().isIntra(), false );
  return true;
}


Bool
LoopFilter::xFilterLeftEdge( const MbDataAccess& rcMbDataAccess, Int iFilterIdc, Bool bInterLayerFlag, LFPass eLFPass )
{
  ROTRS( iFilterIdc == 1,                                           false );
  ROFRS( rcMbDataAccess.isLeftMbExisting(),                         false );
  if(  ! rcMbDataAccess.isAvailableLeft () )
  {
    ROTRS( iFilterIdc == 2,                                         false );
    ROTRS( iFilterIdc == 5,                                         false );
    ROTRS( iFilterIdc == 3 && eLFPass == FIRST_PASS,                false );
    ROTRS( iFilterIdc == 6 && eLFPass == FIRST_PASS,                false );
  }
  else
  {
    ROTRS( iFilterIdc == 3 && eLFPass == SECOND_PASS,               false );
    ROTRS( iFilterIdc == 6 && eLFPass == SECOND_PASS,               false );
  }
  ROTRS( bInterLayerFlag && ! rcMbDataAccess.getMbData().isIntra(), false );
  // NOTE: intra status of neighboring MB must be checked in dependence of the block index (and field mode)
  return true;
}


Bool
LoopFilter::xFilterTopEdge( const MbDataAccess& rcMbDataAccess, Int iFilterIdc, Bool bInterLayerFlag, LFPass eLFPass )
{
  ROTRS( iFilterIdc == 1,                                           false );
  ROFRS( rcMbDataAccess.isAboveMbExisting(),                        false );
  if(  ! rcMbDataAccess.isAvailableAbove () )
  {
    ROTRS( iFilterIdc == 2,                                         false );
    ROTRS( iFilterIdc == 5,                                         false );
    ROTRS( iFilterIdc == 3 && eLFPass == FIRST_PASS,                false );
    ROTRS( iFilterIdc == 6 && eLFPass == FIRST_PASS,                false );
  }
  else
  {
    ROTRS( iFilterIdc == 3 && eLFPass == SECOND_PASS,               false );
    ROTRS( iFilterIdc == 6 && eLFPass == SECOND_PASS,               false );
  }
  ROTRS( bInterLayerFlag && ! rcMbDataAccess.getMbData().isIntra(), false );
  // NOTE: intra status of neighboring MB must be checked in dependence of the block index (and field mode)
  return true;
}


UChar
LoopFilter::xCheckMvDataP( const MbData& rcQMbData,
                           const LumaIdx cQIdx,
                           const MbData& rcPMbData,
                           const LumaIdx cPIdx,
                           const Short   sHorMvThr,
                           const Short   sVerMvThr  )
{
  const MbMotionData& rcMbMotionDataL0Q = rcQMbData.getMbMotionData( LIST_0 );
  const MbMotionData& rcMbMotionDataL0P = rcPMbData.getMbMotionData( LIST_0 );

  //===== check reference pictures =====
  const RefPicIdc&  rcRefPicIdcQ = rcMbMotionDataL0Q.getRefPicIdc( cQIdx );
  const RefPicIdc&  rcRefPicIdcP = rcMbMotionDataL0P.getRefPicIdc( cPIdx );
  ROF   ( rcRefPicIdcQ.isValid() );
  ROF   ( rcRefPicIdcP.isValid() );
  ROTRS ( rcRefPicIdcQ != rcRefPicIdcP, 1 );

  //===== check the motion vector distance =====
  const Mv& cMvQ = rcMbMotionDataL0Q.getMv( cQIdx );
  const Mv& cMvP = rcMbMotionDataL0P.getMv( cPIdx );
  ROTRS( cMvP.getAbsHorDiff( cMvQ ) >= sHorMvThr, 1 );
  ROTRS( cMvP.getAbsVerDiff( cMvQ ) >= sVerMvThr, 1 );
  return 0;
}


UChar
LoopFilter::xCheckMvDataB( const MbData& rcQMbData,
                           const LumaIdx cQIdx,
                           const MbData& rcPMbData,
                           const LumaIdx cPIdx,
                           const Short   sHorMvThr,
                           const Short   sVerMvThr )
{
  const MbMotionData& rcMbMotionDataL0Q = rcQMbData.getMbMotionData( LIST_0 );
  const MbMotionData& rcMbMotionDataL1Q = rcQMbData.getMbMotionData( LIST_1 );
  const MbMotionData& rcMbMotionDataL0P = rcPMbData.getMbMotionData( LIST_0 );
  const MbMotionData& rcMbMotionDataL1P = rcPMbData.getMbMotionData( LIST_1 );

  const RefPicIdc&  rcRefPicIdcL0Q  = rcMbMotionDataL0Q.getRefPicIdc( cQIdx );
  const RefPicIdc&  rcRefPicIdcL1Q  = rcMbMotionDataL1Q.getRefPicIdc( cQIdx );
  const RefPicIdc&  rcRefPicIdcL0P  = rcMbMotionDataL0P.getRefPicIdc( cPIdx );
  const RefPicIdc&  rcRefPicIdcL1P  = rcMbMotionDataL1P.getRefPicIdc( cPIdx );
  UInt              uiNumRefPicQ    = ( rcRefPicIdcL0Q.isValid() ? 1 : 0 ) + ( rcRefPicIdcL1Q.isValid() ? 1 : 0 );
  UInt              uiNumRefPicP    = ( rcRefPicIdcL0P.isValid() ? 1 : 0 ) + ( rcRefPicIdcL1P.isValid() ? 1 : 0 );
  ROF( uiNumRefPicQ );
  ROF( uiNumRefPicP );

  //===== check number of reference pictures =====
  ROTRS( uiNumRefPicQ != uiNumRefPicP, 1 );

  if( uiNumRefPicP == 1 ) // one reference picture
  {
    //====_ check reference pictures =====
    const RefPicIdc& rcRefPicIdcQ = ( rcRefPicIdcL0Q.isValid() ? rcRefPicIdcL0Q : rcRefPicIdcL1Q );
    const RefPicIdc& rcRefPicIdcP = ( rcRefPicIdcL0P.isValid() ? rcRefPicIdcL0P : rcRefPicIdcL1P );
    ROTRS( rcRefPicIdcQ != rcRefPicIdcP, 1 );

    //===== check the motion vector distance =====
    const Mv& cMvQ = ( rcRefPicIdcL0Q.isValid() ? rcMbMotionDataL0Q.getMv( cQIdx ) : rcMbMotionDataL1Q.getMv( cQIdx ) );
    const Mv& cMvP = ( rcRefPicIdcL0P.isValid() ? rcMbMotionDataL0P.getMv( cPIdx ) : rcMbMotionDataL1P.getMv( cPIdx ) );
    ROTRS( cMvP.getAbsHorDiff( cMvQ ) >= sHorMvThr, 1 );
    ROTRS( cMvP.getAbsVerDiff( cMvQ ) >= sVerMvThr, 1 );
    return 0;
  }


  // both ref pic are used for both blocks
  if( rcRefPicIdcL1P != rcRefPicIdcL0P )
  {
    // at least two diff ref pic are in use
    if( rcRefPicIdcL1P != rcRefPicIdcL1Q )
    {
      ROTRS( rcRefPicIdcL1P != rcRefPicIdcL0Q, 1 );
      ROTRS( rcRefPicIdcL0P != rcRefPicIdcL1Q, 1 );

      // rcRefPicL0P == rcRefPicL1Q && rcRefPicL1P == rcRefPicL0Q
      // check the motion vector distance
      const Mv& cMvQ0 = rcMbMotionDataL0Q.getMv( cQIdx );
      const Mv& cMvP0 = rcMbMotionDataL0P.getMv( cPIdx );
      const Mv& cMvQ1 = rcMbMotionDataL1Q.getMv( cQIdx );
      const Mv& cMvP1 = rcMbMotionDataL1P.getMv( cPIdx );
      ROTRS( cMvP0.getAbsHorDiff( cMvQ1 ) >= sHorMvThr, 1 );
      ROTRS( cMvP0.getAbsVerDiff( cMvQ1 ) >= sVerMvThr, 1 );
      ROTRS( cMvP1.getAbsHorDiff( cMvQ0 ) >= sHorMvThr, 1 );
      ROTRS( cMvP1.getAbsVerDiff( cMvQ0 ) >= sVerMvThr, 1 );
      return 0;
    }

    // rcRefPicL1P == rcRefPicL1Q
    ROTRS( rcRefPicIdcL0P != rcRefPicIdcL0Q, 1 );

    // rcRefPicL0P == rcRefPicL0Q && rcRefPicL1P == rcRefPicL1Q
    // check the motion vector distance
    const Mv& cMvQ0 = rcMbMotionDataL0Q.getMv( cQIdx );
    const Mv& cMvP0 = rcMbMotionDataL0P.getMv( cPIdx );
    const Mv& cMvQ1 = rcMbMotionDataL1Q.getMv( cQIdx );
    const Mv& cMvP1 = rcMbMotionDataL1P.getMv( cPIdx );
    ROTRS( cMvP0.getAbsHorDiff( cMvQ0 ) >= sHorMvThr, 1 );
    ROTRS( cMvP0.getAbsVerDiff( cMvQ0 ) >= sVerMvThr, 1 );
    ROTRS( cMvP1.getAbsHorDiff( cMvQ1 ) >= sHorMvThr, 1 );
    ROTRS( cMvP1.getAbsVerDiff( cMvQ1 ) >= sVerMvThr, 1 );
    return 0;
  }
 
  //  rcRefPicL1P == rcRefPicL0P
  ROTRS( rcRefPicIdcL1Q != rcRefPicIdcL0Q, 1 ) ;
  ROTRS( rcRefPicIdcL0P != rcRefPicIdcL0Q, 1 ) ;

  // rcRefPicL0P == rcRefPicL0Q == rcRefPicL1P == rcRefPicL1Q
  // check the motion vector distance
  const Mv& cMvQ0 = rcMbMotionDataL0Q.getMv( cQIdx );
  const Mv& cMvP0 = rcMbMotionDataL0P.getMv( cPIdx );
  const Mv& cMvQ1 = rcMbMotionDataL1Q.getMv( cQIdx );
  const Mv& cMvP1 = rcMbMotionDataL1P.getMv( cPIdx );

  Bool              bSameListCond  = ( (cMvP0.getAbsHorDiff( cMvQ0 ) >= sHorMvThr) );
  bSameListCond = ( bSameListCond || ( (cMvP0.getAbsVerDiff( cMvQ0 ) >= sVerMvThr) ) );
  bSameListCond = ( bSameListCond || ( (cMvP1.getAbsHorDiff( cMvQ1 ) >= sHorMvThr) ) );
  bSameListCond = ( bSameListCond || ( (cMvP1.getAbsVerDiff( cMvQ1 ) >= sVerMvThr) ) );
  Bool              bDiffListCond  = ( (cMvP0.getAbsHorDiff( cMvQ1 ) >= sHorMvThr) );
  bDiffListCond = ( bDiffListCond || ( (cMvP0.getAbsVerDiff( cMvQ1 ) >= sVerMvThr) ) );
  bDiffListCond = ( bDiffListCond || ( (cMvP1.getAbsHorDiff( cMvQ0 ) >= sHorMvThr) ) );
  bDiffListCond = ( bDiffListCond || ( (cMvP1.getAbsVerDiff( cMvQ0 ) >= sVerMvThr) ) );

  ROTRS( bSameListCond && bDiffListCond, 1 );

  return 0;
}



Void
LoopFilter::xFilter( XPel* pFlt, const Int& iOffset, const Int& iIndexA, const Int& iIndexB, const UChar& ucBs, const Bool& bLum )
{
  Int iAlpha    = g_acAlphaClip[ iIndexA ].ucAlpha;
  Int P0        = pFlt[  -iOffset];
  Int Q0        = pFlt[         0];
  Int P1        = pFlt[-2*iOffset];
  Int Q1        = pFlt[   iOffset];
  Int iDelta    = Q0 - P0;
  Int iAbsDelta = abs( iDelta  );
  Int iBeta     = g_aucBetaTab [ iIndexB ];

  ROFVS( ucBs );
  ROFVS( iAbsDelta < iAlpha );
  ROFVS( (abs(P0 - P1) < iBeta) && (abs(Q0 - Q1) < iBeta) );

  if( ucBs < 4 )
  {
    Int C0 = g_acAlphaClip[ iIndexA ].aucClip[ucBs];

    if( bLum )
    {
      Int P2 = pFlt[-3*iOffset] ;
      Int Q2 = pFlt[ 2*iOffset] ;
      Int aq = (( abs( Q2 - Q0 ) < iBeta ) ? 1 : 0 );
      Int ap = (( abs( P2 - P0 ) < iBeta ) ? 1 : 0 );

      if( ap )
      {
        pFlt[-2*iOffset] = P1 + gClipMinMax((P2 + ((P0 + Q0 + 1)>>1) - (P1<<1)) >> 1, -C0, C0 );
      }

      if( aq  )
      {
        pFlt[   iOffset] = Q1 + gClipMinMax((Q2 + ((P0 + Q0 + 1)>>1) - (Q1<<1)) >> 1, -C0, C0 );
      }

      C0 += ap + aq -1;
    }

    C0++;
    Int iDiff      = gClipMinMax(((iDelta << 2) + (P1 - Q1) + 4) >> 3, -C0, C0 ) ;
    pFlt[-iOffset] = gClip( P0 + iDiff );
    pFlt[       0] = gClip( Q0 - iDiff );
    return;
  }

  if( ! bLum )
  {
    pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
    pFlt[  -iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
  }
  else
  {
    Int  P2       = pFlt[-3*iOffset] ;
    Int  Q2       = pFlt[ 2*iOffset] ;
    Bool bEnable  = (iAbsDelta < ((iAlpha >> 2) + 2));
    Bool aq       = bEnable & ( abs( Q2 - Q0 ) < iBeta );
    Bool ap       = bEnable & ( abs( P2 - P0 ) < iBeta );
    Int  PQ0      = P0 + Q0;

    if( aq )
    {
      pFlt[         0] = (P1 + ((Q1 + PQ0) << 1) +  Q2 + 4) >> 3;
      pFlt[   iOffset] = (PQ0 +Q1 + Q2 + 2) >> 2;
      pFlt[ 2*iOffset] = (((pFlt[ 3*iOffset] + Q2) <<1) + Q2 + Q1 + PQ0 + 4) >> 3;
    }
    else
    {
      pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
    }

    if( ap )
    {
      pFlt[  -iOffset] = (Q1 + ((P1 + PQ0) << 1) +  P2 + 4) >> 3;
      pFlt[-2*iOffset] = (PQ0 +P1 + P2 + 2) >> 2;
      pFlt[-3*iOffset] = (((pFlt[-4*iOffset] + P2) <<1) + pFlt[-3*iOffset] + P1 + PQ0 + 4) >> 3;
    }
    else
    {
      pFlt[  -iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
    }
  }
}


ErrVal
LoopFilter::xLumaVerFiltering( const MbDataAccess& rcMbDataAccess, const DBFilterParameter& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getMbData().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  XPel* pPelLum = pcYuvBuffer->getMbLumAddr();

  //===== filtering of left macroblock edge =====
  if( ! m_bVerMixedMode )
  {
    //-----  curr == FRM && left == FRM  or  curr == FLD && left == FLD  -----
    Int iLeftQp = rcMbDataAccess.getMbDataLeft().getQpLF();
    Int iQp     = ( iLeftQp + iCurrQp + 1) >> 1;
    Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int yBlk = 0; yBlk < 4; yBlk++)
    {
      const UChar ucBs = m_aaaucBs[VER][0][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,           1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+  iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2*iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3*iStride, 1, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4*iStride;
    }
  }
  else
  {
    Int iLeftQpTop = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft     ().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
    Int iLeftQpBot = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft     ().getQpLF() );
    Int iQpTop     = ( iLeftQpTop + iCurrQp + 1) >> 1;
    Int iQpBot     = ( iLeftQpBot + iCurrQp + 1) >> 1;

    Int iIndexATop = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpTop, 0, 51);
    Int iIndexABot = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpBot, 0, 51);
    Int iIndexBTop = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpTop, 0, 51);
    Int iIndexBBot = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpBot, 0, 51);

    if( ! rcMbDataAccess.getMbData().getFieldFlag() )
    {
      //-----  curr == FRM && left == FLD  -----
      for( Int yBlk = 0; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelLum,           1, iIndexATop, iIndexBTop, ucBsTop, true );
          xFilter( pPelLum+2*iStride, 1, iIndexATop, iIndexBTop, ucBsTop, true );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelLum+  iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
          xFilter( pPelLum+3*iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
        }
        pPelLum += 4*iStride;
      }
    }
    else
    {
      //-----  curr == FLD && left == FRM  -----
      Int yBlk;
      for( yBlk = 0; yBlk < 2; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelLum,           1, iIndexATop, iIndexBTop, ucBsTop, true );
          xFilter( pPelLum+  iStride, 1, iIndexATop, iIndexBTop, ucBsTop, true );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelLum+2*iStride, 1, iIndexATop, iIndexBTop, ucBsBot, true );
          xFilter( pPelLum+3*iStride, 1, iIndexATop, iIndexBTop, ucBsBot, true );
        }
        pPelLum += 4*iStride;
      }
      for( yBlk = 2; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelLum,           1, iIndexABot, iIndexBBot, ucBsTop, true );
          xFilter( pPelLum+  iStride, 1, iIndexABot, iIndexBBot, ucBsTop, true );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelLum+2*iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
          xFilter( pPelLum+3*iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
        }
        pPelLum += 4*iStride;
      }
    }
  }

  pPelLum -= 16*iStride-4;

  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);

  for( Int xBlk = 1; xBlk < 4; xBlk++)
  {
    for( Int yBlk = 0; yBlk < 4; yBlk++)
    {
      const UChar ucBs = m_aaaucBs[VER][xBlk][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,           1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+  iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2*iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3*iStride, 1, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4*iStride;
    }
    pPelLum -= 16*iStride-4;
  }

  return Err::m_nOK;
}


ErrVal
LoopFilter::xLumaHorFiltering( const MbDataAccess& rcMbDataAccess, const DBFilterParameter& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getMbData().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  XPel* pPelLum = pcYuvBuffer->getMbLumAddr();

  //===== filtering of upper macroblock edge =====
  if( ! m_bHorMixedMode )
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getMbData().getFieldFlag() && ( !rcMbDataAccess.isTopMb()||rcMbDataAccess.getMbDataAboveAbove().getFieldFlag() ) ?
                    rcMbDataAccess.getMbDataAboveAbove().getQpLF() : rcMbDataAccess.getMbDataAbove().getQpLF();
    Int iQp       = ( iAboveQp + iCurrQp + 1 ) >> 1;
    Int iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int xBlk = 0; xBlk < 4; xBlk++)
    {
      const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,   iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+1, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3, iStride, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4;
    }
    pPelLum -= 16;
  }
  else
  {
    //-----  curr = FRM, above = FLD  -----
    AOT_DBG( ! rcMbDataAccess.isTopMb() );
    AOT_DBG( ! rcMbDataAccess.isAboveMbExisting() );

    //===== top field filtering =====
    {
      XPel* pPelTop     = pcYuvBuffer->getMbLumAddr();
      Int   iTopStride  = pcYuvBuffer->getLStride()*2;
      Int   iAboveQp    = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
      Int   iQp         = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA     = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB     = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aucBsHorTop[xBlk];
        if( 0 != ucBs )
        {
          xFilter( pPelTop,   iTopStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelTop+1, iTopStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelTop+2, iTopStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelTop+3, iTopStride, iIndexA, iIndexB, ucBs, true );
        }
        pPelTop += 4;
      }
    }
    //===== bottom field filtering =====
    {
      XPel* pPelBot     = pcYuvBuffer->getMbLumAddr()+pcYuvBuffer->getLStride();
      Int   iBotStride  = pcYuvBuffer->getLStride()*2;
      Int   iAboveQp    = rcMbDataAccess.getMbDataAbove().getQpLF();
      Int   iQp         = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA     = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB     = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
        if( 0 != ucBs )
        {
          xFilter( pPelBot,   iBotStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelBot+1, iBotStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelBot+2, iBotStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelBot+3, iBotStride, iIndexA, iIndexB, ucBs, true );
        }
        pPelBot += 4;
      }
    }
  }

  pPelLum += 4*iStride;

  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);

  for( Int yBlk = 1; yBlk < 4; yBlk++)
  {
    for( Int xBlk = 0; xBlk < 4; xBlk++)
    {
      const UChar ucBs = m_aaaucBs[HOR][xBlk][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,   iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+1, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3, iStride, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4;
    }
    pPelLum += 4*iStride - 16;
  }

  return Err::m_nOK;
}


ErrVal
LoopFilter::xChromaHorFiltering( const MbDataAccess& rcMbDataAccess, const DBFilterParameter& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbData().getQpLF() );
  Int   iStride = pcYuvBuffer->getCStride();
  XPel* pPelCb  = pcYuvBuffer->getMbCbAddr();
  XPel* pPelCr  = pcYuvBuffer->getMbCrAddr();

  //===== filtering of upper macroblock edge =====
  if( ! m_bHorMixedMode )
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getMbData().getFieldFlag() && (!rcMbDataAccess.isTopMb()||rcMbDataAccess.getMbDataAboveAbove().getFieldFlag()) ?
                    rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAboveAbove().getQpLF()) : rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAbove().getQpLF());
    Int iQp       = ( iAboveQp + iCurrQp + 1) >> 1;
    Int iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int xBlk = 0; xBlk < 4; xBlk++)
    {
      const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
      if( 0 != ucBs )
      {
        xFilter( pPelCb,   iStride, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCb+1, iStride, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr,   iStride, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr+1, iStride, iIndexA, iIndexB, ucBs, false );
      }
      pPelCb += 2;
      pPelCr += 2;
    }
    pPelCb -= 8;
    pPelCr -= 8;
  }
  else
  {
    //-----  curr = FRM, above = FLD  -----
    AOT_DBG( ! rcMbDataAccess.isTopMb() );

    //===== top field filtering =====
    {
      XPel* pPelFieldCb   = pcYuvBuffer->getMbCbAddr();
      XPel* pPelFieldCr   = pcYuvBuffer->getMbCrAddr();
      Int   iFieldStride  = pcYuvBuffer->getCStride() * 2;

      Int   iAboveQp      = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAboveAbove().getQpLF() );
      Int   iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aucBsHorTop[xBlk];
        if( 0 != ucBs )
        {
          xFilter( pPelFieldCb,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCb+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
        }
        pPelFieldCb   += 2;
        pPelFieldCr   += 2;
      }
    }
    //===== bottom field filtering =====
    {
      XPel* pPelFieldCb   = pcYuvBuffer->getMbCbAddr()+pcYuvBuffer->getCStride();
      XPel* pPelFieldCr   = pcYuvBuffer->getMbCrAddr()+pcYuvBuffer->getCStride();
      Int   iFieldStride  = pcYuvBuffer->getCStride() * 2;

      Int   iAboveQp      = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAbove().getQpLF() );
      Int   iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
        if( 0 != ucBs )
        {
          xFilter( pPelFieldCb,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCb+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
        }
        pPelFieldCb   += 2;
        pPelFieldCr   += 2;
      }
    }
  }
  pPelCb += 4*iStride;
  pPelCr += 4*iStride;

  // now we filter the remaining edge
  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);

  for( Int xBlk = 0; xBlk < 4; xBlk++)
  {
    const UChar ucBs = m_aaaucBs[HOR][xBlk][2];
    if( 0 != ucBs )
    {
      xFilter( pPelCb,   iStride, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCb+1, iStride, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr,   iStride, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr+1, iStride, iIndexA, iIndexB, ucBs, false );
    }
    pPelCb += 2;
    pPelCr += 2;
  }

  return Err::m_nOK;
}


ErrVal
LoopFilter::xChromaVerFiltering( const MbDataAccess& rcMbDataAccess, const DBFilterParameter& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbData().getQpLF() );
  Int   iStride = pcYuvBuffer->getCStride();
  XPel* pPelCb  = pcYuvBuffer->getMbCbAddr();
  XPel* pPelCr  = pcYuvBuffer->getMbCrAddr();

  //===== filtering of left macroblock edge =====
  if( ! m_bVerMixedMode )
  {
    //-----  curr == FRM && left == FRM  or  curr == FLD && left == FLD  -----
    Int iLeftQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataLeft().getQpLF() );
    Int iQp     = ( iLeftQp + iCurrQp + 1 ) >> 1;
    Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int yBlk = 0; yBlk < 4; yBlk++)
    {
      const UChar ucBs = m_aaaucBs[VER][0][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelCb,         1, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCb+iStride, 1, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr,         1, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr+iStride, 1, iIndexA, iIndexB, ucBs, false );
      }
      pPelCb += 2*iStride;
      pPelCr += 2*iStride;
    }
  }
  else
  {
    Int iLeftQpTop = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft     ().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
    Int iLeftQpBot = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft     ().getQpLF() );
    Int iQpTop     = ( iLeftQpTop + iCurrQp + 1) >> 1;
    Int iQpBot     = ( iLeftQpBot + iCurrQp + 1) >> 1;
    Int iIndexATop = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpTop, 0, 51);
    Int iIndexABot = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpBot, 0, 51);
    Int iIndexBTop = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpTop, 0, 51);
    Int iIndexBBot = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpBot, 0, 51);

    if( ! rcMbDataAccess.getMbData().getFieldFlag() )
    {
      //-----  curr == FRM && left == FLD  -----
      for( Int yBlk = 0; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelCb, 1, iIndexATop, iIndexBTop, ucBsTop, false );
          xFilter( pPelCr, 1, iIndexATop, iIndexBTop, ucBsTop, false );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelCb+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
          xFilter( pPelCr+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
        }
        pPelCb += 2*iStride;
        pPelCr += 2*iStride;
      }
    }
    else
    {
      //-----  curr == FLD && left == FRM  -----
      Int yBlk;
      for( yBlk = 0; yBlk < 2; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelCb, 1, iIndexATop, iIndexBTop, ucBsTop, false );
          xFilter( pPelCr, 1, iIndexATop, iIndexBTop, ucBsTop, false );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelCb+iStride, 1, iIndexATop, iIndexBTop, ucBsBot, false );
          xFilter( pPelCr+iStride, 1, iIndexATop, iIndexBTop, ucBsBot, false );
        }
        pPelCb   += 2*iStride;
        pPelCr   += 2*iStride;
      }

      for( yBlk = 2; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelCb, 1, iIndexABot, iIndexBBot, ucBsTop, false );
          xFilter( pPelCr, 1, iIndexABot, iIndexBBot, ucBsTop, false );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelCb+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
          xFilter( pPelCr+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
        }
        pPelCb += 2*iStride;
        pPelCr += 2*iStride;
      }
    }
  }
  pPelCb -= 8*iStride-4;
  pPelCr -= 8*iStride-4;

  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( Int yBlk = 0; yBlk < 4; yBlk++)
  {
    const UChar ucBs = m_aaaucBs[VER][2][yBlk];
    if( 0 != ucBs )
    {
      xFilter( pPelCb,         1, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCb+iStride, 1, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr,         1, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr+iStride, 1, iIndexA, iIndexB, ucBs, false );
    }
    pPelCb += 2*iStride;
    pPelCr += 2*iStride;
  }

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END

