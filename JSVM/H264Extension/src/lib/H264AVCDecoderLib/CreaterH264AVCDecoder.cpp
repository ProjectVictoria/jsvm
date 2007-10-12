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
#include "GOPDecoder.h"
#include "ControlMngH264AVCDecoder.h"
#include "SliceReader.h"
#include "SliceDecoder.h"
#include "UvlcReader.h"
#include "MbParser.h"
#include "MbDecoder.h"
#include "NalUnitParser.h"
#include "BitReadBuffer.h"
#include "CabacReader.h"
#include "CabaDecoder.h"
#include "FGSSubbandDecoder.h"

#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/IntraPrediction.h"
#include "H264AVCCommonLib/MotionCompensation.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/SampleWeighting.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/PocCalculator.h"

#include "CreaterH264AVCDecoder.h"

#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"



H264AVC_NAMESPACE_BEGIN


CreaterH264AVCDecoder::CreaterH264AVCDecoder():
  m_pcH264AVCDecoder      ( NULL ),
  m_pcRQFGSDecoder        ( NULL ),
  m_pcFrameMng            ( NULL ),
  m_pcParameterSetMng     ( NULL ),
  m_pcSliceReader         ( NULL ),
  m_pcNalUnitParser       ( NULL ),
  m_pcSliceDecoder        ( NULL ),
  m_pcControlMng          ( NULL ),
  m_pcBitReadBuffer       ( NULL ),
  m_pcUvlcReader          ( NULL ),
  m_pcMbParser            ( NULL ),
  m_pcLoopFilter          ( NULL ),
  m_pcMbDecoder           ( NULL ),
  m_pcTransform           ( NULL ),
  m_pcIntraPrediction     ( NULL ),
  m_pcMotionCompensation  ( NULL ),
  m_pcQuarterPelFilter    ( NULL ),
  m_pcCabacReader         ( NULL ),
  m_pcSampleWeighting     ( NULL )
  , m_bBaseSVCActive        ( false ) //JVT-T054_FIX
{
  ::memset( m_apcDecodedPicBuffer,     0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcMCTFDecoder,          0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcPocCalculator,        0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcYuvFullPelBufferCtrl, 0x00, MAX_LAYERS * sizeof( Void* ) );
	m_bDiscard = false;//JVT-X046
}




CreaterH264AVCDecoder::~CreaterH264AVCDecoder()
{
}


ErrVal
CreaterH264AVCDecoder::process( PicBuffer*     pcPicBuffer,
                             PicBufferList& rcPicBufferOutputList,
                             PicBufferList& rcPicBufferUnusedList,
                             PicBufferList& rcPicBufferReleaseList )
{
  RNOK(m_pcH264AVCDecoder->process( pcPicBuffer,
                                   rcPicBufferOutputList,
                                   rcPicBufferUnusedList,
                                   rcPicBufferReleaseList )); //JVT-T054_FIX
  //JVT-T054_FIX{
  m_bBaseSVCActive = m_pcH264AVCDecoder->getBaseSVCActive();
  return Err::m_nOK;
    //JVT-T054}
}


ErrVal
CreaterH264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor,
                                  UInt&             ruiNalUnitType,
                                  UInt&             uiMbX,
                                  UInt&             uiMbY,
                                  UInt&             uiSize
                          //,UInt&        uiNonRequiredPic //NonRequired JVT-Q066
                                                  //JVT-P031
                                  ,Bool             bPreParseHeader //FRAG_FIX
                                  , Bool      bConcatenated //FRAG_FIX_3
                                  ,Bool&            rbStartDecoding,
                                UInt&             ruiStartPos,
                                UInt&             ruiEndPos,
                                Bool&              bDiscardable
                                //~JVT-P031
                                , UInt*           puiNumFragments
                                , UChar**         ppucFragBuffers
                                )
{
  return m_pcH264AVCDecoder->initPacket( pcBinDataAccessor,
                                          ruiNalUnitType,
                                          uiMbX,
                                          uiMbY,
                                          uiSize,
                                          //JVT-P031
                                          bPreParseHeader, //FRAG_FIX
                                          bConcatenated, //FRAG_FIX_3
                                          rbStartDecoding,
                                          ruiStartPos,
                                          ruiEndPos,
                                          bDiscardable
                                          //~JVT-P031
                                          , puiNumFragments, ppucFragBuffers
                                         );
}

//JVT-S036 lsj start
ErrVal
CreaterH264AVCDecoder::initPacketSuffix( BinDataAccessor*  pcBinDataAccessor,
                                  UInt&             ruiNalUnitType,
                               Bool             bPreParseHeader, //FRAG_FIX
                                  Bool      bConcatenated, //FRAG_FIX_3
                          Bool&       rbStarDecoding
                            ,CreaterH264AVCDecoder* pcH264AVCDecoder
                          ,Bool&    SuffixEnable
                                      )
{
  return m_pcH264AVCDecoder->initPacketSuffix( pcBinDataAccessor,
                                          ruiNalUnitType
                                          , bPreParseHeader //FRAG_FIX
                                          , bConcatenated,//FRAG_FIX_3
                                              rbStarDecoding
                        ,pcH264AVCDecoder->getH264AVCDecoder()->getSliceHeader()
                        ,SuffixEnable
                                                );
}
//JVT-S036 lsj end

//JVT-P031
ErrVal
CreaterH264AVCDecoder::initPacket ( BinDataAccessor*  pcBinDataAccessor)
{
  return( m_pcH264AVCDecoder->initPacket(pcBinDataAccessor) );
}
Void
CreaterH264AVCDecoder::decreaseNumOfNALInAU()
{
    m_pcH264AVCDecoder->decreaseNumOfNALInAU();
}
Void
CreaterH264AVCDecoder::setDependencyInitialized(Bool b)
{
    m_pcH264AVCDecoder->setDependencyInitialized(b);
}
UInt
CreaterH264AVCDecoder::getNumOfNALInAU()
{
    return m_pcH264AVCDecoder->getNumOfNALInAU();
}
//~JVT-P031
ErrVal
CreaterH264AVCDecoder::checkSliceLayerDependency( BinDataAccessor*  pcBinDataAccessor,
                                                  Bool&             bFinishChecking )
{
  return m_pcH264AVCDecoder->checkSliceLayerDependency( pcBinDataAccessor, bFinishChecking);
}

//NonRequired JVT-Q066
UInt
CreaterH264AVCDecoder::isNonRequiredPic()
{
  return m_pcH264AVCDecoder->isNonRequiredPic();
}
//TMM_EC {{
ErrVal
CreaterH264AVCDecoder::checkSliceGap( BinDataAccessor*  pcBinDataAccessor,
                                      MyList<BinData*>&  cVirtualSliceList)
{
  //return m_pcH264AVCDecoder->checkSliceGap( pcBinDataAccessor, cVirtualSliceList );
	//JVT-X046 {
	if ( m_pcH264AVCDecoder->checkSliceGap( pcBinDataAccessor, cVirtualSliceList ) == Err::m_nOK )
	{
		m_bDiscard = m_pcH264AVCDecoder->m_bDiscard;
		return Err::m_nOK;
	}
	else if ( m_pcH264AVCDecoder->checkSliceGap( pcBinDataAccessor, cVirtualSliceList ) == Err::m_nERR )
	{
		m_bDiscard = m_pcH264AVCDecoder->m_bDiscard;
		return Err::m_nERR;
	}
	else
	{
		return m_pcH264AVCDecoder->checkSliceGap( pcBinDataAccessor, cVirtualSliceList );
	}
	//JVT-X046 }
}
ErrVal
CreaterH264AVCDecoder::setec( UInt uiErrorConceal)
{
  return m_pcH264AVCDecoder->setec( uiErrorConceal);
}
//TMM_EC }}


// FMO DECODE Init ICU/ETRI DS
Void
CreaterH264AVCDecoder::RoiDecodeInit()
{
  m_pcH264AVCDecoder->RoiDecodeInit();
}

ErrVal CreaterH264AVCDecoder::create( CreaterH264AVCDecoder*& rpcCreaterH264AVCDecoder )
{
  rpcCreaterH264AVCDecoder = new CreaterH264AVCDecoder;
  ROT( NULL == rpcCreaterH264AVCDecoder );
  RNOK( rpcCreaterH264AVCDecoder->xCreateDecoder() )
  return Err::m_nOK;
}



ErrVal CreaterH264AVCDecoder::xCreateDecoder()
{
  RNOK( ParameterSetMng       ::create( m_pcParameterSetMng ) );
  RNOK( FrameMng              ::create( m_pcFrameMng ) );
  RNOK( BitReadBuffer         ::create( m_pcBitReadBuffer ) );
  RNOK( NalUnitParser         ::create( m_pcNalUnitParser) );
  RNOK( SliceReader           ::create( m_pcSliceReader ) );
  RNOK( SliceDecoder          ::create( m_pcSliceDecoder ) );
  RNOK( UvlcReader            ::create( m_pcUvlcReader ) );
  RNOK( CabacReader           ::create( m_pcCabacReader ) );
  RNOK( MbParser              ::create( m_pcMbParser ) );
  RNOK( MbDecoder             ::create( m_pcMbDecoder ) );
  RNOK( LoopFilter            ::create( m_pcLoopFilter ) );
  RNOK( IntraPrediction       ::create( m_pcIntraPrediction ) );
  RNOK( MotionCompensation    ::create( m_pcMotionCompensation ) );
  RNOK( H264AVCDecoder           ::create( m_pcH264AVCDecoder ) );
  RNOK( RQFGSDecoder             ::create( m_pcRQFGSDecoder ) );
  RNOK( ControlMngH264AVCDecoder ::create( m_pcControlMng ) );
  RNOK( ReconstructionBypass     ::create(m_pcReconstructionBypass) );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( DecodedPicBuffer    ::create( m_apcDecodedPicBuffer     [uiLayer] ) );
    RNOK( MCTFDecoder         ::create( m_apcMCTFDecoder          [uiLayer] ) );
    RNOK( PocCalculator       ::create( m_apcPocCalculator        [uiLayer] ) );
    RNOK( YuvBufferCtrl       ::create( m_apcYuvFullPelBufferCtrl [uiLayer] ) );
  }

  RNOK( SampleWeighting     ::create( m_pcSampleWeighting ) );
  RNOK( QuarterPelFilter    ::create( m_pcQuarterPelFilter ) );
  RNOK( Transform           ::create( m_pcTransform ) );

  return Err::m_nOK;
}


ErrVal CreaterH264AVCDecoder::destroy()
{
  RNOK( m_pcFrameMng              ->destroy() );
  RNOK( m_pcSliceDecoder          ->destroy() );
  RNOK( m_pcSliceReader           ->destroy() );
  RNOK( m_pcBitReadBuffer         ->destroy() );
  RNOK( m_pcUvlcReader            ->destroy() );
  RNOK( m_pcMbParser              ->destroy() );
  RNOK( m_pcLoopFilter            ->destroy() );
  RNOK( m_pcMbDecoder             ->destroy() );
  RNOK( m_pcTransform             ->destroy() );
  RNOK( m_pcIntraPrediction       ->destroy() );
  RNOK( m_pcMotionCompensation    ->destroy() );
  RNOK( m_pcQuarterPelFilter      ->destroy() );
  RNOK( m_pcCabacReader           ->destroy() );
  RNOK( m_pcNalUnitParser         ->destroy() );
  RNOK( m_pcParameterSetMng       ->destroy() );
  RNOK( m_pcSampleWeighting       ->destroy() );
  RNOK( m_pcH264AVCDecoder        ->destroy() );
  RNOK( m_pcRQFGSDecoder          ->destroy() );
  RNOK( m_pcControlMng            ->destroy() );
  RNOK( m_pcReconstructionBypass  ->destroy() );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( m_apcDecodedPicBuffer    [uiLayer]->destroy() );
    RNOK( m_apcMCTFDecoder         [uiLayer]->destroy() );
    RNOK( m_apcPocCalculator       [uiLayer]->destroy() );
    RNOK( m_apcYuvFullPelBufferCtrl[uiLayer]->destroy() );
  }

  delete this;
  return Err::m_nOK;
}



ErrVal CreaterH264AVCDecoder::init( Bool bOpenTrace )
{
  if( bOpenTrace )
  {
    INIT_DTRACE;
    OPEN_DTRACE;
  }

  RNOK( m_pcBitReadBuffer         ->init() );
  RNOK( m_pcNalUnitParser         ->init( m_pcBitReadBuffer ));
  RNOK( m_pcUvlcReader            ->init( m_pcBitReadBuffer ) );
  RNOK( m_pcCabacReader           ->init( m_pcBitReadBuffer ) );
  RNOK( m_pcQuarterPelFilter      ->init() );
  RNOK( m_pcParameterSetMng       ->init() );
  RNOK( m_pcSampleWeighting       ->init() );
  RNOK( m_pcFrameMng              ->init( m_apcYuvFullPelBufferCtrl[0] ) );
  RNOK( m_pcSliceDecoder          ->init( m_pcMbDecoder,
                                          m_pcControlMng,
                                          m_pcTransform) );
  RNOK( m_pcSliceReader           ->init( m_pcUvlcReader,
                                          m_pcParameterSetMng,
                                          m_pcMbParser,
                                          m_pcControlMng ) );
  RNOK( m_pcMbParser              ->init( m_pcTransform  ) );

  ErrVal cRet = m_pcLoopFilter    ->init( m_pcControlMng , m_pcReconstructionBypass, false );
  RNOK( cRet );
  RNOK( m_pcIntraPrediction       ->init() );
  RNOK( m_pcMotionCompensation    ->init( m_pcQuarterPelFilter,
                                          m_pcTransform,
                                          m_pcSampleWeighting ) );
  RNOK( m_pcMbDecoder             ->init( m_pcTransform,
                                          m_pcIntraPrediction,
                                          m_pcMotionCompensation,
                                          m_pcFrameMng ) );

  RNOK( m_pcH264AVCDecoder           ->init( m_apcMCTFDecoder,
                                          m_pcSliceReader,
                                          m_pcSliceDecoder,
                                          m_pcRQFGSDecoder,
                                          m_pcFrameMng,
                                          m_pcNalUnitParser,
                                          m_pcControlMng,
                                          m_pcLoopFilter,
                                          m_pcUvlcReader,
                                          m_pcParameterSetMng,
                                          m_apcPocCalculator[0],
                                          m_pcMotionCompensation) );

  RNOK( m_pcRQFGSDecoder          ->init( m_apcYuvFullPelBufferCtrl,
                                          m_pcTransform,
                                          m_pcMbParser,
                                          m_pcMbDecoder,
                                          m_pcUvlcReader,
                                          m_pcCabacReader) );

  RNOK( m_pcReconstructionBypass  ->init() );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( m_apcDecodedPicBuffer[uiLayer]->init ( m_apcYuvFullPelBufferCtrl [uiLayer],
                                                 uiLayer ) );
    RNOK( m_apcMCTFDecoder     [uiLayer]->init ( m_pcH264AVCDecoder,
                                                 m_pcSliceReader,
                                                 m_pcSliceDecoder,
                                                 m_pcRQFGSDecoder,
                                                 m_pcNalUnitParser,
                                                 m_pcControlMng,
                                                 m_pcLoopFilter,
                                                 m_pcUvlcReader,
                                                 m_pcParameterSetMng,
                                                 m_apcPocCalculator        [uiLayer],
                                                 m_apcYuvFullPelBufferCtrl [uiLayer],
                                                 m_apcDecodedPicBuffer     [uiLayer],
                                                 m_pcMotionCompensation,
                                                 m_pcQuarterPelFilter
												 //JVT-U106 Behaviour at slice boundaries{
												 ,m_pcReconstructionBypass
												 //JVT-U106 Behaviour at slice boundaries}
												 ) );
  }

  RNOK( m_pcControlMng            ->init( m_pcFrameMng,
                                          m_pcParameterSetMng,
                                          m_apcPocCalculator,
                                          m_pcSliceReader,
                                          m_pcNalUnitParser,
                                          m_pcSliceDecoder,
                                          m_pcBitReadBuffer,
                                          m_pcUvlcReader,
                                          m_pcMbParser,
                                          m_pcLoopFilter,
                                          m_pcMbDecoder,
                                          m_pcTransform,
                                          m_pcIntraPrediction,
                                          m_pcMotionCompensation,
                                          m_apcYuvFullPelBufferCtrl,
                                          m_pcQuarterPelFilter,
                                          m_pcCabacReader,
                                          m_pcSampleWeighting,
                                          m_apcMCTFDecoder,
                                          m_pcH264AVCDecoder ) );


  return Err::m_nOK;
}




ErrVal CreaterH264AVCDecoder::uninit( Bool bCloseTrace )
{
  RNOK( m_pcSampleWeighting       ->uninit() );
  RNOK( m_pcQuarterPelFilter      ->uninit() );
  RNOK( m_pcFrameMng              ->uninit() );
  RNOK( m_pcParameterSetMng       ->uninit() );
  RNOK( m_pcSliceDecoder          ->uninit() );
  RNOK( m_pcSliceReader           ->uninit() );
  RNOK( m_pcBitReadBuffer         ->uninit() );
  RNOK( m_pcUvlcReader            ->uninit() );
  RNOK( m_pcMbParser              ->uninit() );
  RNOK( m_pcLoopFilter            ->uninit() );
  RNOK( m_pcMbDecoder             ->uninit() );
  RNOK( m_pcIntraPrediction       ->uninit() );
  RNOK( m_pcMotionCompensation    ->uninit() );
  RNOK( m_pcCabacReader           ->uninit() );
  RNOK( m_pcH264AVCDecoder        ->uninit() );
  RNOK( m_pcRQFGSDecoder          ->uninit() );
  RNOK( m_pcControlMng            ->uninit() );
  RNOK( m_pcReconstructionBypass  ->uninit() );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( m_apcDecodedPicBuffer    [uiLayer] ->uninit() );
    RNOK( m_apcMCTFDecoder         [uiLayer] ->uninit() );
    RNOK( m_apcYuvFullPelBufferCtrl[uiLayer] ->uninit() );
  }


  if( bCloseTrace )
  {
    CLOSE_DTRACE;
  }

  return Err::m_nOK;
}



H264AVCPacketAnalyzer::H264AVCPacketAnalyzer()
: m_pcBitReadBuffer       ( NULL )
, m_pcUvlcReader          ( NULL )
, m_pcNalUnitParser       ( NULL )
, m_uiStdAVCOffset         ( 0 )
, m_pcNonRequiredSEI    ( NULL )
//TMM_FIX {
, m_uiNonRequiredSeiFlag  ( 0 )
, m_uiPrevPicLayer        ( 0 )
, m_uiCurrPicLayer        ( 0 )
//TMM_FIX }
, m_bAVCCompatible      (false)//BUG FIX Kai Zhang
{
  for(int iLayer=0;iLayer<MAX_SCALABLE_LAYERS;iLayer++)
  {
    m_silceIDOfSubPicLayer[iLayer] = -1;
  }
}



H264AVCPacketAnalyzer::~H264AVCPacketAnalyzer()
{
}



ErrVal
H264AVCPacketAnalyzer::process( BinData*            pcBinData,
                                PacketDescription&  rcPacketDescription,
                                SEI::SEIMessage*&   pcScalableSEIMessage )
{
  pcScalableSEIMessage      = 0;
  UCharArrayParser cNaluHeader( pcBinData->data() );
  ROT( cNaluHeader.getBit() );                                           // NALU HEADER: forbidden_zero_bit
  NalRefIdc   eNalRefIdc    = NalRefIdc   ( cNaluHeader.getBits( 2 ) );  // NALU HEADER: nal_ref_idc
  NalUnitType eNalUnitType  = NalUnitType ( cNaluHeader.getBits( 5 ) );  // NALU HEADER: nal_unit_type
  UInt        uiLayer       = 0;
  UInt        uiLevel       = 0;
  UInt        uiFGSLayer    = 0;
  Bool        bApplyToNext  = false;


  UInt      uiSimplePriorityId = 0;
  Bool      bLayerBaseFlag=false;
  Bool      bUseBasePredFlag=false;
  Bool      bDiscardableFlag = false;
	Bool      m_bOutputFlag   = true;//JVT-W047
  rcPacketDescription.uiNumLevelsQL = 0;

  // JVT-V068 {
  rcPacketDescription.bDiscardableHRDSEI = false;
  // JVT-V068 }

  for(UInt ui = 0; ui < MAX_NUM_RD_LEVELS; ui++)
  {
    //rcPacketDescription.auiDeltaBytesRateOfLevelQL[ui] = 0; //JVT-W137
    //rcPacketDescription.auiQualityLevelQL[ui] = 0;//SEI changes update
		rcPacketDescription.auiPriorityLevelPR[ui] = 0;//SEI changes update
  }
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
  Bool        bScalable     = ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
                                eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE       );
  UInt        uiSPSid       = 0;
  UInt        uiPPSid       = 0;
  Bool        bParameterSet = ( eNalUnitType == NAL_UNIT_SPS                      ||
																eNalUnitType == NAL_UNIT_SUB_SPS                  ||//SSPS
                                eNalUnitType == NAL_UNIT_PPS                        );


  if( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||
      eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
	    eNalUnitType == NAL_UNIT_PREFIX  )//prefix unit
  {
    // byte 0
    cNaluHeader.getBit();                             // NALU HEADER SVC: reserved_one_bit
    cNaluHeader.getBit();/*dummy*/                    // NALU HEADER SVC: idr_flag
    uiSimplePriorityId = cNaluHeader.getBits( 6 );    // NALU HEADER SVC: priority_id
    // byte 1
    bLayerBaseFlag     = cNaluHeader.getBit();        // NALU HEADER SVC: no_inter_layer_pred_flag
    uiLayer            = cNaluHeader.getBits( 3 );    // NALU HEADER SVC: dependency_id
    uiFGSLayer         = cNaluHeader.getBits( 4 );    // NALU HEADER SVC: quality_id
    // byte 2
    uiLevel            = cNaluHeader.getBits( 3 );    // NALU HEADER SVC: temporal_id
    bUseBasePredFlag   = cNaluHeader.getBit();        // NALU HEADER SVC: use_ref_base_pic_flag
    bDiscardableFlag   = cNaluHeader.getBit();        // NALU HEADER SVC: discardable_flag
    m_bOutputFlag      = cNaluHeader.getBit();        // NALU HEADER SVC: output_flag
    cNaluHeader.getBits( 2 );                         // NALU HEADER SVC: reserved_three_2bits
  }
  else if( eNalUnitType == NAL_UNIT_CODED_SLICE     ||
           eNalUnitType == NAL_UNIT_CODED_SLICE_IDR   )
  {
    uiLevel     = ( eNalRefIdc > 0 ? 0 : 1+m_uiStdAVCOffset);
    m_bAVCCompatible=true;//BUG FIX Kai Zhang
  }
  else if( eNalUnitType == NAL_UNIT_SEI )
  {
    ULong*  pulData = (ULong*)( pcBinData->data() + 1 );
    UInt    uiSize  =     8 * ( pcBinData->size() - 1 ) - 1;
    RNOK( m_pcBitReadBuffer->initPacket( pulData, uiSize ) );

    uiSize = pcBinData->byteSize();
    BinData cBinData( new UChar[uiSize], uiSize );
    memcpy( cBinData.data(), pcBinData->data(), uiSize );
    BinDataAccessor cBinDataAccessor;
    cBinData.setMemAccessor( cBinDataAccessor );

    UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu{{
  //RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor, NULL, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC
  RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu}}
    SEI::MessageList cMessageList;
    // JVT-V068 HRD {
    ParameterSetMng* pcParameterSetMng = NULL;
    // JVT-V068 HRD }
    RNOK( SEI::read( m_pcUvlcReader, cMessageList
    				// JVT-V068 HRD {
					, pcParameterSetMng 
    				// JVT-V068 HRD }
		) );

    SEI::MessageList::iterator iter = cMessageList.begin();
    while( ! cMessageList.empty() )
    {
      SEI::SEIMessage* pcSEIMessage = cMessageList.popBack();

      switch( pcSEIMessage->getMessageType() )
      {
      case SEI::SUB_SEQ_INFO:
        {
          SEI::SubSeqInfo* pcSubSeqInfo = (SEI::SubSeqInfo*) pcSEIMessage;
          uiLevel       = pcSubSeqInfo->getSubSeqLayerNum();
          uiLayer       = 0;
          bApplyToNext  = true;
          delete pcSEIMessage;
          break;
        }
      case SEI::SCALABLE_SEI:
      {
        uiLevel = 0;
        uiLayer = 0;
        pcScalableSEIMessage = pcSEIMessage;
        {
          //====set parameters used for further parsing =====
          SEI::ScalableSei* pcSEI    = (SEI::ScalableSei*)pcSEIMessage;
          UInt uiNumScalableLayers  = pcSEI->getNumLayersMinus1() + 1;
          for(UInt uiIndex = 0; uiIndex < uiNumScalableLayers; uiIndex++ )
          {
            if( pcSEI->getDependencyId( uiIndex ) == 0 )
            {
// BUG_FIX liuhui{
              m_uiStdAVCOffset = pcSEI->getTemporalLevel( uiIndex );
              pcSEI->setStdAVCOffset( m_uiStdAVCOffset-1 ); //bugfix replace
              //bugfix delete
// BUG_FIX liuhui}
            }
            else
              break;
          }
        }

          SEI::ScalableSei* pcSEI    = (SEI::ScalableSei*)pcSEIMessage;
          m_uiNum_layers = pcSEI->getNumLayersMinus1() + 1;
          for(int i=0; i< m_uiNum_layers; i++)
        {
          m_ID_ROI[i] = pcSEI->getRoiId(i);
          m_ID_Dependency[i] = pcSEI->getDependencyId(i);
        }
        break;
      }

    case SEI::MOTION_SEI:
      {
        SEI::MotionSEI* pcSEI           = (SEI::MotionSEI*)pcSEIMessage;

        m_silceIDOfSubPicLayer[m_layer_id] = pcSEI->m_slice_group_id[0];
        break;
      }

// JVT-S080 LMI {
    case SEI::SCALABLE_SEI_LAYERS_NOT_PRESENT:
      case SEI::SCALABLE_SEI_DEPENDENCY_CHANGE:
      {
        pcScalableSEIMessage = pcSEIMessage;
        break;
      }
// JVT-S080 LMI }
      case SEI::SUB_PIC_SEI:
      {
        SEI::SubPicSei* pcSEI    = (SEI::SubPicSei*)pcSEIMessage;
        m_layer_id          = pcSEI->getLayerId();
        bApplyToNext  = true;
              break;
      }
      //SEI changes update {
			//{{Quality level estimation and modified truncation- JVTO044 and m12007
      //France Telecom R&D-(nathalie.cammas@francetelecom.com)
      //case SEI::QUALITYLEVEL_SEI:
      //{
      //UInt uiNum = 0;
      //UInt uiDeltaBytesRateOfLevel = 0;
      //UInt uiQualityLevel = 0;
      //SEI::QualityLevelSEI* pcSEI           = (SEI::QualityLevelSEI*)pcSEIMessage;
      //uiNum = pcSEI->getNumLevel();
      //rcPacketDescription.uiNumLevelsQL = uiNum;
      //for(UInt ui = 0; ui < uiNum; ui++)
      //{
      //  uiQualityLevel = pcSEI->getQualityLevel(ui);
      //  //uiDeltaBytesRateOfLevel = pcSEI->getDeltaBytesRateOfLevel(ui); //JVT-W137 remove
      //  rcPacketDescription.auiQualityLevelQL[ui] = uiQualityLevel;
      //  //rcPacketDescription.auiDeltaBytesRateOfLevelQL[ui] = uiDeltaBytesRateOfLevel; //JVT-W137 remove
      //}
      //uiLayer = pcSEI->getDependencyId();
      //bApplyToNext = true;
      //break;
      //}
      //}}Quality level estimation and modified truncation- JVTO044 and m12007
			case SEI::PRIORITYLEVEL_SEI:
      {
				UInt uiNum = 0;
				UInt uiDeltaBytesRateOfLevel = 0;
				UInt uiPriorityLevel = 0;
				SEI::PriorityLevelSEI* pcSEI           = (SEI::PriorityLevelSEI*)pcSEIMessage;
				uiNum = pcSEI->getNumPriorityIds();
				rcPacketDescription.uiNumLevelsQL = uiNum;
				for(UInt ui = 0; ui < uiNum; ui++)
				{
					uiPriorityLevel = pcSEI->getAltPriorityId(ui);
					rcPacketDescription.auiPriorityLevelPR[ui] = uiPriorityLevel;
				}
				uiLayer = pcSEI->getPrDependencyId();
				bApplyToNext = true;
				break;
			}
			//SEI changes update }
      case SEI::NON_REQUIRED_SEI:
      {
        m_pcNonRequiredSEI = (SEI::NonRequiredSei*) pcSEIMessage;
        m_uiNonRequiredSeiFlag = 1;
        break;
      }
// JVT-T073 {
    case SEI::SCALABLE_NESTING_SEI:
      {
        Bool bAllPicturesInAuFlag;
        UInt uiNumPictures;
        UInt *puiDependencyId, *puiQualityLevel;
        SEI::ScalableNestingSei* pcSEI = (SEI::ScalableNestingSei*)pcSEIMessage;
        bAllPicturesInAuFlag = pcSEI->getAllPicturesInAuFlag();
        if( bAllPicturesInAuFlag == 0 )
        {
          uiNumPictures = pcSEI->getNumPictures();
          ROT( uiNumPictures == 0 );
          puiDependencyId = new UInt[uiNumPictures];
          puiQualityLevel = new UInt[uiNumPictures];
          for( UInt uiIndex = 0; uiIndex < uiNumPictures; uiIndex++ )
          {
            puiDependencyId[uiIndex] = pcSEI->getDependencyId(uiIndex);
            puiQualityLevel[uiIndex] = pcSEI->getQualityLevel(uiIndex);
          }
          //make use of dependency_id and quality_level here

          delete puiDependencyId;
          delete puiQualityLevel;
        }
              bApplyToNext = true;
      // JVT-V068 {
              rcPacketDescription.bDiscardableHRDSEI = true;
      // JVT-V068 }

        break;
      }
// JVT-T073 }

      // JVT-V068 {
    case SEI::BUFFERING_PERIOD:
      {
        rcPacketDescription.bDiscardableHRDSEI = true;
        rcPacketDescription.bDiscardable = true;
        break;
      }

    case SEI::PIC_TIMING:
      {
        rcPacketDescription.bDiscardableHRDSEI = true;
        rcPacketDescription.bDiscardable = true;
        break;
      }
      // JVT-V068 }

			//JVT-W052
		case SEI::INTEGRITY_CHECK_SEI:
			{
				//SEI::IntegrityCheckSEI* pcSEI = (SEI::IntegrityCheckSEI*) pcSEIMessage;
				break;
			}
			//JVT-W052

			//JVT-W049 { 
	case SEI::REDUNDANT_PIC_SEI:
	  {
		SEI::RedundantPicSei* pcSEI = (SEI::RedundantPicSei*) pcSEIMessage;
    UInt uiNumDIdMinus1;
		UInt uiNumDId;
		UInt *puiNumQIdMinus1, *puiDependencyId;
		UInt **ppuiQualityId, **ppuiNumRedundantPicsMinus1;
		UInt ***pppuiRedundantPicCntMinus1;
		
		uiNumDIdMinus1 = pcSEI->getNumDIdMinus1( );
		uiNumDId = uiNumDIdMinus1 + 1;
		puiNumQIdMinus1              = new UInt[uiNumDId];
		puiDependencyId              = new UInt[uiNumDId]; 
		ppuiQualityId                 = new UInt*[uiNumDId];
		ppuiNumRedundantPicsMinus1    = new UInt*[uiNumDId];
    pppuiRedundantPicCntMinus1     = new UInt**[uiNumDId];
		for(UInt ui = 0; ui <= uiNumDIdMinus1; ui++)
		{
	    puiDependencyId[ui] = pcSEI->getDependencyId ( ui );
      puiNumQIdMinus1[ui] = pcSEI->getNumQIdMinus1 ( ui );
		  ppuiQualityId[ui]                = new UInt[puiNumQIdMinus1[ui]+1];
		  ppuiNumRedundantPicsMinus1[ui]   = new UInt[puiNumQIdMinus1[ui]+1];
		  pppuiRedundantPicCntMinus1[ui]    = new UInt*[puiNumQIdMinus1[ui]+1];
		  for(UInt uj = 0; uj <= puiNumQIdMinus1[ui]; uj++)
		  {
			ppuiQualityId[ui][uj]  = pcSEI->getQualityId ( ui, uj );
			ppuiNumRedundantPicsMinus1[ui][uj]  = pcSEI->getNumRedundantPicsMinus1 ( ui, uj );
            pppuiRedundantPicCntMinus1[ui][uj] = new UInt[ppuiNumRedundantPicsMinus1[ui][uj] +1];
			for(UInt uk = 0; uk <= ppuiNumRedundantPicsMinus1[ui][uj]; uk++)
			{
              pppuiRedundantPicCntMinus1[ui][uj][uk] = pcSEI->getRedundantPicCntMinus1 ( ui, uj, uk );
			}
		  }                                                           			                                               		                                
		}
		
        delete puiNumQIdMinus1;
        delete puiDependencyId;
        delete ppuiQualityId;
				delete ppuiNumRedundantPicsMinus1;
        delete pppuiRedundantPicCntMinus1;

		break;
	  }
   //JVT-W049 }
			 //JVT-X032 {
	case SEI::TL_SWITCHING_POINT_SEI:
		{
			SEI::TLSwitchingPointSei* pcSEI = (SEI::TLSwitchingPointSei*) pcSEIMessage;
			UInt uiDeltaFrameNum = pcSEI->getDeltaFrameNum();
		}
   //JVT-X032 }
      default:
        {
          delete pcSEIMessage;
        }
      }
    }
    m_pcNalUnitParser->closeNalUnit();
  }

  if( eNalUnitType != NAL_UNIT_SEI )
  {
    ULong*  pulData = (ULong*)( pcBinData->data() + 1 );
    UInt    uiSize  =     8 * ( pcBinData->size() - 1 ) - 1;
    RNOK( m_pcBitReadBuffer->initPacket( pulData, uiSize ) );

    uiSize = pcBinData->byteSize();
    BinData cBinData( new UChar[uiSize], uiSize );
    memcpy( cBinData.data(), pcBinData->data(), uiSize );
    BinDataAccessor cBinDataAccessor;
    cBinData.setMemAccessor( cBinDataAccessor );
    m_pcNalUnitParser->setCheckAllNALUs(true); //JVT-P031
    UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu{{
  //RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor, NULL, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC
  RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu}}
    m_pcNalUnitParser->setCheckAllNALUs(false);//JVT-P031

    // get the SPSid
    if(eNalUnitType == NAL_UNIT_SPS )
    {
      SequenceParameterSet* pcSPS = NULL;
      RNOK( SequenceParameterSet::create  ( pcSPS   ) );
      RNOK( pcSPS->read( m_pcUvlcReader, eNalUnitType ) );
      // Copy simple priority ID mapping from SPS

      uiSPSid = pcSPS->getSeqParameterSetId();
      pcSPS->destroy();
    }
		//SSPS {
		else if( eNalUnitType == NAL_UNIT_SUB_SPS )
		{
		  SequenceParameterSet* pcSPS = NULL;
			RNOK( SequenceParameterSet::create ( pcSPS ) );
			RNOK( pcSPS->readSubSPS( m_pcUvlcReader, eNalUnitType ) );
      // Copy simple priority ID mapping from SPS

			uiSPSid = pcSPS->getSeqParameterSetId();
      pcSPS->destroy();
		}
		//SSPS }
    // get the PPSid and the referenced SPSid
    else if( eNalUnitType == NAL_UNIT_PPS )
    {
      PictureParameterSet* pcPPS = NULL;
      RNOK( PictureParameterSet::create  ( pcPPS   ) );
      RNOK( pcPPS->read( m_pcUvlcReader, eNalUnitType ) );
      uiPPSid = pcPPS->getPicParameterSetId();
      uiSPSid = pcPPS->getSeqParameterSetId();

    // FMO ROI ICU/ETRI
    m_uiNumSliceGroupsMinus1 = pcPPS->getNumSliceGroupsMinus1();

    for(UInt i=0; i<=m_uiNumSliceGroupsMinus1; i++)
    {
     uiaAddrFirstMBofROIs[uiPPSid ][i] = pcPPS->getTopLeft (i);
     uiaAddrLastMBofROIs[uiPPSid ][i]  = pcPPS->getBottomRight (i);
    }

      pcPPS->destroy();
      rcPacketDescription.SPSidRefByPPS[uiPPSid] = uiSPSid;
    }
    // get the PPSid and SPSid referenced by the slice header
    else if(  eNalUnitType == NAL_UNIT_CODED_SLICE              ||
              eNalUnitType == NAL_UNIT_CODED_SLICE_IDR          ||
              eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||
              eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE   )
    {
      if(!(uiLayer == 0 && uiFGSLayer == 0 &&
      (eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE||eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE))){

      UInt uiTemp;
      //JVT-P031
    RNOK( m_pcUvlcReader->getUvlc( uiTemp,  "SH: first_mb_in_slice" ) );

  // FMO ROI ICU/ETRI
  rcPacketDescription.uiFirstMb = uiTemp;


    RNOK( m_pcUvlcReader->getUvlc( uiTemp,  "SH: slice_type" ) );
          // Normally the following test is not necessary because FGS slice shall be marked as Fragmented.

    RNOK( m_pcUvlcReader->getUvlc( uiPPSid, "SH: pic_parameter_set_id" ) );
    uiSPSid = rcPacketDescription.SPSidRefByPPS[uiPPSid];

    //~JVT-P031
    m_uiCurrPicLayer = (uiLayer << 4) + uiFGSLayer;
    if((m_uiCurrPicLayer < m_uiPrevPicLayer || (m_uiCurrPicLayer == m_uiPrevPicLayer && m_uiCurrPicLayer == 0))&& m_uiNonRequiredSeiFlag != 1) //prefix unit
    {
      m_pcNonRequiredSEI->destroy();
      m_pcNonRequiredSEI = NULL;
    }
    m_uiNonRequiredSeiFlag = 0;
    m_uiPrevPicLayer = m_uiCurrPicLayer;
    }
//BUG_FIX Kai Zhang}
    }
    m_pcNalUnitParser->closeNalUnit();
  }

  rcPacketDescription.NalUnitType   = eNalUnitType;
  rcPacketDescription.SPSid         = uiSPSid;
  rcPacketDescription.PPSid         = uiPPSid;

  rcPacketDescription.Scalable      = bScalable;
  rcPacketDescription.ParameterSet  = bParameterSet;
  rcPacketDescription.Layer         = uiLayer;
  rcPacketDescription.FGSLayer      = uiFGSLayer;
  rcPacketDescription.Level         = uiLevel;
  rcPacketDescription.ApplyToNext   = bApplyToNext;
  rcPacketDescription.uiPId         = uiSimplePriorityId;
  rcPacketDescription.bDiscardable  = bDiscardableFlag;//JVT-P031
  rcPacketDescription.bFragmentedFlag   = false;
  rcPacketDescription.uiFragmentOrder   = 0;
  rcPacketDescription.bLastFragmentFlag = false;
  rcPacketDescription.NalRefIdc     = eNalRefIdc;
  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::create( H264AVCPacketAnalyzer*& rpcH264AVCPacketAnalyzer )
{
  rpcH264AVCPacketAnalyzer = new H264AVCPacketAnalyzer;
  ROT( NULL == rpcH264AVCPacketAnalyzer );
  RNOK( rpcH264AVCPacketAnalyzer->xCreate() )
  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::xCreate()
{
  RNOK( BitReadBuffer::create( m_pcBitReadBuffer ) );
  RNOK( UvlcReader   ::create( m_pcUvlcReader    ) );
  RNOK( NalUnitParser::create( m_pcNalUnitParser  ) );

  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::destroy()
{
  RNOK( m_pcBitReadBuffer ->destroy() );
  RNOK( m_pcUvlcReader    ->destroy() );
  RNOK( m_pcNalUnitParser ->destroy() );

  delete this;

  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::init()
{
  RNOK( m_pcBitReadBuffer ->init() );
  RNOK( m_pcUvlcReader    ->init( m_pcBitReadBuffer ) );
  RNOK( m_pcNalUnitParser ->init( m_pcBitReadBuffer ) );

  ::memset( m_auiDecompositionStages, 0x00, MAX_LAYERS*sizeof(UInt) );

  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::uninit()
{
  RNOK( m_pcBitReadBuffer ->uninit() );
  RNOK( m_pcUvlcReader    ->uninit() );

  return Err::m_nOK;
}


// JVT-Q054 Red. Picture {
Bool
CreaterH264AVCDecoder::isRedundantPic()
{
  return m_pcH264AVCDecoder->isRedundantPic();
}


ErrVal
CreaterH264AVCDecoder::checkRedundantPic()
{
  return m_pcH264AVCDecoder->checkRedundantPic();
}


// JVT-Q054 Red. Picture }

//JVT-W049 {

Void 
CreaterH264AVCDecoder::deleteRedSH()
{
  m_pcH264AVCDecoder->deleteRedSH();
}
Void
CreaterH264AVCDecoder::setRedundantPicDefault( Bool bValue )
{
	m_pcH264AVCDecoder->setRedundantPicDefault( bValue );
}
//JVT-W049 }

#ifdef SHARP_AVC_REWRITE_OUTPUT
bool
CreaterH264AVCDecoder::getAvcRewriteFlag()
{
	return m_pcH264AVCDecoder->xGetAvcRewriteFlag();
}

ErrVal
CreaterH264AVCDecoder::startAvcRewrite(UChar*& avcRewriteBinDataBuffer, BinData*& avcRewriteBinData, ExtBinDataAccessor* avcRewriteBinDataAccessor) {
	return m_pcH264AVCDecoder->xStartAvcRewrite(avcRewriteBinDataBuffer, avcRewriteBinData, avcRewriteBinDataAccessor);
}

ErrVal
CreaterH264AVCDecoder::closeAvcRewrite() {
	return m_pcH264AVCDecoder->xCloseAvcRewriteEncoder();

}
bool
CreaterH264AVCDecoder::writeAvcRewriteParameterSets(int nal_unit_type, UInt index) {

	return m_pcH264AVCDecoder->xWriteAvcRewriteParameterSets((NalUnitType)nal_unit_type, index);
}

#endif

H264AVC_NAMESPACE_END
