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




#include "H264AVCDecoderLib.h"
#include "H264AVCCommonLib/Tables.h"


#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/IntraPrediction.h"
#include "H264AVCCommonLib/MotionCompensation.h"
#include "MbDecoder.h"



H264AVC_NAMESPACE_BEGIN



MbDecoder::MbDecoder()
: m_pcTransform         ( 0 )
, m_pcIntraPrediction   ( 0 )
, m_pcMotionCompensation( 0 )
, m_bInitDone           ( false )
{
}

MbDecoder::~MbDecoder()
{
}

ErrVal
MbDecoder::create( MbDecoder*& rpcMbDecoder )
{
  rpcMbDecoder = new MbDecoder;
  ROT( NULL == rpcMbDecoder );
  return Err::m_nOK;
}

ErrVal
MbDecoder::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal
MbDecoder::init( Transform*          pcTransform,
                 IntraPrediction*    pcIntraPrediction,
                 MotionCompensation* pcMotionCompensation )
{
  ROF( pcTransform );
  ROF( pcIntraPrediction );
  ROF( pcMotionCompensation );

  m_pcTransform           = pcTransform;
  m_pcIntraPrediction     = pcIntraPrediction;
  m_pcMotionCompensation  = pcMotionCompensation;
  m_bInitDone             = true;

  return Err::m_nOK;
}


ErrVal
MbDecoder::uninit()
{
  m_pcTransform           = 0;
  m_pcIntraPrediction     = 0;
  m_pcMotionCompensation  = 0;
  m_bInitDone             = false;
  return Err::m_nOK;
}

ErrVal
MbDecoder::xPredictionFromBaseLayer( MbDataAccess&  rcMbDataAccess,
                                     MbDataAccess*  pcMbDataAccessBase )
{
  MbData& rcMbData = rcMbDataAccess.getMbData();

  if( rcMbData.getBLSkipFlag() )
  {
    ROF( pcMbDataAccessBase );
    Bool bBLSkipFlag  = rcMbData.getBLSkipFlag();
    rcMbData.copyMotion( pcMbDataAccessBase->getMbData() );
    rcMbData.setBLSkipFlag( bBLSkipFlag );
    if( rcMbData.isIntra() )
    {
      rcMbData.setMbMode( INTRA_BL );
    }
  }
  else
  {
    for( ListIdx eListIdx = LIST_0; eListIdx <= LIST_1; eListIdx = ListIdx( eListIdx + 1 ) )
    {
      MbMotionData& rcMbMotionData = rcMbData.getMbMotionData( eListIdx );

      switch( rcMbData.getMbMode() )
      {
      case MODE_16x16:
        {
          if( rcMbData.isBlockFwdBwd( B_8x8_0, eListIdx ) && rcMbMotionData.getMotPredFlag() )
          {
            rcMbMotionData.setRefIdx( pcMbDataAccessBase->getMbMotionData( eListIdx ).getRefIdx() );
          }
        }
        break;
      case MODE_16x8:
        {
          if( rcMbData.isBlockFwdBwd( B_8x8_0, eListIdx ) && rcMbMotionData.getMotPredFlag( PART_16x8_0 ) )
          {
            rcMbMotionData.setRefIdx( pcMbDataAccessBase->getMbMotionData( eListIdx ).getRefIdx( PART_16x8_0 ), PART_16x8_0 );
          }
          if( rcMbData.isBlockFwdBwd( B_8x8_2, eListIdx ) && rcMbMotionData.getMotPredFlag( PART_16x8_1 ) )
          {
            rcMbMotionData.setRefIdx( pcMbDataAccessBase->getMbMotionData( eListIdx ).getRefIdx( PART_16x8_1 ), PART_16x8_1 );
          }
        }
        break;
      case MODE_8x16:
        {
          if( rcMbData.isBlockFwdBwd( B_8x8_0, eListIdx ) && rcMbMotionData.getMotPredFlag( PART_8x16_0 ) )
          {
            rcMbMotionData.setRefIdx( pcMbDataAccessBase->getMbMotionData( eListIdx ).getRefIdx( PART_8x16_0 ), PART_8x16_0 );
          }
          if( rcMbData.isBlockFwdBwd( B_8x8_1, eListIdx ) && rcMbMotionData.getMotPredFlag( PART_8x16_1 ) )
          {
            rcMbMotionData.setRefIdx( pcMbDataAccessBase->getMbMotionData( eListIdx ).getRefIdx( PART_8x16_1 ), PART_8x16_1 );
          }
        }
        break;
      case MODE_8x8:
      case MODE_8x8ref0:
        {
          for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
          {
            if( rcMbData.getBlkMode( c8x8Idx.b8x8Index() ) != BLK_SKIP  &&
              rcMbData.isBlockFwdBwd( c8x8Idx.b8x8Index(), eListIdx ) && rcMbMotionData.getMotPredFlag( c8x8Idx.b8x8() ) )
            {
              rcMbMotionData.setRefIdx( pcMbDataAccessBase->getMbMotionData( eListIdx ).getRefIdx( c8x8Idx.b8x8() ), c8x8Idx.b8x8() );
            }
          }
        }
        break;
      default:
        break;
      }
    }
  }
  return Err::m_nOK;
}


ErrVal
MbDecoder::decode( MbDataAccess&  rcMbDataAccess,
                   MbDataAccess*  pcMbDataAccessBase,
                   Frame*         pcFrame,
                   Frame*         pcResidual,
                   Frame*         pcBaseLayer,
                   Frame*         pcBaseLayerResidual,
                   RefFrameList*  pcRefFrameList0,
                   RefFrameList*  pcRefFrameList1,
                   Bool           bReconstructAll )
{
  ROF( m_bInitDone );
  RNOK( xPredictionFromBaseLayer( rcMbDataAccess, pcMbDataAccessBase ) );

  rcMbDataAccess.setMbDataAccessBase(pcMbDataAccessBase);

  YuvMbBuffer  cPredBuffer;  cPredBuffer.setAllSamplesToZero();

  if( ( rcMbDataAccess.getMbData().getMbMode() == INTRA_BL || rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16) )
	  && rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag() )
  {

	  if( rcMbDataAccess.getMbData().getMbMode() == INTRA_BL )
	  {
		  // We're going to use the BL skip flag to correctly decode the intra prediction mode
		  AOT( rcMbDataAccess.getMbData().getBLSkipFlag() == false );

		  // Inherit the mode of the base block
		  rcMbDataAccess.getMbData().setMbMode( pcMbDataAccessBase->getMbData().getMbMode() );

		  // Inherit intra prediction modes
		  for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
			  rcMbDataAccess.getMbData().intraPredMode(cIdx) = pcMbDataAccessBase->getMbData().intraPredMode(cIdx);

		  rcMbDataAccess.getMbData().setChromaPredMode( pcMbDataAccessBase->getMbData().getChromaPredMode() );
	  }

	  if( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) ||
		  ( rcMbDataAccess.getMbData().isIntra() && rcMbDataAccess.getMbData().getBLSkipFlag() ) )
	  {
		  // The 8x8 transform flag is present in the bit-stream unless transform coefficients
		  // are not transmitted at the enhancement layer.  In this case, inherit the base layer
		  // transform type.  This makes intra predition work correctly, etc.
		  if( !( rcMbDataAccess.getMbData().getMbCbp() & 0x0F )  ||
        !rcMbDataAccess.getMbData().is8x8TrafoFlagPresent(rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag()) )
			  rcMbDataAccess.getMbData().setTransformSize8x8( pcMbDataAccessBase->getMbData().isTransformSize8x8() );
	  }

	  if( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 )
		  || ( rcMbDataAccess.getMbData().isIntra() && rcMbDataAccess.getMbData().getBLSkipFlag() ) )
	  {
		  xAddTCoeffs( rcMbDataAccess, *pcMbDataAccessBase );
	  }
  }

  // overwrite the QP, so the rewritten AVC bitstream can get correct QPDelta
  if (rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag() && (!rcMbDataAccess.getSH().getNoInterLayerPredFlag()) )
  {
    if(( rcMbDataAccess.getMbData().getMbExtCbp() == 0 ) && (!rcMbDataAccess.getMbData().isIntra16x16()))
      rcMbDataAccess.getMbData().setQp( rcMbDataAccess.getLastQp());
  }
  if( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 )
    && rcMbDataAccess.getCoeffResidualPredFlag())
  {
    if( !( rcMbDataAccess.getMbData().getMbCbp() & 0x0F ) ||
      !rcMbDataAccess.getMbData().is8x8TrafoFlagPresent(rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag()) )
      rcMbDataAccess.getMbData().setTransformSize8x8( pcMbDataAccessBase->getMbData().isTransformSize8x8() );
  }

  if( !rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag() &&
      rcMbDataAccess.getMbData().isIntraBL()     &&
      rcMbDataAccess.getCoeffResidualPredFlag()   )
  {
    if( rcMbDataAccess.getMbData().getMbCbp() & 0x0F )
    {
      if( pcMbDataAccessBase->getMbData().getMbCbp() & 0x0F )
      {
        ROF( rcMbDataAccess.getMbData().isTransformSize8x8() == pcMbDataAccessBase->getMbData().isTransformSize8x8() );
      }
    }
    else
    {
      rcMbDataAccess.getMbData().setTransformSize8x8( pcMbDataAccessBase->getMbData().isTransformSize8x8() );
    }
  }


  if( ( rcMbDataAccess.getMbData().isIntra() || !rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) ) 
    && pcBaseLayerResidual )
  {	 
    YuvPicBuffer* pcBaseResidualBuffer = pcBaseLayerResidual->getFullPelYuvBuffer();
    pcBaseResidualBuffer->clearCurrMb();
  }

  //===== scale coefficients =====
  RNOK( xScaleTCoeffs( rcMbDataAccess ) );

  if( rcMbDataAccess.getMbData().isIntra() )
  {
    //===== clear residual signal for intra macroblocks =====
    if( pcResidual )
    {
      YuvMbBuffer  cYuvMbBuffer;  cYuvMbBuffer.setAllSamplesToZero();
      RNOK( pcResidual->getFullPelYuvBuffer()->loadBuffer( &cYuvMbBuffer ) );
    }

    if( rcMbDataAccess.getMbData().isPCM() )
    {
      //===== I_PCM mode =====
      RNOK( xDecodeMbPCM( rcMbDataAccess, pcFrame->getFullPelYuvBuffer() ) );
      cPredBuffer.loadBuffer( pcFrame->getFullPelYuvBuffer() );
    }
    else if( rcMbDataAccess.getMbData().getMbMode() == INTRA_BL )
    {
      //===== I_BL mode =====
      RNOK( xDecodeMbIntraBL( rcMbDataAccess,  pcFrame->getFullPelYuvBuffer(),
                              cPredBuffer, pcBaseLayer->getFullPelYuvBuffer() ) );
    }
    else
    {
      m_pcIntraPrediction->setAvailableMaskMb( rcMbDataAccess.getAvailableMask() );
      YuvMbBuffer cRecBuffer;  cRecBuffer.loadIntraPredictors( pcFrame->getFullPelYuvBuffer() );

      if( rcMbDataAccess.getMbData().isIntra16x16() )
      {
        //===== I_16x16 mode ====
        RNOK( xDecodeMbIntra16x16( rcMbDataAccess, cRecBuffer, cPredBuffer ) );
      }
      else if( rcMbDataAccess.getMbData().isTransformSize8x8() )
      {
        //===== I_8x8 mode =====
        RNOK( xDecodeMbIntra8x8( rcMbDataAccess, cRecBuffer, cPredBuffer ) );
      }
      else
      {
        //===== I_4x4 mode =====
        RNOK( xDecodeMbIntra4x4( rcMbDataAccess, cRecBuffer, cPredBuffer ) );
      }
      RNOK( pcFrame->getFullPelYuvBuffer()->loadBuffer( &cRecBuffer ) );
      rcMbDataAccess.getMbTCoeffs().copyPredictionFrom( cRecBuffer );
    }
  }
  else
  {
    //===== motion-compensated modes =====
    RNOK( xDecodeMbInter( rcMbDataAccess, pcMbDataAccessBase,
                          cPredBuffer, pcFrame->getFullPelYuvBuffer(),
                          pcResidual, pcBaseLayerResidual,
                          *pcRefFrameList0, *pcRefFrameList1, bReconstructAll, pcBaseLayer  ) );
  }

  return Err::m_nOK;
}


ErrVal
MbDecoder::calcMv( MbDataAccess& rcMbDataAccess,
                   MbDataAccess* pcMbDataAccessBaseMotion )
{
  if( rcMbDataAccess.getMbData().getBLSkipFlag() )
  {
    return Err::m_nOK;
  }

  if( rcMbDataAccess.getMbData().getMbMode() == INTRA_4X4 )
  {
    //----- intra prediction -----
    rcMbDataAccess.getMbMotionData( LIST_0 ).setRefIdx( BLOCK_NOT_PREDICTED );
    rcMbDataAccess.getMbMotionData( LIST_0 ).setAllMv ( Mv::ZeroMv() );
    rcMbDataAccess.getMbMotionData( LIST_1 ).setRefIdx( BLOCK_NOT_PREDICTED );
    rcMbDataAccess.getMbMotionData( LIST_1 ).setAllMv ( Mv::ZeroMv() );
  }
  else
  {
    if( rcMbDataAccess.getMbData().getMbMode() == MODE_8x8 || rcMbDataAccess.getMbData().getMbMode() == MODE_8x8ref0 )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        //----- motion compensated prediction -----
        RNOK( m_pcMotionCompensation->calcMvSubMb( c8x8Idx, rcMbDataAccess, pcMbDataAccessBaseMotion ) );
      }
    }
    else
    {
      //----- motion compensated prediction -----
      RNOK( m_pcMotionCompensation->calcMvMb( rcMbDataAccess, pcMbDataAccessBaseMotion ) );
    }
  }

  return Err::m_nOK;
}



ErrVal
MbDecoder::xDecodeMbPCM( MbDataAccess& rcMbDataAccess, YuvPicBuffer *pcRecYuvBuffer )
{
  Pel*  pucSrc  = rcMbDataAccess.getMbTCoeffs().getPelBuffer();
  XPel* pucDest = pcRecYuvBuffer->getMbLumAddr();
  Int   iStride = pcRecYuvBuffer->getLStride();
  Int   iDelta = 1;
  Int   n, m, n1, m1, dest;

  for( n = 0; n < 16; n+=iDelta )
  {
    for( m = 0; m < 16; m+=iDelta )
    {
      dest = *pucSrc++;

      for( n1=0; n1< +iDelta; n1++ )
      for( m1=m; m1<m+iDelta; m1++ )
      {
        pucDest[m1+n1*iStride] = dest;
      }
    }
    pucDest += iDelta*iStride;
  }

  pucDest = pcRecYuvBuffer->getMbCbAddr();
  iStride = pcRecYuvBuffer->getCStride();

  for( n = 0; n < 8; n+=iDelta )
  {
    for( m = 0; m < 8; m+=iDelta )
    {
      dest = *pucSrc++;

      for( n1=0; n1< +iDelta; n1++ )
      for( m1=m; m1<m+iDelta; m1++ )
      {
        pucDest[m1+n1*iStride] = dest;
      }
    }
    pucDest += iDelta*iStride;
  }

  pucDest = pcRecYuvBuffer->getMbCrAddr();

  for( n = 0; n < 8; n+=iDelta )
  {
    for( m = 0; m < 8; m+=iDelta )
    {
      dest = *pucSrc++;

      for( n1=0; n1< +iDelta; n1++ )
      for( m1=m; m1<m+iDelta; m1++ )
      {
        pucDest[m1+n1*iStride] = dest;
      }
    }
    pucDest += iDelta*iStride;
  }


  return Err::m_nOK;
}


ErrVal
MbDecoder::xDecodeMbInter( MbDataAccess&  rcMbDataAccess,
                           MbDataAccess*  pcMbDataAccessBase,
                           YuvMbBuffer&   rcPredBuffer,
                           YuvPicBuffer*  pcRecYuvBuffer,
                           Frame*         pcResidual,
                           Frame*         pcBaseResidual,
                           RefFrameList&  rcRefFrameList0,
                           RefFrameList&  rcRefFrameList1,
                           Bool           bReconstruct,
                           Frame*         pcBaseLayerRec )
{
  YuvMbBuffer      cYuvMbBuffer;         cYuvMbBuffer        .setAllSamplesToZero();
  YuvMbBuffer      cYuvMbBufferResidual; cYuvMbBufferResidual.setAllSamplesToZero();
  MbTransformCoeffs&  rcCoeffs        = m_cTCoeffs;
  Bool                bCalcMv         = false;
  Bool                bFaultTolerant  = true;

  //===== derive motion vectors =====
  calcMv( rcMbDataAccess, pcMbDataAccessBase );

#ifdef SHARP_AVC_REWRITE_OUTPUT
  return Err::m_nOK;
#endif

  //===== get prediction signal when full reconstruction is requested =====
  if( bReconstruct )
  {
    if( rcMbDataAccess.getMbData().getMbMode() == MODE_8x8 || rcMbDataAccess.getMbData().getMbMode() == MODE_8x8ref0 )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        //----- motion compensated prediction -----
        RNOK( m_pcMotionCompensation->compensateSubMb ( c8x8Idx,
                                                        rcMbDataAccess, rcRefFrameList0, rcRefFrameList1,
                                                        &cYuvMbBuffer, bCalcMv, bFaultTolerant ) );
      }
    }
    else
    {
      //----- motion compensated prediction -----
      RNOK(   m_pcMotionCompensation->compensateMb    ( rcMbDataAccess,
                                                        rcRefFrameList0,
                                                        rcRefFrameList1,
                                                        &cYuvMbBuffer,
                                                        bCalcMv ) );
    }
    if(pcBaseLayerRec)
    {
      RNOK(m_pcMotionCompensation->compensateMbBLSkipIntra(rcMbDataAccess, &cYuvMbBuffer, pcBaseLayerRec));
    }
    rcPredBuffer.loadLuma   ( cYuvMbBuffer );
    rcPredBuffer.loadChroma ( cYuvMbBuffer );
  }

  Bool coeffResidualPredFlag = rcMbDataAccess.getCoeffResidualPredFlag();  
  Int oldExtCbp = 0;

  //===== add base layer residual =====
  if (rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ))
  {
    oldExtCbp = rcMbDataAccess.getMbData().getMbExtCbp();
    if (coeffResidualPredFlag)
    { 
      rcCoeffs.add( &pcMbDataAccessBase->getMbData().getMbTCoeffs() );            
      rcMbDataAccess.getMbTCoeffs().copyFrom(rcCoeffs);     // store the sum of the coefficients and base layer coefficients      
      rcMbDataAccess.getMbData().setMbExtCbp( rcMbDataAccess.getMbData().getMbExtCbp() | pcMbDataAccessBase->getMbData().getMbExtCbp() );
    }
    else
      pcMbDataAccessBase->getMbData().getMbTCoeffs().clearLumaLevels();
  }

  //===== reconstruct residual signal by using transform coefficients ======
  m_pcTransform->setClipMode( false );
  if( rcMbDataAccess.getMbData().isTransformSize8x8() )
  {
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      // if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) ) // there may be coefficients propogated from base layer
      {
        RNOK( m_pcTransform->invTransform8x8Blk( cYuvMbBufferResidual.getYBlk( cIdx ),
                                                 cYuvMbBufferResidual.getLStride(),
                                                 rcCoeffs.get8x8(cIdx) ) );
      }
    }
  }
  else
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      // if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx )
      {
        RNOK( m_pcTransform->invTransform4x4Blk( cYuvMbBufferResidual.getYBlk( cIdx ),
                                                 cYuvMbBufferResidual.getLStride(),
                                                 rcCoeffs.get(cIdx) ) );
      }
    }
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma4x4();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBufferResidual, rcPredBuffer, uiChromaCbp, false ) );
  m_pcTransform->setClipMode( true );


  //===== add base layer residual =====
  if( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 )
    && !rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag() )
  {
    YuvMbBuffer cBaseLayerBuffer;
    cBaseLayerBuffer.loadBuffer( pcBaseResidual->getFullPelYuvBuffer() );

    cYuvMbBufferResidual.add( cBaseLayerBuffer );
  }
 
  if (rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) && rcMbDataAccess.getCoeffResidualPredFlag())
  {
    rcMbDataAccess.getMbData().setMbExtCbp( oldExtCbp);  // resume cbp
  }
  if( (rcMbDataAccess.getMbData().isIntra() ||!rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ))
    && pcBaseResidual )
  {	 
    YuvPicBuffer* pcBaseResidualBuffer = pcBaseResidual->getFullPelYuvBuffer();
    pcBaseResidualBuffer->clearCurrMb();
  }

  //===== reconstruct signal =====
  if( bReconstruct )
  {
    cYuvMbBuffer.add( cYuvMbBufferResidual );
    cYuvMbBuffer.clip();
  }


  //===== store reconstructed residual =====
  if( pcResidual )
  {
    RNOK( pcResidual->getFullPelYuvBuffer()->loadBuffer( &cYuvMbBufferResidual ) );
  }

  //===== store reconstructed signal =====
  if( pcRecYuvBuffer )
  {
    RNOK( pcRecYuvBuffer->loadBuffer( &cYuvMbBuffer ) );
  }

  return Err::m_nOK;
}



ErrVal
MbDecoder::xDecodeMbIntra4x4( MbDataAccess&  rcMbDataAccess,
                              YuvMbBuffer&   cYuvMbBuffer,
                              YuvMbBuffer&   rcPredBuffer )
{
#ifndef SHARP_AVC_REWRITE_OUTPUT //JV: not clean at all -> to remove compilation warnings
	Int  iStride = cYuvMbBuffer.getLStride();
  MbTransformCoeffs& rcCoeffs = m_cTCoeffs;
#endif //JV: not clean at all -> to remove compilation warnings

  for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    if( !rcMbDataAccess.getMbData().getBLSkipFlag() || !rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag() )
      rcMbDataAccess.getMbData().intraPredMode( cIdx ) = rcMbDataAccess.decodeIntraPredMode( cIdx );
#ifndef SHARP_AVC_REWRITE_OUTPUT
	XPel* puc = cYuvMbBuffer.getYBlk( cIdx );

    UInt uiPredMode = rcMbDataAccess.getMbData().intraPredMode( cIdx );
    RNOK( m_pcIntraPrediction->predictLumaBlock( puc, iStride, uiPredMode, cIdx ) );

    rcPredBuffer.loadLuma( cYuvMbBuffer, cIdx );

    if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) )
    {
      RNOK( m_pcTransform->invTransform4x4Blk( puc, iStride, rcCoeffs.get( cIdx ) ) );
    }
#endif
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma4x4();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBuffer, rcPredBuffer, uiChromaCbp, true ) );

  return Err::m_nOK;
}


ErrVal
MbDecoder::xDecodeMbIntra8x8( MbDataAccess& rcMbDataAccess,
                              YuvMbBuffer&  cYuvMbBuffer,
                              YuvMbBuffer&  rcPredBuffer )
{
#ifndef SHARP_AVC_REWRITE_OUTPUT //JV : Not clean at all -> to remove compilation warnings
	Int  iStride = cYuvMbBuffer.getLStride();
    MbTransformCoeffs& rcCoeffs = m_cTCoeffs;
#endif //JV : Not clean at all

  for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    if( !rcMbDataAccess.getMbData().getBLSkipFlag() || !rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag() )
    {
      Int iPredMode = rcMbDataAccess.decodeIntraPredMode( cIdx );
      for( S4x4Idx cIdx4x4( cIdx ); cIdx4x4.isLegal( cIdx ); cIdx4x4++ )
      {
        rcMbDataAccess.getMbData().intraPredMode( cIdx4x4 ) = iPredMode;
      }
    }
#ifndef SHARP_AVC_REWRITE_OUTPUT
    XPel* puc = cYuvMbBuffer.getYBlk( cIdx );

    const UInt uiPredMode = rcMbDataAccess.getMbData().intraPredMode( cIdx );

    RNOK( m_pcIntraPrediction->predictLuma8x8Block( puc, iStride, uiPredMode, cIdx ) );

    rcPredBuffer.loadLuma( cYuvMbBuffer, cIdx );

    if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) )
    {
      RNOK( m_pcTransform->invTransform8x8Blk( puc, iStride, rcCoeffs.get8x8( cIdx ) ) );
    }
#endif
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma4x4();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBuffer, rcPredBuffer, uiChromaCbp, true ) );

  return Err::m_nOK;
}



ErrVal
MbDecoder::xDecodeMbIntraBL( MbDataAccess&  rcMbDataAccess,
                             YuvPicBuffer*  pcRecYuvBuffer,
                             YuvMbBuffer&   rcPredBuffer,
                             YuvPicBuffer*  pcBaseYuvBuffer )
{
#ifdef SHARP_AVC_REWRITE_OUTPUT
  return Err::m_nOK;
#endif
  YuvMbBuffer      cYuvMbBuffer;
  MbTransformCoeffs&  rcCoeffs = m_cTCoeffs;

  Bool bAddBaseCoeffs = false;
  if( rcMbDataAccess.getSH().getCoeffResidualPredFlag() )
  {
    rcMbDataAccess.getMbDataAccessBase()->getMbTCoeffs().copyPredictionTo( cYuvMbBuffer );
    bAddBaseCoeffs = rcMbDataAccess.getMbDataAccessBase()->getMbData().isIntraBL();
    if( bAddBaseCoeffs )
    {
      rcCoeffs.add( &rcMbDataAccess.getMbDataAccessBase()->getMbTCoeffs(), true, false );
    }
  }
  else
  {
    cYuvMbBuffer.loadBuffer ( pcBaseYuvBuffer );
  }
  rcPredBuffer.loadLuma   ( cYuvMbBuffer );
  rcPredBuffer.loadChroma ( cYuvMbBuffer );

  if( rcMbDataAccess.getMbData().isTransformSize8x8() )
  {
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform8x8Blk( cYuvMbBuffer.getYBlk( cIdx ),
                                               cYuvMbBuffer.getLStride(),
                                               rcCoeffs.get8x8(cIdx) ) );
    }
  }
  else
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform4x4Blk( cYuvMbBuffer.getYBlk( cIdx ),
                                               cYuvMbBuffer.getLStride(),
                                               rcCoeffs.get(cIdx) ) );
    }
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma4x4();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBuffer, rcPredBuffer, uiChromaCbp, false, bAddBaseCoeffs ) );
  // Note that the following also copies pred buffer inside of MbTransformCoeffs
  rcMbDataAccess.getMbTCoeffs().copyFrom( rcCoeffs );
  rcMbDataAccess.getMbTCoeffs().copyPredictionFrom( rcPredBuffer );
  pcRecYuvBuffer->loadBuffer( &cYuvMbBuffer );

  return Err::m_nOK;
}



ErrVal 
MbDecoder::xDecodeMbIntra16x16( MbDataAccess& rcMbDataAccess,
                                YuvMbBuffer&  cYuvMbBuffer,
                                YuvMbBuffer&  rcPredBuffer )
{
#ifdef SHARP_AVC_REWRITE_OUTPUT
  return Err::m_nOK;
#endif

  Int  iStride = cYuvMbBuffer.getLStride();

  RNOK( m_pcIntraPrediction->predictLumaMb( cYuvMbBuffer.getMbLumAddr(), iStride, rcMbDataAccess.getMbData().intraPredMode() ) );

  rcPredBuffer.loadLuma( cYuvMbBuffer );

  MbTransformCoeffs& rcCoeffs = m_cTCoeffs;


  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  Bool                b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool                b16x16    = rcMbDataAccess.getMbData().isIntra16x16();
  UInt                uiYScalId = ( bIntra ? ( b8x8 && !b16x16 ? 6 : 0 ) : ( b8x8 ? 7 : 3 ) );
  const UChar*        pucScaleY = rcMbDataAccess.getSH().getScalingMatrix( uiYScalId );
	const Int aaiDequantDcCoef[6] = { 10, 11, 13, 14, 16, 18 };
	const Int iQp = rcMbDataAccess.getMbData().getQp();
	Int iQpScale = aaiDequantDcCoef[iQp%6];
  if( pucScaleY )
  {
    iQpScale  *= pucScaleY[0];
  }
  else
  {
    iQpScale *= 16;
  }
  RNOK( m_pcTransform->invTransformDcCoeff( rcCoeffs.get( B4x4Idx(0) ), iQpScale, iQp/6) );

  for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    RNOK( m_pcTransform->invTransform4x4Blk( cYuvMbBuffer.getYBlk( cIdx ), iStride, rcCoeffs.get( cIdx ) ) );
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma16x16();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBuffer, rcPredBuffer, uiChromaCbp, true ) );

  return Err::m_nOK;
}


ErrVal
MbDecoder::xDecodeChroma( MbDataAccess& rcMbDataAccess,
                          YuvMbBuffer&  rcRecYuvBuffer,
                          YuvMbBuffer&  rcPredMbBuffer,
                          UInt          uiChromaCbp,
                          Bool          bPredChroma,
                          Bool          bAddBaseCoeffsChroma )
{
#ifdef SHARP_AVC_REWRITE_OUTPUT
  return Err::m_nOK;
#endif
  MbTransformCoeffs& rcCoeffs = m_cTCoeffs;

  XPel* pucCb   = rcRecYuvBuffer.getMbCbAddr();
  XPel* pucCr   = rcRecYuvBuffer.getMbCrAddr();
  Int   iStride = rcRecYuvBuffer.getCStride();

  if( bPredChroma )
  {
    RNOK( m_pcIntraPrediction->predictChromaBlock( pucCb, pucCr, iStride, rcMbDataAccess.getMbData().getChromaPredMode() ) );
    rcPredMbBuffer.loadChroma( rcRecYuvBuffer );
  }

  // ChromaCbp value is not correct when AVCRewriteFlag is true
  // ROTRS( 0 == uiChromaCbp && !rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag(), Err::m_nOK );

  Int                 iScale;
  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );

  // scaling has already been performed on DC coefficients
  iScale = ( pucScaleU ? pucScaleU[0] : 16 );
  m_pcTransform->invTransformChromaDc( rcCoeffs.get( CIdx(0) ), iScale );
  iScale = ( pucScaleV ? pucScaleV[0] : 16 );
  m_pcTransform->invTransformChromaDc( rcCoeffs.get( CIdx(4) ), iScale );

  if( bAddBaseCoeffsChroma )
  {
    rcCoeffs.add( &rcMbDataAccess.getMbDataAccessBase()->getMbTCoeffs(), false, true );
  }

  RNOK( m_pcTransform->invTransformChromaBlocks( pucCb, iStride, rcCoeffs.get( CIdx(0) ) ) );
  RNOK( m_pcTransform->invTransformChromaBlocks( pucCr, iStride, rcCoeffs.get( CIdx(4) ) ) );

  return Err::m_nOK;
}



ErrVal
MbDecoder::xScale4x4Block( TCoeff*            piCoeff,
                           const UChar*       pucScale,
                           UInt               uiStart,
                           const QpParameter& rcQP )
{
  if( pucScale )
  {
    Int iAdd = ( rcQP.per() <= 3 ? ( 1 << ( 3 - rcQP.per() ) ) : 0 );

    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 4;
    }
  }
  else
  {
    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] *= ( g_aaiDequantCoef[rcQP.rem()][ui] << rcQP.per() );
    }
  }

  return Err::m_nOK;
}


ErrVal
MbDecoder::xScale8x8Block( TCoeff*            piCoeff,
                           const UChar*       pucScale,
                           const QpParameter& rcQP )
{
  Int iAdd = ( rcQP.per() <= 5 ? ( 1 << ( 5 - rcQP.per() ) ) : 0 );

  if( pucScale )
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 6;
    }
  }
  else
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * 16 + iAdd ) << rcQP.per() ) >> 6;
    }
  }

  return Err::m_nOK;
}


ErrVal
MbDecoder::xScaleTCoeffs( MbDataAccess& rcMbDataAccess )
{
  Quantizer cQuantizer;
  cQuantizer.setQp( rcMbDataAccess, false );

  const QpParameter&  cLQp      = cQuantizer.getLumaQp  ();
  const QpParameter&  cCQp      = cQuantizer.getChromaQp();
  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  Bool                b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool                b16x16    = rcMbDataAccess.getMbData().isIntra16x16();
  UInt                uiYScalId = ( bIntra ? ( b8x8 && !b16x16 ? 6 : 0 ) : ( b8x8 ? 7 : 3 ) );
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleY = rcMbDataAccess.getSH().getScalingMatrix( uiYScalId );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );

  //===== store coefficient level values ==
  if (!rcMbDataAccess.getMbData().isPCM() )
      rcMbDataAccess.getMbTCoeffs().storeLevelData();

  //===== copy all coefficients =====
  MbTransformCoeffs& rcTCoeffs = m_cTCoeffs;
  rcTCoeffs.copyFrom( rcMbDataAccess.getMbTCoeffs() );

  //===== luma =====
  if( b16x16 )
  {
    //===== INTRA_16x16 =====
    const Int aaiDequantDcCoef[6] = { 10, 11, 13, 14, 16, 18 };

    Int iScaleY  = aaiDequantDcCoef[cLQp.rem()] << cLQp.per();
    if( pucScaleY )
    {
      iScaleY  *= pucScaleY[0];
      iScaleY >>= 4;
    }

    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcTCoeffs.get( cIdx ), pucScaleY, 1, cLQp ) );
    }
  }
  else if( b8x8 )
  {
    //===== 8x8 BLOCKS =====
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale8x8Block( rcTCoeffs.get8x8( cIdx ), pucScaleY, cLQp ) );
    }
  }
  else
  {
    //===== 4x4 BLOCKS =====
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcTCoeffs.get( cIdx ), pucScaleY, 0, cLQp ) );
    }
  }

  //===== chroma =====
  Int iScaleU  = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
  Int iScaleV  = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
  /* HS: old scaling modified:
     (It did not work for scaling matrices, when QpPer became less than 5 in an FGS enhancement) */
  for( CIdx cIdx; cIdx.isLegal(); cIdx++ )
  {
    RNOK( xScale4x4Block( rcTCoeffs.get( cIdx ), ( cIdx.plane() ? pucScaleV : pucScaleU ), 1, cCQp ) );
  }
  UInt    uiDCIdx;
  TCoeff* piCoeff = rcTCoeffs.get( CIdx(0) );
  for( uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
  {
    piCoeff[16*uiDCIdx] *= iScaleU;
  }
  piCoeff = rcTCoeffs.get( CIdx(4) );
  for( uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
  {
    piCoeff[16*uiDCIdx] *= iScaleV;
  }

  // store the coefficient for non intra 16x16 block
  if (!rcMbDataAccess.getMbData().isIntra16x16() && (!rcMbDataAccess.getMbData().isPCM() ))
    rcMbDataAccess.getMbTCoeffs().copyFrom(rcTCoeffs);

  return Err::m_nOK;
}


ErrVal
MbDecoder::xAddTCoeffs( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase )
{

	UInt uiBCBP = 0;
	UInt uiCoded = 0;
	Bool bCoded = false;
	Bool bChromaAC = false;
	Bool bChromaDC = false;

	rcMbDataAccessBase.getMbTCoeffs().switchLevelCoeffData();

	// Add the luma coefficients and track the new BCBP
	if( rcMbDataAccess.getMbData().isTransformSize8x8() )
	{
		for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
		{
			bCoded = false;

			m_pcTransform->addPrediction8x8Blk( rcMbDataAccess.getMbTCoeffs().get8x8( c8x8Idx ),
				rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx ),
				rcMbDataAccess.getMbData().getQp(),
				rcMbDataAccessBase.getMbData().getQp(), bCoded );

			if( rcMbDataAccess.getMbData().isIntra16x16() )
				AOT(1);

			if( bCoded )
				uiBCBP |= (0x33 << c8x8Idx.b4x4());
		}

	}
	else
	{

		for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
		{
			uiCoded = 0;

			m_pcTransform->addPrediction4x4Blk( rcMbDataAccess.getMbTCoeffs().get( cIdx ),
				rcMbDataAccessBase.getMbTCoeffs().get( cIdx ),
				rcMbDataAccess.getMbData().getQp(),
				rcMbDataAccessBase.getMbData().getQp(), uiCoded );

			if( rcMbDataAccess.getMbData().isIntra16x16() )
			{
				if( *(rcMbDataAccess.getMbTCoeffs().get( cIdx )) )
					uiCoded--;
			}

			if( uiCoded )
				uiBCBP |= (1<<cIdx);
		}

		if( rcMbDataAccess.getMbData().isIntra16x16() )
		{
			uiBCBP = uiBCBP?((1<<16)-1):0;
		}
	}

	// Add the chroma coefficients and update the BCBP
	m_pcTransform->addPredictionChromaBlocks( rcMbDataAccess.getMbTCoeffs().get( CIdx(0) ),
		rcMbDataAccessBase.getMbTCoeffs().get( CIdx(0) ),
		rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbData().getQp() ),
		rcMbDataAccessBase.getSH().getChromaQp( rcMbDataAccessBase.getMbData().getQp() ),
		bChromaDC, bChromaAC );

	m_pcTransform->addPredictionChromaBlocks( rcMbDataAccess.getMbTCoeffs().get( CIdx(4) ),
		rcMbDataAccessBase.getMbTCoeffs().get( CIdx(4) ),
		rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbData().getQp() ),
		rcMbDataAccessBase.getSH().getChromaQp( rcMbDataAccessBase.getMbData().getQp() ),
		bChromaDC, bChromaAC );

	uiBCBP |= (bChromaAC?2:(bChromaDC?1:0))<<16;

	// Update the CBP
	rcMbDataAccess.getMbData().setAndConvertMbExtCbp( uiBCBP );

	// Update the Intra16x16 mode
	if( rcMbDataAccess.getMbData().isIntra16x16() )
	{
		UInt uiMbType = INTRA_4X4 + 1;
		UInt uiPredMode = rcMbDataAccess.getMbData().intraPredMode();
		UInt uiChromaCbp = uiBCBP>>16;
		Bool bACcoded = (uiBCBP && ((1<<16)-1));

		uiMbType += uiPredMode;
        uiMbType += ( bACcoded ) ? 12 : 0;
        uiMbType += uiChromaCbp << 2;

		rcMbDataAccess.getMbData().setMbMode( MbMode(uiMbType) );

		// Sanity checks
		if( rcMbDataAccess.getMbData().intraPredMode() != uiPredMode )
			AOT(1);
		if( rcMbDataAccess.getMbData().getCbpChroma16x16() != uiChromaCbp )
			AOT(1);
		if( rcMbDataAccess.getMbData().isAcCoded() != bACcoded )
			AOT(1);
	}

	rcMbDataAccessBase.getMbTCoeffs().switchLevelCoeffData();

	return Err::m_nOK;

}

H264AVC_NAMESPACE_END
