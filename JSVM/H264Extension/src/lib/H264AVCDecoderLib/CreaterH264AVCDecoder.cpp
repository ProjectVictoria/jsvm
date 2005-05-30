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
{
  ::memset( m_apcDecodedPicBuffer,     0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcMCTFDecoder,          0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcPocCalculator,        0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcYuvFullPelBufferCtrl, 0x00, MAX_LAYERS * sizeof( Void* ) );
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
  return m_pcH264AVCDecoder->process( pcPicBuffer,
                                   rcPicBufferOutputList,
                                   rcPicBufferUnusedList,
                                   rcPicBufferReleaseList );
}



ErrVal
CreaterH264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor,
                                UInt&             ruiNalUnitType,
                                UInt&             uiMbX,
                                UInt&             uiMbY,
                                UInt&             uiSize )
{
  return m_pcH264AVCDecoder->initPacket( pcBinDataAccessor,
                                      ruiNalUnitType,
                                      uiMbX,
                                      uiMbY,
                                      uiSize );
}


ErrVal
CreaterH264AVCDecoder::checkSliceLayerDependency( BinDataAccessor*  pcBinDataAccessor,
                                                  Bool&             bFinishChecking )
{
  return m_pcH264AVCDecoder->checkSliceLayerDependency( pcBinDataAccessor, bFinishChecking );
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



ErrVal CreaterH264AVCDecoder::init()
{
  INIT_DTRACE;
  OPEN_DTRACE;
  
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
  RNOK( m_pcLoopFilter            ->init( m_pcControlMng , m_pcReconstructionBypass  ) );
     
  RNOK( m_pcIntraPrediction       ->init() );
  RNOK( m_pcMotionCompensation    ->init( m_pcQuarterPelFilter,
                                          m_pcSampleWeighting ) );
  RNOK( m_pcMbDecoder             ->init( m_pcTransform,
                                          m_pcIntraPrediction,
                                          m_pcMotionCompensation,
                                          m_pcFrameMng ) );

  RNOK( m_pcH264AVCDecoder        ->init( m_apcMCTFDecoder,
                                          m_pcSliceReader,
                                          m_pcSliceDecoder,
                                          m_pcRQFGSDecoder,
                                          m_pcFrameMng,
                                          m_pcNalUnitParser,
                                          m_pcControlMng,
                                          m_pcLoopFilter,
                                          m_pcUvlcReader,
                                          m_pcParameterSetMng,
                                          m_apcPocCalculator[0] ) );

  RNOK( m_pcRQFGSDecoder          ->init( m_apcYuvFullPelBufferCtrl,
                                          m_pcTransform,
                                          m_pcCabacReader ) );
  
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
                                                 m_pcQuarterPelFilter ) );
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




ErrVal CreaterH264AVCDecoder::uninit()
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


  CLOSE_DTRACE;
  return Err::m_nOK;
}



















H264AVCPacketAnalyzer::H264AVCPacketAnalyzer()
: m_pcBitReadBuffer       ( NULL )
, m_pcUvlcReader          ( NULL )
, m_pcNalUnitParser       ( NULL )
{
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
  UChar       ucByte        = (pcBinData->data())[0];
  NalUnitType eNalUnitType  = NalUnitType ( ucByte  & 0x1F );
  NalRefIdc   eNalRefIdc    = NalRefIdc   ( ucByte >> 5 );
  UInt        uiLayer       = 0;
  UInt        uiLevel       = 0;
  UInt        uiFGSLayer    = 0;
  Bool        bApplyToNext  = false;
  Bool        bScalable     = ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
                                eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE       );
  UInt        uiSPSid       = 0;
  UInt        uiPPSid       = 0;
  Bool        bParameterSet = ( eNalUnitType == NAL_UNIT_SPS                      ||
                                eNalUnitType == NAL_UNIT_PPS                        );


  if( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||
      eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE   )
  {
    ucByte      = (pcBinData->data())[1];
    uiLevel     = ( ucByte >> 5 );
    uiLayer     = ( ucByte >> 2 ) & 7;
    uiFGSLayer  = ( ucByte      ) & 3;
  }
  else if( eNalUnitType == NAL_UNIT_CODED_SLICE     ||
           eNalUnitType == NAL_UNIT_CODED_SLICE_IDR   )
  {
    uiLevel     = ( eNalRefIdc > 0 ? 0 : 1+m_uiStdAVCOffset);
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

    RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor ) );
    SEI::MessageList cMessageList;
    RNOK( SEI::read( m_pcUvlcReader, cMessageList ) );

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
          uiLevel               = 0;
          uiLayer               = 0;
          pcScalableSEIMessage  = pcSEIMessage;
          {
            //===== set parameters used for further parsing =====
            SEI::ScalableSei* pcSEI           = (SEI::ScalableSei*)pcSEIMessage;
            UInt              uiMaxDecStages  = pcSEI->getMaxDecStages();
            for( UInt uiIndex = 0; uiIndex < pcSEI->getNumLayers(); uiIndex++ )
            {
              m_auiDecompositionStages[uiIndex] = uiMaxDecStages - pcSEI->getTemporalResolutionFactor(uiIndex);
            }
            m_uiStdAVCOffset = m_auiDecompositionStages[0] - pcSEI->getAVCTempResStages();
          }
          break;
        }
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
    
    RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor ) );
  
    // get the SPSid
    if(eNalUnitType == NAL_UNIT_SPS )
    {
      SequenceParameterSet* pcSPS = NULL;
      RNOK( SequenceParameterSet::create  ( pcSPS   ) );
      RNOK( pcSPS->read( m_pcUvlcReader, eNalUnitType ) );
      uiSPSid = pcSPS->getSeqParameterSetId();
      pcSPS->destroy();
    }
    // get the PPSid and the referenced SPSid
    else if( eNalUnitType == NAL_UNIT_PPS )
    {
      PictureParameterSet* pcPPS = NULL;
      RNOK( PictureParameterSet::create  ( pcPPS   ) );
      RNOK( pcPPS->read( m_pcUvlcReader, eNalUnitType ) );
      uiPPSid = pcPPS->getPicParameterSetId();
      uiSPSid = pcPPS->getSeqParameterSetId();
      pcPPS->destroy();
      rcPacketDescription.SPSidRefByPPS[uiPPSid] = uiSPSid;
    }
    // get the PPSid and SPSid referenced by the slice header
    else if(  eNalUnitType == NAL_UNIT_CODED_SLICE              ||
              eNalUnitType == NAL_UNIT_CODED_SLICE_IDR          ||
              eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||
              eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE   )
    {
      UInt uiTemp;
      RNOK( m_pcUvlcReader->getUvlc( uiTemp,  "SH: first_mb_in_slice" ) );
      RNOK( m_pcUvlcReader->getUvlc( uiTemp,  "SH: slice_type" ) );
      RNOK( m_pcUvlcReader->getUvlc( uiPPSid, "SH: pic_parameter_set_id" ) );
      uiSPSid = rcPacketDescription.SPSidRefByPPS[uiPPSid];
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



H264AVC_NAMESPACE_END
