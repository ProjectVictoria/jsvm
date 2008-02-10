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




#include "H264AVCEncoderLib.h"
#include "MbCoder.h"
#include "H264AVCCommonLib/Tables.h"
#include "H264AVCCommonLib/TraceFile.h"

//JVT-X046 {
#include "CabacWriter.h"
#include "UvlcWriter.h"
//JVT-X046 }

H264AVC_NAMESPACE_BEGIN


MbCoder::MbCoder():
  m_pcMbSymbolWriteIf( NULL ),
  m_pcRateDistortionIf( NULL ),
  m_bInitDone( false ),
  m_bCabac( false ),
  m_bPrevIsSkipped( false )
{
	//JVT-X046 {
	CabacWriter* pcCabacWrite;
	UvlcWriter* pcUvlcWrite;
	CabacWriter::create(pcCabacWrite);
	UvlcWriter::create(pcUvlcWrite);
	m_pcCabacSymbolWriteIf = pcCabacWrite;
	m_pcUvlcSymbolWriteIf = pcUvlcWrite;
	//JVT-X046 }
}


MbCoder::~MbCoder()
{
	//JVT-X046 {
	delete m_pcCabacSymbolWriteIf;
	delete m_pcUvlcSymbolWriteIf;
	//JVT-X046 }
}


ErrVal MbCoder::create( MbCoder*& rpcMbCoder )
{
  rpcMbCoder = new MbCoder;

  ROT( NULL == rpcMbCoder );

  return Err::m_nOK;
}

ErrVal MbCoder::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal MbCoder::initSlice(  const SliceHeader& rcSH,
                            MbSymbolWriteIf*   pcMbSymbolWriteIf,
                            RateDistortionIf*  pcRateDistortionIf )
{
  ROT( NULL == pcMbSymbolWriteIf );
  ROT( NULL == pcRateDistortionIf );

  m_pcMbSymbolWriteIf = pcMbSymbolWriteIf;
  m_pcRateDistortionIf = pcRateDistortionIf;

  m_bCabac          = rcSH.getPPS().getEntropyCodingModeFlag();
  m_bPrevIsSkipped  = false;

  m_bInitDone = true;

  return Err::m_nOK;
}


ErrVal MbCoder::uninit()
{
  m_pcMbSymbolWriteIf = NULL;
  m_pcRateDistortionIf = NULL;

  m_bInitDone = false;
  return Err::m_nOK;
}






ErrVal MbCoder::encode( MbDataAccess& rcMbDataAccess,
                        MbDataAccess* pcMbDataAccessBase,
                        Int           iSpatialScalabilityType,
                        Bool          bTerminateSlice,
                        Bool          bSendTerminateSlice)
{
  ROF( m_bInitDone );
  ROTRS( rcMbDataAccess.getSH().getSliceSkipFlag(), Err::m_nOK );

  rcMbDataAccess.getMbData().setBCBP(0);
  
  //===== skip flag =====
  Bool  bIsCoded  = ! rcMbDataAccess.isSkippedMb();

  RNOK( m_pcMbSymbolWriteIf->skipFlag( rcMbDataAccess, false ) );

  MbSymbolWriteIf *pcCurrentWriter = m_pcMbSymbolWriteIf;
  UInt uiSourceLayer = g_nLayer;
  UInt uiMGSFragment = 0;
  for( uiMGSFragment = 0;
       rcMbDataAccess.getSH().getSPS().getMGSCoeffStop( uiMGSFragment ) < 16;
       uiMGSFragment++ )
  {
    g_nLayer++;
    ETRACE_DECLARE( Bool m_bTraceEnable = true );
    ETRACE_LAYER( g_nLayer );
    ETRACE_NEWMB( rcMbDataAccess.getMbAddress() );
    pcCurrentWriter = pcCurrentWriter->getSymbolWriteIfNextSlice();
    RNOK( pcCurrentWriter->skipFlag( rcMbDataAccess, false ) );
    if( bIsCoded  && rcMbDataAccess.getSH().isMbaffFrame() && ( rcMbDataAccess.isTopMb() || m_bPrevIsSkipped ) )
    {
      RNOK( pcCurrentWriter->fieldFlag( rcMbDataAccess ) );
    }
  }

  {
    g_nLayer = uiSourceLayer;
    ETRACE_DECLARE( Bool m_bTraceEnable = true; );
    ETRACE_LAYER( g_nLayer );
  }

  if( bIsCoded )
  {
    Bool bFieldFlagCoded = true;
    if( bFieldFlagCoded && rcMbDataAccess.getSH().isMbaffFrame() && ( rcMbDataAccess.isTopMb() || m_bPrevIsSkipped ) )
    {
      RNOK( m_pcMbSymbolWriteIf->fieldFlag( rcMbDataAccess) );
    }

    Bool bBaseLayerAvailable = (NULL != pcMbDataAccessBase) && !rcMbDataAccess.getSH().getNoInterLayerPredFlag();

    //===== base layer mode flag and base layer refinement flag =====
    if( bBaseLayerAvailable )
    {
      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() == true )// TMM_ESS
      {
				if( rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() )
				{
					RNOK  ( m_pcMbSymbolWriteIf->BLSkipFlag( rcMbDataAccess ) );
				}
      }
      else  
      {
          ROT  ( rcMbDataAccess.getMbData().getBLSkipFlag () );
      }
    }
    else
    {
      ROT  ( rcMbDataAccess.getMbData().getBLSkipFlag () );
    }

    //===== macroblock mode =====
    if( ! rcMbDataAccess.getMbData().getBLSkipFlag() )
    {
      MbMode  eMbModeOrg = rcMbDataAccess.getMbData().getMbMode();
      MbMode  eMbModeSet = ( eMbModeOrg == INTRA_BL ? INTRA_4X4 : eMbModeOrg );
      rcMbDataAccess.getMbData().setMbMode( eMbModeSet );
      RNOK( m_pcMbSymbolWriteIf->mbMode( rcMbDataAccess ) );
      rcMbDataAccess.getMbData().setMbMode( eMbModeOrg );
    }

    //--- reset motion pred flags ---
    if( rcMbDataAccess.getMbData().getBLSkipFlag() || rcMbDataAccess.getMbData().isIntra() || rcMbDataAccess.getMbData().getMbMode() == MODE_SKIP )
    {
      rcMbDataAccess.getMbMotionData( LIST_0 ).setMotPredFlag( false );
      rcMbDataAccess.getMbMotionData( LIST_1 ).setMotPredFlag( false );
    }
    else if( rcMbDataAccess.getSH().isBSlice() )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP == rcMbDataAccess.getMbData().getBlkMode   ( c8x8Idx.b8x8Index() ) ||
            !rcMbDataAccess            .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), LIST_0 ) )
        {
          rcMbDataAccess.getMbMotionData( LIST_0 ).setMotPredFlag( false, c8x8Idx );
        }
        if( BLK_SKIP == rcMbDataAccess.getMbData().getBlkMode   ( c8x8Idx.b8x8Index() ) ||
            !rcMbDataAccess            .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), LIST_1 ) )
        {
          rcMbDataAccess.getMbMotionData( LIST_1 ).setMotPredFlag( false, c8x8Idx );
        }
      }
    }

    //===== prediction info =====
    if( ! rcMbDataAccess.getMbData().getBLSkipFlag() )
    {
      //===== BLOCK MODES =====
      if( rcMbDataAccess.getMbData().isInter8x8() )
      {
        RNOK( m_pcMbSymbolWriteIf->blockModes( rcMbDataAccess ) );
      }

      if( rcMbDataAccess.getMbData().isPCM() )
      {
        //===== PCM SAMPLES =====
        RNOK( m_pcMbSymbolWriteIf->samplesPCM( rcMbDataAccess ) );
      }
      else if( rcMbDataAccess.getMbData().isIntra() )
      {
        //===== INTRA PREDICTION MODES =====
        RNOK( xWriteIntraPredModes( rcMbDataAccess ) );
      }
      else
      {
        //===== MOTION INFORMATION =====
        MbMode eMbMode = rcMbDataAccess.getMbData().getMbMode();
        if( rcMbDataAccess.getSH().isBSlice() )
        {
          RNOK( xWriteMotionPredFlags( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteMotionPredFlags( rcMbDataAccess, eMbMode, LIST_1 ) );
          RNOK( xWriteReferenceFrames( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteReferenceFrames( rcMbDataAccess, eMbMode, LIST_1 ) );
          RNOK( xWriteMotionVectors  ( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteMotionVectors  ( rcMbDataAccess, eMbMode, LIST_1 ) );
        }
        else
        {
          RNOK( xWriteMotionPredFlags( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteReferenceFrames( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteMotionVectors  ( rcMbDataAccess, eMbMode, LIST_0 ) );
        }
      }
    }


    //===== TEXTURE =====
    if( ! rcMbDataAccess.getMbData().isPCM() )
    {
      Bool bTrafo8x8Flag = ( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() &&
                           ( rcMbDataAccess.getMbData().getBLSkipFlag() ||
                           ( rcMbDataAccess.getMbData().is8x8TrafoFlagPresent( rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag() ) &&
                             !rcMbDataAccess.getMbData().isIntra4x4() ) ) );
 			//-- JVT-R091
      MbSymbolWriteIf *pcMasterWriter = m_pcMbSymbolWriteIf;
      uiMGSFragment = 0;
      uiSourceLayer = g_nLayer;
      while( true )
      {
        RNOK( xWriteTextureInfo( rcMbDataAccess,
                                 pcMbDataAccessBase,
                                 rcMbDataAccess.getMbTCoeffs(),
                                 bTrafo8x8Flag,
                                 rcMbDataAccess.getSH().getSPS().getMGSCoeffStart( uiMGSFragment ),
                                 rcMbDataAccess.getSH().getSPS().getMGSCoeffStop( uiMGSFragment ),
                                 uiMGSFragment ) );
         
        if( rcMbDataAccess.getSH().getSPS().getMGSCoeffStop( uiMGSFragment ) >= 16 )
        {
          break;
        }
        uiMGSFragment++;

        // update bTrafo8x8Flag according to following slices
        bTrafo8x8Flag = rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag();

        m_pcMbSymbolWriteIf = m_pcMbSymbolWriteIf->getSymbolWriteIfNextSlice();

        {
          g_nLayer++;
          ETRACE_DECLARE( Bool m_bTraceEnable = true; );
          ETRACE_LAYER( g_nLayer );
        }

      }
      m_pcMbSymbolWriteIf = pcMasterWriter;

      {
        g_nLayer = uiSourceLayer;
        ETRACE_DECLARE( Bool m_bTraceEnable = true; );
        ETRACE_LAYER( g_nLayer );
      }
    }
  }
  m_bPrevIsSkipped = !bIsCoded;

  ROTRS( ! bSendTerminateSlice, Err::m_nOK );

	//JVT-X046 {
  if (m_uiSliceMode==2)
  {
		if (m_bCabac)
		{
			if ((getBitsWritten() >= m_uiSliceArgument))
			{
				m_pcMbSymbolWriteIf->loadCabacWrite(m_pcCabacSymbolWriteIf);
				RNOK( m_pcMbSymbolWriteIf->terminatingBit ( true ? 1:0 ) );
				RNOK( m_pcMbSymbolWriteIf->finishSlice() );
				bSliceCodedDone=true;
				return Err::m_nOK;
			}
			m_pcCabacSymbolWriteIf->loadCabacWrite(m_pcMbSymbolWriteIf);
		}
		else
		{
			if ((getBitsWritten() >= m_uiSliceArgument))
			{
				m_pcMbSymbolWriteIf->loadUvlcWrite(m_pcUvlcSymbolWriteIf);
				RNOK( m_pcMbSymbolWriteIf->terminatingBit ( true ? 1:0 ) );
				RNOK( m_pcMbSymbolWriteIf->finishSlice() );
				bSliceCodedDone=true;
				return Err::m_nOK;
			}
			m_pcUvlcSymbolWriteIf->loadUvlcWrite(m_pcMbSymbolWriteIf);
		}
	}
	//JVT-X046 }


  //--- write terminating bit ---
  RNOK( m_pcMbSymbolWriteIf->terminatingBit ( bTerminateSlice ? 1:0 ) );

  if( bTerminateSlice )
  {
    RNOK( m_pcMbSymbolWriteIf->finishSlice() );
  }
  MbSymbolWriteIf *pcMasterWriter = m_pcMbSymbolWriteIf;
  uiSourceLayer = g_nLayer;
  for( uiMGSFragment = 0; rcMbDataAccess.getSH().getSPS().getMGSCoeffStop( uiMGSFragment ) < 16; uiMGSFragment++ )
  {
    {
      g_nLayer++;
      ETRACE_DECLARE( Bool m_bTraceEnable = true; );
      ETRACE_LAYER( g_nLayer );
    }
    m_pcMbSymbolWriteIf = m_pcMbSymbolWriteIf->getSymbolWriteIfNextSlice();
    RNOK( m_pcMbSymbolWriteIf->terminatingBit ( bTerminateSlice ? 1:0 ) );
    if( bTerminateSlice )
    {
      RNOK( m_pcMbSymbolWriteIf->finishSlice() );
    }
  }
  m_pcMbSymbolWriteIf = pcMasterWriter;
  {
    g_nLayer = uiSourceLayer;
    ETRACE_DECLARE( Bool m_bTraceEnable = true; );
    ETRACE_LAYER( g_nLayer );
  }
  return Err::m_nOK;
}


ErrVal MbCoder::xWriteIntraPredModes( MbDataAccess& rcMbDataAccess )
{
  ROFRS( rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  if( rcMbDataAccess.getMbData().isIntra4x4() )
  {
    if( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() )
    {
      RNOK( m_pcMbSymbolWriteIf->transformSize8x8Flag( rcMbDataAccess ) );
    }

    if( rcMbDataAccess.getMbData().isTransformSize8x8() )
    {
      for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        RNOK( m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbDataAccess, cIdx ) );
      }
    }
    else
    {
      for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        RNOK( m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbDataAccess, cIdx ) );
      }
    }
  }

  if( rcMbDataAccess.getMbData().isIntra4x4() || rcMbDataAccess.getMbData().isIntra16x16() )
  {
    RNOK( m_pcMbSymbolWriteIf->intraPredModeChroma( rcMbDataAccess ) );
  }

  return Err::m_nOK;
}




ErrVal MbCoder::xWriteBlockMv( MbDataAccess& rcMbDataAccess, B8x8Idx c8x8Idx, ListIdx eLstIdx )
{
  BlkMode eBlkMode = rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() );

  ParIdx8x8 eParIdx = c8x8Idx.b8x8();
  switch( eBlkMode )
  {
    case BLK_8x8:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx ) );
      break;
    }
    case BLK_8x4:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_8x4_0 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_8x4_1 ) );
      break;
    }
    case BLK_4x8:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x8_0 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x8_1 ) );
      break;
    }
    case BLK_4x4:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_0 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_1 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_2 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_3 ) );
      break;
    }
    case BLK_SKIP:
    {
      break;
    }
    default:
    {
      AF();
      return Err::m_nERR;
    }
  }
  return Err::m_nOK;
}




ErrVal
MbCoder::xWriteMotionPredFlags( MbDataAccess&  rcMbDataAccess,
                                MbMode         eMbMode,
                                ListIdx        eLstIdx )
{ 
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  MbDataAccess* pcMbDataAccessBase = rcMbDataAccess.getMbDataAccessBase();
  ROFRS  ( pcMbDataAccessBase,                                    Err::m_nOK );
  ROFRS  ( pcMbDataAccessBase->getMbData().getInCropWindowFlag(), Err::m_nOK );
// JVT-U160 LMI
  ROFRS ( rcMbDataAccess.getSH().getAdaptiveMotionPredictionFlag(), Err::m_nOK );
  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      break;
    }

  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx ) );
      }
      break;
    }

  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      break;
    }

  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      break;
    }

  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode   ( c8x8Idx.b8x8Index() ) &&
           rcMbDataAccess             .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx) )
        {
          RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
        }
      }
      break;
    }

  default:
    {
      AF();
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}


ErrVal
MbCoder::xWriteReferenceFrames( MbDataAccess& rcMbDataAccess,
                                MbMode        eMbMode,
                                ListIdx       eLstIdx )
{
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
  {
    return Err::m_nOK;
  }

  Bool          bPred = rcMbDataAccess.getSH().getAdaptiveBaseModeFlag();
  MbMotionData& rcMot = rcMbDataAccess.getMbMotionData( eLstIdx );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      break;
    }
    
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag() ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx ) );
      }
      break;
    }
    
  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_16x8_0 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_16x8_1 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      break;
    }
    
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_8x16_0 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_8x16_1 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      break;
    }
    
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) &&
            rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) && ( ! bPred || ! rcMot.getMotPredFlag( c8x8Idx.b8x8() ) ) )
        {
          RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
        }
      }
      break;
    }
    
  default:
    {
      AF();
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}




ErrVal
MbCoder::xWriteMotionVectors( MbDataAccess& rcMbDataAccess,
                              MbMode        eMbMode,
                              ListIdx       eLstIdx )
{
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      return Err::m_nOK;
    }
  
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx ) );
      }
      return Err::m_nOK;
    }

  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      return Err::m_nOK;
    }
  
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      return Err::m_nOK;
    }
  
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) )
        {
          RNOK( xWriteBlockMv( rcMbDataAccess, c8x8Idx, eLstIdx ) );
        }
      }
      return Err::m_nOK;
    }
   
  default:
    {
      AF();
      return Err::m_nERR;
    }
  }

  return Err::m_nERR;
}





ErrVal MbCoder::xWriteTextureInfo( MbDataAccess&            rcMbDataAccess,
																	 MbDataAccess*						pcMbDataAccessBase,	// JVT-R091
                                   const MbTransformCoeffs& rcMbTCoeff,
                                   Bool											bTrafo8x8Flag
                                  ,UInt                     uiStart,
                                   UInt                     uiStop,
                                   UInt                     uiMGSFragment
                                   )
{

  Bool bWriteDQp = true;
  if( uiStart != 0 || uiStop != 16 )
  {
    ROT( rcMbDataAccess.getMbData().isIntraButnotIBL() );
  }
  const UInt uiCbp = rcMbDataAccess.getMbData().calcMbCbp( uiStart, uiStop );

  if( uiStart != uiStop && !rcMbDataAccess.getMbData().isIntra16x16() )
  {
    RNOK( m_pcMbSymbolWriteIf->cbp( rcMbDataAccess, uiStart, uiStop ) );
    bWriteDQp = ( 0 != uiCbp );
  }


  if( uiStart != uiStop && bTrafo8x8Flag && (uiCbp & 0x0F) )
  {
    ROT( rcMbDataAccess.getMbData().isIntra16x16() );
    ROT( rcMbDataAccess.getMbData().isIntra4x4  () );
    RNOK( m_pcMbSymbolWriteIf->transformSize8x8Flag( rcMbDataAccess, uiStart, uiStop ) );
  }

  if( uiStart != uiStop && bWriteDQp )
  {
    RNOK( m_pcMbSymbolWriteIf->deltaQp( rcMbDataAccess ) );
  }

  if( uiMGSFragment == 0 ) // required, since we don't have the correct slice header
  if( rcMbDataAccess.getMbData().getBLSkipFlag() ||
     !rcMbDataAccess.getMbData().isIntra() )
  {
    if( rcMbDataAccess.getSH().getAdaptiveResidualPredictionFlag() && pcMbDataAccessBase->getMbData().getInCropWindowFlag() )
    {
      if( ! rcMbDataAccess.getSH().isIntraSlice() )
      {
        RNOK( m_pcMbSymbolWriteIf->resPredFlag( rcMbDataAccess ) );
      }
    }
  }

  if( uiStart != uiStop && rcMbDataAccess.getMbData().isIntra16x16() )
  {
    ROT( uiStart != 0 || uiStop != 16 );
    RNOK( xScanLumaIntra16x16( rcMbDataAccess, rcMbTCoeff, rcMbDataAccess.getMbData().isAcCoded() ) );
    RNOK( xScanChromaBlocks  ( rcMbDataAccess, rcMbTCoeff, rcMbDataAccess.getMbData().getCbpChroma16x16() ) );

    return Err::m_nOK;
  }

  if( uiStart != uiStop )
  {
    if( rcMbDataAccess.getMbData().isTransformSize8x8() )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( (uiCbp >> c8x8Idx.b8x8Index()) & 1 )
        {
          RNOK( m_pcMbSymbolWriteIf->residualBlock8x8( rcMbDataAccess, c8x8Idx, LUMA_SCAN, uiStart, uiStop ) );
        }
      }
    }
    else
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( (uiCbp >> c8x8Idx.b8x8Index()) & 1 )
        {
          for( S4x4Idx cIdx(c8x8Idx); cIdx.isLegal(c8x8Idx); cIdx++ )
          {
            RNOK( xScanLumaBlock( rcMbDataAccess, rcMbTCoeff, cIdx, uiStart, uiStop ) );
          }
        }
      }
    }

    RNOK( xScanChromaBlocks( rcMbDataAccess, rcMbTCoeff, uiCbp >> 4, uiStart, uiStop ) );
  }

  return Err::m_nOK;
}






ErrVal MbCoder::xScanLumaIntra16x16( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, Bool bAC )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, B4x4Idx(0), LUMA_I16_DC ) );
  ROFRS( bAC, Err::m_nOK );

  for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++)
  {
    RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, cIdx, LUMA_I16_AC ) );
  }

  return Err::m_nOK;
}

ErrVal MbCoder::xScanLumaBlock( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, LumaIdx cIdx, UInt uiStart, UInt uiStop )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, cIdx, LUMA_SCAN, uiStart, uiStop ) );
  return Err::m_nOK;
}


ErrVal MbCoder::xScanChromaDc( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, UInt uiStart, UInt uiStop )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(0), CHROMA_DC, uiStart, uiStop ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(4), CHROMA_DC, uiStart, uiStop ) );
  return Err::m_nOK;
}

ErrVal MbCoder::xScanChromaAcU( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, UInt uiStart, UInt uiStop )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(0), CHROMA_AC, uiStart, uiStop ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(1), CHROMA_AC, uiStart, uiStop ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(2), CHROMA_AC, uiStart, uiStop ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(3), CHROMA_AC, uiStart, uiStop ) );
  return Err::m_nOK;
}

ErrVal MbCoder::xScanChromaAcV( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, UInt uiStart, UInt uiStop )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(4), CHROMA_AC, uiStart, uiStop ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(5), CHROMA_AC, uiStart, uiStop ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(6), CHROMA_AC, uiStart, uiStop ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(7), CHROMA_AC, uiStart, uiStop ) );
  return Err::m_nOK;
}

ErrVal MbCoder::xScanChromaBlocks( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, UInt uiChromCbp, UInt uiStart, UInt uiStop )
{
  ROTRS( 1 > uiChromCbp, Err::m_nOK );

  if( uiStart == 0 )
  {
    RNOK( xScanChromaDc ( rcMbDataAccess, rcTCoeff, uiStart, uiStop ) );
  }

  ROTRS( 2 > uiChromCbp, Err::m_nOK );

  if( uiStop > 1 )
  {
    RNOK( xScanChromaAcU( rcMbDataAccess, rcTCoeff, uiStart, uiStop ) );
    RNOK( xScanChromaAcV( rcMbDataAccess, rcTCoeff, uiStart, uiStop ) );
  }
  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
