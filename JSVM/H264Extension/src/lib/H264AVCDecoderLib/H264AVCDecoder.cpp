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
#include "H264AVCDecoder.h"

#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/TraceFile.h"

#include "SliceReader.h"
#include "SliceDecoder.h"

#include "CreaterH264AVCDecoder.h"
#include "ControlMngH264AVCDecoder.h"
#include "FGSSubbandDecoder.h"

#include "GOPDecoder.h"


H264AVC_NAMESPACE_BEGIN


H264AVCDecoder::H264AVCDecoder()
: m_pcSliceReader                 ( NULL  )
, m_pcSliceDecoder                ( NULL  )
, m_pcFrameMng                    ( NULL  )
, m_pcNalUnitParser               ( NULL  )
, m_pcControlMng                  ( NULL  )
, m_pcLoopFilter                  ( NULL  )
, m_pcHeaderSymbolReadIf          ( NULL  )
, m_pcParameterSetMng             ( NULL  )
, m_pcPocCalculator               ( NULL  )
, m_pcSliceHeader                 ( NULL  )
, m_pcPrevSliceHeader             ( NULL  )
, m_bInitDone                     ( false )
, m_bLastFrame                    ( false )
, m_bFrameDone                    ( true  )
, m_bEnhancementLayer             ( false )
, m_bSpatialScalability           ( false )
, m_bBaseLayerIsAVCCompatible     ( false )
, m_uiSPSCount                    ( 0 )
, m_uiRecLayerId                  ( 0 )
, m_uiLastLayerId                 ( MSYS_UINT_MAX )
, m_pcVeryFirstSPS                ( NULL )
, m_bCheckNextSlice               ( false )
, m_iFirstLayerIdx               ( 0 )
, m_iLastLayerIdx                 ( 0 )
, m_iLastPocChecked               (-1 )
, m_iFirstSlicePoc                ( 0 )
, m_bBaseLayerAvcCompliant        ( false )
, m_bDependencyInitialized        ( false )
, m_uiQualityLevelForPrediction   ( 3 )
#if MULTIPLE_LOOP_DECODING
, m_bCompletelyDecodeLayer        ( false )
#endif
{
  ::memset( m_apcMCTFDecoder, 0x00, MAX_LAYERS * sizeof( Void* ) );
  m_pcVeryFirstSliceHeader = NULL;
}



H264AVCDecoder::~H264AVCDecoder()
{
}



ErrVal H264AVCDecoder::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal H264AVCDecoder::init( MCTFDecoder*        apcMCTFDecoder[MAX_LAYERS],
                             SliceReader*        pcSliceReader,
                             SliceDecoder*       pcSliceDecoder,
                             RQFGSDecoder*       pcRQFGSDecoder,
                             FrameMng*           pcFrameMng,
                             NalUnitParser*      pcNalUnitParser,
                             ControlMngIf*       pcControlMng,
                             LoopFilter*         pcLoopFilter,
                             HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                             ParameterSetMng*    pcParameterSetMng,
                             PocCalculator*      pcPocCalculator )
{

  ROT( NULL == pcSliceReader );
  ROT( NULL == pcSliceDecoder );
  ROT( NULL == pcFrameMng );
  ROT( NULL == pcNalUnitParser );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcLoopFilter );
  ROT( NULL == pcHeaderSymbolReadIf );
  ROT( NULL == pcParameterSetMng );
  ROT( NULL == pcPocCalculator );
  ROT( NULL == apcMCTFDecoder );
  ROT( NULL == pcRQFGSDecoder );

  m_pcSliceReader             = pcSliceReader;
  m_pcSliceDecoder            = pcSliceDecoder;
  m_pcRQFGSDecoder            = pcRQFGSDecoder;
  m_pcFrameMng                = pcFrameMng;
  m_pcNalUnitParser           = pcNalUnitParser;
  m_pcControlMng              = pcControlMng;
  m_pcLoopFilter              = pcLoopFilter;
  m_pcHeaderSymbolReadIf      = pcHeaderSymbolReadIf;
  m_pcParameterSetMng         = pcParameterSetMng;
  m_pcPocCalculator           = pcPocCalculator;
  m_pcFGSPicBuffer            = 0;
  m_bEnhancementLayer         = false;
  m_bSpatialScalability       = false;
  m_bBaseLayerIsAVCCompatible = false;
  m_uiSPSCount                = 0;
  m_uiRecLayerId              = 0;
  m_uiLastLayerId             = MSYS_UINT_MAX;
  m_pcVeryFirstSPS            = 0;

  m_bActive = false;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    ROT( NULL == apcMCTFDecoder[uiLayer] );

    m_apcMCTFDecoder[uiLayer] = apcMCTFDecoder[uiLayer];
  }

  m_bInitDone = true;

  return Err::m_nOK;
}



ErrVal H264AVCDecoder::uninit()
{
  m_pcSliceReader         = NULL;
  m_pcSliceDecoder        = NULL;
  m_pcFrameMng            = NULL;
  m_pcNalUnitParser       = NULL;
  m_pcControlMng          = NULL;
  m_pcLoopFilter          = NULL;
  m_pcHeaderSymbolReadIf  = NULL;
  m_pcParameterSetMng     = NULL;
  m_pcPocCalculator       = NULL;
  m_bInitDone             = false;
  m_bLastFrame            = false;
  m_bFrameDone            = true;

  delete m_pcSliceHeader;
  delete m_pcPrevSliceHeader;
  m_pcSliceHeader         = NULL;
  m_pcPrevSliceHeader     = NULL;

  m_pcSliceReader         = NULL;
  m_pcSliceDecoder        = NULL;
  m_pcFrameMng            = NULL;
  m_pcNalUnitParser       = NULL;
  m_pcControlMng          = NULL;
  m_pcLoopFilter          = NULL;
  m_pcHeaderSymbolReadIf  = NULL;
  m_pcParameterSetMng     = NULL;
  m_pcPocCalculator       = NULL;

  delete m_pcVeryFirstSliceHeader;
  m_pcVeryFirstSliceHeader = NULL;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_apcMCTFDecoder[uiLayer] = NULL;
  }

  m_bInitDone = false;
  
  return Err::m_nOK;
}



ErrVal H264AVCDecoder::create( H264AVCDecoder*& rpcH264AVCDecoder )
{
  rpcH264AVCDecoder = new H264AVCDecoder;
  ROT( NULL == rpcH264AVCDecoder );
  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::calculatePoc( NalUnitType   eNalUnitType,
                              SliceHeader&  rcSliceHeader,
                              Int&          slicePoc )
{
  PocCalculator *pcLocalPocCalculator;

  if( eNalUnitType == NAL_UNIT_CODED_SLICE ||  eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    m_pcPocCalculator->copy( pcLocalPocCalculator );
  }
  else
  {
    m_apcMCTFDecoder[m_iFirstLayerIdx]->getPocCalculator()->copy( pcLocalPocCalculator );
  }

  pcLocalPocCalculator->calculatePoc( rcSliceHeader );

  slicePoc = rcSliceHeader.getPoc();

  pcLocalPocCalculator->destroy();

  return Err::m_nOK;
}


// not tested with multiple slices
ErrVal
H264AVCDecoder::checkSliceLayerDependency( BinDataAccessor*  pcBinDataAccessor,
                                           Bool&             bFinishChecking )
{
  Bool bEos;
  NalUnitType eNalUnitType;
  SliceHeader* pcSliceHeader = NULL;
  Int slicePoc;

  bFinishChecking = false;
  ROT( NULL == pcBinDataAccessor );

  if( m_bDependencyInitialized )
  {
    bFinishChecking = true;
    return Err::m_nOK;
  }

  bEos = ( NULL == pcBinDataAccessor->data() ) || ( 0 == pcBinDataAccessor->size() );
  slicePoc = 0;

  if( ! bEos )
  {
    RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, NULL ) );

    eNalUnitType = m_pcNalUnitParser->getNalUnitType();

    if( eNalUnitType != NAL_UNIT_CODED_SLICE        &&
        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR      && 
        eNalUnitType != NAL_UNIT_CODED_SLICE_SCALABLE && 
        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
    {
      // NAL units other than slices are ignored
      if(! m_bCheckNextSlice )
        // skipped, without looking further
        bFinishChecking = true;

      return Err::m_nOK;
    }
    else
    {
      // read the slice header
      RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser->getNalUnitType   (),
                                              m_pcNalUnitParser->getNalRefIdc     (),
                                              m_pcNalUnitParser->getLayerId       (),
                                              m_pcNalUnitParser->getTemporalLevel (),
                                              m_pcNalUnitParser->getQualityLevel  (),
                                              pcSliceHeader ) );

      calculatePoc( eNalUnitType, *pcSliceHeader, slicePoc );

      // F slices are also ignored
      if( pcSliceHeader->getSliceType() == F_SLICE )
      {
        if(! m_bCheckNextSlice )
          // skipped, without looking further
          bFinishChecking = true;

        return Err::m_nOK;
      }

      if( slicePoc == m_iLastPocChecked)
      {
        // for the same picture with all its layers, we only check the dependency once
        bFinishChecking = true;

        if( pcSliceHeader != NULL )
          delete pcSliceHeader;

        printf("   >> same frame.\n");

        return Err::m_nOK;
      }

      if( ! m_bCheckNextSlice )
      {
        m_iFirstLayerIdx = m_pcNalUnitParser->getLayerId();

        m_iFirstSlicePoc = slicePoc;

        if( eNalUnitType == NAL_UNIT_CODED_SLICE ||  eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
          m_bBaseLayerAvcCompliant = true;
        else
          m_bBaseLayerAvcCompliant = false;
      }

      if( slicePoc == m_iFirstSlicePoc )
      {
        m_iLastLayerIdx = m_pcNalUnitParser->getLayerId();

        if (m_iLastLayerIdx == 0)
        {
          m_auiBaseLayerId[m_iLastLayerIdx]      = MSYS_UINT_MAX;
          m_auiBaseQualityLevel[m_iLastLayerIdx] = 0;
#if MULTIPLE_LOOP_DECODING
          m_abCompletlyDecodeBaseLayer[m_iLastLayerIdx] = false;
#endif
        }
        else
        {
          m_auiBaseLayerId[m_iLastLayerIdx]      = pcSliceHeader->getBaseLayerId();
          m_auiBaseQualityLevel[m_iLastLayerIdx] = pcSliceHeader->getBaseQualityLevel();
#if MULTIPLE_LOOP_DECODING
          m_abCompletlyDecodeBaseLayer[m_iLastLayerIdx] = pcSliceHeader->getSPS().getAlwaysDecodeBaseLayer();
#endif
        }
      }

      m_bCheckNextSlice = true;
    }
  }

  if( bEos || ( m_bCheckNextSlice && m_iFirstLayerIdx == m_pcNalUnitParser->getLayerId() && slicePoc != m_iFirstSlicePoc ))
  {
    // setup the state information for the previous slices
    bFinishChecking   = true;
    m_bCheckNextSlice = false;
    m_iLastPocChecked = m_iFirstSlicePoc;

    m_apcMCTFDecoder[m_iLastLayerIdx]->setQualityLevelForPrediction( 3 );
    if( m_iFirstLayerIdx < m_iLastLayerIdx )
    {
      // set the base layer dependency
      if( m_iFirstLayerIdx == 0 )
      {
        if( m_bBaseLayerAvcCompliant )
        {
          setQualityLevelForPrediction( m_auiBaseQualityLevel[1] );
#if MULTIPLE_LOOP_DECODING
          setCompletelyDecodeLayer( m_abCompletlyDecodeBaseLayer[1] );
#endif
        }
        else
        {
          m_apcMCTFDecoder[0]->setQualityLevelForPrediction( m_auiBaseQualityLevel[1] );
#if MULTIPLE_LOOP_DECODING
          m_apcMCTFDecoder[0]->setCompletelyDecodeLayer( m_abCompletlyDecodeBaseLayer[1] );
#endif
        }
      }

      for( Int iLayer = (m_iFirstLayerIdx == 0) ? 1 : m_iFirstLayerIdx; 
        iLayer <= m_iLastLayerIdx - 1; iLayer ++ )
      {
        m_apcMCTFDecoder[iLayer]->setQualityLevelForPrediction( m_auiBaseQualityLevel[iLayer + 1] );
#if MULTIPLE_LOOP_DECODING
        m_apcMCTFDecoder[iLayer]->setCompletelyDecodeLayer( m_abCompletlyDecodeBaseLayer[iLayer + 1] );
#endif
      }
    }

    m_bDependencyInitialized = true;
  }

  if( pcSliceHeader != NULL )
    delete pcSliceHeader;

  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor,
                            UInt&             ruiNalUnitType,
                            UInt&             ruiMbX,
                            UInt&             ruiMbY,
                            UInt&             ruiSize )
{
  ROF( m_bInitDone );

  ROT( NULL == pcBinDataAccessor );
  if ( NULL == pcBinDataAccessor->data() || 0 == pcBinDataAccessor->size() )
  {
    // switch 
    SliceHeader* pTmp = m_pcSliceHeader;
    m_pcSliceHeader   = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader = pTmp;


    m_bLastFrame = true;
    delete m_pcSliceHeader;
    m_pcSliceHeader = 0;
    return Err::m_nOK;
  }

  Bool KeyPicFlag = false;
  RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, &KeyPicFlag ) );

  ruiNalUnitType = m_pcNalUnitParser->getNalUnitType();
  
  switch ( m_pcNalUnitParser->getNalUnitType() )
  {
  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
    RNOK( xStartSlice() );
    RNOK( m_pcControlMng      ->initSlice0(m_pcSliceHeader) );
    m_pcSliceHeader->setKeyPictureFlag (KeyPicFlag);
    m_bActive = true;
    break;

  case NAL_UNIT_SPS:
    {
      SequenceParameterSet* pcSPS = NULL;
      RNOK( SequenceParameterSet::create  ( pcSPS   ) );
      RNOK( pcSPS               ->read    ( m_pcHeaderSymbolReadIf,
                                            m_pcNalUnitParser->getNalUnitType() ) );
      // It is assumed that layer 0 and layer 1 use the first two SPSs, respectively.
      if( NULL == m_pcVeryFirstSPS )
      {
        setVeryFirstSPS( pcSPS );
      }
      if (m_uiSPSCount == 1)
      {
        m_bEnhancementLayer = true;
        m_bSpatialScalability = pcSPS->getFrameHeightInMbs() != m_pcVeryFirstSPS->getFrameHeightInMbs();
      }
      m_uiSPSCount++;
      RNOK( m_pcParameterSetMng ->store   ( pcSPS   ) );

      // Copy simple priority ID mapping from SPS to NAL unit parser
      if ( !pcSPS->getNalUnitExtFlag() )
      {
        for ( UInt uiPriId = 0; uiPriId < pcSPS->getNumSimplePriIdVals(); uiPriId++)
        {
            UInt uiLayer, uiTempLevel, uiQualLevel;
            pcSPS->getSimplePriorityMap( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
            m_pcNalUnitParser->setSimplePriorityMap( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
        }
      }


      ruiMbX  = pcSPS->getFrameWidthInMbs ();
      ruiMbY  = pcSPS->getFrameHeightInMbs();
      ruiSize = max( ruiSize, ( (ruiMbX << 3 ) + YUV_X_MARGIN ) * ( ( ruiMbY << 3 ) + YUV_Y_MARGIN ) * 6 );
      m_pcControlMng->initSPS(*pcSPS, m_uiSPSCount-1); // TMM_ESS
    }
    break;

  case NAL_UNIT_PPS:
    {
      PictureParameterSet* pcPPS = NULL;
      RNOK( PictureParameterSet::create( pcPPS  ) );
      RNOK( pcPPS->read( m_pcHeaderSymbolReadIf,
                         m_pcNalUnitParser->getNalUnitType() ) );
      RNOK( m_pcParameterSetMng->store( pcPPS   ) );
    }
    break;

  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    {
      RNOK( xStartSlice() );
      RNOK( m_pcControlMng      ->initSlice0(m_pcSliceHeader) );
	    m_pcSliceHeader->setKeyPictureFlag (KeyPicFlag);
    }
    break;

  case NAL_UNIT_SEI:
    {
      //===== just for trace file =====
      SEI::MessageList  cMessageList;
      RNOK( SEI::read( m_pcHeaderSymbolReadIf, cMessageList ) );
      while( ! cMessageList.empty() )
      {
        SEI::SEIMessage*  pcSEIMessage = cMessageList.popBack();
        delete pcSEIMessage;
      }
    }
    break;

  default:
    return Err::m_nERR;
    break;
  }
  return Err::m_nOK;
}



ErrVal
H264AVCDecoder::getBaseLayerData( IntFrame*&      pcFrame,
                                  IntFrame*&      pcResidual,
                                  MbDataCtrl*&    pcMbDataCtrl,
                                  Bool&           rbConstrainedIPred,
                                  Bool&           rbSpatialScalability,
                                  UInt            uiLayerId,
                                  UInt            uiBaseLayerId,
                                  Int             iPoc )
{
  if( uiBaseLayerId || m_apcMCTFDecoder[uiBaseLayerId]->isActive() )
  {
    //===== base layer is scalable extension =====
    //--- get spatial resolution ratio ---
    // TMM_ESS {
	  if( m_apcMCTFDecoder[uiLayerId]->getFrameHeight() != m_apcMCTFDecoder[uiBaseLayerId]->getFrameHeight() &&
        m_apcMCTFDecoder[uiLayerId]->getFrameWidth () != m_apcMCTFDecoder[uiBaseLayerId]->getFrameWidth ()   )
    {
      rbSpatialScalability = true;
    }
    else
    {
      rbSpatialScalability = false;
    }
	  // TMM_ESS }

    //--- get data ---
    RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerData( pcFrame, pcResidual, pcMbDataCtrl, rbConstrainedIPred, rbSpatialScalability, iPoc ) );
  }
  else
  {
    //===== base layer is standard H.264/AVC =====
    //--- get spatial resolution ratio ---
    // TMM_ESS {
	  if( m_apcMCTFDecoder[uiLayerId]->getFrameHeight() == 16 * m_pcVeryFirstSliceHeader->getSPS().getFrameHeightInMbs () &&
        m_apcMCTFDecoder[uiLayerId]->getFrameWidth () == 16 * m_pcVeryFirstSliceHeader->getSPS().getFrameWidthInMbs  ()   )
    {
      rbSpatialScalability = false;
    }
    else
    {
      rbSpatialScalability = true;
    }
	  // TMM_ESS }

    FrameUnit*  pcFrameUnit = m_pcFrameMng->getReconstructedFrameUnit( iPoc );
    ROF( pcFrameUnit );

    pcFrame             = rbSpatialScalability ? m_pcFrameMng->getRefinementIntFrame2() : m_pcFrameMng->getRefinementIntFrame();
    pcResidual          = pcFrameUnit ->getResidual();
    pcMbDataCtrl        = pcFrameUnit ->getMbDataCtrl();
    rbConstrainedIPred  = pcFrameUnit ->getContrainedIntraPred();
  }

  return Err::m_nOK;
}




ErrVal
H264AVCDecoder::process( PicBuffer*       pcPicBuffer,
                         PicBufferList&   rcPicBufferOutputList,
                         PicBufferList&   rcPicBufferUnusedList,
                         PicBufferList&   rcPicBufferReleaseList )
{
  ROF( m_bInitDone );

  RNOK( xInitSlice( m_pcSliceHeader ) );


  if( m_bLastFrame )
  {
    if( m_uiRecLayerId > 0 || !m_bBaseLayerIsAVCCompatible ) // we have an MCTF reconstruction layer
    {
      PicBufferList cDummyList;
      Int           iMaxPoc;
      for( UInt uiLayer = 0; uiLayer < m_uiRecLayerId; uiLayer++ )
      {
        if( uiLayer == 0 && m_bBaseLayerIsAVCCompatible )
        {
          RNOK( m_pcFrameMng->outputAll() );
          RNOK( m_pcFrameMng->setPicBufferLists( cDummyList, rcPicBufferReleaseList ) );
        }
        RNOK( m_apcMCTFDecoder[uiLayer]       ->finishProcess( cDummyList,
                                                               rcPicBufferUnusedList,
                                                               iMaxPoc ) );
      }
      RNOK  ( m_apcMCTFDecoder[m_uiRecLayerId]->finishProcess( rcPicBufferOutputList,
                                                               rcPicBufferUnusedList,
                                                               iMaxPoc = MSYS_INT_MIN ) );
    }
    else
    {
      //===== output all remaining frames in decoded picture buffer =====
      RNOK( m_pcFrameMng->outputAll() );
      RNOK( m_pcFrameMng->setPicBufferLists( rcPicBufferOutputList, rcPicBufferReleaseList ) );
    }

    rcPicBufferUnusedList.pushBack( pcPicBuffer );
    return Err::m_nOK;
  }

  const NalUnitType eNalUnitType  = m_pcNalUnitParser->getNalUnitType();

  //====== check for AVC baselayer FGS refinement =====
  if( ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ) &&
      m_bActive &&
      m_pcSliceHeader->getLayerId()   == 0 &&
      m_pcSliceHeader->getSliceType() == F_SLICE ) 
      
  {
    RNOK( xDecodeFGSRefinement( m_pcSliceHeader, pcPicBuffer ) );

    rcPicBufferUnusedList.pushBack( pcPicBuffer );
    RNOK( m_pcNalUnitParser->closeNalUnit() );
    return Err::m_nOK;
  }


  //===== decode NAL unit =====
  switch( eNalUnitType )
  {
  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    {
      ROT( m_pcSliceHeader == NULL );
      m_uiLastLayerId = m_pcSliceHeader->getLayerId();
      
      PicBufferList   cDummyList;
      PicBufferList&  rcOutputList  = ( m_uiLastLayerId == m_uiRecLayerId ? rcPicBufferOutputList : cDummyList );
      Bool            bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId );
      RNOK( m_apcMCTFDecoder[m_uiLastLayerId] ->process     ( m_pcSliceHeader,
                                                              pcPicBuffer,
                                                              rcOutputList,
                                                              rcPicBufferUnusedList,
                                                              bHighestLayer ) );
      RNOK( m_pcNalUnitParser                 ->closeNalUnit() );
      return Err::m_nOK;
    }
    break;

  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
    {
      ROF( m_pcSliceHeader );
      ROF( m_pcSliceHeader->getLayerId() == 0 );
      m_uiLastLayerId    = m_pcSliceHeader->getLayerId();

      RNOK( xProcessSlice( *m_pcSliceHeader, m_pcPrevSliceHeader, pcPicBuffer ) );

      PicBufferList   cDummyList;
      PicBufferList&  rcOutputList  = ( !m_bEnhancementLayer ? rcPicBufferOutputList : cDummyList );
      RNOK( m_pcFrameMng->setPicBufferLists( rcOutputList, rcPicBufferReleaseList ) );
    }
    break;
  
  case NAL_UNIT_SPS:
    {
      break;
    }


  case NAL_UNIT_PPS:
  case NAL_UNIT_SEI:
    {
    }
    break;
  
  default:
    {
      AOT(1);
      return Err::m_nERR;
    }
    break;
  }

  rcPicBufferUnusedList.pushBack( pcPicBuffer );
  RNOK( m_pcNalUnitParser->closeNalUnit() );

  return Err::m_nOK;
}





ErrVal H264AVCDecoder::xStartSlice()
{
  delete m_pcPrevSliceHeader;
  m_pcPrevSliceHeader = m_pcSliceHeader;
  m_pcSliceHeader     = NULL;

  RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser->getNalUnitType   (),
                                          m_pcNalUnitParser->getNalRefIdc     (),
                                          m_pcNalUnitParser->getLayerId       (),
                                          m_pcNalUnitParser->getTemporalLevel (),
                                          m_pcNalUnitParser->getQualityLevel  (),
                                          m_pcSliceHeader ) );

  return Err::m_nOK;
}



ErrVal
H264AVCDecoder::xZeroIntraMacroblocks( IntFrame*    pcFrame,
                                       MbDataCtrl*  pcMbDataCtrl,
                                       SliceHeader* pcSliceHeader )
{
  IntYuvPicBuffer*  pcPicBuffer       = pcFrame->getFullPelYuvBuffer ();
  UInt              uiFrameWidthInMB  = pcSliceHeader->getSPS().getFrameWidthInMbs ();
  UInt              uiFrameHeightInMB = pcSliceHeader->getSPS().getFrameHeightInMbs();
  UInt              uiMbNumber        = uiFrameWidthInMB * uiFrameHeightInMB;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  IntYuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

  for( UInt uiMbIndex = 0; uiMbIndex < uiMbNumber; uiMbIndex++ )
  {
    MbDataAccess* pcMbDataAccess  = 0;
    RNOK( m_pcControlMng->initMbForFiltering( *pcMbDataAccess, uiMbIndex ) );

    UInt          uiMbY           = uiMbIndex / uiFrameWidthInMB;
    UInt          uiMbX           = uiMbIndex % uiFrameWidthInMB;
    RNOK( pcMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );


    if( pcMbDataAccess->getMbData().isIntra() )
    {
      pcPicBuffer->loadBuffer( &cZeroMbBuffer );
    }
  }

  return Err::m_nOK;
}



ErrVal
H264AVCDecoder::xProcessSlice( SliceHeader& rcSH,
                               SliceHeader* pcPrevSH,
                               PicBuffer*&  rpcPicBuffer )
{
  UInt  uiMbRead;
  Bool  bNewFrame;
  Bool  bNewPic;

  
  //===== set some basic parameters =====
  if( NULL == m_pcVeryFirstSliceHeader )
  {
    m_pcVeryFirstSliceHeader  = new SliceHeader( rcSH.getSPS(), rcSH.getPPS() );
  }

  
  printf("  Frame %4d ( LId 0, TL X, QL 0, AVC-%c, BId-1, AP 0, QP%3d )\n",
    rcSH.getPoc                    (),
    rcSH.getSliceType              () == B_SLICE ? 'B' : 
    rcSH.getSliceType              () == P_SLICE ? 'P' : 'I',
    rcSH.getPicQp                  () );


  //===== check if new pictures and initialization =====
  rcSH.compare( pcPrevSH, bNewFrame );
  bNewPic = bNewFrame;

  if( bNewFrame || m_bFrameDone )
  {
    RNOK( m_pcFrameMng->initFrame( rcSH, rpcPicBuffer ) );
    rpcPicBuffer = NULL;
  }
  if( bNewPic )
  {
    RNOK( m_pcFrameMng->initPic  ( rcSH ) );
  }
  else
  {
    rcSH.setFrameUnit( pcPrevSH->getFrameUnit() );
  }


  //===== set reference lists =====
  RNOK( m_pcFrameMng->setRefPicLists( rcSH, false ) );


  //===== parse slice =====
  RNOK( m_pcControlMng  ->initSlice ( rcSH, PARSE_PROCESS ) );
  RNOK( m_pcSliceReader ->process   ( rcSH, uiMbRead ) );


  //===== decode slice =====
  Bool  bKeyPicture     = rcSH.getKeyPictureFlag();
  Bool  bConstrainedIP  = rcSH.getPPS().getConstrainedIntraPredFlag();
  Bool  bReconstruct    = !m_bEnhancementLayer || ! bConstrainedIP;
#if MULTIPLE_LOOP_DECODING
  bReconstruct = ( bReconstruct || m_bCompletelyDecodeLayer );
#endif
  RNOK( m_pcControlMng  ->initSlice ( rcSH, DECODE_PROCESS ) );
  RNOK( m_pcSliceDecoder->process   ( rcSH, bReconstruct, uiMbRead ) );

  Bool bPicDone;
  RNOK( m_pcControlMng->finishSlice( rcSH, bPicDone, m_bFrameDone ) );

  if( bPicDone )
  {
    // copy intra and inter prediction signal
    m_pcFrameMng->getPredictionIntFrame()->getFullPelYuvBuffer()->copy( rcSH.getFrameUnit()->getFGSIntFrame()->getFullPelYuvBuffer());
    // delete intra prediction
    RNOK( m_pcControlMng->initSlice( rcSH, POST_PROCESS));
    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getFGSIntFrame(), rcSH.getFrameUnit()->getMbDataCtrl(), &rcSH ) );
    // add residual and intra signal
    rcSH.getFrameUnit()->getFGSIntFrame()->add( rcSH.getFrameUnit()->getResidual() );
    rcSH.getFrameUnit()->getFGSIntFrame()->clip();

    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getResidual(),
                                 rcSH.getFrameUnit()->getMbDataCtrl(), m_pcVeryFirstSliceHeader ) );

    //===== deblocking of base representation =====
    RNOK( m_pcLoopFilter->process( rcSH ) );
  
    
    RNOK( m_pcFrameMng->storePicture( rcSH ) );

    //===== init FGS decoder =====
    RNOK( m_pcRQFGSDecoder->initPicture( &rcSH, rcSH.getFrameUnit()->getMbDataCtrl() ) );
  }

  if( m_bFrameDone )
  {
    DTRACE_NEWFRAME;
  }

  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::xInitSlice( SliceHeader* pcSliceHeader )
{
  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
// *LMH: Inverse MCTF
    UInt  uiLastLayer;
    if( m_uiLastLayerId != MSYS_UINT_MAX && m_uiLastLayerId == m_uiRecLayerId )
      uiLastLayer = m_uiRecLayerId;
    else
      uiLastLayer = MAX_LAYERS;
    RNOK( m_apcMCTFDecoder[uiLayer]->initSlice( pcSliceHeader, uiLastLayer ) );
  }
  ROFRS( m_bActive, Err::m_nOK );

  //===== calculate POC =====
  if( pcSliceHeader && pcSliceHeader->getLayerId() == 0 )
  {
    RNOK( m_pcPocCalculator->calculatePoc( *pcSliceHeader ) );
  }

  //===== check if an FGS enhancement needs to be reconstructed =====
  if( m_pcRQFGSDecoder->isInitialized  ()                                                               &&
      m_pcRQFGSDecoder->getSliceHeader ()->getLayerId() == 0                                            &&
    (!pcSliceHeader                                                                                     ||
      pcSliceHeader->getLayerId       () != m_pcRQFGSDecoder->getSliceHeader()->getLayerId      ()      ||
      pcSliceHeader->getQualityLevel  () != m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  ||
      pcSliceHeader->getPoc           () != m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()        ) )
  {
    RNOK( xReconstructLastFGS() );
  }

  return Err::m_nOK;
}



ErrVal
H264AVCDecoder::xReconstructLastFGS()
{
  MbDataCtrl*   pcMbDataCtrl        = m_pcRQFGSDecoder->getMbDataCtrl   ();
  SliceHeader*  pcSliceHeader       = m_pcRQFGSDecoder->getSliceHeader  ();
  IntFrame*     pcResidual          = m_pcRQFGSDecoder->getSliceHeader()->getFrameUnit()->getResidual();
  IntFrame*     pcRecFrame          = m_pcRQFGSDecoder->getSliceHeader()->getFrameUnit()->getFGSIntFrame();
  IntFrame*     pcILPredFrame       = m_pcFrameMng    ->getRefinementIntFrame();
  IntFrame*     pcILPredFrameSpatial= m_pcFrameMng    ->getRefinementIntFrame2();
  Bool          bReconstructFGS     = m_pcRQFGSDecoder->changed();

  //===== reconstruct FGS =====
  if( bReconstructFGS )
  {
    RNOK( m_pcRQFGSDecoder->reconstruct   ( pcRecFrame ) );
    RNOK( pcResidual      ->copy          ( pcRecFrame ) )
    RNOK( xZeroIntraMacroblocks           ( pcResidual, pcMbDataCtrl, pcSliceHeader ) );
    RNOK( pcRecFrame      ->add           ( m_pcFrameMng->getPredictionIntFrame() ) );
    RNOK( pcRecFrame      ->clip          () );
  }
  RNOK  ( m_pcRQFGSDecoder->finishPicture () );

  //===== store intra signal for inter-layer prediction =====
  if( m_bEnhancementLayer )
  {
    RNOK( pcILPredFrame->copy( pcRecFrame ) );
  }

  //===== loop filter =====
#if MULTIPLE_LOOP_DECODING
  if( !m_bEnhancementLayer || m_bCompletelyDecodeLayer /* for in-layer mc prediction */ )
#else
  if( !m_bEnhancementLayer )
#endif
  {
    RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcRecFrame->getFullPelYuvBuffer() ) );
  }

  //===== store in FGS pic buffer =====
  if( bReconstructFGS )
  {
    //===== update DPB =====
    m_pcFrameMng->storeFGSPicture( m_pcFGSPicBuffer );
    m_pcFGSPicBuffer = NULL;
  }


  //===== loop-filter for spatial scalable coding =====
  if( m_bEnhancementLayer && m_bSpatialScalability )
  {
    RNOK( pcILPredFrameSpatial->copy( pcILPredFrame ) );
    
#if MULTIPLE_LOOP_DECODING
    if( pcSliceHeader->getFrameUnit()->getContrainedIntraPred() && !m_bCompletelyDecodeLayer )
#else
    if( pcSliceHeader->getFrameUnit()->getContrainedIntraPred() )
#endif
    {
      m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );
      RNOK( m_pcLoopFilter->process( *m_pcVeryFirstSliceHeader,
                                     pcILPredFrameSpatial,
                                     pcMbDataCtrl,
                                     pcMbDataCtrl,
                                     m_pcVeryFirstSliceHeader->getSPS().getFrameWidthInMbs(),
                                     NULL,
                                     NULL,
                                     false ) );  // SSUN@SHARP
      m_pcLoopFilter->setFilterMode();
    }
    else
    {
      RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcILPredFrameSpatial->getFullPelYuvBuffer() ) );
    }
  }

  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::xDecodeFGSRefinement( SliceHeader*& rpcSliceHeader, PicBuffer*& rpcPicBuffer )
{
  ROFRS( m_pcRQFGSDecoder->isInitialized(), Err::m_nOK );

  //===== check slice header =====
  if(!m_pcRQFGSDecoder->isFinished    ()                                                                 &&
      m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()      == rpcSliceHeader->getPoc           () &&
      m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  == rpcSliceHeader->getQualityLevel  ()  )
  {

    if( rpcSliceHeader->getQualityLevel() <= m_uiQualityLevelForPrediction )
    {
      printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, PrRef,              QP%3d )\n",
        rpcSliceHeader->getPoc                    (),
        rpcSliceHeader->getLayerId                (),
        rpcSliceHeader->getTemporalLevel          (),
        rpcSliceHeader->getQualityLevel           (),
        rpcSliceHeader->getPicQp                  () );

      //===== set mem when first FGS layer ====
      if( rpcSliceHeader->getQualityLevel() == 1 )
      {
        ROT( m_pcFGSPicBuffer );
        ROF( rpcPicBuffer );

        m_pcFGSPicBuffer  = rpcPicBuffer;
        rpcPicBuffer      = 0;
      }

      RNOK( m_pcRQFGSDecoder->decodeNextLayer( rpcSliceHeader ) );
    }


    //===== switch slice headers and update =====
    SliceHeader*  pcTemp  = m_pcSliceHeader;
    m_pcSliceHeader       = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader   = pcTemp;
  }

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END

