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

#include "H264AVCCommonLib/CFMO.h"


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
, m_pcPrefixSliceHeader			  ( NULL  )//prefix unit
, m_pcPrevSliceHeader             ( NULL  )
, m_pcTempSliceHeader			        ( NULL  )//EIDR bug-fix
, m_pcSliceHeader_backup          ( NULL  ) // JVT-Q054 Red. Picture
, m_bFirstSliceHeaderBackup       ( true  ) // JVT-Q054 Red. Picture
, m_bRedundantPic                 ( false ) // JVT-Q054 Red. Picture
, m_bInitDone                     ( false )
, m_bLastFrame                    ( false )
, m_bFrameDone                    ( true  )
, m_bEnhancementLayer             ( false )
, m_bSpatialScalability           ( false )
, m_bBaseLayerIsAVCCompatible     ( false )
, m_bNewSPS                       ( false )
, m_bReconstruct                  ( false )
, m_uiRecLayerId                  ( 0 )
, m_uiLastLayerId                 ( MSYS_UINT_MAX )
, m_pcVeryFirstSPS                ( NULL )
, m_bCheckNextSlice               ( false )
, m_iFirstLayerIdx                ( 0 )
, m_iLastLayerIdx                 ( 0 )
, m_iLastPocChecked               (-1 )
, m_iFirstSlicePoc                ( 0 )
, m_bBaseLayerAvcCompliant        ( false )
, m_bDependencyInitialized        ( false )
, m_uiQualityLevelForPrediction   ( 15 )
, m_pcNonRequiredSei			  ( NULL )
, m_uiNonRequiredSeiReadFlag	  ( 0 )
, m_uiNonRequiredSeiRead    	  ( 0 )
, m_uiPrevPicLayer				  ( 0 )
, m_uiCurrPicLayer				  ( 0 )
//JVT-P031
, m_uiNumberOfSPS                 ( 0 )
, m_uiDecodedLayer                ( 0 )
, m_uiNumOfNALInAU                ( 0 )
//~JVT-P031
, m_pcBaseLayerCtrlEL						  ( 0 )		// ICU/ETRI FGS_MOT_USE
, m_iCurNalSpatialLayer		(-1)
, m_iNextNalSpatialLayer	(-1)
, m_iCurNalPOC				(-1)
, m_iNextNalPOC				(-1)
, m_bCurNalIsEndOfPic		(false)
, m_iCurNalFirstMb			(-1)
//JVT-T054{
, m_bLastNalInAU                  ( false )
, m_bCGSSNRInAU                   ( false )
//JVT-T054}
, m_bBaseSVCActive                ( false ) //JVT-T054_FIX
, m_bLastFrameReconstructed       ( false ) //JVT-T054_FIX
#ifdef SHARP_AVC_REWRITE_OUTPUT
,m_pcAvcRewriteEncoder      (NULL)   // JVT-V035
,m_avcRewriteBindata        (NULL)
,m_avcRewriteBinDataBuffer  (NULL)
,m_avcRewriteBinDataAccessor (NULL)
,m_avcRewriteBufsize        (0)
,m_avcRewriteFlag           (false)
#endif

{
  ::memset( m_apcMCTFDecoder, 0x00, MAX_LAYERS * sizeof( Void* ) );
  m_pcVeryFirstSliceHeader = NULL;

  UInt uiLayer;
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
      m_uiSPSId[uiLayer] = 0;
      m_abMGSAtLayer[uiLayer] = false; //MGS_FIX_FT_09_2007               
  }

//	TMM	EC {{
	m_uiNextFrameNum	=	0;
	m_uiNextLayerId		=	0;
	m_uiNextPoc				=	0;
	m_uiNumLayers			=	1;
	m_uiMaxGopSize		=	16;
	m_uiMaxDecompositionStages	=	4;
	m_uiMaxLayerId	=	0;
	//JVT-W049 {
	m_uiNextFrameNumRedu  = 0;
	m_uiNextLayerIdRedu   = 0;
	m_uiNextPocLsbRedu    = 0;
	bKeyPicReduUseAvc     = false;
  bIfNextFrame          = false;
	uiLastFrameNum        = 0;
	uiLastPocLsb          = 0;
  //JVT-W049 }
  UInt ui;
	for ( ui=0; ui<MAX_LAYERS; ui++)
	{
		//JVT-W049 {
		//m_pauiPocInGOP         [ui]  =	NULL;
		//m_pauiFrameNumInGOP    [ui]  =	NULL;
		//JVT-W049 }
		m_pauiTempLevelInGOP   [ui]  =	NULL;
		m_uiDecompositionStages[ui]	 =	4;
		m_uiFrameIdx           [ui]  =	0;
		m_uiGopSize            [ui] 	 =	16;
		//JVT-W049 {
		m_bNextRedu            [ui]  =  0;
		bKeyPicReduUseSvc      [ui]  =  false;
    //JVT-W049 }
	}

  m_eErrorConceal  =	EC_NONE;
	m_bNotSupport	=	false;
  m_baseMode = 1;
	//JVT-X046 {
	m_bPicDone = false;
	m_bDiscard = false;
	m_uiDecodedMbNum = 0;
	m_uiNextFirstMb = 0;
	m_uiLostMbNum = 0;
	for ( uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
	{
		m_uiMbNumInFrame[uiLayer] = 0;
	}
	for ( ui=0; ui<MAX_LAYERS; ui++ )
	{
		m_bMbStatus[ui] = NULL;
		m_bLayerStatus[ui] = true;
	}
	//JVT-X046 }
//  TMM_EC }}
}



H264AVCDecoder::~H264AVCDecoder()
{
  if( m_pcBaseLayerCtrlEL )
  {
    m_pcBaseLayerCtrlEL->uninit();
    delete m_pcBaseLayerCtrlEL;
    m_pcBaseLayerCtrlEL = 0;
  }
	//JVT-X046 {
	for ( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
	{
		if ( m_bMbStatus[uiLayer] != NULL )
		{
			delete [] m_bMbStatus[uiLayer];
			m_bMbStatus[uiLayer] = NULL;
		}
	}
	//JVT-X046 }
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
                             PocCalculator*      pcPocCalculator,
                             MotionCompensation* pcMotionCompensation )
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
	m_bNewSPS                   = false;
  m_uiRecLayerId              = 0;
  m_uiLastLayerId             = MSYS_UINT_MAX;
  m_pcVeryFirstSPS            = 0;
  m_pcMotionCompensation      = pcMotionCompensation;

  m_bActive = false;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    ROT( NULL == apcMCTFDecoder[uiLayer] );

    m_apcMCTFDecoder[uiLayer] = apcMCTFDecoder[uiLayer];

//	TMM EC {{
		m_apcMCTFDecoder[uiLayer]->m_pcFrameMng	=	m_pcFrameMng;
		m_apcMCTFDecoder[uiLayer]->m_eErrorConceal	=	m_eErrorConceal;
//  TMM_EC }}
  }

  RNOK( m_acLastPredWeightTable[LIST_0].init( 64 ) );
  RNOK( m_acLastPredWeightTable[LIST_1].init( 64 ) );

#ifdef SHARP_AVC_REWRITE_OUTPUT
  m_pcAvcRewriteEncoder       = NULL;
  m_avcRewriteBindata         = NULL;
  m_avcRewriteBinDataBuffer   = NULL;
  m_avcRewriteBinDataAccessor = NULL;
  m_avcRewriteBufsize         = 0;
  m_avcRewriteFlag           = false;
#endif

  m_bInitDone = true;

  return Err::m_nOK;
}



ErrVal H264AVCDecoder::uninit()
{
  RNOK( m_acLastPredWeightTable[LIST_0].uninit() );
  RNOK( m_acLastPredWeightTable[LIST_1].uninit() );

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
  m_pcMotionCompensation  = NULL;
  //NS leak fix begin
  if (m_pcSliceHeader)
  {
    FrameUnit* fu = m_pcSliceHeader->getFrameUnit();
    if (fu) fu->destroy();
  }
  //NS leak fix end
  delete m_pcSliceHeader; 
  delete m_pcPrefixSliceHeader;//prefix unit
  delete m_pcPrevSliceHeader;
  delete m_pcTempSliceHeader;     //EIDR bug-fix
  delete m_pcSliceHeader_backup;  // JVT-Q054 Red. Pic
  m_pcSliceHeader         = NULL;
  m_pcPrevSliceHeader     = NULL;
  m_pcTempSliceHeader	  = NULL;   //EIDR bug-fix
  m_pcSliceHeader_backup  = NULL; // JVT-Q054 Red. Pic 
  m_pcPrefixSliceHeader	  = NULL;//prefix unit
  m_pcSliceReader         = NULL;
  m_pcSliceDecoder        = NULL;
  m_pcFrameMng            = NULL;
  m_pcNalUnitParser       = NULL;
  m_pcControlMng          = NULL;
  m_pcLoopFilter          = NULL;
  m_pcHeaderSymbolReadIf  = NULL;
  m_pcParameterSetMng     = NULL;
  m_pcPocCalculator       = NULL;

	m_bNewSPS               = false;
	if( m_pcVeryFirstSliceHeader )
  delete m_pcVeryFirstSliceHeader;
  m_pcVeryFirstSliceHeader = NULL;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_apcMCTFDecoder[uiLayer] = NULL;
  }

#ifdef SHARP_AVC_REWRITE_OUTPUT
  m_pcAvcRewriteEncoder       = NULL;
  m_avcRewriteBindata         = NULL;
  m_avcRewriteBinDataBuffer   = NULL;
  m_avcRewriteBinDataAccessor = NULL;
  m_avcRewriteBufsize         = 0;
  m_avcRewriteFlag            = false;
#endif

  m_bInitDone = false;
  //NS leak fix begin
 for ( UInt i=0; i< m_uiNumLayers; i++)
  {
    //JVT-W049 {
		//if (m_pauiPocInGOP[i])       delete	[] m_pauiPocInGOP[i];
    //if (m_pauiFrameNumInGOP[i])  delete	[] m_pauiFrameNumInGOP[i];
		//JVT-W049 }
    if (m_pauiTempLevelInGOP[i]) delete	[] m_pauiTempLevelInGOP[i];
  }
  //NS leak fix end
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
                              Int&          riSlicePoc )
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

   //EIDR bug-fix {
	if(rcSliceHeader.getLayerId() == 0 )
	{
		rcSliceHeader.setInIDRAccess(rcSliceHeader.isIdrNalUnit());
	}
	else
	{
		rcSliceHeader.setInIDRAccess(m_pcTempSliceHeader?m_pcTempSliceHeader->getInIDRAccess():false);
	}
   //EIDR bug-fix }

 pcLocalPocCalculator->calculatePoc( rcSliceHeader );

	riSlicePoc = rcSliceHeader.getPoc();

  pcLocalPocCalculator->destroy();

  return Err::m_nOK;
}


ErrVal H264AVCDecoder::xInitParameters(SliceHeader* pcSliceHeader)
{
  //activate corresponding SPS
  PictureParameterSet * rcPPS;
  m_pcParameterSetMng->get( rcPPS, pcSliceHeader->getPicParameterSetId() );
  UInt uiSPSId = rcPPS->getSeqParameterSetId();
  Bool bFound = false;
  for(UInt ui = 0; ui < m_uiNumberOfSPS; ui++)
  {
    if(m_uiSPSId[ui] == uiSPSId)
    {
      bFound = true;
      break;
    }
  }
  if(!bFound)
  {
    m_uiSPSId[m_uiNumberOfSPS] = uiSPSId;
    m_uiNumberOfSPS++;
    SequenceParameterSet * rcSPS;
    m_pcParameterSetMng->get(rcSPS,uiSPSId);
    rcSPS->setLayerId(pcSliceHeader->getLayerId());
  }
  return Err::m_nOK;
}

// not tested with multiple slices
ErrVal
H264AVCDecoder::checkSliceLayerDependency( BinDataAccessor*  pcBinDataAccessor,
                                           Bool&             bFinishChecking
										 )
{
  Bool bEos;
  NalUnitType eNalUnitType;
  SliceHeader* pcSliceHeader = NULL;
  Int slicePoc;
  bFinishChecking = false;

  ROT( NULL == pcBinDataAccessor );

  bEos = ( NULL == pcBinDataAccessor->data() ) || ( 0 == pcBinDataAccessor->size() );
  slicePoc = 0;

  if(m_uiNumOfNALInAU == 0)
  {
    //new AU: initialization
    m_bDependencyInitialized = false;
    //if(!bEos)                     MGS fix by Heiko Schwarz
    //  m_bCGSSNRInAU = false;      MSG fix by Heiko Schwarz

  }
  else
  {
    if(m_bDependencyInitialized)
    {
       bFinishChecking = true;
      return Err::m_nOK;
    }
  }
  if(bEos)
  {
	  if (-1 == m_iNextNalSpatialLayer)
		  m_bCurNalIsEndOfPic = true;
    m_bDependencyInitialized = true; //JVT-T054
    bFinishChecking = true;
    return Err::m_nOK;
  }

  if( ! bEos )
  {
    m_pcNalUnitParser->setCheckAllNALUs(true);
    UInt uiNumBytesRemoved;
    RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor,  uiNumBytesRemoved ) );
    m_pcNalUnitParser->setCheckAllNALUs(false);

    eNalUnitType = m_pcNalUnitParser->getNalUnitType();
    if(!m_bDependencyInitialized)
      m_uiNumOfNALInAU++;//JVT-P031 
    if( eNalUnitType != NAL_UNIT_CODED_SLICE        &&
        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR      && 
        eNalUnitType != NAL_UNIT_CODED_SLICE_SCALABLE && 
        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE &&
				eNalUnitType != NAL_UNIT_PREFIX )//prefix unit
    {
      if(m_uiNumOfNALInAU > 1)
      {
        m_uiNumOfNALInAU--;
        m_bDependencyInitialized = true;
      }
       m_bCheckNextSlice = false;
       bFinishChecking = true;

       return Err::m_nOK;
    }
    else
    {
      // read the slice header
      //JVT-P031
//prefix unit{{
    if( eNalUnitType == NAL_UNIT_PREFIX )
		{
			if(m_bCheckNextSlice)
			{
				if(m_uiNumOfNALInAU > 1)
				{
					m_uiNumOfNALInAU--;
					m_bDependencyInitialized = true;
				} 
				m_bCheckNextSlice = false;
				bFinishChecking = true;

				return Err::m_nOK;
			}
			else
			{
				SequenceParameterSet* pcSPS;
				PictureParameterSet*  pcPPS;
				RNOK( m_pcParameterSetMng ->get    ( pcPPS, 0) );
				RNOK( m_pcParameterSetMng ->get    ( pcSPS, 0) );
				if(m_pcPrefixSliceHeader)
				{
					delete m_pcPrefixSliceHeader;
					m_pcPrefixSliceHeader = NULL;
				}
				m_pcPrefixSliceHeader = new SliceHeader(*pcSPS, *pcPPS);
				RNOK( m_pcSliceReader->readSliceHeaderPrefix( m_pcNalUnitParser->getNalUnitType   (),
																m_pcNalUnitParser->getNalRefIdc     (),
																m_pcNalUnitParser->getLayerId		  (),
																m_pcNalUnitParser->getQualityLevel  (),
																m_pcNalUnitParser->getUseBasePredFlag(),
																m_pcPrefixSliceHeader
																) );

				return Err::m_nOK;
			}

		}
		else
//prefix unit}}
		{
				RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser,
																								pcSliceHeader ) );
//JVT-W049 {
			if((pcSliceHeader->getRedundantPicCnt()))
			{
				m_bNextRedu[pcSliceHeader->getLayerId()]=1;
			}	  
//JVT-W049 }
//prefix unit{{
					if(m_pcPrefixSliceHeader )
					{
						if(eNalUnitType == NAL_UNIT_CODED_SLICE ||eNalUnitType == NAL_UNIT_CODED_SLICE_IDR)
						{
							pcSliceHeader->copyPrefix(*m_pcPrefixSliceHeader);
						}
						delete m_pcPrefixSliceHeader;
						m_pcPrefixSliceHeader = NULL;
					}
//prefix unit}}

        xInitParameters(pcSliceHeader);

				m_bCGSSNRInAU = (m_bCGSSNRInAU || pcSliceHeader->getQualityLevel() != 0);

        //MGS_FIX_FT_09_2007
        if(m_pcNalUnitParser->getQualityLevel() == 0)
        {
          m_abMGSAtLayer[m_pcNalUnitParser->getLayerId()] = false;
        }
        else
        {
          m_abMGSAtLayer[m_pcNalUnitParser->getLayerId()] = (m_abMGSAtLayer[m_pcNalUnitParser->getLayerId()] || m_pcNalUnitParser->getQualityLevel() != 0);
        }
        //~MGS_FIX_FT_09_2007


			calculatePoc( eNalUnitType, *pcSliceHeader, slicePoc );

			// <-- ROI DECODE ICU/ETRI
			// first NAL check
			if (-1 == m_iCurNalSpatialLayer)
			{
				m_iCurNalSpatialLayer 	= m_pcNalUnitParser->getLayerId();
				m_iCurNalPOC  			= slicePoc;
				m_iCurNalFirstMb			= pcSliceHeader->getFirstMbInSlice();
			}

			// second NAL check
			else if (-1 == m_iNextNalSpatialLayer)
			{
				m_iNextNalSpatialLayer	= m_pcNalUnitParser->getLayerId();
				m_iNextNalPOC			= slicePoc;

				if (m_iCurNalSpatialLayer != m_iNextNalSpatialLayer)
					m_bCurNalIsEndOfPic = true;

				if (m_iCurNalPOC != m_iNextNalPOC)
					m_bCurNalIsEndOfPic = true;

				Bool bNewFrame =false;
				RNOK( pcSliceHeader->compareRedPic ( m_pcSliceHeader_backup, bNewFrame ) );
				if(m_iCurNalFirstMb ==pcSliceHeader->getFirstMbInSlice())
					m_bCurNalIsEndOfPic = true;
			}
			// --> ROI DECODE ICU/ETRI

				if( ! m_bCheckNextSlice )
				{
				m_iFirstLayerIdx = m_pcNalUnitParser->getLayerId();
				m_iFirstSlicePoc = slicePoc;
				if( eNalUnitType == NAL_UNIT_CODED_SLICE ||  eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
				{
					m_bBaseLayerAvcCompliant = true;
					m_iFirstLayerIdx = m_pcNalUnitParser->getLayerId();
				}
				else
				{
						m_bBaseLayerAvcCompliant = false;
				}
				}

				if( slicePoc == m_iFirstSlicePoc )
				{
				m_iLastLayerIdx = m_pcNalUnitParser->getLayerId();
				if (m_iLastLayerIdx == 0)
				{
					m_auiBaseLayerId[m_iLastLayerIdx]      = MSYS_UINT_MAX;
					m_auiBaseQualityLevel[m_iLastLayerIdx] = 0;
				}
				else
				{
					m_auiBaseLayerId[m_iLastLayerIdx]      = pcSliceHeader->getBaseLayerId();
					m_auiBaseQualityLevel[m_iLastLayerIdx] = pcSliceHeader->getBaseQualityLevel();

					if( pcSliceHeader->getBaseLayerId() && pcSliceHeader->getBaseLayerId() != MSYS_UINT_MAX ) //prefix unit
					{
						m_apcMCTFDecoder[pcSliceHeader->getBaseLayerId()]->setQualityLevelForPrediction( pcSliceHeader->getBaseQualityLevel() );
					}
					else 
					{
						setQualityLevelForPrediction( pcSliceHeader->getBaseQualityLevel() );
					}
				}
				}

				m_bCheckNextSlice = true;
			}
    }
  }

  if( bEos || slicePoc != m_iFirstSlicePoc)
  {
	// ROI DECODE ICU/ETRI
	if (-1 != m_iCurNalSpatialLayer && -1 != m_iNextNalSpatialLayer)
		bFinishChecking   = true;
    // setup the state information for the previous slices
    m_bCheckNextSlice = false;
//JVT-T054{
    if(!bEos)
      decreaseNumOfNALInAU();
//JVT-T054}
    m_apcMCTFDecoder[m_iLastLayerIdx]->setQualityLevelForPrediction( 3 );
    if( m_iFirstLayerIdx < m_iLastLayerIdx )
    {
      // set the base layer dependency
      if( m_iFirstLayerIdx == 0 )
      {
        if( m_bBaseLayerAvcCompliant )
        {
          setQualityLevelForPrediction( m_auiBaseQualityLevel[1] );
        }
        else
        {
          m_apcMCTFDecoder[0]->setQualityLevelForPrediction( m_auiBaseQualityLevel[1] );
        }
      }

      for( Int iLayer = (m_iFirstLayerIdx == 0) ? 1 : m_iFirstLayerIdx;
        iLayer <= m_iLastLayerIdx - 1; iLayer ++ )
      {
        m_apcMCTFDecoder[iLayer]->setQualityLevelForPrediction( m_auiBaseQualityLevel[iLayer + 1] );
      }
    }

    m_bDependencyInitialized = true;
  }

  if( pcSliceHeader != NULL )
 {
  delete m_pcTempSliceHeader;  //EIDR bug-fix
	m_pcTempSliceHeader = pcSliceHeader;//EIDR bug-fix
  }

  return Err::m_nOK;
}

//JVT-P031
Void H264AVCDecoder::getDecodedResolution(UInt &uiLayerId)
{
 UInt uiSPSId = 0;
 UInt uiX     = 0;
 UInt uiY     = 0;
 UInt uiMBX   = 0;
 UInt uiMBY   = 0;
 SequenceParameterSet *rcSPS;

 uiSPSId = 0;
 uiMBX = 0;
 uiMBY = 0;
 UInt uiSPS = 0;
 while(uiSPS < m_uiNumberOfSPS)
 {
   if(m_pcParameterSetMng->isValidSPS(uiSPSId))
   {
    m_pcParameterSetMng->get(rcSPS,uiSPSId);
    uiX = rcSPS->getFrameWidthInMbs();
    uiY = rcSPS->getFrameHeightInMbs();
    if(uiX >= uiMBX && uiY >= uiMBY) //FIX_FRAG_CAVLC
    {
       uiMBX = uiX;
       uiMBY = uiY;
       uiLayerId = rcSPS->getLayerId();
    }
    uiSPS++;
   }
   uiSPSId++;
 }

}
//~JVT-P031
//TMM_EC{{
Bool
H264AVCDecoder::checkSEIForErrorConceal()
{
	Bool	ret	=	true;
	//UInt	i	=	0;
	if ( m_bNotSupport)
	{
		return	false;
	}
	if ( m_uiNumLayers > 2 || m_uiNumLayers == 0 )
	{
		return	false;
	}
	return	ret;
}
//JVT-X046 {
Bool 
H264AVCDecoder::checkOrderFromPoc(UInt uiPocLsb1,UInt uiPocLsb2,UInt& id1,UInt& id2,UInt uiDecompositionStages)
{
	UInt uiGOPSize = 1<<uiDecompositionStages;	
	UInt *Mapping = new UInt[uiGOPSize];
	memset(Mapping,0,uiGOPSize*sizeof(UInt));
	UInt *MappingTmp = new UInt[uiGOPSize];
	memset(MappingTmp,0,uiGOPSize*sizeof(UInt));
	UInt uiCurrIdx;

	uiPocLsb1 = uiPocLsb1 % uiGOPSize;
	uiPocLsb2 = uiPocLsb2 % uiGOPSize;

	Mapping[0] = 0;	
	UInt uiAUIndex = 0;
    for ( uiAUIndex = 0; uiAUIndex < uiGOPSize; uiAUIndex++ )
	{
		uiCurrIdx = 1;
		for( Int iLevel = (Int)uiDecompositionStages - 1; iLevel >= 0; iLevel-- )
		{
			for( UInt uiFrame = 1; uiFrame <= ( uiGOPSize >> iLevel ); uiFrame += 2, uiCurrIdx++ )
			{
				if( uiAUIndex == uiCurrIdx )
				{ 
					Mapping[uiCurrIdx] = uiFrame << iLevel;
				}
			}
		}
	}

	for ( uiAUIndex = 0; uiAUIndex < uiGOPSize; uiAUIndex++ )
	{
		UInt i = 0;
		for (i = 0; i < uiGOPSize;i++)
		{
			if ( Mapping[i] == uiAUIndex )
				break;
		}
		MappingTmp[uiAUIndex] = i;
	}
	for ( uiAUIndex = 0; uiAUIndex < uiGOPSize; uiAUIndex++ )
	{
		Mapping[uiAUIndex] = MappingTmp[uiAUIndex];;
	}

	id1 = Mapping[uiPocLsb1];
	id2 = Mapping[uiPocLsb2];
	return true;
}
//JVT-X046 }
ErrVal
H264AVCDecoder::checkSliceGap( BinDataAccessor*  pcBinDataAccessor,
                               MyList<BinData*>& cVirtualSliceList )
{
  static  Bool  bFinished=false;//TMM_EC
  Bool bEos;
  NalUnitType eNalUnitType;
  SliceHeader* pcSliceHeader = NULL;
  Int slicePoc;

  ROT( NULL == pcBinDataAccessor );

  bEos = ( NULL == pcBinDataAccessor->data() ) || ( 0 == pcBinDataAccessor->size() );
  slicePoc = 0;

	UInt	frame_num;
	UInt	uiLayerId;
	UInt	uiPocLsb;
	UInt	uiMaxFrameNum;
	UInt	uiMaxPocLsb;


//TMM_EC {{
	//JVT-X046 {
	UInt	uiFirstMb;
	m_bDiscard = false;
	//JVT-X046 }
  if ( bEos && !bFinished)
  {
    bFinished = true;
    goto bEOS;
  }
//TMM_EC }}

  if( ! bEos )
  {
    UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu{{
  //RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, NULL, uiNumBytesRemoved, true, false, true ) );
  RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, uiNumBytesRemoved, true, false, true ) );
//bug-fix suffix shenqiu}}
//prefix unit{{
	if(m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_PREFIX)
	{
		return Err::m_nOK;
	}
//prefix unit}}
    if ( m_pcNalUnitParser->getQualityLevel() != 0)
		{

			BinData	*pcBinData = new BinData;
			pcBinData->set( new UChar[pcBinDataAccessor->size()], pcBinDataAccessor->size());
			memcpy( pcBinData->data(), pcBinDataAccessor->data(), pcBinDataAccessor->size());
			cVirtualSliceList.pushBack( pcBinData);
//*///xk
			return  Err::m_nERR;
		}

    if ( ( m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_SCALABLE
          || m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
          && m_pcNalUnitParser->getLayerId() == 0)
		{
			return  Err::m_nERR;
		}

		if ( !checkSEIForErrorConceal())
		{
			return	Err::m_nInvalidParameter;
		}
    eNalUnitType = m_pcNalUnitParser->getNalUnitType();

    if( eNalUnitType != NAL_UNIT_CODED_SLICE        &&
//        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR      &&
        eNalUnitType != NAL_UNIT_CODED_SLICE_SCALABLE &&
//        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE &&
        eNalUnitType != NAL_UNIT_END_OF_STREAM &&
		1)
    {
			if(eNalUnitType==NAL_UNIT_CODED_SLICE_IDR||eNalUnitType==NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
      {
        RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser,
                                                pcSliceHeader  ) );
				m_bRedundantPic = ( pcSliceHeader->getRedundantPicCnt() > 0 ? true:false );		//JVT-W049
        if(pcSliceHeader->getFrameNum()==0)
					m_uiMaxLayerId=pcSliceHeader->getLayerId()>m_uiMaxLayerId ? pcSliceHeader->getLayerId(): m_uiMaxLayerId;
				//JVT-W047
				m_apcMCTFDecoder[pcSliceHeader->getLayerId()]->getDecodedPicBuffer()->setMaxLayerId(m_uiMaxLayerId);
				//JVT-W047
      }
      return Err::m_nOK;
    }
    else
    {
      if ( eNalUnitType != NAL_UNIT_END_OF_STREAM)
			{
				// read the slice header
	      RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser,
                                              pcSliceHeader ) );
			//JVT-W049 {
  	  m_bRedundantPic = ( pcSliceHeader->getRedundantPicCnt() > 0 ? true:false );
      if ( pcSliceHeader->getRedundantPicCnt()	  )
      {
	      if((m_uiNextFrameNumRedu==pcSliceHeader->getFrameNum())&&(m_uiNextLayerIdRedu==pcSliceHeader->getLayerId())&&(m_uiNextPocLsbRedu==pcSliceHeader->getPicOrderCntLsb()))
	      {
		      m_bRedundantPic = true;
		      bKeyPicReduUseAvc = false;
		      bKeyPicReduUseSvc[pcSliceHeader->getLayerId()] = false;
          delete pcSliceHeader;
		      return Err::m_nOK;
	      }
	      else
	      {
		      pcSliceHeader->setRedundantPicCnt(0);
		      m_bRedundantPic = false;
		      if( pcSliceHeader->getLayerId() == 0 )
		      {
		        bKeyPicReduUseAvc = true; bKeyPicReduUseSvc[pcSliceHeader->getLayerId()] = 0;
		      }
		      else if(pcSliceHeader->getLayerId() >= 1 )
		      {
		        bKeyPicReduUseAvc = false; bKeyPicReduUseSvc[pcSliceHeader->getLayerId()] = 1;
		      }
	      }
      }
      //JVT-W049 }
				if(pcSliceHeader->getFrameNum()==0)
					m_uiMaxLayerId=pcSliceHeader->getLayerId()>m_uiMaxLayerId ? pcSliceHeader->getLayerId(): m_uiMaxLayerId;
				//JVT-W047
				m_apcMCTFDecoder[pcSliceHeader->getLayerId()]->getDecodedPicBuffer()->setMaxLayerId(m_uiMaxLayerId);
				//JVT-W047

	      calculatePoc( eNalUnitType, *pcSliceHeader, slicePoc );
//        if(pcSliceHeader->getFrameNum()==1&&m_uiMaxLayerId!=(m_uiNumLayers-1))
//					m_bNotSupport=true;
       	if ( pcSliceHeader->getFrameNum() == 1 && pcSliceHeader->getLayerId() == m_uiNumLayers-1)
				{
					if ( pcSliceHeader->getPicOrderCntLsb() != m_uiMaxGopSize)
					{
						m_bNotSupport	=	true;
					}
					/*if ( pcSliceHeader->getFirstMbInSlice() != 0)//JVT-X046 commented
					{
						m_bNotSupport	=	true;
					}*/
					if ( pcSliceHeader->getFrameNum() == 1)
					{
						UInt	i=pcSliceHeader->getPicOrderCntLsb();
						while ( i % 2 == 0)
						{
							i /=	2;
						}
						if ( i!= 1)
						{
							m_bNotSupport	=	true;
						}
					}
				}
				if( false || pcSliceHeader->getFrameNum() == 1 && pcSliceHeader->getLayerId() < m_uiMaxLayerId)
				{
					if( pcSliceHeader != NULL )
						delete pcSliceHeader;

					return Err::m_nOK;
				}
//	detection the gap of slice
				frame_num	=	pcSliceHeader->getFrameNum();
				uiLayerId	=	pcSliceHeader->getLayerId();
				uiPocLsb	=	pcSliceHeader->getPicOrderCntLsb();
				uiFirstMb = pcSliceHeader->getFirstMbInSlice();//JVT-X046
				UInt uiGopSize	=	m_uiGopSize[uiLayerId];
				if ( frame_num == 1 &&m_uiMaxLayerId==uiLayerId&& uiPocLsb > uiGopSize)
				{
					m_bNotSupport = true;
					if( pcSliceHeader != NULL )
						delete pcSliceHeader;
					return	Err::m_nOK;
				}
				uiMaxFrameNum	=	1 << pcSliceHeader->getSPS().getLog2MaxFrameNum();
				uiMaxPocLsb		=	1 << pcSliceHeader->getSPS().getLog2MaxPicOrderCntLsb();
			}
			else
			{
        bEOS://xk
        SliceHeader *pcSH;
        if ( m_pcVeryFirstSliceHeader == NULL)
        {
          pcSH  = m_apcMCTFDecoder[m_uiNextLayerId]->m_pcVeryFirstSliceHeader;
        }
        else
        {
          pcSH  = m_pcVeryFirstSliceHeader;
        }
				uiMaxFrameNum	=	1 << pcSH->getSPS().getLog2MaxFrameNum();
				uiMaxPocLsb		=	1 << pcSH->getSPS().getLog2MaxPicOrderCntLsb();
				uiLayerId	=	m_uiNextLayerId;
				UInt uiGopSize	=	1 << m_uiDecompositionStages[uiLayerId];
				uiFirstMb = pcSH->getFirstMbInSlice();//JVT-X046
//				uiGopSize	=	m_uiGopSize[uiLayerId];
				if ( m_uiFrameIdx[uiLayerId] % uiGopSize != 1)
				{
					frame_num	=	m_pauiFrameNumInGOP[uiLayerId][m_uiGopSize[uiLayerId]-1] % uiMaxFrameNum;
					uiPocLsb	=	m_pauiPocInGOP[uiLayerId][m_uiGopSize[uiLayerId]-1] % uiMaxPocLsb;
				}
				else
				{
					frame_num	=	m_pauiFrameNumInGOP[uiLayerId][0] % uiMaxFrameNum;
					uiPocLsb	=	m_pauiPocInGOP[uiLayerId][0] % uiMaxPocLsb;
				}
//				if ( m_uiFrameIdx[uiLayerId] % uiGopSize == 1)
				{
					frame_num	-=	(uiGopSize >> 1);
//xk
          if ( uiGopSize == 1)
            frame_num -=  1;
					uiPocLsb	-=	m_uiMaxGopSize;
				}
			}
		}

//JVT-X046 {
		if ( m_uiNextFirstMb == 0 && !bEos )
		{			
			UInt	uiGopSize = 1 << m_uiDecompositionStages[m_uiNextLayerId];
			UInt	uiMaxGopSize	=	m_uiMaxGopSize;
			do
			  {
				  m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
				  uiGopSize	=	1 << m_uiDecompositionStages[m_uiNextLayerId];
				  if (m_uiFrameIdx[m_uiNextLayerId] % uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
					  break;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  while( true);
			m_bLayerStatus[m_uiNextLayerId] = true;//initilization
			if ( ((m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE)&&(m_uiNextLayerId != 0))
				||(m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR) )
			{
				m_uiNextPoc = 0;
				m_uiNextFrameNum = 0;
			}
			else
			{
				m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
				m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];

				if ( uiGopSize == 1)
				{
					m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1;
				}
				else
				{
					m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiGopSize >> 1;
				}
				m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiMaxGopSize;				
				m_uiFrameIdx[m_uiNextLayerId]++;
			}
		}
		if ( m_uiNextFirstMb == -1 )//treat it special when m_bDiscard is true
		{
			m_uiNextFirstMb = 0;
		}
		if ( frame_num != m_uiNextFrameNum || uiLayerId != m_uiNextLayerId || uiPocLsb != (m_uiNextPoc % uiMaxPocLsb) || uiFirstMb != m_uiNextFirstMb)
		//if ( frame_num != m_uiNextFrameNum || uiLayerId != m_uiNextLayerId || uiPocLsb != (m_uiNextPoc % uiMaxPocLsb))
		//JVT-X046 }
		{
//	judge if the uncompleted GOP
			UInt	uiFrameIdx	=	0;
			UInt	uiGopSize	=	1 << m_uiDecompositionStages[uiLayerId];
//			UInt	uiGopSize	=	m_uiGopSize[uiLayerId];

			//JVT-X046
			/*for ( ;uiFrameIdx < uiGopSize; uiFrameIdx++)
			{
				if ( m_pauiPocInGOP[uiLayerId][uiFrameIdx]	% uiMaxPocLsb ==	uiPocLsb)
					break;
			}
			if ( ( m_pauiFrameNumInGOP[uiLayerId][uiFrameIdx] % uiMaxFrameNum ) == frame_num || (uiPocLsb-1) / m_uiMaxGopSize != ((m_uiNextPoc-1) % uiMaxPocLsb) / m_uiMaxGopSize)
			{
				if ( m_uiNextLayerId == 0 && m_baseMode == 1)
				{
					BinData	*pcBinData = new BinData;
					pcBinData->set( new UChar[11], 11);
//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_BASELAYER;
					*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
					*(Int*)(pcBinData->data()+1)	=	0xdeadface;
					*(Short*)(pcBinData->data()+5)	=	(Short) m_uiNextFrameNum;
					*(Short*)(pcBinData->data()+7)	=	(Short) m_uiNextPoc;
					*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
					*(pcBinData->data()+pcBinData->size()-1)	=	0;
					cVirtualSliceList.pushFront( pcBinData);

				}
				else
				{
					BinData	*pcBinData = new BinData;
					pcBinData->set( new UChar[11], 11);
//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_ENHANCELAYER;
					*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
					*(Int*)(pcBinData->data()+1)	=	0xdeadface;
					*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
					*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
					*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
					*(pcBinData->data()+pcBinData->size()-1)	=	0;
					cVirtualSliceList.pushFront( pcBinData);

          m_uiNumOfNALInAU++;  //TMM_EC
				}
			}
			else
			{
//	uncomplete GOP structure
//	calculate the gop size of the layer current packet belongs to
				if ( uiPocLsb % ( 2<<(m_uiMaxDecompositionStages-m_uiDecompositionStages[uiLayerId])) != 0)
				{
					m_uiGopSize[uiLayerId]	=	(((frame_num-1) % (uiGopSize>>1))<< 1) + 1;
				}
				else
				{
					UInt	uiFrameDiff	=	( m_pauiFrameNumInGOP[uiLayerId][uiFrameIdx] % uiMaxFrameNum)  - frame_num;
					UInt	uiTL	=	m_uiDecompositionStages[uiLayerId];
					UInt	uiTmp	=	uiPocLsb >> (m_uiMaxDecompositionStages - m_uiDecompositionStages[uiLayerId]);
					while( uiTmp % 2 == 0)
					{
						uiTmp	>>=1;
						uiTL--;
					}
          UInt ui = 0;
					for ( ui=m_uiGopSize[uiLayerId]&(-2); ui>0; ui-=2)
					{
						UInt	uiTempLevel	=	m_uiDecompositionStages[uiLayerId];
						uiTmp	=	ui;
						while( uiTmp % 2 == 0)
						{
							uiTmp	>>=1;
							uiTempLevel--;
						}
						if ( uiTL <= uiTempLevel)
						{
							continue;
						}
						uiFrameDiff--;
						if ( uiFrameDiff == 0)
						{
							break;
						}
					}
					m_uiGopSize[uiLayerId]	=	ui - 1;
				}
        //	calculate the gop size of other layers
        UInt  ui=0;
				for ( ui=0; ui<uiLayerId; ui++)
				{
					m_uiGopSize[ui]	=	m_uiGopSize[uiLayerId] >> (m_uiDecompositionStages[uiLayerId] - m_uiDecompositionStages[ui]);
				}
				for ( ui=uiLayerId+1; ui<m_uiNumLayers; ui++)
				{
					m_uiGopSize[ui]	=	( m_uiGopSize[uiLayerId] << (m_uiDecompositionStages[ui] - m_uiDecompositionStages[uiLayerId])) + ((1<<(m_uiDecompositionStages[ui] - m_uiDecompositionStages[uiLayerId])) -1);
				}
//	calculate the correct frame_num and poc of every packet in this uncompleted gop each layer
				m_uiFrameIdx[m_uiNextLayerId]--;
				for ( ui=0; ui<m_uiNumLayers; ui++)
				{
					uiFrameIdx      	=	0;
					UInt	uiFrameNum	=	m_pauiFrameNumInGOP[ui][0];
					UInt	uiPoc	=	( m_uiNextPoc - 1 ) & -(1<<m_uiMaxDecompositionStages);
					if ( ui == 0 || m_uiFrameIdx[ui] % (1<<m_uiDecompositionStages[ui]) != 0)
						uiFrameNum	-=	( 1 << ( m_uiDecompositionStages[ui] - 1 ) );
					UInt	uiDecompositionStagesSub	=	m_uiMaxDecompositionStages - m_uiDecompositionStages[ui];
					for( UInt uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages[ui]; uiTemporalLevel++ )
					{
						UInt      uiStep    = ( 1 << ( m_uiDecompositionStages[ui] - uiTemporalLevel ) );
						for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGopSize[ui]; uiFrameId += ( uiStep << 1 ) )
						{
							m_pauiPocInGOP[ui][uiFrameIdx]	=	(uiFrameId << uiDecompositionStagesSub ) + uiPoc;
							m_pauiFrameNumInGOP[ui][uiFrameIdx]	=	uiFrameNum;
							m_pauiTempLevelInGOP[ui][uiFrameIdx]	=	uiTemporalLevel;
							uiFrameIdx++;
							if ( uiFrameId % 2 == 0)
								uiFrameNum++;
						}
					}
					for ( uiFrameIdx=0; uiFrameIdx<m_uiFrameIdx[ui] % (1<<m_uiDecompositionStages[ui]); uiFrameIdx++)
					{
						m_pauiPocInGOP[ui][uiFrameIdx]	+=	m_uiMaxGopSize;
						m_pauiFrameNumInGOP[ui][uiFrameIdx]	+=	(1<<(m_uiDecompositionStages[ui]-1));
					}
				}
			  do
			  {
				  if (m_uiFrameIdx[m_uiNextLayerId] % m_uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
					  break;
				  m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
			  }
				while( true);
				if (m_uiFrameIdx[m_uiNextLayerId] % m_uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
				{
					uiMaxFrameNum				=	1 << pcSliceHeader->getSPS().getLog2MaxFrameNum();
					m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
					m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
					m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
          // TMM_EC
					//JVT-W049 {
					if(pcSliceHeader->getNalRefIdc()==NAL_REF_IDC_PRIORITY_HIGHEST)
					{
						m_uiNextFrameNumRedu = pcSliceHeader->getFrameNum();
						m_uiNextPocLsbRedu   = pcSliceHeader->getPicOrderCntLsb();
						m_uiNextLayerIdRedu  = pcSliceHeader->getLayerId();
					}
					//JVT-W049 }
          if ( uiGopSize == 1)
					  m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1 ;
          else
            m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1 << ( m_uiDecompositionStages[m_uiNextLayerId] - 1);

					m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	m_uiMaxGopSize;
					m_uiFrameIdx[m_uiNextLayerId]++;
				}
				if ( frame_num != m_uiNextFrameNum || uiLayerId != m_uiNextLayerId || uiPocLsb != (m_uiNextPoc % uiMaxPocLsb))
				{
					if ( m_uiNextLayerId == 0)
					{
						BinData	*pcBinData = new BinData;
						pcBinData->set( new UChar[11], 11);
//						*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_BASELAYER;
						*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
						*(Int*)(pcBinData->data()+1)	=	0xdeadface;
						*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
						*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
						*(pcBinData->data()+9)	=	(UChar)m_uiNextTempLevel;
						*(pcBinData->data()+pcBinData->size()-1)	=	0;
						cVirtualSliceList.pushFront( pcBinData);

					}
					else
					{
						BinData	*pcBinData = new BinData;
						pcBinData->set( new UChar[11], 11);
//						*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_ENHANCELAYER;
						*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
						*(Int*)(pcBinData->data()+1)	=	0xdeadface;
						*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
						*(Short*)(pcBinData->data()+7)	=	 (Short)m_uiNextPoc;
						*(pcBinData->data()+9)	=	(UChar)m_uiNextTempLevel;
						*(pcBinData->data()+pcBinData->size()-1)	=	0;
						cVirtualSliceList.pushFront( pcBinData);

            m_uiNumOfNALInAU++;  //TMM_EC
					}
				}
			}*/
			//JVT-X046 {
			if ( frame_num == m_uiNextFrameNum && uiPocLsb == (m_uiNextPoc % uiMaxPocLsb) )
			{
				if ( m_uiNextLayerId == uiLayerId && m_uiNextFirstMb != uiFirstMb )
					m_uiLostMbNum = uiFirstMb - m_uiNextFirstMb;
				else
					m_uiLostMbNum = m_uiMbNumInFrame[m_uiNextLayerId] - m_uiNextFirstMb;
				if ( m_uiNextLayerId == 0) 
				{
					BinData	*pcBinData = new BinData;
					pcBinData->set( new UChar[11], 11);
					*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
					*(Int*)(pcBinData->data()+1)	=	0xdeadface;
					*(Short*)(pcBinData->data()+5)	=	(Short) m_uiNextFrameNum;
					*(Short*)(pcBinData->data()+7)	=	(Short) m_uiNextPoc;
					*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
					*(pcBinData->data()+pcBinData->size()-1)	=	0;
					cVirtualSliceList.pushFront( pcBinData);
				}
				else
				{
					BinData	*pcBinData = new BinData;
					pcBinData->set( new UChar[11], 11);
					*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
					*(Int*)(pcBinData->data()+1)	=	0xdeadface;
					*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
					*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
					*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
					*(pcBinData->data()+pcBinData->size()-1)	=	0;
					cVirtualSliceList.pushFront( pcBinData);
					m_uiNumOfNALInAU++;  //TMM_EC
				}
			}
			else
			{
				//check which packet should be discarded
				UInt id1,id2;
				id1 = uiPocLsb/m_uiMaxGopSize + ( (uiPocLsb % m_uiMaxGopSize)? 1:0 );
				if ( m_uiNextPoc%uiMaxPocLsb == 0 )
				{
					id2 = ( (m_uiNextPoc-1) % uiMaxPocLsb + 1 )/m_uiMaxGopSize + ( (m_uiNextPoc % m_uiMaxGopSize)? 1:0 );
				}
				else
				{
					id2 = ( (m_uiNextPoc) % uiMaxPocLsb )/m_uiMaxGopSize + ( (m_uiNextPoc % m_uiMaxGopSize)? 1:0 );
				}
				if ( (id2 > id1)&&(id2 < id1 + uiMaxPocLsb/m_uiMaxGopSize/2) || id1 > id2 + uiMaxPocLsb/m_uiMaxGopSize/2 )
				{
					m_bDiscard = true;
					m_uiNextFirstMb = (UInt)-1;//treat it special
					return Err::m_nOK;
				}
				if ( (id2 > id1)&&((id2-id1)%(uiMaxPocLsb/m_uiMaxGopSize) == 0) )
				{
					id2 = id1;
				}
				if ( id2 == id1 )
					checkOrderFromPoc(uiPocLsb,m_uiNextPoc % uiMaxPocLsb,id1,id2,m_uiDecompositionStages[uiLayerId]);
				else
					id2 = id1;
				if ( id2 > id1)
				{
					m_bDiscard = true;
					m_uiNextFirstMb = (UInt)-1;
					return Err::m_nOK;
				}
				//check end
				for ( ;uiFrameIdx < uiGopSize; uiFrameIdx++)
				{
					if ( m_pauiPocInGOP[uiLayerId][uiFrameIdx]	% uiMaxPocLsb ==	uiPocLsb)
						break;
				}				
				if ( ( m_pauiFrameNumInGOP[uiLayerId][uiFrameIdx] % uiMaxFrameNum ) != frame_num && (uiPocLsb-1) / m_uiMaxGopSize == ((m_uiNextPoc-1) % uiMaxPocLsb) / m_uiMaxGopSize)
				{
	//	uncomplete GOP structure
	//	calculate the gop size of the layer current packet belongs to
					if ( uiPocLsb % ( 2<<(m_uiMaxDecompositionStages-m_uiDecompositionStages[uiLayerId])) != 0)
					{
						m_uiGopSize[uiLayerId]	=	(((frame_num-1) % (uiGopSize>>1))<< 1) + 1;
					}
					else
					{
						UInt	uiFrameDiff	=	( m_pauiFrameNumInGOP[uiLayerId][uiFrameIdx] % uiMaxFrameNum)  - frame_num;
						UInt	uiTL	=	m_uiDecompositionStages[uiLayerId];
						UInt	uiTmp	=	uiPocLsb >> (m_uiMaxDecompositionStages - m_uiDecompositionStages[uiLayerId]);
						while( uiTmp % 2 == 0)
						{
							uiTmp	>>=1;
							uiTL--;
						}
						UInt ui = 0;
						for ( ui=m_uiGopSize[uiLayerId]&(-2); ui>0; ui-=2)
						{
							UInt	uiTempLevel	=	m_uiDecompositionStages[uiLayerId];
							uiTmp	=	ui;
							while( uiTmp % 2 == 0)
							{
								uiTmp	>>=1;
								uiTempLevel--;
							}
							if ( uiTL <= uiTempLevel)
							{
								continue;
							}
							uiFrameDiff--;
							if ( uiFrameDiff == 0)
							{
								break;
							}
						}
						m_uiGopSize[uiLayerId]	=	ui - 1;
					}
					//	calculate the gop size of other layers
					UInt  ui=0; 
					for ( ui=0; ui<uiLayerId; ui++)
					{
						m_uiGopSize[ui]	=	m_uiGopSize[uiLayerId] >> (m_uiDecompositionStages[uiLayerId] - m_uiDecompositionStages[ui]);
					}
					for ( ui=uiLayerId+1; ui<m_uiNumLayers; ui++)
					{
						m_uiGopSize[ui]	=	( m_uiGopSize[uiLayerId] << (m_uiDecompositionStages[ui] - m_uiDecompositionStages[uiLayerId])) + ((1<<(m_uiDecompositionStages[ui] - m_uiDecompositionStages[uiLayerId])) -1);
					}
	//	calculate the correct frame_num and poc of every packet in this uncompleted gop each layer
					m_uiFrameIdx[m_uiNextLayerId]--;
					for ( ui=0; ui<m_uiNumLayers; ui++)
					{
						uiFrameIdx      	=	0;
						UInt	uiFrameNum	=	m_pauiFrameNumInGOP[ui][0];
						UInt	uiPoc	=	( m_uiNextPoc - 1 ) & -(1<<m_uiMaxDecompositionStages);
						if ( ui == 0 || m_uiFrameIdx[ui] % (1<<m_uiDecompositionStages[ui]) != 0)
							uiFrameNum	-=	( 1 << ( m_uiDecompositionStages[ui] - 1 ) );
						UInt	uiDecompositionStagesSub	=	m_uiMaxDecompositionStages - m_uiDecompositionStages[ui];
						for( UInt uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages[ui]; uiTemporalLevel++ )
						{
							UInt      uiStep    = ( 1 << ( m_uiDecompositionStages[ui] - uiTemporalLevel ) );
							for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGopSize[ui]; uiFrameId += ( uiStep << 1 ) )
							{
								m_pauiPocInGOP[ui][uiFrameIdx]	=	(uiFrameId << uiDecompositionStagesSub ) + uiPoc;
								m_pauiFrameNumInGOP[ui][uiFrameIdx]	=	uiFrameNum;
								m_pauiTempLevelInGOP[ui][uiFrameIdx]	=	uiTemporalLevel;
								uiFrameIdx++;
								if ( uiFrameId % 2 == 0)
									uiFrameNum++;
							}
						}
						for ( uiFrameIdx=0; uiFrameIdx<m_uiFrameIdx[ui] % (1<<m_uiDecompositionStages[ui]); uiFrameIdx++)
						{
							m_pauiPocInGOP[ui][uiFrameIdx]	+=	m_uiMaxGopSize;
							m_pauiFrameNumInGOP[ui][uiFrameIdx]	+=	(1<<(m_uiDecompositionStages[ui]-1));
						}
					}
					//07.09.11
					if ( m_uiNextFirstMb == 0 && !bEos )
					{
						do
						{
							if (m_uiFrameIdx[m_uiNextLayerId] % m_uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
								break;
							m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
						}
						while( true);
						if (m_uiFrameIdx[m_uiNextLayerId] % m_uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
						{
							uiMaxFrameNum				=	1 << pcSliceHeader->getSPS().getLog2MaxFrameNum();
							m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
							m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
							m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
							// TMM_EC
							if ( uiGopSize == 1)
								m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1 ;
							else
								m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1 << ( m_uiDecompositionStages[m_uiNextLayerId] - 1);

							m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	m_uiMaxGopSize;
							m_uiFrameIdx[m_uiNextLayerId]++;
						}
					}
					//07.09.11
					if ( frame_num != m_uiNextFrameNum || uiLayerId != m_uiNextLayerId || uiPocLsb != (m_uiNextPoc % uiMaxPocLsb) || uiFirstMb != m_uiNextFirstMb)
					{
						if ( frame_num == m_uiNextFrameNum && uiPocLsb == (m_uiNextPoc % uiMaxPocLsb) )
						{
							if ( m_uiNextLayerId == uiLayerId && m_uiNextFirstMb != uiFirstMb )
								m_uiLostMbNum = uiFirstMb - m_uiNextFirstMb;
							else
								m_uiLostMbNum = m_uiMbNumInFrame[m_uiNextLayerId] - m_uiNextFirstMb;
							if ( m_uiNextLayerId == 0) 
							{
								BinData	*pcBinData = new BinData;
								pcBinData->set( new UChar[11], 11);
								*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
								*(Int*)(pcBinData->data()+1)	=	0xdeadface;
								*(Short*)(pcBinData->data()+5)	=	(Short) m_uiNextFrameNum;
								*(Short*)(pcBinData->data()+7)	=	(Short) m_uiNextPoc;
								*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
								*(pcBinData->data()+pcBinData->size()-1)	=	0;
								cVirtualSliceList.pushFront( pcBinData);
							}
							else
							{
								BinData	*pcBinData = new BinData;
								pcBinData->set( new UChar[11], 11);
								*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
								*(Int*)(pcBinData->data()+1)	=	0xdeadface;
								*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
								*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
								*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
								*(pcBinData->data()+pcBinData->size()-1)	=	0;
								cVirtualSliceList.pushFront( pcBinData);
								m_uiNumOfNALInAU++;  //TMM_EC
							}
						}
						else
						{
							m_uiLostMbNum = m_uiMbNumInFrame[m_uiNextLayerId] - m_uiNextFirstMb;
							if ( m_uiNextLayerId == 0 && m_baseMode == 1) 
							{
								BinData	*pcBinData = new BinData;
								pcBinData->set( new UChar[11], 11);
								//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_BASELAYER;
								*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
								*(Int*)(pcBinData->data()+1)	=	0xdeadface;
								*(Short*)(pcBinData->data()+5)	=	(Short) m_uiNextFrameNum;
								*(Short*)(pcBinData->data()+7)	=	(Short) m_uiNextPoc;
								*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
								*(pcBinData->data()+pcBinData->size()-1)	=	0;
								cVirtualSliceList.pushFront( pcBinData);
							}
							else
							{
								BinData	*pcBinData = new BinData;
								pcBinData->set( new UChar[11], 11);
								//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_ENHANCELAYER;
								*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
								*(Int*)(pcBinData->data()+1)	=	0xdeadface;
								*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
								*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
								*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
								*(pcBinData->data()+pcBinData->size()-1)	=	0;
								cVirtualSliceList.pushFront( pcBinData);
								m_uiNumOfNALInAU++;  //TMM_EC
							}
						}
					}
				}
				else
				{					
					m_uiLostMbNum = m_uiMbNumInFrame[m_uiNextLayerId] - m_uiNextFirstMb;
					if ( m_uiNextLayerId == 0 && m_baseMode == 1) 
					{
						BinData	*pcBinData = new BinData;
						pcBinData->set( new UChar[11], 11);
						//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_BASELAYER;
						*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
						*(Int*)(pcBinData->data()+1)	=	0xdeadface;
						*(Short*)(pcBinData->data()+5)	=	(Short) m_uiNextFrameNum;
						*(Short*)(pcBinData->data()+7)	=	(Short) m_uiNextPoc;
						*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
						*(pcBinData->data()+pcBinData->size()-1)	=	0;
						cVirtualSliceList.pushFront( pcBinData);
					}
					else
					{
						BinData	*pcBinData = new BinData;
						pcBinData->set( new UChar[11], 11);
						//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_ENHANCELAYER;
						*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
						*(Int*)(pcBinData->data()+1)	=	0xdeadface;
						*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
						*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
						*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
						*(pcBinData->data()+pcBinData->size()-1)	=	0;
						cVirtualSliceList.pushFront( pcBinData);
						m_uiNumOfNALInAU++;  //TMM_EC
					}
				}
			}
   		//JVT-X046 }
		}
	}

  if( pcSliceHeader != NULL )
    delete pcSliceHeader;

  return Err::m_nOK;
}
//TMM_EC }}

ErrVal
H264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor,
													  UInt&             ruiNalUnitType,
														UInt&             ruiMbX,
														UInt&             ruiMbY,
														UInt&             ruiSize
														//,UInt&             ruiNonRequiredPic  //NonRequired JVT-Q066
														//JVT-P031
														, Bool            bPreParseHeader //FRAG_FIX
														, Bool			      bConcatenated //FRAG_FIX_3
														, Bool&           rbStartDecoding,
														UInt&             ruiStartPos,
														UInt&             ruiEndPos,
														Bool&             bDiscardable
                            //~JVT-P031
                            , UInt*           puiNumFragments
                            , UChar**         ppucFragBuffers
                            )
{
  ROF( m_bInitDone );
  UInt uiLayerId;

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
    rbStartDecoding = true; //JVT-P031
    return Err::m_nOK;
  }

//  Bool KeyPicFlag = false; //bug-fix suffix shenqiu
  static Bool bSuffixUnit = false;  //JVT-S036 lsj

  //JVT-P031
  UInt uiHeaderBits;
  getDecodedResolution(m_uiDecodedLayer);
  m_pcNalUnitParser->setDecodedLayer(m_uiDecodedLayer);
//  ruiStartPos = m_pcNalUnitParser->getNalHeaderSize(pcBinDataAccessor);
  ruiStartPos = 0; //FRAG_FIX
  //~JVT-P031
  UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu{{
  //RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, &KeyPicFlag,uiNumBytesRemoved, bPreParseHeader , bConcatenated) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC

  RNOK( m_pcNalUnitParser->initNalUnit(
    pcBinDataAccessor, uiNumBytesRemoved, bPreParseHeader,
    bConcatenated, false, puiNumFragments, ppucFragBuffers) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC

//bug-fix suffix shenqiu}}
  UInt uiBitsLeft = m_pcNalUnitParser->getBitsLeft(); //JVT-P031

  ruiNalUnitType = m_pcNalUnitParser->getNalUnitType();

	//JVT-W047
	if(ruiNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ||
		ruiNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
		ruiNalUnitType == NAL_UNIT_PREFIX) 
	{
		if(m_pcSliceHeader)
			m_pcSliceHeader->setOutputFlag( m_pcNalUnitParser->getOutputFlag() );
	}
	//JVT-W047

//prefix unit{{
	//JVT-X046
	//if(m_pcPrefixSliceHeader && (ruiNalUnitType != NAL_UNIT_CODED_SLICE && ruiNalUnitType != NAL_UNIT_CODED_SLICE_IDR))
	if(m_pcPrefixSliceHeader && (ruiNalUnitType != NAL_UNIT_CODED_SLICE && ruiNalUnitType != NAL_UNIT_CODED_SLICE_IDR &&ruiNalUnitType != NAL_UNIT_CODED_SLICE_SCALABLE && ruiNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE ))
	{
		delete m_pcPrefixSliceHeader;
		m_pcPrefixSliceHeader = NULL;
		bDiscardable = true;//JVT-P031
    rbStartDecoding = true;//JVT-P031
		ruiEndPos = pcBinDataAccessor->size(); //JVT-P031
		return Err::m_nOK;
	}
//prefix unit}}
  //TMM_EC {{
  if(!bPreParseHeader && ruiNalUnitType==NAL_UNIT_END_OF_STREAM)
  {
	  SliceHeader* pTmp = m_pcSliceHeader;
    m_pcSliceHeader   = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader = pTmp;

		for ( UInt i=0; i< m_uiNumLayers; i++)
		{
			//JVT-W049 {
			//if (m_pauiPocInGOP[i])       delete	[] m_pauiPocInGOP[i];
   //   if (m_pauiFrameNumInGOP[i])  delete	[] m_pauiFrameNumInGOP[i];
			//JVT-W049 }
			if (m_pauiTempLevelInGOP[i]) delete	[] m_pauiTempLevelInGOP[i];
		}
    m_bLastFrame = true;
    delete m_pcSliceHeader;
    m_pcSliceHeader = NULL;
    return Err::m_nOK;
  }
//TMM_EC }}
  switch ( m_pcNalUnitParser->getNalUnitType() )
  {
  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
		{
    //JVT-P031
    RNOK( xStartSlice( bPreParseHeader, bDiscardable ) ); //FRAG_FIX //TMM_EC //JVT-S036 lsj
//prefix unit{{
	if(m_pcPrefixSliceHeader)
	{
		m_pcSliceHeader->copyPrefix(*m_pcPrefixSliceHeader);
		delete m_pcPrefixSliceHeader;
		m_pcPrefixSliceHeader = NULL;
	}
//prefix unit}}
    ruiEndPos = pcBinDataAccessor->size();
    bDiscardable = false;
    uiHeaderBits = uiBitsLeft - m_pcNalUnitParser->getBitsLeft();
    ruiStartPos += (uiHeaderBits+7)>>3;
  	ruiStartPos = 0; //FRAG_FIX
    //~JVT-P031
    RNOK( m_pcControlMng      ->initSlice0(m_pcSliceHeader) );
    m_bActive = true;
    rbStartDecoding = true; //JVT-P031
  	bSuffixUnit = true; //JVT-S036 lsj

//TMM_EC {{
//JVT-X046 {
	if ( m_eErrorConceal == EC_NONE )
	{
//JVT-X046 }
		if (!m_bNotSupport && !bPreParseHeader)
		{
				UInt	uiMaxGopSize	=	m_uiMaxGopSize;
			  UInt	uiGopSize;
			  m_uiNextLayerId		=	m_pcSliceHeader->getLayerId();
			  do
			  {
				  m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
				  uiGopSize	=	1 << m_uiDecompositionStages[m_uiNextLayerId];

				  if (m_uiFrameIdx[m_uiNextLayerId] % uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
					  break;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  while( true);

			  if ( m_pcNalUnitParser->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR || m_uiNextLayerId == 0)
			  {
				  UInt	uiMaxFrameNum	=	1 << m_pcSliceHeader->getSPS().getLog2MaxFrameNum();
  				//UInt	uiMaxPocLsb		=	1 << m_pcSliceHeader->getSPS().getLog2MaxPicOrderCntLsb();
				  m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
				  m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				  m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				  if ( uiGopSize == 1)
					  m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1;
				  else
					  m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiGopSize >> 1;
				  m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiMaxGopSize;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  else
			  {
				  m_uiNextFrameNum	=	0;
				  m_uiNextPoc			=	0;
					//JVT-W049 {
				  if(m_pcNalUnitParser->getNalRefIdc()==NAL_REF_IDC_PRIORITY_HIGHEST)
				  {
					  m_uiNextFrameNumRedu  =   0;
					  m_uiNextPocLsbRedu	=	0;
						m_uiNextLayerIdRedu = 0;
				  } 
				  //JVT-W049 }
			  }
		}
//JVT-X046 {
	}
//JVT-X046 }
//TMM_EC }}
	}
	break;
  case NAL_UNIT_SPS:
	case NAL_UNIT_SUB_SPS://SSPS
    {
      SequenceParameterSet* pcSPS = NULL;
      RNOK( SequenceParameterSet::create  ( pcSPS   ) );
			//SSPS {
			if( m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_SPS )
			{
        RNOK( pcSPS               ->read    ( m_pcHeaderSymbolReadIf,
                                            m_pcNalUnitParser->getNalUnitType() ) );
			}
			else
			{
        RNOK( pcSPS               ->readSubSPS( m_pcHeaderSymbolReadIf,
                                            m_pcNalUnitParser->getNalUnitType() ) );
			  
			}
			//SSPS }
      // It is assumed that layer 0 and layer 1 use the first two SPSs, respectively.
      if( NULL == m_pcVeryFirstSPS )
      {
        setVeryFirstSPS( pcSPS );
      }
//TMM_EC {{
			for ( UInt i=0; i<m_uiNumLayers; i++)
			{
				UInt	uiDecompositionStagesSub	=	m_uiMaxDecompositionStages - m_uiDecompositionStages[i];
				UInt	uiGopSize	=	m_uiGopSize[i];
				 //NS leak fix begin
				//JVT-W049 {
        //if (m_pauiPocInGOP[i])       delete	[] m_pauiPocInGOP[i];
        //if (m_pauiFrameNumInGOP[i])  delete	[] m_pauiFrameNumInGOP[i];
				//JVT-W049 }
        if (m_pauiTempLevelInGOP[i]) delete	[] m_pauiTempLevelInGOP[i];
        //NS leak fix end
				//JVT-W049 {
    //    m_pauiPocInGOP[i]				=	new	UInt[uiGopSize];
				//m_pauiFrameNumInGOP[i]	=	new	UInt[uiGopSize];
				//JVT-W049 }
				m_pauiTempLevelInGOP[i]	=	new	UInt[uiGopSize];
				UInt	uiFrameIdx	=	0;
				UInt	uiFrameNum	=	1;
				for( UInt uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages[i]; uiTemporalLevel++ )
				{
					UInt      uiStep    = ( 1 << ( m_uiDecompositionStages[i] - uiTemporalLevel ) );
					for( UInt uiFrameId = uiStep; uiFrameId <= uiGopSize; uiFrameId += ( uiStep << 1 ) )
					{
						m_pauiPocInGOP[i][uiFrameIdx]	=	uiFrameId << uiDecompositionStagesSub;
						m_pauiFrameNumInGOP[i][uiFrameIdx]	=	uiFrameNum;
						m_pauiTempLevelInGOP[i][uiFrameIdx]	=	uiTemporalLevel;
						uiFrameIdx++;
						if ( uiFrameId % 2 == 0)
							uiFrameNum++;
					}
				}
			}
//TMM_EC }}
			if ( pcSPS->getProfileIdc()==SCALABLE_PROFILE )
      {
//TMM_EC {{
				if ( pcSPS->getSeqParameterSetId() == 0)
				{
					 // m_bNotSupport	=	true;
					m_baseMode = 0;
				}
//TMM_EC }}
				m_bEnhancementLayer = true;
        m_bSpatialScalability = pcSPS->getFrameHeightInMbs() != m_pcVeryFirstSPS->getFrameHeightInMbs();
      }
			m_bNewSPS = true;
      RNOK( m_pcParameterSetMng ->store   ( pcSPS   ) );

      // Copy simple priority ID mapping from SPS to NAL unit parser
    /*  if ( !pcSPS->getNalUnitExtFlag() )
      {
        for ( UInt uiPriId = 0; uiPriId < pcSPS->getNumSimplePriIdVals(); uiPriId++)
        {
            UInt uiLayer, uiTempLevel, uiQualLevel;
            pcSPS->getSimplePriorityMap( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
            m_pcNalUnitParser->setSimplePriorityMap( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
        }

      }
 JVT-S036 lsj */

      ruiMbX  = pcSPS->getFrameWidthInMbs ();
      ruiMbY  = pcSPS->getFrameHeightInMbs();
      ruiSize = max( ruiSize, ( (ruiMbX << 3 ) + YUV_X_MARGIN ) * ( ( ruiMbY << 3 ) + YUV_Y_MARGIN ) * 6 );
			m_pcControlMng->initSPS( *pcSPS, m_uiRecLayerId );
      ruiEndPos = pcBinDataAccessor->size(); //JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
			//JVT-X046 {
			if ( pcSPS->getSeqParameterSetId() == 0 )
			{
				m_uiFrameHeightInMb = pcSPS->getFrameHeightInMbs();	
				m_uiFrameWidthInMb = pcSPS->getFrameWidthInMbs();
			}
			m_uiMbNumInFrame[pcSPS->getSeqParameterSetId()] = pcSPS->getFrameHeightInMbs()*pcSPS->getFrameWidthInMbs();
			if ( m_bMbStatus[pcSPS->getSeqParameterSetId()] != NULL )
			{
				delete [] m_bMbStatus[pcSPS->getSeqParameterSetId()];				
				m_bMbStatus[pcSPS->getSeqParameterSetId()] = NULL;
			}
			m_bMbStatus[pcSPS->getSeqParameterSetId()] = new Bool[m_uiMbNumInFrame[pcSPS->getSeqParameterSetId()]];
			for ( UInt uiMbAddress = 0; uiMbAddress < m_uiMbNumInFrame[pcSPS->getSeqParameterSetId()]; uiMbAddress++ )
			{
				m_bMbStatus[pcSPS->getSeqParameterSetId()][uiMbAddress] = true;
			}
			//JVT-X046 }
    }
    break;

  case NAL_UNIT_PPS:
    {
      PictureParameterSet* pcPPS = NULL;
      RNOK( PictureParameterSet::create( pcPPS  ) );
      RNOK( pcPPS->read( m_pcHeaderSymbolReadIf,
                         m_pcNalUnitParser->getNalUnitType() ) );
      RNOK( m_pcParameterSetMng->store( pcPPS   ) );
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;

  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    {
      //JVT-P031
      getDecodedResolution(m_uiDecodedLayer);
      if(m_pcNalUnitParser->getLayerId() < m_uiDecodedLayer && m_pcNalUnitParser->getDiscardableFlag())
        bDiscardable = true;
      else
        bDiscardable = false;

      RNOK( xStartSlice( bPreParseHeader, bDiscardable) );

      if(bDiscardable)
        ruiEndPos = 0;
      else
        ruiEndPos = pcBinDataAccessor->size();

      uiHeaderBits = uiBitsLeft - m_pcNalUnitParser->getBitsLeft();
      // JVT-U116 LMI {
      UChar ucByte = pcBinDataAccessor->data()[3];
      if ( ucByte & 1 )
        uiHeaderBits += 8;
      // JVT-U116 LMI }

      ruiStartPos = 0;

      if(!bDiscardable)//~JVT-P031
      {
        RNOK( m_pcControlMng->initSlice0(m_pcSliceHeader) );
      }
      //bug-fix suffix shenqiu{{
      // if(m_pcSliceHeader) //JVT-P031

      //bug-fix suffix shenqiu
      //JVT-P031

      rbStartDecoding = true;

      //~JVT-P031
      //TMM_EC {{
			//JVT-W049 {
			if(m_pcNalUnitParser->getNalRefIdc()==NAL_REF_IDC_PRIORITY_HIGHEST)
			{
				if((!bKeyPicReduUseSvc[m_pcNalUnitParser->getLayerId()])&&(isRedundantPic()))
				{
					if(bIfNextFrame)
					{
						bIfNextFrame = false;
					    return Err::m_nOK;
					}
					else
					{
						bIfNextFrame = true;
						//return Err::m_nOK;
					}
				}
				else if((bKeyPicReduUseSvc[m_pcNalUnitParser->getLayerId()])&&(m_pcSliceHeader->getRedundantPicCnt()))
				{					
					if(bIfNextFrame)
					{
						bIfNextFrame = false;
					    m_pcSliceHeader->setRedundantPicCnt(0);
					}
					else
					{
						bIfNextFrame = true;
					}
				}
			}
      //JVT-W049 }
//JVT-X046 {
		if ( m_eErrorConceal == EC_NONE )
		{
//JVT-X046 }
      if ( !m_bNotSupport && !bPreParseHeader && m_pcNalUnitParser->getQualityLevel() == 0)
      {
        UInt	uiMaxGopSize	=	m_uiMaxGopSize;
			  UInt	uiGopSize;
			  m_uiNextLayerId		=	m_pcSliceHeader->getLayerId();
			  do
			  {
				  m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
				  uiGopSize	=	1 << m_uiDecompositionStages[m_uiNextLayerId];
				  if (m_uiFrameIdx[m_uiNextLayerId] % uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
					  break;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  while( true);
//EC bug fix
				if ( m_pcNalUnitParser->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_SCALABLE || m_uiNextLayerId == 0)
				{
				  UInt	uiMaxFrameNum	=	1 << m_pcSliceHeader->getSPS().getLog2MaxFrameNum();
 				  //UInt	uiMaxPocLsb		=	1 << m_pcSliceHeader->getSPS().getLog2MaxPicOrderCntLsb();
				  m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
				  m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
					//JVT-W049 {
				  if(m_pcNalUnitParser->getNalRefIdc()==NAL_REF_IDC_PRIORITY_HIGHEST)
				  {
					  m_uiNextFrameNumRedu  =   m_pcSliceHeader->getFrameNum();
					  m_uiNextPocLsbRedu	  =	  m_pcSliceHeader->getPicOrderCntLsb();
						m_uiNextLayerIdRedu   =   m_pcSliceHeader->getLayerId();
				  } 
				  //JVT-W049 }
				  m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				  if ( uiGopSize == 1)
				      m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1;
				  else
					  m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiGopSize >> 1;
				  m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiMaxGopSize;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  else
			  {
				  m_uiNextFrameNum	=	0;
				  m_uiNextPoc			=	0;
					//JVT-W049 {
				  if(m_pcNalUnitParser->getNalRefIdc()==NAL_REF_IDC_PRIORITY_HIGHEST)
				  {
					  m_uiNextFrameNumRedu  =   0;
					  m_uiNextPocLsbRedu	    =	0;
						m_uiNextLayerIdRedu = m_pcNalUnitParser->getLayerId();
				  } 
				  //JVT-W049 }
			  }
		  }
//JVT-X046 {
		}
		else
		{		
			if ( m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
					if ( !m_bNotSupport && !bPreParseHeader && m_pcNalUnitParser->getQualityLevel() == 0 && m_uiNextFirstMb == 0)
					{
						UInt	uiMaxGopSize	=	m_uiMaxGopSize;
						UInt	uiGopSize;
						m_uiNextLayerId		=	m_pcSliceHeader->getLayerId();		  			
						do
						{
							m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;						
							uiGopSize	=	1 << m_uiDecompositionStages[m_uiNextLayerId];
							if (m_uiFrameIdx[m_uiNextLayerId] % uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
								break;
							m_uiFrameIdx[m_uiNextLayerId]++;
						}
						while( true);
		//EC bug fix					
						if ( m_pcNalUnitParser->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_SCALABLE || m_uiNextLayerId == 0)
						{
							UInt	uiMaxFrameNum	=	1 << m_pcSliceHeader->getSPS().getLog2MaxFrameNum();
 							//UInt	uiMaxPocLsb		=	1 << m_pcSliceHeader->getSPS().getLog2MaxPicOrderCntLsb();
							m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
							m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
							m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
							if ( uiGopSize == 1) 
									m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1;
							else
								m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiGopSize >> 1;
							m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiMaxGopSize;
							m_uiFrameIdx[m_uiNextLayerId]++;
						}
						else
						{
							m_uiNextFrameNum	=	0;
							m_uiNextPoc			=	0;
						}
					}
		}
//JVT-X046 }
//TMM_EC }}
  }
  break;

  case NAL_UNIT_SEI:
    {
      //===== just for trace file =====
      SEI::MessageList  cMessageList;
			UInt	i;
      RNOK( SEI::read( m_pcHeaderSymbolReadIf, cMessageList
    				// JVT-V068 HRD }
					, m_pcParameterSetMng 
    				// JVT-V068 HRD }
			) );

      while( ! cMessageList.empty() )
      {
        SEI::SEIMessage*  pcSEIMessage = cMessageList.popBack();
		if(pcSEIMessage->getMessageType() == SEI::NON_REQUIRED_SEI)
		{
			m_pcNonRequiredSei = (SEI::NonRequiredSei*) pcSEIMessage;
			m_uiNonRequiredSeiReadFlag = 1;
		}
		else
		{
				  if ( pcSEIMessage->getMessageType() == SEI::SCALABLE_SEI)
				  {
//	trick
            m_uiNumLayers						=	((SEI::ScalableSei*)pcSEIMessage)->getDependencyId( ((SEI::ScalableSei*)pcSEIMessage)->getNumLayersMinus1()) + 1;
					  for ( uiLayerId=0; uiLayerId<((SEI::ScalableSei*)pcSEIMessage)->getNumLayersMinus1()+1; uiLayerId++)
            {
              if ( ((SEI::ScalableSei*)pcSEIMessage)->getDependencyId( uiLayerId) != 0)
                break;
            }
            uiLayerId--;
            m_uiDecompositionStages[0]	=	((SEI::ScalableSei*)pcSEIMessage)->getTemporalLevel( uiLayerId);
            m_uiDecompositionStages[m_uiNumLayers-1]	=	((SEI::ScalableSei*)pcSEIMessage)->getTemporalLevel( ((SEI::ScalableSei*)pcSEIMessage)->getNumLayersMinus1());
					  m_uiMaxDecompositionStages = m_uiDecompositionStages[m_uiNumLayers-1];
					  m_uiMaxGopSize	=	1 << m_uiMaxDecompositionStages;
						for ( i=0; i< m_uiNumLayers; i++)
						  m_uiGopSize[i]	=	1 << m_uiDecompositionStages[i];
				  }
// JVT-T073 {
				  else if( pcSEIMessage->getMessageType() == SEI::SCALABLE_NESTING_SEI )
				  {
				      //do nothing, or add your code here
				  }
// JVT-T073 }
					// JVT-W049 {				  
				  else if( pcSEIMessage->getMessageType() == SEI::REDUNDANT_PIC_SEI)
				  {
             m_uiNumDId = ((SEI::RedundantPicSei*)pcSEIMessage)->getNumDIdMinus1()+1;                       
             for( UInt ui = 0; ui < m_uiNumDId; ui++ )
             {
               m_uiNumQId[ui] = ((SEI::RedundantPicSei*)pcSEIMessage)->getNumQIdMinus1(ui)+1;
               for( UInt uj =0; uj < m_uiNumQId[ui]; uj++ )
               {
                 m_uiHaveRed[ui][uj] = 0;
                 if(ui > 0 )
                   m_uiHaveRed[ui][uj] = 1;
               }
             }
				  }
          // JVT-W049 }
					//JVT-W052
					else if( pcSEIMessage->getMessageType() == SEI::INTEGRITY_CHECK_SEI )
					{
						//do nothing,or add your feedback information check here
					}
					//JVT-W052
					//JVT-X032 {
					else if(pcSEIMessage->getMessageType() == SEI::TL_SWITCHING_POINT_SEI)
					{
					}
					//JVT-X032 }
			    delete pcSEIMessage;
          pcSEIMessage = NULL;
		}
      }
      ruiEndPos = (uiBitsLeft+7)/8; //FRAG_FIX
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;
  case NAL_UNIT_ACCESS_UNIT_DELIMITER:
    {
      RNOK ( m_pcNalUnitParser->readAUDelimiter());
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;
  case NAL_UNIT_END_OF_SEQUENCE:
    {
      RNOK ( m_pcNalUnitParser->readEndOfSeqence());
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
  case NAL_UNIT_END_OF_STREAM:
    {
      RNOK ( m_pcNalUnitParser->readEndOfStream());
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;
//prefix unit{{
  case NAL_UNIT_PREFIX:  
	{
	  SequenceParameterSet* pcSPS;
    PictureParameterSet*  pcPPS;
	  RNOK( m_pcParameterSetMng ->get    ( pcPPS, 0) );
    RNOK( m_pcParameterSetMng ->get    ( pcSPS, 0) );
		if(m_pcPrefixSliceHeader)
			{
				delete m_pcPrefixSliceHeader;
				m_pcPrefixSliceHeader = NULL;
			}
	  m_pcPrefixSliceHeader = new SliceHeader(*pcSPS, *pcPPS);
	  RNOK( m_pcSliceReader->readSliceHeaderPrefix( m_pcNalUnitParser->getNalUnitType   (),
														  m_pcNalUnitParser->getNalRefIdc     (),
														  m_pcNalUnitParser->getLayerId		  (),
														  m_pcNalUnitParser->getQualityLevel  (),
														  m_pcNalUnitParser->getUseBasePredFlag(),
														  m_pcPrefixSliceHeader
														  ) 
											);
	  ruiEndPos = pcBinDataAccessor->size(); //JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
	}
	break;
//prefix unit}}
  default:
    return Err::m_nERR;
    break;
  }

  m_uiNonRequiredPic = 0; //NonRequired JVT-Q066
  //ruiNonRequiredPic = 0;

  if(m_pcSliceHeader)
  {
	  m_uiCurrPicLayer = (m_pcSliceHeader->getLayerId() << 4) + m_pcSliceHeader->getQualityLevel();
	  if(m_uiCurrPicLayer == 0 || m_uiCurrPicLayer <= m_uiPrevPicLayer)
	  {
		  if(m_uiNonRequiredSeiReadFlag == 0 && m_pcNonRequiredSei)
		  {
			  m_pcNonRequiredSei->destroy();
			  m_pcNonRequiredSei = NULL;
		  }
		  m_uiNonRequiredSeiRead = m_uiNonRequiredSeiReadFlag;
		  m_uiNonRequiredSeiReadFlag = 0;
	  }
	  m_uiPrevPicLayer = m_uiCurrPicLayer;

	  if(m_uiNonRequiredSeiRead == 1)
	  {
		  for(UInt i = 0; i <= m_pcNonRequiredSei->getNumInfoEntriesMinus1(); i++)
		  {
			  if(m_pcNonRequiredSei->getEntryDependencyId(i))  // it should be changed to if(DenpendencyId == LayerId of the shown picture)
			  {
				  for(UInt j = 0; j <= m_pcNonRequiredSei->getNumNonRequiredPicsMinus1(i); j++)
				  {
					  if(m_pcSliceHeader->getLayerId() == m_pcNonRequiredSei->getNonRequiredPicDependencyId(i,j) &&
						  m_pcSliceHeader->getQualityLevel() == m_pcNonRequiredSei->getNonRequiredPicQulityLevel(i,j))  // it should be add something about FragmentFlag
					  {
						  m_uiNonRequiredPic = 1;  //NonRequired JVT-Q066
						//  ruiNonRequiredPic = 1;
						  ROTRS( m_apcMCTFDecoder[m_pcSliceHeader->getLayerId()]->getWaitForIdr() && !m_pcSliceHeader->isIdrNalUnit(), Err::m_nOK );
						  m_apcMCTFDecoder[m_pcSliceHeader->getLayerId()]->setWaitForIdr(false);
						  return Err::m_nOK;
					  }
				  }
			  }
		  }
	  }
#ifdef SHARP_AVC_REWRITE_OUTPUT
    if ((m_pcSliceHeader->getAVCRewriteFlag()))
    {
      m_avcRewriteFlag = true;
    }
    else
      m_avcRewriteFlag = false;
#endif
  }
  return Err::m_nOK;
}

//JVT-P031
ErrVal
H264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor)
{
  return m_pcNalUnitParser->initSODBNalUnit(pcBinDataAccessor);
}
//~JVT-P031

//JVT-S036 lsj start
ErrVal
H264AVCDecoder::initPacketSuffix( BinDataAccessor*  pcBinDataAccessor,
											UInt&             ruiNalUnitType
											, Bool            bPreParseHeader
											, Bool			      bConcatenated
											, Bool&           rbStartDecoding
											 ,SliceHeader     *pcSliceHeader
											  ,Bool&		  SuffixEnable
								)
{
  ROF( m_bInitDone );
//  UInt uiLayerId;

  ROT( NULL == pcBinDataAccessor );
  if ( NULL == pcBinDataAccessor->data() || 0 == pcBinDataAccessor->size() )
  {
    // switch
    SliceHeader* pTmp = m_pcSliceHeader;
    m_pcSliceHeader   = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader = pTmp;

    m_bLastFrame = true;
    delete m_pcSliceHeader;
    m_pcSliceHeader = NULL;
    rbStartDecoding = true; //JVT-P031

	if((m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_SCALABLE || m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
		 && m_pcNalUnitParser->getLayerId() == 0 && m_pcNalUnitParser->getQualityLevel() == 0)
	{
		SuffixEnable = true;
	}
	else
	{
		SuffixEnable = false;
	}
    return Err::m_nOK;
  }


  //JVT-P031
//  Bool KeyPicFlag = false; //bug-fix suffix shenqiu
  getDecodedResolution(m_uiDecodedLayer);
  m_pcNalUnitParser->setDecodedLayer(m_uiDecodedLayer);
  //~JVT-P031
  UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu{{
  //RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, &KeyPicFlag,uiNumBytesRemoved, bPreParseHeader , bConcatenated) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC
  RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, uiNumBytesRemoved, bPreParseHeader , bConcatenated) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu}}
  ruiNalUnitType = m_pcNalUnitParser->getNalUnitType();


//TMM_EC }}
  switch ( m_pcNalUnitParser->getNalUnitType() )
  {

  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    {
		if( m_pcNalUnitParser->getLayerId() == 0 && m_pcNalUnitParser->getQualityLevel() == 0)
		{
			RNOK( m_pcSliceReader->readSliceHeaderSuffix( m_pcNalUnitParser->getNalUnitType   (),
														  m_pcNalUnitParser->getNalRefIdc     (),
														  m_pcNalUnitParser->getLayerId		  (),
														  m_pcNalUnitParser->getQualityLevel  (),
                              m_pcNalUnitParser->getUseBasePredFlag(),
														  pcSliceHeader
														  )
											);
			SuffixEnable = true;
			return Err::m_nOK;
		}
		else
		{
			SuffixEnable = false;
			return Err::m_nOK;
		}

    }
    break;

  default:
	  {
		  SuffixEnable = false;
			return Err::m_nOK;
	  }
    break;
  }

  return Err::m_nOK;
}

//JVT-S036 lsj end

ErrVal
H264AVCDecoder::getBaseLayerPWTable( SliceHeader::PredWeightTable*& rpcPredWeightTable,
                                     UInt                           uiBaseLayerId,
                                     ListIdx                        eListIdx,
                                     Int                            iPoc )
{
  if( uiBaseLayerId || m_apcMCTFDecoder[uiBaseLayerId]->isActive() )
  {
    RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerPWTable( rpcPredWeightTable, eListIdx, iPoc ) );
    return Err::m_nOK;
  }
  rpcPredWeightTable = &m_acLastPredWeightTable[eListIdx];
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerUnit(UInt            uiBaseLayerId,
                                 Int             iPoc,
                                 DPBUnit*      &pcBaseDPBUnit)
{
  RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerUnit( iPoc , pcBaseDPBUnit) );
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerData( IntFrame*&      pcFrame,
                                  IntFrame*&      pcResidual,
                                  MbDataCtrl*&    pcMbDataCtrl,
                                  MbDataCtrl*&    pcMbDataCtrlEL,
                                  Bool&           rbConstrainedIPred,
                                  Bool&           rbSpatialScalability,
                                  UInt            uiLayerId,
                                  UInt            uiBaseLayerId,
                                  Int             iPoc,
                                  UInt            uiBaseQualityLevel) //JVT-T054
{
  if(uiBaseLayerId != 0 || !m_apcMCTFDecoder[uiBaseLayerId]->getAVCBased() ||
    (uiBaseQualityLevel != 0 && m_abMGSAtLayer[uiBaseLayerId])) //MGS_FIX_FT_09_2007
    //(uiBaseQualityLevel != 0 && m_bCGSSNRInAU))
  {
    //===== base layer is scalable extension =====
      //--- get data ---
	  RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerData( pcFrame,
	                                                         pcResidual,
	                                                         pcMbDataCtrl,
	                                                         pcMbDataCtrlEL,
	                                                         rbConstrainedIPred,
	                                                         rbSpatialScalability,
	                                                         iPoc ) );
  }
  else
  {
    //===== base layer is standard H.264/AVC =====
    FrameUnit*  pcFrameUnit = m_pcFrameMng->getReconstructedFrameUnit( iPoc );
    ROF( pcFrameUnit );

    pcFrame           = rbSpatialScalability ? m_pcFrameMng->getRefinementIntFrame2():
                                                 m_pcFrameMng->getRefinementIntFrame();

    pcResidual          = pcFrameUnit ->getResidual();
    pcMbDataCtrl        = pcFrameUnit ->getMbDataCtrl();
		pcMbDataCtrlEL      = m_pcBaseLayerCtrlEL;
    rbConstrainedIPred  = pcFrameUnit ->getContrainedIntraPred();
  }
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerResidual( IntFrame*&  pcFrame,
                                     IntFrame*&      pcResidual,
                                     UInt            uiLayerId,
                                     UInt            uiQualityLevel,
                                     Int             iPoc)
{
  if(uiLayerId != 0 || !m_apcMCTFDecoder[uiLayerId]->getAVCBased() ||
    (uiQualityLevel != 0 && m_abMGSAtLayer[uiLayerId])) //MGS_FIX_FT_09_2007
    //(uiQualityLevel != 0 && m_bCGSSNRInAU))
  {
    //===== base layer is scalable extension =====
    pcResidual = m_apcMCTFDecoder[uiLayerId]->getBaseLayerResidual();
    return Err::m_nOK;
  }
  else
  {
    return Err::m_nERR;
  }  
}

ErrVal
H264AVCDecoder::getBaseLayerDataAvailability( IntFrame*&      pcFrame,
                                              IntFrame*&      pcResidual,
                                              MbDataCtrl*&    pcMbDataCtrl,
                                              Bool&           rbBaseDataAvailable,
                                              Bool&           rbSpatialScalability,
                                              UInt            uiLayerId,
                                              UInt            uiBaseLayerId,
                                              Int             iPoc,
                                              UInt            uiBaseQualityLevel) //JVT-T054
{
  if(uiBaseLayerId != 0 || !m_apcMCTFDecoder[uiBaseLayerId]->getAVCBased() ||
    (uiBaseQualityLevel != 0 && m_abMGSAtLayer[uiBaseLayerId])) //MGS_FIX_FT_09_2007
    //(uiBaseQualityLevel != 0 && m_bCGSSNRInAU))
  {
    //===== base layer is scalable extension =====
    //--- get spatial resolution ratio ---
    // TMM_ESS
	  if( m_apcMCTFDecoder[uiLayerId]->getFrameHeight() != m_apcMCTFDecoder[uiBaseLayerId]->getFrameHeight() &&
        m_apcMCTFDecoder[uiLayerId]->getFrameWidth () != m_apcMCTFDecoder[uiBaseLayerId]->getFrameWidth ()   )
    {
      rbSpatialScalability = true;
    }
    else
    {
      rbSpatialScalability = false;
    }
	  // TMM_ESS

    //--- get data ---
	  RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerDataAvailability( pcFrame, pcResidual, pcMbDataCtrl, rbBaseDataAvailable, rbSpatialScalability, iPoc ) );
  }
  else
  {
    //===== base layer is standard H.264/AVC =====
    //--- get spatial resolution ratio ---
    // TMM_ESS
	  if( m_apcMCTFDecoder[uiLayerId]->getFrameHeight() == 16 * m_pcVeryFirstSliceHeader->getSPS().getFrameHeightInMbs () &&
        m_apcMCTFDecoder[uiLayerId]->getFrameWidth () == 16 * m_pcVeryFirstSliceHeader->getSPS().getFrameWidthInMbs  ()   )
    {
      rbSpatialScalability = false;
    }
    else
    {
      rbSpatialScalability = true;
    }
	  // TMM_ESS

    FrameUnit*  pcFrameUnit = m_pcFrameMng->getReconstructedFrameUnit( iPoc );
    ROF( pcFrameUnit );

    pcMbDataCtrl        = pcFrameUnit ->getMbDataCtrl();
    if(uiLayerId == uiBaseLayerId)
      m_apcMCTFDecoder[0]->setILPrediction(pcFrameUnit->getFGSIntFrame(), FRAME); // TMM_INTERLACE

    rbBaseDataAvailable = pcMbDataCtrl>0;
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

  //EIDR bug-fix {
	if(m_pcSliceHeader )
	{
		if(m_pcSliceHeader->getLayerId() == 0 )
		{
			m_pcSliceHeader->setInIDRAccess(m_pcSliceHeader->isIdrNalUnit());
		}
		else
		{
			//JVT-W049 {
			//m_pcSliceHeader->setInIDRAccess(m_pcPrevSliceHeader?m_pcPrevSliceHeader->getInIDRAccess():false);
			m_pcSliceHeader->setInIDRAccess(m_pcPrevSliceHeader?m_pcPrevSliceHeader->isIdrNalUnit():false);
			//JVT-W049 }
		}
	}
 //EIDR bug-fix }

  //JVT-T054_FIX{
  if(m_bBaseSVCActive)
  {
    RNOK( xInitSlice( m_pcSliceHeader ) );
    m_apcMCTFDecoder[0]->GetAVCFrameForDPB(m_pcSliceHeader,pcPicBuffer, rcPicBufferOutputList, rcPicBufferUnusedList);
    m_bBaseSVCActive = false;
    m_bLastFrameReconstructed = true;
    return Err::m_nOK;
  }
  if(!m_bLastFrameReconstructed)
  {
   //JVT-T054}
    RNOK( xInitSlice( m_pcSliceHeader ) );
    //JVT-T054_FIX{
  }
  else
  {
    if( m_pcSliceHeader && m_pcSliceHeader->getLayerId() == 0 )
    {
      RNOK( m_pcPocCalculator->calculatePoc( *m_pcSliceHeader ) );
    }
  }

  m_bLastFrameReconstructed = false; //JVT-T054}


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

      PicBufferList cPicBufferList;
      Int           iMaxPoc;
      RNOK( m_pcFrameMng->outputAll() );
      RNOK( m_pcFrameMng->setPicBufferLists( cPicBufferList, rcPicBufferReleaseList ) );
      if(m_bCGSSNRInAU)
      {
        RNOK  ( m_apcMCTFDecoder[m_uiRecLayerId]->finishProcess( rcPicBufferOutputList,
                                                               rcPicBufferUnusedList,
                                                               iMaxPoc = MSYS_INT_MIN ) );
      }
      else
      {
        rcPicBufferOutputList += cPicBufferList;
      }
    }

    rcPicBufferUnusedList.pushBack( pcPicBuffer );
    return Err::m_nOK;
  }

  const NalUnitType eNalUnitType  = m_pcNalUnitParser->getNalUnitType();

#ifdef SHARP_AVC_REWRITE_OUTPUT
	  if (m_avcRewriteBindata!=NULL)
		  m_avcRewriteBindata->reset();
#endif

  //===== decode NAL unit =====
  switch( eNalUnitType )
  {
  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    {
      ROT( m_pcSliceHeader == NULL );
#ifdef SHARP_AVC_REWRITE_OUTPUT
      if (m_pcSliceHeader->getLayerId() !=MSYS_UINT_MAX  && !xGetAvcRewriteFlag()) {
       ::fprintf(stderr, "\nERROR:   AVC rewrite flag is not present in the input bitstream\n"); assert(0); return Err::m_nERR;
      }
#endif
      m_uiLastLayerId = m_pcSliceHeader->getLayerId();
      m_bLastNalInAU = m_uiNumOfNALInAU == 0; //JVT-T054
// JVT-Q054 Red. Picture {
      if ( NULL != m_pcSliceHeader_backup )
      {
        RNOK( m_pcSliceHeader->sliceHeaderBackup( m_pcSliceHeader_backup ) );
      }
// JVT-Q054 Red. Picture }
			//JVT-W049 {
      if( m_pcSliceHeader->getLayerId() == m_uiNumDId-1 &&m_pcSliceHeader->getTrueSlice() &&
        m_uiHaveRed[m_pcSliceHeader->getLayerId()][m_pcSliceHeader->getQualityLevel()] )
        m_bLastNalInAU = 1; 
      //JVT-W049 }

      PicBufferList   cDummyList;
      PicBufferList&  rcOutputList  = ( m_uiLastLayerId == m_uiRecLayerId ? rcPicBufferOutputList : cDummyList );
//JVT-T054{
      Bool bHighestLayer;
      bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId && ( m_bLastNalInAU || m_pcSliceHeader->getStoreBaseRepresentationFlag() ) );
//JVT-T054}
//	TMM EC {{
      if ( m_pcNalUnitParser->getQualityLevel() == 0)
      {
			  m_apcMCTFDecoder[m_uiLastLayerId]->m_uiDecompositionStages	=	m_uiDecompositionStages[m_uiLastLayerId];
			  m_apcMCTFDecoder[m_uiLastLayerId]->m_uiDecompositionStagesBase	=	m_uiDecompositionStages[0];
      }
//TMM_EC }}
//JVT-T054{
      if(m_bBaseLayerAvcCompliant && m_uiLastLayerId == 0 && m_pcSliceHeader->getQualityLevel() == 1)
      {
         m_apcMCTFDecoder[m_uiLastLayerId]->setAVCBased(true);
      }
      else
      {
        m_apcMCTFDecoder[m_uiLastLayerId]->setAVCBased(false);
      }
//JVT-T054}
#ifdef SHARP_AVC_REWRITE_OUTPUT
      m_pcSliceHeader->setReconstructionLayer(bHighestLayer);
      if ( xGetAvcRewriteFlag())
      {
        if (bHighestLayer)
        {
          m_avcRewriteBindata->set(m_avcRewriteBinDataBuffer, m_avcRewriteBufsize);
          m_avcRewriteBindata->setMemAccessor(*m_avcRewriteBinDataAccessor );
          m_pcAvcRewriteEncoder->xAvcRewriteInitNalUnit(m_avcRewriteBinDataAccessor);
          RNOK( m_pcAvcRewriteEncoder->xAvcRewriteSliceHeader(m_avcRewriteBinDataAccessor, *m_pcSliceHeader) );
        }
        RNOK(xInitSliceForAvcRewriteCoding(*m_pcSliceHeader));
      }
#endif
			//JVT-X046 {			
			m_apcMCTFDecoder[m_uiLastLayerId]->m_uiNextFirstMb = m_uiNextFirstMb;
			m_apcMCTFDecoder[m_uiLastLayerId]->m_uiLostMbNum = m_uiLostMbNum;
			m_apcMCTFDecoder[m_uiLastLayerId]->m_uiMbNumInFrame =	m_pcSliceHeader->getSPS().getFrameHeightInMbs()*m_pcSliceHeader->getSPS().getFrameWidthInMbs();
			if ( !m_apcMCTFDecoder[m_uiLastLayerId]->m_bMbStatus )
			{				
				m_apcMCTFDecoder[m_uiLastLayerId]->m_bMbStatus = new Bool[m_apcMCTFDecoder[m_uiLastLayerId]->m_uiMbNumInFrame];
			}
			if ( m_uiLastLayerId > 0 && m_bLayerStatus[m_uiLastLayerId-1] == false || !m_pcSliceHeader->getTrueSlice() )
			{
				if (m_uiLastLayerId > 0 && m_bLayerStatus[m_uiLastLayerId-1] == false)
				{
					m_uiLostMbNum = m_uiMbNumInFrame[m_uiLastLayerId] - m_uiNextFirstMb;
					m_apcMCTFDecoder[m_uiLastLayerId]->m_uiLostMbNum = m_uiLostMbNum;
				}
				m_pcSliceHeader->setTrueSlice(false);
				m_pcSliceHeader->setBaseLayerId(MSYS_UINT_MAX);
				if ( m_pcSliceHeader->isIntra() == true )
					m_pcSliceHeader->setSliceType(P_SLICE);
			}
			//JVT-X046 }
      RNOK( m_apcMCTFDecoder[m_uiLastLayerId] ->process     ( m_pcSliceHeader,
                                                              pcPicBuffer,
                                                              rcOutputList,
                                                              rcPicBufferUnusedList,
                                                              bHighestLayer ) );
      RNOK( m_pcNalUnitParser                 ->closeNalUnit() );
#ifdef SHARP_AVC_REWRITE_OUTPUT
      if (xGetAvcRewriteFlag() && bHighestLayer) {
        m_pcAvcRewriteEncoder->xAvcRewriteCloseNalUnit();
        m_avcRewriteBindata->set(m_avcRewriteBinDataBuffer, m_avcRewriteBinDataAccessor->size());
      }
#endif
			//JVT-X046 {
			m_uiDecodedMbNum = m_apcMCTFDecoder[m_uiLastLayerId] -> m_uiDecodedMbNum;
			m_uiNextFirstMb = m_uiDecodedMbNum % m_uiMbNumInFrame[m_uiLastLayerId];
			//JVT-X046 }

      return Err::m_nOK;
    }
    break;

  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
    {
      ROF( m_pcSliceHeader );
      ROF( m_pcSliceHeader->getLayerId() == 0 );
      m_uiLastLayerId    = m_pcSliceHeader->getLayerId();
//JVT-T054{
      Bool bHighestLayer;
      m_bLastNalInAU = (m_uiNumOfNALInAU == 0);
			//JVT-W049 {
      if( m_pcSliceHeader->getLayerId() == m_uiNumDId-1 &&
        m_uiHaveRed[m_pcSliceHeader->getLayerId()] )
        m_bLastNalInAU = 1; 
      //JVT-W049 }
      bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId && m_bLastNalInAU);
//JVT-T054}
// JVT-Q054 Red. Picture {
      if ( NULL != m_pcSliceHeader_backup )
      {
        RNOK( m_pcSliceHeader->sliceHeaderBackup( m_pcSliceHeader_backup ) );
      }
// JVT-Q054 Red. Picture }

//	TMM EC {{
			//JVT-X046
			//m_apcMCTFDecoder[m_uiLastLayerId+1]->m_bBaseLayerLost	=	!m_pcSliceHeader->getTrueSlice();

//			if(eNalUnitType==NAL_UNIT_VIRTUAL_BASELAYER)
			if ( !m_pcSliceHeader->getTrueSlice())
			{
				//JVT-X046 {
				//RNOK( xProcessSliceVirtual( *m_pcSliceHeader, m_pcPrevSliceHeader, pcPicBuffer ) );
				if ( m_eErrorConceal == EC_FRAME_COPY )
				{
					RNOK( xDecodeBaseLayerVirtual( *m_pcSliceHeader, m_pcPrevSliceHeader, pcPicBuffer ) );
				}
				else
				{
					RNOK( xProcessSliceVirtual( *m_pcSliceHeader, m_pcPrevSliceHeader, pcPicBuffer ) );
				}
				m_bLayerStatus[m_pcSliceHeader->getLayerId()] = false;
				//JVT-X046 }
			}
			else
			{
        g_nLayer = m_pcSliceHeader->getQualityLevel();
        DTRACE_LAYER( g_nLayer );

        RNOK( xProcessSlice( *m_pcSliceHeader,
                             m_pcPrevSliceHeader,
                             pcPicBuffer,
                             bHighestLayer ) ); //JVT-T054
			}
			m_apcMCTFDecoder[m_uiLastLayerId+1]->m_bBaseLayerLost = !m_bLayerStatus[m_uiLastLayerId];//JVT-X046
//TMM_EC }}

      PicBufferList   cDummyList;
//TMM
			//JVT-X046 {
      //PicBufferList&  rcOutputList  = ( (!m_bCGSSNRInAU && m_uiRecLayerId == 0 && m_pcFrameMng->getPicBufferOutputList().size() ) ? rcPicBufferOutputList : cDummyList ); //JVT-T054
			PicBufferList& rcOutputList = ( (!m_bCGSSNRInAU && (m_uiRecLayerId == 0 || bHighestLayer) ) ? rcPicBufferOutputList : cDummyList );
			//JVT-X046 }

      RNOK( m_pcFrameMng->setPicBufferLists( rcOutputList, rcPicBufferReleaseList ) );


//JVT-T054_FIX{
      m_apcMCTFDecoder[m_uiLastLayerId]->setAVCBased(true);
      m_bBaseSVCActive = (bHighestLayer && m_apcMCTFDecoder[0]->isActive());

//JVT-T054}
    }
    break;

  case NAL_UNIT_SPS:
	case NAL_UNIT_SUB_SPS: //SSPS
    {
      break;
    }


  case NAL_UNIT_PPS:
  case NAL_UNIT_SEI:
	case NAL_UNIT_ACCESS_UNIT_DELIMITER:
	case NAL_UNIT_END_OF_SEQUENCE:
	case NAL_UNIT_END_OF_STREAM:
    {
    }
    break;

  default:
    {
      AF();
      return Err::m_nERR;
    }
    break;
  }

  rcPicBufferUnusedList.pushBack( pcPicBuffer );
  RNOK( m_pcNalUnitParser->closeNalUnit() );

  return Err::m_nOK;
}



ErrVal H264AVCDecoder::xStartSlice(Bool& bPreParseHeader, Bool& bDiscardable) //FRAG_FIX //TMM_EC  //JVT-S036 lsj
{
  //JVT-P031
  SliceHeader * pSliceHeader = NULL;

  if(m_pcNalUnitParser->getDiscardableFlag() == false || m_pcNalUnitParser->getLayerId() == m_uiDecodedLayer)
  {
    //TMM_EC {{
      //UInt uiPPSId = (m_pcSliceHeader != NULL) ? m_pcSliceHeader->getPicParameterSetId() : 0;
		if ( !m_pcNalUnitParser->isTrueNalUnit())
		{

      SliceHeader*    rpcSliceHeader=NULL;
      if(m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE_SCALABLE && m_uiNextLayerId==0)
      {
        rpcSliceHeader=m_apcMCTFDecoder[0]->m_pcVeryFirstSliceHeader;
      }
      else if(m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE_SCALABLE)
      {
        rpcSliceHeader=m_apcMCTFDecoder[1]->m_pcVeryFirstSliceHeader;
      }
      else
      {
        rpcSliceHeader=m_pcVeryFirstSliceHeader;
      }

			RNOK( m_pcSliceReader->readSliceHeaderVirtual(m_pcNalUnitParser->getNalUnitType(),
																										rpcSliceHeader,
																										m_uiDecompositionStages[m_uiNextLayerId],
																										m_uiMaxDecompositionStages,
																										m_uiGopSize[m_uiNextLayerId],
																										m_uiMaxGopSize,
																										*(Short*)(&m_pcNalUnitParser->m_pucBuffer[4]),
																										*(Short*)(&m_pcNalUnitParser->m_pucBuffer[6]),
																										m_pcNalUnitParser->m_pucBuffer[8],
                                                    m_uiNextLayerId,
																										pSliceHeader) );
			pSliceHeader->setTrueSlice( false);
		}
		else
		{
				RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser,
																						    pSliceHeader ));
				pSliceHeader->setTrueSlice( true);
		}
//TMM_EC {{

    SliceHeader* pSH = NULL;
   // if(m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE_SCALABLE && m_uiNextLayerId==1 && !m_bBaseLayerIsAVCCompatible && (m_apcMCTFDecoder[0]->m_bBaseLayerLost || m_apcMCTFDecoder[1]->m_bBaseLayerLost))
   if(m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE_SCALABLE && m_uiNextLayerId==1 && !m_bBaseLayerIsAVCCompatible)
    {
      pSH=m_apcMCTFDecoder[0]->m_pcVeryFirstSliceHeader;
    }
    else
    {
      pSH = m_pcSliceHeader;
    }
//TMM_EC }}

    {
      if(bPreParseHeader) //FRAG_FIX
      {
// JVT-Q054 Red. Picture {
        //if ( isRedundantPic() )
        //{
        //  delete m_pcSliceHeader;
        //  m_pcSliceHeader = pSliceHeader;
        //}
        //else
        //{
				//JVT-W049 {       
		     if(pSliceHeader->getRedundantPicCnt())
         { 
					 if(pSliceHeader->getNalRefIdc()==NAL_REF_IDC_PRIORITY_HIGHEST)
					 {
							Bool bNotNeedRedu = m_uiNextFrameNumRedu == pSliceHeader->getFrameNum() 
								&& m_uiNextPocLsbRedu ==  pSliceHeader->getPicOrderCntLsb()
								&& m_uiNextLayerIdRedu == pSliceHeader->getLayerId();
							bKeyPicReduUseAvc = !bNotNeedRedu && m_uiNextLayerIdRedu == 0;
							bKeyPicReduUseSvc[pSliceHeader->getLayerId()] = !bNotNeedRedu && m_uiNextLayerIdRedu == pSliceHeader->getLayerId() && m_uiNextLayerIdRedu != 0;
							if(bKeyPicReduUseAvc==true||bKeyPicReduUseSvc[pSliceHeader->getLayerId()])
							{
								if(bKeyPicReduUseAvc==true||bKeyPicReduUseSvc[1])
                  m_pcPrevSliceHeader = m_pcSliceHeader;
								m_pcSliceHeader = pSliceHeader;
							}
							else
							{
								m_bRedundantPic=true;
                //pSliceHeader->setRedundantPicCnt(0);
                //if( m_pcSliceHeader )
                //m_pcPrevSliceHeader = m_pcSliceHeader;
							  m_pcSliceHeader = pSliceHeader;
                //delete pSliceHeader;
							}
					 }
					 else
					 {
							m_bRedundantPic=true;
						  //delete m_pcSliceHeader;
              pSliceHeader->setRedundantPicCnt(0);
							m_pcSliceHeader = pSliceHeader;
					 }
					 
         }

        else
        {
				  if(pSliceHeader->getNalRefIdc()==NAL_REF_IDC_PRIORITY_HIGHEST)
				  {
					  m_uiNextLayerIdRedu  =   pSliceHeader->getLayerId();
					  m_uiNextFrameNumRedu  =  pSliceHeader->getFrameNum();
					  m_uiNextPocLsbRedu    =  pSliceHeader->getPicOrderCntLsb();
				  } 

		
         //JVT-W049 }
           //EIDR bug-fix
		      if(m_pcSliceHeader)
            {
              //delete m_pcPrevSliceHeader; //TMM_INTERLACE
							m_pcSliceHeader->setRedundantPicCnt(0);//JVT-W049
              m_pcPrevSliceHeader = m_pcSliceHeader;
            }
              m_pcSliceHeader     = pSliceHeader;
        }
// JVT-Q054 Red. Picture }
      } // FRAG_FIX
      else // memory leak fix provided by Nathalie
      {
        delete pSliceHeader;
        pSliceHeader = NULL;
      }
    }
  }
  //~JVT-P031

  return Err::m_nOK;
}

//JVT-T054_FIX{
ErrVal
H264AVCDecoder::getAVCFrame( IntFrame*&      pcFrame,
                             IntFrame*&      pcResidual,
                             MbDataCtrl*&    pcMbDataCtrl,
                             Int             iPoc)
{
  FrameUnit*  pcFrameUnit = m_pcFrameMng->getReconstructedFrameUnit( iPoc );
  ROF( pcFrameUnit );
  pcFrame             = m_pcFrameMng->getBaseRepFrame();
  pcResidual          = pcFrameUnit ->getResidual();
  pcMbDataCtrl        = m_pcRQFGSDecoder->getMbDataCtrl();

  return Err::m_nOK;
}
//JVT-T054}


ErrVal
H264AVCDecoder::replaceSNRCGSBaseFrame( IntFrame* pcELFrame,
                                        const PicType ePicType,          //TMM_INTERLACE
                                        const Bool    bFrameMbsOnlyFlag  //TMM_INTERLACE
                                       ) // MGS fix by Heiko Schwarz
{
  ROFRS ( m_bCGSSNRInAU, Err::m_nOK );
  RNOK  ( m_pcFrameMng->updateLastFrame( pcELFrame,ePicType,bFrameMbsOnlyFlag ) );//TMM_INTERLACE
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::xZeroIntraMacroblocks( IntFrame*    pcFrame,
                                       MbDataCtrl*  pcMbDataCtrl,
                                       SliceHeader* pcSliceHeader )
{
	SliceHeader    *  pcSliceHeaderV    = pcMbDataCtrl->getSliceHeader();//TMM_EC
  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  IntYuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

  const PicType ePicType = pcSliceHeader->getPicType();
	const Bool    bMbAff   = pcSliceHeader->isMbAff   ();
	if( ePicType!=FRAME )
	{
		RNOK( pcFrame->addFieldBuffer     ( ePicType ) );
	}
	else if( bMbAff )
	{
		RNOK( pcFrame->addFrameFieldBuffer() );
	}

	//===== loop over macroblocks =====
  const UInt uiMbNumber = pcSliceHeader->getMbInPic();
	for(UInt uiMbAddress = 0 ; uiMbAddress < uiMbNumber; uiMbAddress++ )
  {
		MbDataAccess* pcMbDataAccess = NULL;
		UInt          uiMbY, uiMbX;

		pcSliceHeader->getMbPositionFromAddress ( uiMbY, uiMbX, uiMbAddress               );

		RNOK( m_pcControlMng->initMbForFiltering( *pcMbDataAccess, uiMbY, uiMbX, bMbAff ) );
    RNOK( pcMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );


    if( pcMbDataAccess->getMbData().isIntra() )
    {
			const PicType eMbPicType = pcMbDataAccess->getMbPicType();
      pcFrame->getPic( eMbPicType )->getFullPelYuvBuffer()->loadBuffer( &cZeroMbBuffer );
    }

  }
//TMM_EC{{ bug fix
	if(!pcSliceHeaderV->getTrueSlice()){
		RNOK(pcMbDataCtrl->initSlice(*pcSliceHeaderV,POST_PROCESS,false, NULL));}
//TMM_EC}} bug fix

	if( ePicType!=FRAME )
	{
		RNOK( pcFrame->removeFieldBuffer     ( ePicType ) );
	}
	else if( bMbAff )
	{
		RNOK( pcFrame->removeFrameFieldBuffer()           );
	}

  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::xProcessSlice( SliceHeader& rcSH,
                               SliceHeader* pcPrevSH,
                               PicBuffer*&  rpcPicBuffer,
                               Bool         bHighestLayer) //JVT-T054
{
  set4Tap( &rcSH );  // V090

  UInt  uiMbRead;
  Bool  bNewFrame = false;
  Bool  bNewPic   = false;
  Bool  bVeryFirstSlice=false;

  //===== calculate Poc =====
  if( rcSH.getLayerId() == 0 )
  {
    RNOK( m_pcPocCalculator->calculatePoc( rcSH ) );
  }

  //===== store prediction weights table for inter-layer prediction =====
  m_acLastPredWeightTable[LIST_0].copy( rcSH.getPredWeightTable( LIST_0 ) );
  m_acLastPredWeightTable[LIST_1].copy( rcSH.getPredWeightTable( LIST_1 ) );

  //===== set some basic parameters =====
  if( NULL == m_pcVeryFirstSliceHeader )
  {
     m_pcVeryFirstSliceHeader = new SliceHeader( rcSH );
    //--ICU/ETRI FMO Implementation
    m_pcVeryFirstSliceHeader->setSliceGroupChangeCycle( rcSH.getSliceGroupChangeCycle() ); // fix HS
    m_pcVeryFirstSliceHeader->FMOInit();
    bVeryFirstSlice= true;

		// ICU/ETRI FGS_MOT_USE
    m_pcBaseLayerCtrlEL = new MbDataCtrl();
    m_pcBaseLayerCtrlEL->init(rcSH.getSPS());
  }
	else if ( m_bNewSPS && (rcSH.getSPS().getProfileIdc() != SCALABLE_PROFILE) )
	{
		delete m_pcVeryFirstSliceHeader;
		m_pcVeryFirstSliceHeader  = new SliceHeader( rcSH );
    m_pcVeryFirstSliceHeader->setSliceGroupChangeCycle( rcSH.getSliceGroupChangeCycle() ); // fix HS
    m_pcVeryFirstSliceHeader->FMOInit();  // fix HS
	}
	m_bNewSPS = false;


  //===== check if new pictures and initialization =====
	rcSH.compare( pcPrevSH, bNewPic, bNewFrame );

	if( rcSH.getBottomFieldFlag())
	{
 	  bNewFrame = false;
	  bNewPic = true;
	}

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
  rcSH.setNumMbsInSlice( uiMbRead );

  //===== decode slice =====
  Bool  bUseBaseRep     = rcSH.getUseBasePredictionFlag();
  Bool  bConstrainedIP  = rcSH.getPPS().getConstrainedIntraPredFlag();
  Bool  bReconstruct    = (m_uiRecLayerId == 0) || ! bConstrainedIP || bHighestLayer; //JVT-T054
  m_bReconstruct  = bReconstruct;
  //***** NOTE: Motion-compensated prediction for non-key pictures is done in xReconstructLastFGS()
  bReconstruct    = bReconstruct && bUseBaseRep || ! bConstrainedIP;
	//JVT-X046 {
	UInt uiMbAddress = rcSH.getFirstMbInSlice();
	for ( UInt uiMbNum = uiMbRead; uiMbNum ; uiMbNum -- )
	{
		m_bMbStatus[rcSH.getLayerId()][uiMbAddress] = true;
		uiMbAddress = rcSH.getFMO() ->getNextMBNr(uiMbAddress);
	}
	m_uiDecodedMbNum += uiMbRead;
	m_uiDecodedMbNum = m_uiDecodedMbNum % m_uiMbNumInFrame[rcSH.getLayerId()];
	m_uiNextFirstMb = m_uiDecodedMbNum % m_uiMbNumInFrame[rcSH.getLayerId()];//should be modified later
	//JVT-X046 }
  RNOK( m_pcControlMng  ->initSlice ( rcSH, DECODE_PROCESS ) );
  RNOK( m_pcSliceDecoder->process   ( rcSH, bReconstruct, uiMbRead ) );

  const PicType ePicType = rcSH.getPicType();

  Bool bPicDone;
  RNOK( m_pcControlMng->finishSlice( rcSH, bPicDone, m_bFrameDone ) );

  if( bPicDone )
  {
    // copy intra and inter prediction signal
		RNOK( m_pcFrameMng->getPredictionIntFrame()->copy( rcSH.getFrameUnit()->getFGSIntFrame(), ePicType                              ) );
    // delete intra prediction
    RNOK( m_pcControlMng->initSlice( rcSH, POST_PROCESS));
    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getFGSIntFrame(), rcSH.getFrameUnit()->getMbDataCtrl(), &rcSH ) );
    // add residual and intra signal
		rcSH.getFrameUnit()->getFGSIntFrame()->add ( rcSH.getFrameUnit()->getResidual   (), ePicType                                      );
    rcSH.getFrameUnit()->getFGSIntFrame()->clip();

    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getResidual(),
                                 rcSH.getFrameUnit()->getMbDataCtrl(), m_pcVeryFirstSliceHeader ) );

    //===== deblocking of base representation =====
		//JVT-X046 {
		rcSH.FMOInit();
		//reset FirstMbInSlice and LastMbInSlice
		rcSH.setFirstMbInSlice(0);
		rcSH.setLastMbInSlice(m_uiMbNumInFrame[rcSH.getLayerId()]-1);		
		//RNOK( m_pcLoopFilter->process( rcSH ) );
    RNOK( m_pcLoopFilter->process( rcSH, NULL, false, m_bMbStatus[rcSH.getLayerId()]) );
		//JVT-X046 }
    RNOK( m_pcFrameMng->storePicture( rcSH ) );

    //===== init FGS decoder =====
		//RNOK( m_pcRQFGSDecoder->initPicture( &rcSH, rcSH.getFrameUnit()->getMbDataCtrl()) );
    RNOK( m_pcRQFGSDecoder->initPicture( &rcSH, rcSH.getFrameUnit()->getMbDataCtrl(), m_bMbStatus[rcSH.getLayerId()] ) );//JVT-X046
   }

	  printf("  %s %4d ( LId 0, TL X, QL 0, AVC-%c, BId 1, AP 0, QP%3d )\n",
		(ePicType==FRAME) ?  "Frame" : ( (ePicType==TOP_FIELD) ? "TopFd" : "BotFd"),
		rcSH.getPoc                (),
		rcSH.getSliceType              () == B_SLICE ? 'B' :
	  rcSH.getSliceType              () == P_SLICE ? 'P' : 'I',
		rcSH.getPicQp                  () );

  if( m_bFrameDone )
  {
    DTRACE_NEWFRAME;
  }

  return Err::m_nOK;
}


//TMM_EC {{
//JVT-X046 {
ErrVal
H264AVCDecoder::xDecodeBaseLayerVirtual( SliceHeader&    rcSH,
                                      SliceHeader*    pcPrevSH,
                                      PicBuffer* &    rpcPicBuffer)
{
  set4Tap( &rcSH );  // V090

  UInt  uiMbRead;
  Bool  bNewFrame = false;
  Bool  bNewPic   = false; 

   //===== calculate Poc =====
  if( rcSH.getLayerId() == 0 )
  {
    RNOK( m_pcPocCalculator->calculatePoc( rcSH ) );
  }

  //===== store prediction weights table for inter-layer prediction =====
  m_acLastPredWeightTable[LIST_0].copy( rcSH.getPredWeightTable( LIST_0 ) );
  m_acLastPredWeightTable[LIST_1].copy( rcSH.getPredWeightTable( LIST_1 ) );

  m_uiNumOfNALInAU++;  //TMM_EC
 
  //===== set some basic parameters =====
  if( NULL == m_pcVeryFirstSliceHeader )
  {
    m_pcVeryFirstSliceHeader  = new SliceHeader( rcSH);
  }

 
  //===== check if new pictures and initialization =====
  rcSH.compare( pcPrevSH, bNewPic, bNewFrame );
	
	if( rcSH.getBottomFieldFlag())
	{
 	  bNewFrame = false;
	  bNewPic = true;
	}

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
  uiMbRead	=	rcSH.getMbInPic();

  //===== decode slice =====
//   Bool  bUseBaseRep     = rcSH.getUseBasePredictionFlag();
  Bool  bUseBaseRep     = 1;
  Bool  bConstrainedIP  = rcSH.getPPS().getConstrainedIntraPredFlag();

  Bool  bReconstruct    = (m_uiRecLayerId == 0) || ! bConstrainedIP;

  m_bReconstruct = bReconstruct;
  //***** NOTE: Motion-compensated prediction for non-key pictures is done in xReconstructLastFGS()
  bReconstruct   = bReconstruct && bUseBaseRep || ! bConstrainedIP;

  RNOK( m_pcControlMng  ->initSlice ( rcSH, DECODE_PROCESS ) );

  const PicType ePicType = rcSH.getPicType();
	
	Frame	*frame	=	(Frame*)(rcSH.getRefPicList( ePicType, LIST_0 ).get(0).getFrame());

	//slice copy
	//m_pcFrameMng->getCurrentFrameUnit()->getPic(ePicType)->getFullPelYuvBuffer()->loadBuffer( frame->getFullPelYuvBuffer());
	m_pcFrameMng->getCurrentFrameUnit()->getPic(ePicType)->getFullPelYuvBuffer()->loadSliceBuffer( frame->getFullPelYuvBuffer(), m_uiNextFirstMb, m_uiNextFirstMb+m_uiLostMbNum );
	rcSH.getFrameUnit()->getFGSIntFrame()->copySlice( m_pcFrameMng->getCurrentFrameUnit()->getPic(ePicType), ePicType, m_uiNextFirstMb, m_uiNextFirstMb+m_uiLostMbNum);

	//set Mb status
	for ( UInt uiMbAddress = m_uiNextFirstMb; uiMbAddress < m_uiNextFirstMb+m_uiLostMbNum; uiMbAddress++ )
	{
		m_bMbStatus[rcSH.getLayerId()][uiMbAddress] = false;//should be modified later
	}

	m_pcControlMng->getMbDataCtrl()->updateMBProcessed( m_uiLostMbNum );
	//reset NextFirstMb and LostMbNum
	m_uiDecodedMbNum += m_uiLostMbNum;
	m_uiNextFirstMb += m_uiLostMbNum;
	m_uiLostMbNum = 0;
	m_uiNextFirstMb = m_uiNextFirstMb % m_uiMbNumInFrame[rcSH.getLayerId()];	
    
  Bool bPicDone;
  RNOK( m_pcControlMng->finishSlice( rcSH, bPicDone, m_bFrameDone ) );

// TMM_EC
  if( bPicDone )
  {
    // copy intra and inter prediction signal
    m_pcFrameMng->getPredictionIntFrame()->copy( rcSH.getFrameUnit()->getFGSIntFrame(),ePicType);
    // delete intra prediction
    RNOK( m_pcControlMng->initSlice( rcSH, POST_PROCESS));
    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getFGSIntFrame(), rcSH.getFrameUnit()->getMbDataCtrl(), &rcSH ) );
    // add residual and intra signal
    rcSH.getFrameUnit()->getFGSIntFrame()->add( rcSH.getFrameUnit()->getResidual(),ePicType );
    rcSH.getFrameUnit()->getFGSIntFrame()->clip();

    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getResidual(),
                                 rcSH.getFrameUnit()->getMbDataCtrl(), m_pcVeryFirstSliceHeader ) );

    //===== deblocking of base representation =====
		rcSH.FMOInit();
		//reset FirstMbInSlice and LastMbInSlice
		rcSH.setFirstMbInSlice(0);
		rcSH.setLastMbInSlice(m_uiMbNumInFrame[rcSH.getLayerId()]-1);

		RNOK( m_pcLoopFilter->process( rcSH, NULL, false, m_bMbStatus[rcSH.getLayerId()]) );
  
    RNOK( m_pcFrameMng->storePicture( rcSH ) );
    //===== init FGS decoder =====
    RNOK( m_pcRQFGSDecoder->initPicture( &rcSH, rcSH.getFrameUnit()->getMbDataCtrl(), m_bMbStatus[rcSH.getLayerId()] ) );
  }
	
	printf("  Frame %4d ( LId 0, TL X, QL 0, AVC-%c, BId-1, AP 0, QP%3d, V )\n",
    rcSH.getPoc                    (),
    rcSH.getSliceType              () == B_SLICE ? 'B' : 
    rcSH.getSliceType              () == P_SLICE ? 'P' : 'I',
    rcSH.getPicQp                  () );


  if( m_bFrameDone )
  {
    DTRACE_NEWFRAME;
  }

  return Err::m_nOK;
}
//JVT-X046 }
ErrVal
H264AVCDecoder::xProcessSliceVirtual( SliceHeader&    rcSH,
                                      SliceHeader*    pcPrevSH,
                                      PicBuffer* &    rpcPicBuffer)
{
  set4Tap( &rcSH );  // V090

  UInt  uiMbRead;
  Bool  bNewFrame = false;
  Bool  bNewPic   = false;

   //===== calculate Poc =====
  if( rcSH.getLayerId() == 0 )
  {
    RNOK( m_pcPocCalculator->calculatePoc( rcSH ) );
  }

  //===== store prediction weights table for inter-layer prediction =====
  m_acLastPredWeightTable[LIST_0].copy( rcSH.getPredWeightTable( LIST_0 ) );
  m_acLastPredWeightTable[LIST_1].copy( rcSH.getPredWeightTable( LIST_1 ) );

  m_uiNumOfNALInAU++;  //TMM_EC

  //===== set some basic parameters =====
  if( NULL == m_pcVeryFirstSliceHeader )
  {
    m_pcVeryFirstSliceHeader  = new SliceHeader( rcSH);
  }




  //===== check if new pictures and initialization =====
  rcSH.compare( pcPrevSH, bNewPic, bNewFrame );

 if( rcSH.getBottomFieldFlag())
	{
 	  bNewFrame = false;
	  bNewPic = true;
	}

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
  uiMbRead	=	rcSH.getMbInPic();

  //===== decode slice =====
//   Bool  bUseBaseRep     = rcSH.getUseBasePredictionFlag();
  Bool  bUseBaseRep     = 1;
  Bool  bConstrainedIP  = rcSH.getPPS().getConstrainedIntraPredFlag();

  Bool  bReconstruct    = (m_uiRecLayerId == 0) || ! bConstrainedIP;

  m_bReconstruct = bReconstruct;
  //***** NOTE: Motion-compensated prediction for non-key pictures is done in xReconstructLastFGS()
  bReconstruct   = bReconstruct && bUseBaseRep || ! bConstrainedIP;

  RNOK( m_pcControlMng  ->initSlice ( rcSH, DECODE_PROCESS ) );

  const PicType ePicType = rcSH.getPicType();
	if ( m_eErrorConceal == EC_BLSKIP || m_eErrorConceal == EC_TEMPORAL_DIRECT)

	{
		RNOK( m_pcSliceDecoder->processVirtual( rcSH, bReconstruct, uiMbRead ) );
	}
	else
	{
		Frame	*frame	=	(Frame*)(rcSH.getRefPicList( ePicType, LIST_0 ).get(0).getFrame());
//		m_pcFrameMng->getCurrentFrameUnit()->getFrame().getFullPelYuvBuffer()->loadBuffer( frame->getFullPelYuvBuffer());
		  m_pcFrameMng->getCurrentFrameUnit()->getPic(ePicType)->getFullPelYuvBuffer()->loadBuffer( frame->getFullPelYuvBuffer());
	}


  Bool bPicDone;
  RNOK( m_pcControlMng->finishSlice( rcSH, bPicDone, m_bFrameDone ) );

  bPicDone	=	true;
  m_bFrameDone	=	true;
// TMM_EC
  if( bPicDone )
  {
		if (  m_eErrorConceal == EC_FRAME_COPY)
		{
			rcSH.getFrameUnit()->getFGSIntFrame()->copy( m_pcFrameMng->getCurrentFrameUnit()->getPic(ePicType),ePicType);
		}
    // copy intra and inter prediction signal
    m_pcFrameMng->getPredictionIntFrame()->copy( rcSH.getFrameUnit()->getFGSIntFrame(),ePicType);
    // delete intra prediction
    RNOK( m_pcControlMng->initSlice( rcSH, POST_PROCESS));
    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getFGSIntFrame(), rcSH.getFrameUnit()->getMbDataCtrl(), &rcSH ) );
    // add residual and intra signal
    rcSH.getFrameUnit()->getFGSIntFrame()->add( rcSH.getFrameUnit()->getResidual(),ePicType );
    rcSH.getFrameUnit()->getFGSIntFrame()->clip();

    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getResidual(),
                                 rcSH.getFrameUnit()->getMbDataCtrl(), m_pcVeryFirstSliceHeader ) );

    //===== deblocking of base representation =====


    RNOK( m_pcFrameMng->storePicture( rcSH ) );
    //===== init FGS decoder =====
//    RNOK( m_pcRQFGSDecoder->initPicture( &rcSH, rcSH.getFrameUnit()->getMbDataCtrl() ) );
  }

 printf("  Frame %4d ( LId 0, TL X, QL 0, AVC-%c, BId-1, AP 0, QP%3d, V )\n",
    rcSH.getPoc                    (),
    rcSH.getSliceType              () == B_SLICE ? 'B' :
    rcSH.getSliceType              () == P_SLICE ? 'P' : 'I',
    rcSH.getPicQp                  () );


  if( m_bFrameDone )
  {
    DTRACE_NEWFRAME;
  }

  return Err::m_nOK;
}
//TMM_EC }}




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
    if( !( m_bBaseLayerAvcCompliant && (m_pcRQFGSDecoder->getSliceHeader() &&
      m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel() == 0 &&
      m_pcRQFGSDecoder->getSliceHeader()->getLayerId() == 0 )) || (pcSliceHeader && pcSliceHeader->getLayerId() != 0) ) //JVT-T054


      RNOK( m_apcMCTFDecoder[uiLayer]->initSlice( pcSliceHeader, uiLastLayer, m_bLastNalInAU, m_bCGSSNRInAU ) );
  }
  ROFRS( m_bActive, Err::m_nOK );

  //===== calculate POC =====
  if( pcSliceHeader && pcSliceHeader->getLayerId() == 0 )
  {
    RNOK( m_pcPocCalculator->calculatePoc( *pcSliceHeader ) );
  }

  //===== check if an FGS enhancement needs to be reconstructed =====
  if( m_pcRQFGSDecoder->isInitialized() &&
      m_pcRQFGSDecoder->getSliceHeader()->getLayerId() == 0 )
  {
    Bool bHighestLayer = ( m_bLastNalInAU ); //JVT-T054

    if (NULL == pcSliceHeader || pcSliceHeader->getQualityLevel  () == m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1 )
    {
      if(bHighestLayer && m_bCGSSNRInAU)
      {
        RNOK( m_apcMCTFDecoder[0]->ReconstructLastFGS(bHighestLayer, true ) );
        m_apcMCTFDecoder[0]->getLastDPBUnit()->setMbDataCtrl(0);
      }
      else
      {
        RNOK( xReconstructLastFGS(bHighestLayer/*,pcSliceHeader*/) );
      }
    }
    else
    {
      if(pcSliceHeader->getQualityLevel  () != m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () ||
          0 == pcSliceHeader->getQualityLevel  ())
      {
        RNOK( xReconstructLastFGS(bHighestLayer/*,pcSliceHeader*/) );
      }
  	}
  }

  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::xReconstructLastFGS(Bool bHighestLayer) //JVT-T054
{
   set4Tap( m_pcRQFGSDecoder->getSliceHeader() );  // V090

  MbDataCtrl*   pcMbDataCtrl        = m_pcRQFGSDecoder->getMbDataCtrl   ();
  SliceHeader*  pcSliceHeader       = m_pcRQFGSDecoder->getSliceHeader  ();
  ROF( pcSliceHeader );
  const PicType ePicType         = pcSliceHeader->getPicType                    ();
  IntFrame* pcResidual           = pcSliceHeader->getFrameUnit()->getResidual   ();
  IntFrame* pcRecFrame           = pcSliceHeader->getFrameUnit()->getFGSIntFrame();
  IntFrame*     pcILPredFrame       = m_pcFrameMng    ->getRefinementIntFrame();
  IntFrame*     pcILPredFrameSpatial= m_pcFrameMng    ->getRefinementIntFrame2();
  IntFrame*     pcBaseRepFrame      = m_pcFrameMng    ->getBaseRepFrame();
  Bool          bUseBaseRep         = pcSliceHeader   ->getUseBasePredictionFlag(); // HS: fix by Nokia
  Bool          bConstrainedIP      = pcSliceHeader   ->getPPS().getConstrainedIntraPredFlag();

  //===== reconstruct FGS =====
  if( !bUseBaseRep && bConstrainedIP )
  {
		RNOK( pcRecFrame->removeFrameFieldBuffer() );

    //RNOK( m_pcRQFGSDecoder            ->reconstruct( pcRecFrame, true                                ) );
		RNOK( m_pcRQFGSDecoder            ->reconstruct( pcRecFrame, true ,m_bMbStatus[pcSliceHeader->getLayerId()]) );//JVT-X046
    RNOK( pcResidual                  ->copy       ( pcRecFrame,                            ePicType ) );
    RNOK( xZeroIntraMacroblocks           ( pcResidual, pcMbDataCtrl, pcSliceHeader ) );

    if( m_bReconstruct && !pcSliceHeader->isIntra() )
    {
      //----- "normal" motion-compensated prediction -----
      //RNOK( m_pcSliceDecoder->compensatePrediction( *pcSliceHeader ) );
			RNOK( m_pcSliceDecoder->compensatePrediction( *pcSliceHeader,m_bMbStatus[pcSliceHeader->getLayerId()] ) );//JVT-X046
    }
		//JVT-X046 {
		for ( UInt uiMBY = 0; uiMBY < m_uiFrameHeightInMb; uiMBY++ )
			for ( UInt uiMBX = 0; uiMBX < m_uiFrameWidthInMb; uiMBX++ )
			{
				if ( m_bMbStatus[pcSliceHeader->getLayerId()][uiMBY*m_uiFrameWidthInMb+uiMBX] == false )
					m_pcFrameMng->getPredictionIntFrame()->setMBZero(uiMBY,uiMBX);
			}
		//JVT-X046 }

    RNOK( pcRecFrame      ->add           ( m_pcFrameMng->getPredictionIntFrame(), ePicType ) );
    RNOK( pcRecFrame      ->clip          () );

  }
//JVT-S036 lsj start

  if( pcSliceHeader->getUseBasePredictionFlag() )  //bug-fix suffix shenqiu
	{
		if( pcSliceHeader->getAdaptiveRefPicMarkingFlag() )
		{
			RNOK( m_pcFrameMng->xMMCOUpdateBase(pcSliceHeader) );
		}
		else
		{
			RNOK( m_pcFrameMng->xSlidingWindowUpdateBase( pcSliceHeader->getFrameNum() ) );
		}
	}
//JVT-S036 lsj end

  RNOK  ( m_pcRQFGSDecoder->finishPicture () );

  //===== store intra signal for inter-layer prediction =====
  if( m_bEnhancementLayer )
  {
    RNOK( pcILPredFrame->copy( pcRecFrame, ePicType ) );
  }
//TMM_EC {{
	//JVT-X046 {
  /*if ( pcSliceHeader->getTrueSlice())
  {
  //===== loop filter =====
  if( (m_uiRecLayerId == 0) || bUseBaseRep || bHighestLayer ) // HS: fix by Nokia //JVT-T054
  {
		RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcRecFrame ) );
  }
  }*/
	if( (m_uiRecLayerId == 0) || bUseBaseRep || bHighestLayer ) // HS: fix by Nokia //JVT-T054
  {
		RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcRecFrame, false, m_bMbStatus[pcSliceHeader->getLayerId()] ) );
  }
	//JVT-X046
//TMM_EC }}
  //===== store in FGS pic buffer =====
  if( !bUseBaseRep && bConstrainedIP )
  {
    RNOK( m_pcFrameMng->storeFGSPicture( pcSliceHeader->getFrameUnit()->getPicBuffer() ) );
  }

  m_pcFrameMng->RefreshOrderedPocList();  //JVT-S036 lsj



  //===== loop-filter for spatial scalable coding =====
  if( m_bEnhancementLayer )
  {
    RNOK( pcILPredFrameSpatial->copy( pcILPredFrame, ePicType ) );
    RNOK( pcBaseRepFrame      ->copy( pcILPredFrame, ePicType ) );
//TMM_EC {{
   
      if( pcSliceHeader->getFrameUnit()->getContrainedIntraPred() )
      {
        m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );
       
				RNOK( m_pcLoopFilter->process( *m_pcVeryFirstSliceHeader,
                                      pcILPredFrameSpatial,
                                      pcMbDataCtrl,
                                      pcMbDataCtrl,
                                      m_pcVeryFirstSliceHeader->getSPS().getFrameWidthInMbs(),
                                      NULL,
                                      NULL,
																			false,
                                      false,
																			m_bMbStatus[m_pcVeryFirstSliceHeader->getLayerId()]) );//JVT-X046

        m_pcLoopFilter->setFilterMode();
        //  bug fix TMM_EC
        RNOK(pcMbDataCtrl->initSlice(*pcSliceHeader, POST_PROCESS, false, NULL));
        // end bug fix TMM_EC
      }
      else
      {
       RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcILPredFrameSpatial, false, m_bMbStatus[pcSliceHeader->getLayerId()] ) );//JVT-X046
      }

      if( m_bCGSSNRInAU )
      {
        pcBaseRepFrame->setPoc( *pcSliceHeader );
       	RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcBaseRepFrame, false, m_bMbStatus[pcSliceHeader->getLayerId()] ) );//JVT-X046
      }
    //}//JVT-X046
//  TMM_EC }}
  }

  return Err::m_nOK;
}


// JVT-Q054 Red. Picture {
ErrVal
H264AVCDecoder::checkRedundantPic()
{
  m_bRedundantPic = false;
  // TMM_INTERLACE {
  if( m_pcSliceHeader && m_pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
 // TMM_INTERLACE }
  {
    if ( m_bFirstSliceHeaderBackup && (NULL != m_pcSliceHeader) )
    {
      ROF( ( m_pcSliceHeader_backup = new SliceHeader ( m_pcSliceHeader->getSPS(), m_pcSliceHeader->getPPS()) ) );
      m_bFirstSliceHeaderBackup = false;
    }
    else
    {
      if ( ( NULL != m_pcSliceHeader ) && ( NULL != m_pcSliceHeader_backup ) )
      {
        const NalUnitType eNalUnitType  = m_pcNalUnitParser->getNalUnitType();
        if ( ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ) || ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
          || ( eNalUnitType == NAL_UNIT_CODED_SLICE ) || ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR ) )
        {
          Bool  bNewFrame  = true;
          RNOK( m_pcSliceHeader->compareRedPic ( m_pcSliceHeader_backup, bNewFrame ) );
          if (!bNewFrame)
          {
            m_bRedundantPic = true;
          }
        }
      }
    }
  } // TMM_INTERLACE
  return Err::m_nOK;
}


// JVT-Q054 Red. Picture }

// JVT-V035
#ifdef SHARP_AVC_REWRITE_OUTPUT
ErrVal
H264AVCDecoder::xStartAvcRewrite(UChar*& i_avcRewriteBinDataBuffer, BinData*& i_avcRewriteBinData, ExtBinDataAccessor* i_avcRewriteBinDataAccessor)
{
	// create and initialize the avc rewrite encoder
	RNOK( H264AVCEncoder              ::create( m_pcAvcRewriteEncoder ) );

	m_pcAvcRewriteEncoder->xCreateAvcRewriteEncoder();
	m_pcAvcRewriteEncoder->xInitAvcRewriteEncoder();

	// set the bin data buffer
	ROT( NULL == i_avcRewriteBinData );
	ROT( NULL == m_pcSliceHeader);

	m_avcRewriteBindata = i_avcRewriteBinData;
	m_avcRewriteBinDataAccessor = i_avcRewriteBinDataAccessor;

	m_avcRewriteBufsize = 3 * m_pcSliceHeader->getSPS().getFrameWidthInMbs() * m_pcSliceHeader->getSPS().getFrameHeightInMbs() * 16 * 16; // as initialized in MCTFEncoder::XCreateData
	ROFS(i_avcRewriteBinDataBuffer = new UChar[m_avcRewriteBufsize]);
	m_avcRewriteBinDataBuffer = i_avcRewriteBinDataBuffer;

	m_avcRewriteBindata->reset();
	m_avcRewriteBindata->set(i_avcRewriteBinDataBuffer, m_avcRewriteBufsize);
	m_avcRewriteBindata->setMemAccessor( *m_avcRewriteBinDataAccessor );

	return Err::m_nOK;
}

ErrVal
H264AVCDecoder::xInitSliceForAvcRewriteCoding(const SliceHeader& rcSH) {
	m_pcSliceDecoder->xSetAvcRewriteEncoder(m_pcAvcRewriteEncoder);
	m_pcAvcRewriteEncoder->xInitSliceForAvcRewriteCoding(rcSH);
	return Err::m_nOK;
}

ErrVal
H264AVCDecoder::xCloseAvcRewriteEncoder() {
	ROT( NULL == m_pcAvcRewriteEncoder );

	m_avcRewriteBindata->reset();

	if (m_avcRewriteBinDataBuffer!= NULL)
		delete [] m_avcRewriteBinDataBuffer;

	m_pcAvcRewriteEncoder->xCloseAvcRewriteEncoder();
	return Err::m_nOK;
}

Bool
H264AVCDecoder::xGetAvcRewriteFlag() {
	return m_avcRewriteFlag;
}

bool
H264AVCDecoder::xWriteAvcRewriteParameterSets(NalUnitType nal_unit_type, UInt index) {
	int size = 0;
	m_avcRewriteBindata->reset();
	m_avcRewriteBindata->set(m_avcRewriteBinDataBuffer, m_avcRewriteBufsize);
	m_avcRewriteBindata->setMemAccessor( *m_avcRewriteBinDataAccessor);

	/*if (nal_unit_type == NAL_UNIT_SPS)
	{
		RNOK( m_pcAvcRewriteEncoder->xAvcRewriteParameterSets( m_avcRewriteBinDataAccessor, *m_pcSliceHeader, NAL_UNIT_SPS) );
	}
	else if (nal_unit_type == NAL_UNIT_PPS)
		RNOK( m_pcAvcRewriteEncoder->xAvcRewriteParameterSets( m_avcRewriteBinDataAccessor, *m_pcSliceHeader, NAL_UNIT_PPS) );

	size = m_avcRewriteBinDataAccessor->size();
	m_avcRewriteBindata->set(m_avcRewriteBinDataBuffer, size);*/

	//if (nal_unit_type == NAL_UNIT_SPS)
	if (nal_unit_type == NAL_UNIT_SPS || nal_unit_type == NAL_UNIT_SUB_SPS )//SSPS
	{
		SequenceParameterSet *rcSps;
		if (m_pcParameterSetMng->isValidSPS(index)) {
			m_pcParameterSetMng->get(rcSps, index);
			m_pcAvcRewriteEncoder->xAvcRewriteParameterSets(m_avcRewriteBinDataAccessor, *rcSps);

			size = m_avcRewriteBinDataAccessor->size();
			m_avcRewriteBindata->set(m_avcRewriteBinDataBuffer, size);

			return m_pcParameterSetMng->isValidSPS(index+1);
		}
	}

	else if (nal_unit_type == NAL_UNIT_PPS) {
		PictureParameterSet *rcPps;

		if (m_pcParameterSetMng->isValidPPS(index)) {
			m_pcParameterSetMng->get(rcPps, index);
			m_pcAvcRewriteEncoder->xAvcRewriteParameterSets(m_avcRewriteBinDataAccessor, *rcPps);

			size = m_avcRewriteBinDataAccessor->size();
			m_avcRewriteBindata->set(m_avcRewriteBinDataBuffer, size);

			return m_pcParameterSetMng->isValidPPS(index+1);
		}
	}

	return false;
}
#endif

H264AVC_NAMESPACE_END

