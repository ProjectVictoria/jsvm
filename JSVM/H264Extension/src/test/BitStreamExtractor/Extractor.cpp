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



#include "BStreamExtractor.h"
#include "Extractor.h"
#include "ScalableModifyCode.h"
#include "ScalableTestCode.h"
#include <math.h>





Extractor::Extractor()
: m_pcReadBitstream       ( 0 )
, m_pcWriteBitstream      ( 0 )
, m_pcExtractorParameter  ( 0 )
//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
, m_bInInputStreamQL      ( false )
, m_bInInputStreamDS      ( false )
//}}Quality level estimation and modified truncation- JVTO044 and m12007
// HS: packet trace
, m_pcTraceFile           ( 0 )
, m_pcExtractionTraceFile ( 0 )
, m_uiMaxSize             ( 0 )
{
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    for(UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
	{
		m_aSizeDeadSubstream[uiLayer] = 0;
	}
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
}



Extractor::~Extractor()
{
}



ErrVal
Extractor::create( Extractor*& rpcExtractor )
{
  rpcExtractor = new Extractor;
  ROT( NULL == rpcExtractor );
  return Err::m_nOK;
}



ErrVal
Extractor::init( ExtractorParameter *pcExtractorParameter )
{
  ROT( NULL == pcExtractorParameter );

  m_pcExtractorParameter  = pcExtractorParameter;
  m_pcExtractorParameter->setResult( -1 );

  ReadBitstreamFile*  pcReadBitstreamFile;
  RNOKS( ReadBitstreamFile::create( pcReadBitstreamFile ) );
  RNOKS( pcReadBitstreamFile->init( m_pcExtractorParameter->getInFile() ) );
  m_pcReadBitstream = (ReadBitstreamIf*)pcReadBitstreamFile;

  if( !m_pcExtractorParameter->getAnalysisOnly() )
  {
    WriteBitstreamToFile*  pcWriteBitstreamFile;
    RNOKS( WriteBitstreamToFile::create( pcWriteBitstreamFile ) );
    RNOKS( pcWriteBitstreamFile->init( m_pcExtractorParameter->getOutFile() ) );
    m_pcWriteBitstream = (WriteBitstreamIf*)pcWriteBitstreamFile;
  }
  else
  {
    m_pcWriteBitstream = NULL;
  }

  RNOK( h264::H264AVCPacketAnalyzer::create( m_pcH264AVCPacketAnalyzer ) );

  // HS: packet trace
  if( m_pcExtractorParameter->getTraceEnabled() )
  {
    m_pcTraceFile = ::fopen( m_pcExtractorParameter->getTraceFile().c_str(), "wt" );
		if (!m_pcTraceFile)
			fprintf( stderr, "\nCANNOT OPEN TRACE FILE \"%s\"\n\n", m_pcExtractorParameter->getTraceFile().c_str() );
    ROF( m_pcTraceFile );
  }
  else
  {
    m_pcTraceFile = 0;
  }
  if( m_pcExtractorParameter->getExtractTrace() )
  {
    m_pcExtractionTraceFile = ::fopen( m_pcExtractorParameter->getExtractTraceFile().c_str(), "rt" );
    if (!m_pcExtractionTraceFile)
			fprintf( stderr, "\nCANNOT OPEN TRACE FILE \"%s\"\n\n", m_pcExtractorParameter->getExtractTraceFile().c_str() );
    ROF( m_pcExtractionTraceFile );

    RNOK( m_cLargeFile.open( m_pcExtractorParameter->getInFile(), LargeFile::OM_READONLY ) );
  }
  else
  {
    m_pcExtractionTraceFile = 0;
  }


  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset();
  m_cBinDataStartCode.set( m_aucStartCodeBuffer, 4 );

#if NON_REQUIRED_SEI_ENABLE  //shenqiu 05-10-09
  m_uiExtractNonRequiredPics = pcExtractorParameter->getExtractNonRequiredPics();
#endif

  return Err::m_nOK;
}



ErrVal
Extractor::destroy()
{
  m_cBinDataStartCode.reset();

  if( NULL != m_pcH264AVCPacketAnalyzer )
  {
    RNOK( m_pcH264AVCPacketAnalyzer->destroy() );
  }

  if( NULL != m_pcReadBitstream )
  {
    RNOK( m_pcReadBitstream->uninit() );
    RNOK( m_pcReadBitstream->destroy() );
  }

  if( NULL != m_pcWriteBitstream )
  {
    RNOK( m_pcWriteBitstream->uninit() );
    RNOK( m_pcWriteBitstream->destroy() );
  }

  // HS: packet trace
  if( m_pcTraceFile )
  {
    ::fclose( m_pcTraceFile );
    m_pcTraceFile = 0;
  }
  if( m_pcExtractionTraceFile )
  {
    ::fclose( m_pcExtractionTraceFile );
    m_pcExtractionTraceFile = 0;
  }
  if( m_cLargeFile.is_open() )
  {
    RNOK( m_cLargeFile.close() );
  }

  delete this;

  return Err::m_nOK;
}



ErrVal
Extractor::xAnalyse()
{
  UInt                    uiLayer       = 0;
  UInt                    uiLevel       = 0;
  UInt                    uiFGSLayer    = 0;
  Bool                    bNewPicture   = false;
  Bool                    bApplyToNext  = false;
  Bool                    bEOS          = false;
  Bool                    bFirstPacket  = true;
  BinData*                pcBinData     = 0;
  h264::SEI::SEIMessage*  pcScalableSei = 0;
  h264::PacketDescription cPacketDescription;
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  //Dead substream information
  UInt uiMaxRateDS = 0;
  Bool bMaxRateDS = false;
  
  //Quality levels information
  UInt uiSEI = 0;
  UInt uiNumFrame;
 
  //Common information to dead substreams and quality levels
  //arrays initialization
  static UInt auiNumImage[MAX_LAYERS] = 
  { 
    0,0,0,0,0,0,0,0
  };
  
  
  for(uiNumFrame=0; uiNumFrame<MAX_NBFRAMES; uiNumFrame++)
  {
    for(uiLayer=0; uiLayer<MAX_LAYERS; uiLayer++)
    {
      m_aaadMaxRate[uiLayer][uiNumFrame] = 0;
      m_aaiNumLevels[uiLayer][uiNumFrame] = 0;
      for(uiFGSLayer=0; uiFGSLayer<MAX_FGS_LAYERS; uiFGSLayer++)
	  {
		  m_aaiLevelForFrame[uiLayer][uiNumFrame] = -1;
		  m_aaadBytesForFrameFGS[uiLayer][uiNumFrame][uiFGSLayer] = 0;
      }
    }
  }
  uiLayer = 0;
  uiFGSLayer = 0;
  //}}Quality level estimation and modified truncation- JVTO044 and m12007

  // HS: packet trace
  Int64                   i64StartPos   = 0;
  Int64                   i64EndPos     = 0;
  Int                     iLastTempLevel= 0;
  m_uiMaxSize                           = 0;

  //========== initialize (scalable SEI message shall be the first packet of the stream) ===========
  {
    RNOK( m_pcH264AVCPacketAnalyzer->init() );
    //--- get first packet ---
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    ROT ( bEOS );
    //--- analyse packet ---
    RNOK( m_pcH264AVCPacketAnalyzer ->process( pcBinData, cPacketDescription, pcScalableSei ) );
    ROF ( pcScalableSei );

    // HS: packet trace
    if( ! cPacketDescription.ApplyToNext )
    {
      i64EndPos     = static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->getFilePos();
      UInt  uiStart = (UInt)( i64StartPos & 0xFFFFFFFF  );
      UInt  uiSize  = (UInt)( i64EndPos   - i64StartPos );
      i64StartPos   = i64EndPos;
      if( m_pcTraceFile )
      {
        ::fprintf( m_pcTraceFile, "Start-Pos.  Length  LId  TId  QId   Packet-Type  Discardable  Truncatable""\n" );
        ::fprintf( m_pcTraceFile, "==========  ======  ===  ===  ===  ============  ===========  ===========""\n" );
        ::fprintf( m_pcTraceFile, "0x%08x"   "%8d"   "%5d""%5d""%5d""  %s"        "         %s""         %s" "\n",
          uiStart,
          uiSize,
          cPacketDescription.Layer,
          cPacketDescription.Level,
          cPacketDescription.FGSLayer,
          "StreamHeader",
          " No", " No" );
      }
      m_uiMaxSize = max( m_uiMaxSize, uiSize );
    }

    //--- initialize stream description ----
    RNOK( m_cScalableStreamDescription.init( (h264::SEI::ScalableSei*)pcScalableSei ) );
    delete pcScalableSei;
    //---- set packet length ----
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    //---- update stream description -----
    RNOK( m_cScalableStreamDescription.addPacket( 4+pcBinData->size(), 0, 0, 0, false ) );
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    addPacket(0,0,0,4+pcBinData->size());
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
  }



  RNOK( m_cScalableStreamDescription.analyse() );
  m_cScalableStreamDescription.output( stdout );

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->init  ( m_pcExtractorParameter->getInFile() ) );
  return Err::m_nOK;
}






ErrVal
Extractor::xSetParameters()
{
  UInt  uiLayer, uiLevel, uiFGSLayer;

  //=========== clear all ===========
  for( uiLayer = 0; uiLayer <  MAX_LAYERS;  uiLayer++ )
  for( uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    m_aadTargetSNRLayer[uiLayer][uiLevel] = -1;
  }


#define ERROR(x,t)   {if(x) {::printf("\nERROR:   %s\n",t); assert(0); return Err::m_nERR;} }
#define WARNING(x,t) {if(x) {::printf("\nWARNING: %s\n",t); } }


  const MyList<ExtractorParameter::Point>&          rcExtList   = m_pcExtractorParameter->getExtractionList();
  ROT( rcExtList.size() != 1 );
  MyList<ExtractorParameter::Point>::const_iterator cIter       = rcExtList.begin ();
  MyList<ExtractorParameter::Point>::const_iterator cEnd        = rcExtList.end   ();
  const ExtractorParameter::Point&                  rcExtPoint  = *cIter;
  UInt                                              uiExtLayer  = MSYS_UINT_MAX;
  UInt                                              uiExtLevel  = MSYS_UINT_MAX;
  //----- layer -----
  for( uiLayer = 0; uiLayer <= m_cScalableStreamDescription.getNumberOfLayers(); uiLayer++ )
  {
    if( rcExtPoint.uiWidth  == m_cScalableStreamDescription.getFrameWidth (uiLayer) &&
        rcExtPoint.uiHeight == m_cScalableStreamDescription.getFrameHeight(uiLayer)    )
    {
      uiExtLayer = uiLayer;
      break;
    }
  }
  ERROR( uiExtLayer==MSYS_UINT_MAX, "Spatial resolution of extraction/inclusion point not supported" );
  m_pcExtractorParameter->setLayer(uiExtLayer);
  //--- level ---
  for( uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    if( rcExtPoint.dFrameRate == (Double)(1<<uiLevel)*m_cScalableStreamDescription.getFrameRateUnit() )
    {
      uiExtLevel = uiLevel;
      break;
    }
  }
  ERROR( uiExtLevel==MSYS_UINT_MAX, "Temporal resolution of extraction/inclusion point not supported" );
  ERROR( uiExtLevel>m_cScalableStreamDescription.getMaxLevel(uiExtLayer), "Spatio-temporal resolution of extraction/inclusion point not supported" );
  //--- target number of bytes -----
  Double  dTargetNumExtBytes  = rcExtPoint.dBitRate / 8.0 * 1000.0 / ((Double)(1<<uiExtLevel)*m_cScalableStreamDescription.getFrameRateUnit() ) * (Double)m_cScalableStreamDescription.getNumPictures(uiExtLayer,uiExtLevel);


  //===== get and set required base layer packets ======
  Double  dRemainingBytes     = dTargetNumExtBytes;
  for( uiLayer = 0; uiLayer <= uiExtLayer; uiLayer++ )
  for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
  {
     Int64 i64NALUBytes                    = m_cScalableStreamDescription.getNALUBytes( uiLayer, uiLevel, 0 );
     dRemainingBytes                      -= (Double)i64NALUBytes;
     m_aadTargetSNRLayer[uiLayer][uiLevel] = 0;
  }
  if( dRemainingBytes < 0.0 )
  {
    WARNING( true, "Bit-rate overflow for extraction/inclusion point" );
    return Err::m_nOK;
  }


  //===== set maximum possible bytes for included layers ======
  for( uiLayer = 0; uiLayer <  uiExtLayer; uiLayer++ )
  {
    for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
    {
      for( uiFGSLayer = 1; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
      {
        Int64 i64NALUBytes = m_cScalableStreamDescription.getNALUBytes( uiLayer, uiLevel, uiFGSLayer );
        if( (Double)i64NALUBytes <= dRemainingBytes )
        {
          dRemainingBytes                      -= (Double)i64NALUBytes;
          m_aadTargetSNRLayer[uiLayer][uiLevel] = (Double)uiFGSLayer;
        }
        else
        {
          //====== set fractional FGS layer and exit =====
          Double  dFGSLayer = dRemainingBytes / (Double)i64NALUBytes;
          m_aadTargetSNRLayer[uiLayer][uiLevel] += dFGSLayer;
          return Err::m_nOK;
        }
      }
    }
  }


  //===== set FGS layer for current layer =====
  for( uiFGSLayer = 1; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
  {
    Int64 i64FGSLayerBytes = 0;
    for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
    {
      i64FGSLayerBytes += m_cScalableStreamDescription.getNALUBytes( uiExtLayer, uiLevel, uiFGSLayer );
    }
    if( (Double)i64FGSLayerBytes <= dRemainingBytes )
    {
      dRemainingBytes -= (Double)i64FGSLayerBytes;
      for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
      {
        m_aadTargetSNRLayer[uiExtLayer][uiLevel] = (Double)uiFGSLayer;
      }
    }
    else
    {
      Double dFGSLayer = dRemainingBytes / (Double)i64FGSLayerBytes;
      for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
      {
        m_aadTargetSNRLayer[uiExtLayer][uiLevel] += dFGSLayer;
      }
      return Err::m_nOK;
    }
  }
  WARNING( dRemainingBytes>0.0, "Bit-rate underflow for extraction/inclusion point" );


#undef ERROR
#undef WARNING

  return Err::m_nOK;
}







ErrVal
Extractor::go()
{
  RNOK ( xAnalyse() );
  ROTRS( m_pcExtractorParameter->getAnalysisOnly(), Err::m_nOK );

  if( m_pcExtractionTraceFile ) // HS: packet trace
  {
    RNOK( xExtractTrace() ); // HS: packet trace
  }
  else if( m_pcExtractorParameter->getExtractionList().empty() )
  {
    RNOK( xExtractLayerLevel() );
  }
  else
  {
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    //if there is dead substream in the input stream
    // and but no R/D information
    if(m_bInInputStreamDS && !m_bInInputStreamQL)
    {
        go_DS();
	    return Err::m_nOK;
    }
    //if there is R/D information in the input stream
    //with or without dead substream
    if(m_bInInputStreamQL)
    {
	    go_QL();
	    return Err::m_nOK;
    }
    //default case: there is no dead substream, nor R/D information
    //in the input stream
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
    RNOK( xSetParameters() );
    RNOK( xExtractPoints() );
  }

  return Err::m_nOK;
}





ErrVal
Extractor::xExtractPoints()
{
  UInt  uiNumInput    = 0;
  UInt  uiNumKept     = 0;
  UInt  uiNumCropped  = 0;
  Bool  bKeep         = false;
  Bool  bApplyToNext  = false;
  Bool  bEOS          = false;
  Bool  bCrop         = false;

  UInt  uiLayer       = 0;
  UInt  uiLevel       = 0;
  UInt  uiFGSLayer    = 0;
  UInt  uiPacketSize  = 0;
  UInt  uiShrinkSize  = 0;

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  while( ! bEOS )
  {
    //=========== get packet ===========
    BinData*  pcBinData;
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }


    //========== get packet description ==========
    h264::SEI::SEIMessage*  pcScalableSEIMessage = 0;
    h264::PacketDescription cPacketDescription;
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEIMessage ) );
    delete pcScalableSEIMessage;

    //============ get packet size ===========
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    uiPacketSize  = 4 + pcBinData->size();
    uiShrinkSize  = 0;

    //============ set parameters ===========
    if( ! bApplyToNext  )
    {
      uiLayer    = cPacketDescription.Layer;
      uiLevel    = cPacketDescription.Level;
      uiFGSLayer = cPacketDescription.FGSLayer;
    }
    bApplyToNext = cPacketDescription.ApplyToNext;

    //============ check packet ===========
    Double  dSNRLayerDiff = m_aadTargetSNRLayer[uiLayer][uiLevel] - (Double)uiFGSLayer;
    Double  dUpRound      = ceil  ( dSNRLayerDiff );
    Double  dDownRound    = floor ( dSNRLayerDiff );
    bKeep                 =           ( dUpRound   >= 0.0 );
    bCrop                 = bKeep &&  ( dDownRound <  0.0 );
    if( bCrop && uiFGSLayer == 0 )
    {
      bKeep = bCrop = false;
    }
    if( bCrop )
    {
      Double  dWeight     = -dSNRLayerDiff;
      uiShrinkSize        = (UInt)ceil( (Double)uiPacketSize * dWeight );
      if( uiPacketSize - uiShrinkSize > 25 ) // 25 bytes should be enough for the slice headers
      {
        RNOK( pcBinData->decreaseEndPos( uiShrinkSize ) );
        pcBinData->data()[pcBinData->size()-1]  |= 0x01; // trailing one
      }
      else
      {
        bKeep = bCrop = false;
      }
    }

    UInt eNalUnitType = cPacketDescription.NalUnitType;
    Bool bRequired = false;
    if(  eNalUnitType == NAL_UNIT_SPS )
    {
      for( UInt layer = 0; layer <= m_pcExtractorParameter->getLayer(); layer ++ )
      {
        if( m_cScalableStreamDescription.m_bSPSRequired[layer][cPacketDescription.SPSid] )
        {
          bRequired = true;
          break;
        }
      }
      bKeep = bRequired;
    }
    else if( eNalUnitType == NAL_UNIT_PPS )
    {
      for( UInt layer = 0; layer <= m_pcExtractorParameter->getLayer(); layer ++ )
      {
        if( m_cScalableStreamDescription.m_bPPSRequired[layer][cPacketDescription.PPSid] )
        {
          bRequired = true;
          break;
        }
      }
      bKeep = bRequired;
    }

    uiNumInput++;
    if( bKeep ) uiNumKept   ++;
    if( bCrop ) uiNumCropped++;


    //============ write and release packet ============
    if( bKeep )
    {
      RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
      RNOK( m_pcWriteBitstream->writePacket( pcBinData ) );
    }
    RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
  }

  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );


  printf("\n\nNumber of input packets:  %d\nNumber of output packets: %d (cropped: %d)\n\n", uiNumInput, uiNumKept, uiNumCropped );

  return Err::m_nOK;
}

ErrVal
Extractor::xWriteScalableSEIToBuffer(h264::SEI::ScalableSei* pcScalableSei, BinData* pcBinData )
{
	const UInt uiSEILength = 1000;
	UChar		pulStreamPacket[uiSEILength];
	pcBinData->reset();
	pcBinData->set( new UChar[uiSEILength], uiSEILength );

	UChar *m_pucBuffer = pcBinData->data();

	ScalableModifyCode cScalableModifyCode;
	ScalableTestCode cScalableTestCode;
	RNOK( cScalableTestCode.init() );
	RNOK( cScalableModifyCode.init( (ULong*) pulStreamPacket ) );
	RNOK( cScalableTestCode.SEICode( pcScalableSei, &cScalableTestCode ) );
	UInt uiBits = cScalableTestCode.getNumberOfWrittenBits();
	UInt uiSize = (uiBits+7)/8;
	RNOK( cScalableModifyCode.WriteFlag( 0 ) );
	RNOK( cScalableModifyCode.WriteCode( 0 ,2 ) );
	RNOK( cScalableModifyCode.WriteCode( NAL_UNIT_SEI, 5 ) );
	RNOK( cScalableModifyCode.WritePayloadHeader( pcScalableSei->getMessageType(), uiSize ) );
	RNOK( cScalableModifyCode.SEICode( pcScalableSei, &cScalableModifyCode ) );
	uiBits = cScalableModifyCode.getNumberOfWrittenBits();
	uiSize = (uiBits+7)/8;
	UInt uiAlignedBits = 8 - (uiBits&7);
	if( uiAlignedBits != 0 && uiAlignedBits != 8 )
	{
		RNOK( cScalableModifyCode.WriteCode( 1 << (uiAlignedBits-1), uiAlignedBits ) );
	}
	RNOK ( cScalableModifyCode.WriteTrailingBits() );
	RNOK ( cScalableModifyCode.flushBuffer() );
	uiBits = cScalableModifyCode.getNumberOfWrittenBits();
	uiBits              = ( uiBits >> 3 ) + ( 0 != ( uiBits & 0x07 ) );
	uiSize = uiBits;
	RNOK( cScalableModifyCode.ConvertRBSPToPayload( m_pucBuffer, pulStreamPacket, uiSize, 2 ) );
	pcBinData->decreaseEndPos( uiSEILength - uiSize );
	return Err::m_nOK;
}

ErrVal
Extractor::xChangeScalableSEIMesssage( BinData *pcBinData, h264::SEI::SEIMessage* pcScalableSEIMessage,
						UInt uiKeepScalableLayer,UInt& uiMaxLayer, UInt& uiMaxTempLevel, UInt& uiMaxFGSLayer, UInt uiMaxBitrate)
{
	h264::SEI::ScalableSei* pcNewScalableSei;
	RNOK( h264::SEI::ScalableSei::create(pcNewScalableSei) );

	h264::SEI::ScalableSei* pcOldScalableSei = ( h264::SEI::ScalableSei*) pcScalableSEIMessage;
	pcNewScalableSei->setNumLayersMinus1( uiKeepScalableLayer-1);

	UInt uiNumScalableLayer = 0;
	for( UInt uiScalableLayer = 0; uiScalableLayer <= pcOldScalableSei->getNumLayersMinus1(); uiScalableLayer++ )
	{
		ROF( pcOldScalableSei->getDecodingDependencyInfoPresentFlag( uiScalableLayer ) );
		if( uiMaxLayer == MSYS_UINT_MAX )
		{
			if( pcOldScalableSei->getQualityLevel( uiScalableLayer ) > uiMaxFGSLayer ||
				  pcOldScalableSei->getTemporalLevel( uiScalableLayer ) > uiMaxTempLevel )
				continue;
		}
		else
		{
			if( pcOldScalableSei->getDependencyId(uiScalableLayer) > uiMaxLayer || pcOldScalableSei->getTemporalLevel(uiScalableLayer) > uiMaxTempLevel )
				continue;
			if( pcOldScalableSei->getDependencyId(uiScalableLayer) == uiMaxLayer && pcOldScalableSei->getTemporalLevel(uiScalableLayer) == uiMaxTempLevel 
				&& pcOldScalableSei->getQualityLevel( uiScalableLayer) > uiMaxFGSLayer )
				continue;
		}
		pcNewScalableSei->setLayerId( uiNumScalableLayer, uiNumScalableLayer );
		pcNewScalableSei->setFGSlayerFlag( uiNumScalableLayer, pcOldScalableSei->getFGSLayerFlag( uiScalableLayer ) );
		pcNewScalableSei->setSubPicLayerFlag( uiNumScalableLayer, pcOldScalableSei->getSubPicLayerFlag( uiScalableLayer ) );
		pcNewScalableSei->setSubRegionLayerFlag( uiNumScalableLayer, pcOldScalableSei->getSubRegionLayerFlag( uiScalableLayer ) );
		pcNewScalableSei->setProfileLevelInfoPresentFlag( uiNumScalableLayer, pcOldScalableSei->getProfileLevelInfoPresentFlag( uiScalableLayer ) );
		pcNewScalableSei->setDecodingDependencyInfoPresentFlag( uiNumScalableLayer, pcOldScalableSei->getDecodingDependencyInfoPresentFlag( uiScalableLayer ) );
		pcNewScalableSei->setBitrateInfoPresentFlag( uiNumScalableLayer, pcOldScalableSei->getBitrateInfoPresentFlag( uiScalableLayer ) );
		pcNewScalableSei->setFrmRateInfoPresentFlag( uiNumScalableLayer, pcOldScalableSei->getFrmRateInfoPresentFlag( uiScalableLayer ) );
		pcNewScalableSei->setFrmSizeInfoPresentFlag( uiNumScalableLayer, pcOldScalableSei->getFrmSizeInfoPresentFlag( uiScalableLayer ) );
		pcNewScalableSei->setLayerDependencyInfoPresentFlag( uiNumScalableLayer, pcOldScalableSei->getLayerDependencyInfoPresentFlag( uiScalableLayer ) );
		pcNewScalableSei->setInitParameterSetsInfoPresentFlag( uiNumScalableLayer, pcOldScalableSei->getInitParameterSetsInfoPresentFlag( uiScalableLayer ) );

		if(pcNewScalableSei->getProfileLevelInfoPresentFlag(uiNumScalableLayer))
		{
			pcNewScalableSei->setLayerProfileIdc(uiNumScalableLayer, pcOldScalableSei->getLayerProfileIdc( uiScalableLayer ) );
			pcNewScalableSei->setLayerConstraintSet0Flag(uiNumScalableLayer, pcOldScalableSei->getLayerConstraintSet0Flag( uiScalableLayer ) );
			pcNewScalableSei->setLayerConstraintSet1Flag(uiNumScalableLayer, pcOldScalableSei->getLayerConstraintSet1Flag( uiScalableLayer ) );
			pcNewScalableSei->setLayerConstraintSet2Flag(uiNumScalableLayer, pcOldScalableSei->getLayerConstraintSet2Flag( uiScalableLayer ) );
			pcNewScalableSei->setLayerConstraintSet3Flag(uiNumScalableLayer, pcOldScalableSei->getLayerConstraintSet3Flag( uiScalableLayer ) );
			pcNewScalableSei->setLayerLevelIdc(uiNumScalableLayer, pcOldScalableSei->getLayerLevelIdc( uiScalableLayer ) );
		}

		if(pcNewScalableSei->getDecodingDependencyInfoPresentFlag(uiNumScalableLayer))
		{
			pcNewScalableSei->setTemporalLevel(uiNumScalableLayer, pcOldScalableSei->getTemporalLevel( uiScalableLayer ) );
			pcNewScalableSei->setDependencyId(uiNumScalableLayer, pcOldScalableSei->getDependencyId( uiScalableLayer ) );
			pcNewScalableSei->setQualityLevel(uiNumScalableLayer, pcOldScalableSei->getQualityLevel( uiScalableLayer ) );
		}

		if(pcNewScalableSei->getBitrateInfoPresentFlag(uiNumScalableLayer))
		{
			if( uiNumScalableLayer == uiKeepScalableLayer-1 && uiMaxBitrate != MSYS_UINT_MAX )
			{
				pcNewScalableSei->setAvgBitrate( uiNumScalableLayer, uiMaxBitrate );
			}
			else
				pcNewScalableSei->setAvgBitrate(uiNumScalableLayer, pcOldScalableSei->getAvgBitrate( uiScalableLayer ) );
			pcNewScalableSei->setMaxBitrate(uiNumScalableLayer, pcOldScalableSei->getMaxBitrate( uiScalableLayer ) );
		}

		if(pcNewScalableSei->getFrmRateInfoPresentFlag(uiNumScalableLayer))
		{
			pcNewScalableSei->setConstantFrmRateIdc(uiNumScalableLayer, pcOldScalableSei->getConstantFrmRateIdc( uiScalableLayer ) );
			pcNewScalableSei->setAvgFrmRate(uiNumScalableLayer, pcOldScalableSei->getAvgFrmRate( uiScalableLayer ) );
		}

		if(pcNewScalableSei->getFrmSizeInfoPresentFlag(uiNumScalableLayer))
		{
			pcNewScalableSei->setFrmWidthInMbsMinus1(uiNumScalableLayer, pcOldScalableSei->getFrmWidthInMbsMinus1( uiScalableLayer ) );
			pcNewScalableSei->setFrmHeightInMbsMinus1(uiNumScalableLayer, pcOldScalableSei->getFrmHeightInMbsMinus1( uiScalableLayer ) );
		}

		if(pcNewScalableSei->getSubRegionLayerFlag(uiNumScalableLayer))
		{
			pcNewScalableSei->setBaseRegionLayerId(uiNumScalableLayer, pcOldScalableSei->getBaseRegionLayerId( uiScalableLayer ) );
			pcNewScalableSei->setDynamicRectFlag(uiNumScalableLayer, pcOldScalableSei->getDynamicRectFlag( uiScalableLayer ) );
			if(pcNewScalableSei->getDynamicRectFlag(uiNumScalableLayer))
			{
				pcNewScalableSei->setHorizontalOffset(uiNumScalableLayer, pcOldScalableSei->getHorizontalOffset( uiScalableLayer ) );
				pcNewScalableSei->setVerticalOffset(uiNumScalableLayer, pcOldScalableSei->getVerticalOffset( uiScalableLayer ) );
				pcNewScalableSei->setRegionWidth(uiNumScalableLayer, pcOldScalableSei->getRegionWidth( uiScalableLayer ) );
				pcNewScalableSei->setRegionHeight(uiNumScalableLayer, pcOldScalableSei->getRegionHeight( uiScalableLayer ) );
			}
		}

		if(pcNewScalableSei->getLayerDependencyInfoPresentFlag(uiNumScalableLayer))
		{
			pcNewScalableSei->setNumDirectlyDependentLayers(uiNumScalableLayer, pcOldScalableSei->getNumDirectlyDependentLayers( uiScalableLayer ) );
			for( UInt j = 0; j < pcNewScalableSei->getNumDirectlyDependentLayers(uiNumScalableLayer); j++)
			{
				//pcScalableSEI->m_directly_dependent_layer_id_delta;
			}
		}

		if(pcNewScalableSei->getInitParameterSetsInfoPresentFlag(uiNumScalableLayer))
		{
			UInt uiNumInitSPSMinus1 = 0;	//should be changed
			UInt uiNumInitPPSMinus1 = 0;	//should be changed
			pcNewScalableSei->setNumInitSeqParameterSetMinus1(uiNumScalableLayer, pcOldScalableSei->getNumInitSPSMinus1( uiScalableLayer ) );
			pcNewScalableSei->setNumInitPicParameterSetMinus1(uiNumScalableLayer, pcOldScalableSei->getNumInitPPSMinus1( uiScalableLayer ) );
			for( UInt j = 0; j <= pcNewScalableSei->getNumInitSPSMinus1(uiNumScalableLayer); j++)
			{
				//
			}
			for( j = 0; j <= pcNewScalableSei->getNumInitPPSMinus1(uiNumScalableLayer); j++)
			{
				//
			}
		}

		uiNumScalableLayer++;
	
	}
	RNOK( xWriteScalableSEIToBuffer( pcNewScalableSei, pcBinData ) );
	// write pcNewScalableSei into bitstream pcBinData
	return Err::m_nOK;
}

ErrVal
Extractor::xExtractLayerLevel()
{
	UInt uiWantedScalableLayer = m_pcExtractorParameter->getLayer();
	UInt uiMaxTempLevel			= m_pcExtractorParameter->getLevel();
	UInt uiMaxFGSLayer			= (UInt) m_pcExtractorParameter->getFGSLayer();
	UInt uiMaxBitrate				= (UInt) m_pcExtractorParameter->getBitrate();
	UInt uiMaxLayer = MSYS_UINT_MAX;
	UInt uiKeepScalableLayer= 0;

	UInt uiDecreaseBitrate = MSYS_UINT_MAX;

#if NON_REQUIRED_SEI_ENABLE  //shenqiu 05-10-02
	h264::SEI::NonRequiredSei* pcNonRequiredDescription = NULL;  
#endif

	if( uiMaxFGSLayer != 10 || uiMaxTempLevel != MSYS_UINT_MAX )
	{	
		//TL or QL
		uiKeepScalableLayer = 0;
		for ( UInt uiScalableLayer = 0; uiScalableLayer < m_cScalableStreamDescription.getNumOfScalableLayers(); uiScalableLayer++ )
		{
			if( m_cScalableStreamDescription.getFGSLevel( uiScalableLayer ) <= uiMaxFGSLayer &&
				  m_cScalableStreamDescription.getTempLevel( uiScalableLayer ) <= uiMaxTempLevel )	
			uiKeepScalableLayer++;
		}
	}
	else
	{	
		// L or B
		uiKeepScalableLayer = 0;
		if( uiWantedScalableLayer != MSYS_UINT_MAX && uiWantedScalableLayer >= m_cScalableStreamDescription.getNumOfScalableLayers() )
		{
			//specific scalable layer
			uiWantedScalableLayer = m_cScalableStreamDescription.getNumOfScalableLayers()-1;
		}
		else if( uiMaxBitrate != MSYS_UINT_MAX )
		{
			UInt uiCurrBitrate, uiPrevBitrate = 0, uiFinalBitrate = MSYS_UINT_MAX;
			UInt uiPos = 0;
			for( UInt uiScalableLayer = 0; uiScalableLayer < m_cScalableStreamDescription.getNumOfScalableLayers(); uiScalableLayer++ )
			{
				uiCurrBitrate = m_cScalableStreamDescription.getBitrateOfScalableLayers( uiScalableLayer );
				if( uiCurrBitrate == uiMaxBitrate )
				{
					uiFinalBitrate = uiCurrBitrate;
					uiPos = uiScalableLayer;
					uiDecreaseBitrate = 0;
					break;
				}
				else if( uiCurrBitrate > uiMaxBitrate && uiPrevBitrate < uiMaxBitrate && 
					m_cScalableStreamDescription.getFGSLevel( uiScalableLayer ) != 0 )
				{
					uiFinalBitrate = uiCurrBitrate;
					uiPos = uiScalableLayer;
					uiDecreaseBitrate = uiMaxBitrate - uiPrevBitrate;
					break;
				}
				uiPrevBitrate = uiCurrBitrate;
			}
			if( uiDecreaseBitrate == MSYS_UINT_MAX )	// No FGS layer satisfy
			{
				uiFinalBitrate = 0;
				uiPrevBitrate = m_cScalableStreamDescription.getBitrateOfScalableLayers( 0 );
				if( uiPrevBitrate > uiMaxBitrate )
				{
					printf( "Too small bitrate. No packet will be released. \n" );
					exit( 1 );
				}
				for( UInt uiScalableLayer = 1; uiScalableLayer < m_cScalableStreamDescription.getNumOfScalableLayers(); uiScalableLayer++ )
				{
					uiCurrBitrate = m_cScalableStreamDescription.getBitrateOfScalableLayers( uiScalableLayer );
					if( uiCurrBitrate <= uiMaxBitrate && uiCurrBitrate > uiFinalBitrate )
					{
						uiFinalBitrate = uiCurrBitrate;
						uiPos = uiScalableLayer;
					}
				}
				uiMaxBitrate = uiFinalBitrate;
			}		
			uiWantedScalableLayer = uiPos;
		}
		if( uiWantedScalableLayer != MSYS_UINT_MAX )
		{
			uiMaxLayer		 = m_cScalableStreamDescription.getDependencyId( uiWantedScalableLayer );
			uiMaxTempLevel = m_cScalableStreamDescription.getTempLevel( uiWantedScalableLayer );
			uiMaxFGSLayer  = m_cScalableStreamDescription.getFGSLevel ( uiWantedScalableLayer );
		}
		for( UInt uiScalableLayer = 0; uiScalableLayer < m_cScalableStreamDescription.getNumOfScalableLayers(); uiScalableLayer++ )
		{
			if( m_cScalableStreamDescription.getDependencyId( uiScalableLayer ) > uiMaxLayer || 
				m_cScalableStreamDescription.getTempLevel( uiScalableLayer ) > uiMaxTempLevel )
				continue;
			if( m_cScalableStreamDescription.getDependencyId( uiScalableLayer ) == uiMaxLayer &&
				m_cScalableStreamDescription.getTempLevel( uiScalableLayer ) == uiMaxTempLevel &&
				m_cScalableStreamDescription.getFGSLevel ( uiScalableLayer ) > uiMaxFGSLayer )
				continue;
			uiKeepScalableLayer++;
		}
	}
	Bool bTruncated = ( uiDecreaseBitrate != 0 && uiDecreaseBitrate != MSYS_UINT_MAX );
	UInt uiNumInput					= 0;
	UInt uiNumKept					= 0;
	Bool bKeep							= false;
	Bool bApplyToNext				= false;
	Bool bEOS								= false;
	UInt uiScalableLayer		= 0;
	UInt uiLayer						= 0;
	UInt uiTempLevel				= 0;
	UInt uiFGSLayer					= 0;
  UInt    uiPacketSize  = 0;
  UInt    uiShrinkSize  = 0;


	RNOK( m_pcH264AVCPacketAnalyzer->init() );

	while( ! bEOS )
	{
		UInt uiScalableLayer = 0;
		//========== get packet ==============
		BinData * pcBinData;
		RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
		if( bEOS )
		{
			continue;
		}
		//===== get packet description ======
		h264::SEI::SEIMessage*  pcScalableSEIMessage = 0;
		h264::PacketDescription cPacketDescription;
		RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEIMessage ) );
		if( pcScalableSEIMessage )
		{
			if( pcScalableSEIMessage->getMessageType() == h264::SEI::SCALABLE_SEI )
			{
				//RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
				RNOK( xChangeScalableSEIMesssage( pcBinData, pcScalableSEIMessage, uiKeepScalableLayer,
					uiMaxLayer, uiMaxTempLevel, uiMaxFGSLayer, uiMaxBitrate ) );
			}
		}

#if NON_REQUIRED_SEI_ENABLE  //shenqiu	
		pcNonRequiredDescription = m_pcH264AVCPacketAnalyzer->getNonRequiredSEI();
#endif
		delete pcScalableSEIMessage;

		//============ get packet size ===========
		while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
		{
			RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
		}


		//============ set parameters ===========
		if( ! bApplyToNext  )
		{
			uiLayer					= cPacketDescription.Layer;
			uiTempLevel			= cPacketDescription.Level;
			uiFGSLayer			= cPacketDescription.FGSLayer;
			uiScalableLayer = m_cScalableStreamDescription.getNumberOfScalableLayers( uiLayer,uiTempLevel, uiFGSLayer );
		}
		bApplyToNext = cPacketDescription.ApplyToNext;

		//============ check packet ===========
		if( uiWantedScalableLayer == MSYS_UINT_MAX )	//input: -t,-f
			bKeep = ( uiFGSLayer <= uiMaxFGSLayer && uiTempLevel <= uiMaxTempLevel );
		else	//input: -l,-b
		{
			if( uiLayer > uiMaxLayer || uiTempLevel > uiMaxTempLevel )
				bKeep = false;
			else if( uiLayer == uiMaxLayer && uiTempLevel == uiMaxTempLevel && uiFGSLayer > uiMaxFGSLayer )
				bKeep = false;
			else
				bKeep = true;
		}

#if NON_REQUIRED_SEI_ENABLE  //shenqiu 05-11-03	
		if(m_pcH264AVCPacketAnalyzer->getNonRequiredSEIRead() == 1)
			bKeep = 0;
		if( m_uiExtractNonRequiredPics && pcNonRequiredDescription)
		{
			for(UInt i = 0; i <= pcNonRequiredDescription->getNumInfoEntriesMinus1(); i++)
			{
				if(pcNonRequiredDescription->getEntryDependencyId(i))  // it should be changed to if(DenpendencyId == LayerId of the shown picture) 
				{
					for(UInt j = 0; j <= pcNonRequiredDescription->getNumNonRequiredPicsMinus1(i); j++)
					{
						if(cPacketDescription.Layer == pcNonRequiredDescription->getNonRequiredPicDependencyId(i,j) &&
							cPacketDescription.FGSLayer == pcNonRequiredDescription->getNonRequiredPicQulityLevel(i,j))  // it should be add something about FragmentFlag
						{
							bKeep = 0;
						}
					}
				}
			}
		}
#endif

		UInt eNalUnitType = cPacketDescription.NalUnitType;
		Bool bRequired = false;
		if(  eNalUnitType == NAL_UNIT_SPS )
		{
			bRequired = true;
			bKeep = bRequired;
		}
		else if( eNalUnitType == NAL_UNIT_PPS )
		{
			bRequired = true;
			bKeep = bRequired;
		}
		uiNumInput++;
		if( bKeep )   uiNumKept++;


		//============ write and release packet ============
		if( bKeep )
		{
			uiPacketSize  += 4 + pcBinData->size();
			//first check if truncated FGS layer :liuhui 2005-10-12
			if( cPacketDescription.NalUnitType != NAL_UNIT_PPS &&
					cPacketDescription.NalUnitType != NAL_UNIT_SPS &&
					cPacketDescription.NalUnitType != NAL_UNIT_SEI )
				if( uiScalableLayer == uiWantedScalableLayer && bTruncated )
				{
					Double dTemp = (Double)uiMaxBitrate/m_cScalableStreamDescription.getBitrateOfScalableLayers( uiWantedScalableLayer );
					UInt uiSize = floor(pcBinData->size()  * dTemp );
					pcBinData->decreaseEndPos( pcBinData->size() - uiSize );
				}
			RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
			RNOK( m_pcWriteBitstream->writePacket( pcBinData ) );
		}
		RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
	}

	RNOK( m_pcH264AVCPacketAnalyzer->uninit() );
	printf("Total Packet Size: %d\n", uiPacketSize );


	printf("\n\nNumber of input packets :  %d\n" 
		"Number of output packets:  %d \n", uiNumInput, uiNumKept );
	if( bTruncated )
	{
		printf( "The scalable layer %d is truncated. \n\n", uiWantedScalableLayer );
	}

	return Err::m_nOK;
}


// HS: packet trace
ErrVal
Extractor::xReadLineExtractTrace( Char* pcFormatString,
                                  UInt* puiStart,
                                  UInt* puiLength )
{
  if( NULL != puiStart && NULL != puiLength )
  {
    //--- don't ask me why ----
    ROTR( 0 == fscanf( m_pcExtractionTraceFile, pcFormatString, puiStart, puiLength ), Err::m_nInvalidParameter );
  }

  for( Int n = 0; n < 0x400; n++ )
  {
    ROTRS( '\n' == fgetc( m_pcExtractionTraceFile ), Err::m_nOK );
  }

  return Err::m_nERR;
}


// HS: packet trace
ErrVal
Extractor::xExtractTrace()
{
  Bool    bEOS            = false;
  Int64   i64StartPos     = 0;
  Int64   i64EndPos       = 0;
  Int     iLastTempLevel  = 0;
  UInt    uiNextStart     = 0;
  UInt    uiNextLength    = 0;

  UInt    uiNumDiscarded  = 0;
  UInt    uiNumTruncated  = 0;
  UInt    uiNumKept       = 0;

  UChar*  pucPacketBuffer = new UChar[ m_uiMaxSize + 1 ];
  ROF( pucPacketBuffer );

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  RNOK( xReadLineExtractTrace( "",      NULL,          NULL ) );  // skip first line
  RNOK( xReadLineExtractTrace( "",      NULL,          NULL ) );  // skip second line
  RNOK( xReadLineExtractTrace( "%x %d", &uiNextStart,  &uiNextLength ) );

  while( ! bEOS )
  {
    //=========== get packet ===========
    BinData*  pcBinData;
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }

    //========== get packet description ==========
    h264::SEI::SEIMessage*  pcScalableSEIMessage = 0;
    h264::PacketDescription cPacketDescription;
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEIMessage ) );
    delete pcScalableSEIMessage;

    if( ! cPacketDescription.ApplyToNext )
    {
      i64EndPos       = static_cast<ReadBitstreamFile*>( m_pcReadBitstream )->getFilePos();
      UInt  uiStart   = (UInt)( i64StartPos & 0xFFFFFFFF  );
      UInt  uiSize    = (UInt)( i64EndPos   - i64StartPos );
      i64StartPos     = i64EndPos;

      printf("PACKET 0x%08x (%6d)     ", uiStart, uiSize );

      //////////////////////////////////////////////////////////////////////////
      if( uiStart == uiNextStart )
      {
        //===== read packet =====
        ROT ( uiSize > m_uiMaxSize );
        RNOK( m_cLargeFile.seek( uiStart, SEEK_SET ) );
        UInt  uiReadSize = 0;
        RNOK( m_cLargeFile.read( pucPacketBuffer, uiSize, uiReadSize ) );
        ROF ( uiSize == uiReadSize );

        //===== modify last bytes if necessary ====
        if( uiSize < uiNextLength )
        {
          fprintf( stderr, "\nERROR: The packet at start pos. 0x%08x is shorter than %d bytes\n", uiNextStart, uiNextLength );
          ROT( true );
        }
        else if( uiSize > uiNextLength )
        {
          if( cPacketDescription.FGSLayer == 0 )
          {
            fprintf( stderr, "\nERROR: The packet at start pos. 0x%08x is not truncatable\n", uiNextStart );
            ROT( true );
          }

          //===== truncate packet =====
          if( pcBinData->size() - uiSize + uiNextLength < 25 )
          {
            uiNextLength = 25 + uiSize - pcBinData->size();

            fprintf( stderr, "\nWARNING: The size of the packet at start pos. 0x%08x was increased to %d bytes\n", uiNextStart, uiNextLength );
          }

          pucPacketBuffer[uiNextLength-1] |= 0x01; // trailing one

          printf("truncated to %d bytes\n", uiNextLength );
          uiNumTruncated++;
        }
        else
        {
          printf("kept\n");
          uiNumKept++;
        }

        //===== write packet =====
        static_cast<WriteBitstreamToFile*>( m_pcWriteBitstream )->writePacket( pucPacketBuffer, uiNextLength );

        //===== get next traget packet ====
        if( xReadLineExtractTrace( "%x %d", &uiNextStart,  &uiNextLength ) != Err::m_nOK )
        {
          uiNextStart   = 0xFFFFFFFF;
          uiNextLength  = 1;
        }
      }
      else if( uiStart > uiNextStart )
      {
        fprintf( stderr, "\nERROR: It exists no packet with start pos. 0x%08x\n", uiNextStart );
        ROT( true );
      }
      else
      {
        printf("discarded\n");
        uiNumDiscarded++;
      }
    }
    else
    {
      iLastTempLevel  = cPacketDescription.Level;
    }

    RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
  }

  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );

  delete pucPacketBuffer;

  printf("\n\n\n");
  printf("%d packets kept (%d truncated)\n", uiNumKept+uiNumTruncated, uiNumTruncated );
  printf("%d packets discarded\n", uiNumDiscarded );
  printf("\n");

  return Err::m_nOK;
}

//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
ErrVal
Extractor::go_DS()
{
	//if we want to keep or throw away the dead substream
	for(UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
	{
        setExtractDeadSubstream(m_pcExtractorParameter->getExtractDeadSubstream(uiLayer),uiLayer);
	}

	//Calculate the size of the dead substream
	CalculateSizeDeadSubstream();

	//UInt uiExtLayer, uiExtLevel;
	RNOK( xSetParameters_DS() );
	RNOK( xExtractPoints_DS() );
	
	return Err::m_nOK;
}

ErrVal
Extractor::go_QL()
{
  //UInt uiExtLayer;
  //Double dRateTarget;
  //UInt uiExtLevel;
  //determine layer, level and rateTarget for output stream
  GetExtParameters();

  //search optimal quality for target rate
  QualityLevelSearch();
  
  //extract NALs for optimal quality
  RNOK(ExtractPointsFromRate());
	return Err::m_nOK;
}

Void Extractor::setQualityLevel()
{
	UInt uiLevel;
    UInt uiLayer;
    UInt uiNumImage;
    for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
    {
        for(uiNumImage = 0; uiNumImage < MAX_NBFRAMES; uiNumImage++)
        {
            UInt uiRateOld = m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][0];
	        for(uiLevel = 0; uiLevel < m_aaiNumLevels[uiLayer][uiNumImage]; uiLevel++)
	        {
		        if(uiLevel == 0)
		        {
			        m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][uiLevel] = (UInt)m_aaadBytesForFrameFGS[uiLayer][uiNumImage][0];
			        m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][uiLevel] += m_aauiSEIQLPacketSize[uiLayer][uiNumImage];
			        uiRateOld = m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][uiLevel];
		        }
		        else
		        {
                    UInt uiDeltaRate = m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][uiLevel];
			        m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][uiLevel] = uiRateOld + uiDeltaRate;
			        uiRateOld = m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][uiLevel];
		        }
            }
        }
    }
}

Void Extractor::GetExtParameters()
{
  UInt uiLayer,uiLevel;

  const MyList<ExtractorParameter::Point>&          rcExtList   = m_pcExtractorParameter->getExtractionList();
  MyList<ExtractorParameter::Point>::const_iterator cIter       = rcExtList.begin ();
  MyList<ExtractorParameter::Point>::const_iterator cEnd        = rcExtList.end   ();
  const ExtractorParameter::Point&                  rcExtPoint  = *cIter;
  UInt uiExtLayer  = MSYS_UINT_MAX;
  UInt uiExtLevel  = MSYS_UINT_MAX;
  //----- layer -----
  for( uiLayer = 0; uiLayer <= m_cScalableStreamDescription.getNumberOfLayers(); uiLayer++ )
  {
    if( rcExtPoint.uiWidth  == m_cScalableStreamDescription.getFrameWidth (uiLayer) &&
        rcExtPoint.uiHeight == m_cScalableStreamDescription.getFrameHeight(uiLayer)    )
    {
      uiExtLayer = uiLayer;
      break;
    }
  }
  m_pcExtractorParameter->setLayer(uiExtLayer);
  //--- level ---
  for( uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    if( rcExtPoint.dFrameRate == (Double)(1<<uiLevel)*m_cScalableStreamDescription.getFrameRateUnit() )
    {
      uiExtLevel = uiLevel;
      break;
    }
  }
  m_pcExtractorParameter->setLevel(uiLevel);
   //--- target number of bytes -----
  Double  dTargetNumExtBytes  = rcExtPoint.dBitRate / 8.0 * 1000.0 / ((Double)(1<<uiExtLevel)*m_cScalableStreamDescription.getFrameRateUnit() ) * (Double)m_cScalableStreamDescription.getNumPictures(uiExtLayer,uiExtLevel);

  m_pcExtractorParameter->setTargetRate(dTargetNumExtBytes);
}

ErrVal Extractor::QualityLevelSearch()
{

  //Double R,D;
  UInt uiNFrames;
  //UInt uiDecStages;
  UInt uiNumPictures;
  UInt uiLayer;
  //Int uiPoint;
  //UInt uiLevel;
  //UInt uiFGSLayer;
  Double rate; 
  //Int64 i64NALUBytes;

  UInt uiExtLayer = m_pcExtractorParameter->getLayer();
  UInt uiExtLevel = m_pcExtractorParameter->getLevel();
  Double dRateConstraint = m_pcExtractorParameter->getTargetRate();
  UInt uiMaxLayers = uiExtLayer+1;
  
  // Getting min and max QualityLevel
  Double QualityLevelMin = 0;
  Double QualityLevelMax = 255;
 
  printf("---------Rate target: %f \n", dRateConstraint);

  Double minRate = 0;

  minRate= GetRateForQualityLevel(QualityLevelMax, uiMaxLayers, uiExtLevel, uiExtLayer);
  Double maxRate = GetRateForQualityLevel(QualityLevelMin, uiMaxLayers, uiExtLevel, uiExtLayer);

  // iteration loop
  int iter;
  int iterMax = 10;
  Double midQualityLevel;
  Double midRate;
  for(iter=0; iter<iterMax; iter++)
  {  
    midQualityLevel = (QualityLevelMin+QualityLevelMax)/2;
    midRate = GetRateForQualityLevel(midQualityLevel, uiMaxLayers, uiExtLevel, uiExtLayer);

    if (midRate > dRateConstraint)
    {
      QualityLevelMin = midQualityLevel;
      maxRate = midRate;
    }
    else
    {
      QualityLevelMax = midQualityLevel;
      minRate = midRate;
    }
  }
  printf("---------Rate generated: %f (target=%f) QualityLevel %f\n", minRate, dRateConstraint, QualityLevelMax);
  
  
  // set the rate for each frames
  for(uiLayer = 0; uiLayer < uiMaxLayers; uiLayer++)
  {
	uiNumPictures = m_auiNbImages[uiLayer];	
    for(uiNFrames = 0; uiNFrames < uiNumPictures; uiNFrames++)
    {
	if(m_aaiLevelForFrame[uiLayer][uiNFrames] <= (Int)uiExtLevel)
      {
		  rate = GetRateForQualityLevel(uiLayer, uiNFrames,QualityLevelMax, uiExtLayer);
	      m_aadTargetByteForFrame[uiNFrames][uiLayer] = rate;		  
	  }
    }
  }

  return Err::m_nOK;

}

Double Extractor::GetRateForQualityLevel(double QualityLevel, UInt uiMaxLayers, UInt uiExtLevel, UInt uiExtLayer)
{
  UInt uiNFrames;
  UInt uiNumPictures;
  UInt uiLayer;

  Double sum=0;

  for(uiLayer = 0; uiLayer < uiMaxLayers; uiLayer++)
  {
    uiNumPictures = m_auiNbImages[uiLayer];
    for(uiNFrames = 0; uiNFrames < uiNumPictures; uiNFrames++)
    {
	 if(m_aaiLevelForFrame[uiLayer][uiNFrames] <= (Int)uiExtLevel)
	 {
		sum += GetRateForQualityLevel(uiLayer, uiNFrames,QualityLevel,uiExtLayer);
	  }
    }
  }
  return sum;
}


Double Extractor::GetRateForQualityLevel(UInt uiLayer, UInt uiNumImage, Double QualityLevel, UInt uiExtLayer)
{
	Int i = 0;
	//minimal rate for the frame (BL)
	Double rate = m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][0];
	Bool stop = true;

	while (i<m_aaiNumLevels[uiLayer][uiNumImage] && stop)
  {
    if (m_aaadQualityLevel[uiLayer][uiNumImage][i]>QualityLevel)
		rate = m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][i];
	if(m_aaadQualityLevel[uiLayer][uiNumImage][i]<=QualityLevel)
		stop = false;
    i++;

  }
  i--;
  //if target lambda is lower than min lambda of the frame, max rate of the frame is returned 
  if(QualityLevel < m_aaadQualityLevel[uiLayer][uiNumImage][m_aaiNumLevels[uiLayer][uiNumImage]-1])
  {
	  rate = m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][m_aaiNumLevels[uiLayer][uiNumImage]-1];
  }
  else
  {
	  //QualityLevel = 0 is the minimum value of QualityLevel: corresponds to max rate of the frame
	  if(QualityLevel == 0)
		  rate = m_aaauiBytesForQualityLevel[uiLayer][uiNumImage][m_aaiNumLevels[uiLayer][uiNumImage]-1];

  }
 
  if(rate > m_aaadMaxRate[uiLayer][uiNumImage])
	  rate = m_aaadMaxRate[uiLayer][uiNumImage];

  return rate;
}


ErrVal
Extractor::ExtractPointsFromRate()
{
  UInt  uiNumInput    = 0;
  UInt  uiNumKept     = 0;
  UInt  uiNumCropped  = 0;
  Bool  bKeep         = false;
  Bool  bApplyToNext  = false;
  Bool  bNewPicture   = false;
  Bool  bEOS          = false;
  Bool  bCrop         = false;
  Bool  bSEIPacket    = true;
  Bool	bQualityLevelSEI  = false;
  Bool  bDSSEI  = false;
  Bool bAlreadyExtracted = false;

  UInt  uiLayer       = 0;
  UInt  uiLevel       = 0;
  UInt  uiFGSLayer    = 0;
  UInt  uiPacketSize  = 0;
  UInt  uiShrinkSize  = 0;

  Double totalPackets = 0;
	
  Double dTot = 0;

  UInt uiSEI = 0;
  Bool bPrevPacketWasSEI = false;
  UInt uiSEISize = 0;
  Int currFrame[MAX_LAYERS];
  UInt uiTotalSEI = 0;
  Bool bKeepSEI = false;
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	    currFrame[uiLayer] = 0;
  }


  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  while( ! bEOS )
  {
    //=========== get packet ===========
    BinData*  pcBinData;
  	RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
	{
		continue;
	}
	
    //========== get packet description ==========
    h264::SEI::SEIMessage*  pcScalableSEIMessage = 0;
    h264::PacketDescription cPacketDescription;
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEIMessage ) );
    delete pcScalableSEIMessage;

    //============ get packet size ===========
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    uiPacketSize  = 4 + pcBinData->size();
    uiShrinkSize  = 0;
	
	bKeep = false;
	bCrop = false;
    //============ set parameters ===========
	uiLayer    = cPacketDescription.Layer;
    uiLevel    = cPacketDescription.Level;
    uiFGSLayer = cPacketDescription.FGSLayer;
    
    bApplyToNext = cPacketDescription.ApplyToNext;
    bNewPicture   = ( ! cPacketDescription.ParameterSet && ! cPacketDescription.ApplyToNext );
	
	bQualityLevelSEI = (cPacketDescription.uiNumLevelsQL != 0);
	bDSSEI = (cPacketDescription.MaxRateDS != 0);

	UInt uiCurrFrame = 0;

	if (bSEIPacket)
    {
      bNewPicture = false;
      //bSEIPacket = false;
    }
	// update frame number
    if (bNewPicture && uiFGSLayer == 0)
    {
      currFrame[uiLayer]++;
    }
	
	if(bSEIPacket || cPacketDescription.ParameterSet)
		uiCurrFrame = 0;
	else
	{
        if(!bApplyToNext)
            uiCurrFrame = currFrame[uiLayer]-1;
		else
			uiCurrFrame = currFrame[uiLayer];
	}
	
	if(bSEIPacket)
		bSEIPacket = false;

	/*if(bPrevPacketWasSEI)
	{
		bPrevPacketWasSEI = false;
		//Bool bKeepSEI = //m_abKeepQualityLevelSEI[uiSEI-1];
		if(bKeepSEI)
			m_aadTargetByteForFrame[uiCurrFrame][uiLayer] -= uiSEISize;
        bKeepSEI = false;
	}*/

	if(bQualityLevelSEI)
	{
		//RD SEI packet
		//look if packet is kept or not
		bKeep = (m_aaiLevelForFrame[uiLayer][uiCurrFrame] <= m_pcExtractorParameter->getLevel()
			&& uiLayer <= m_pcExtractorParameter->getLayer());//m_abKeepQualityLevelSEI[uiSEI];
        //bKeepSEI = bKeep;
		//bPrevPacketWasSEI = true;
		uiSEISize = uiPacketSize;
        if(bKeep)
            m_aadTargetByteForFrame[uiCurrFrame][uiLayer] -= uiSEISize;
		uiTotalSEI += uiPacketSize;
		uiSEI++;
	}
	else
	{
        //look if parameter sets NAL Unit
        UInt eNalUnitType = cPacketDescription.NalUnitType;
        if(eNalUnitType == NAL_UNIT_SPS || eNalUnitType == NAL_UNIT_PPS)
        {
            Bool bRequired = false;
            if(  eNalUnitType == NAL_UNIT_SPS )
            {
                for( UInt layer = 0; layer <= m_pcExtractorParameter->getLayer(); layer ++ )
                {
                    if( m_cScalableStreamDescription.m_bSPSRequired[layer][cPacketDescription.SPSid] )
                    {
                        bRequired = true;
                        break;
                    }
                }
            bKeep = bRequired;
            }
            else if( eNalUnitType == NAL_UNIT_PPS )
            {
                for( UInt layer = 0; layer <= m_pcExtractorParameter->getLayer(); layer ++ )
                {
                    if( m_cScalableStreamDescription.m_bPPSRequired[layer][cPacketDescription.PPSid] )
                    {
                        bRequired = true;
                        break;
                    }
                }
            bKeep = bRequired;
            }
        }
        else
        {
            //remove Dead substream SEI NAL
		    if(bDSSEI)
		    {
			    bKeep = false;
			    bCrop = false;
		    }
		    else
		    {
			    //============ check packet ===========
			    Double dRemainingBytes = m_aadTargetByteForFrame[uiCurrFrame][uiLayer];
				if (uiLayer>m_pcExtractorParameter->getLayer())
		        {
		            bKeep=false;
		            bCrop=false;
		        }
		        else
		        {
			        if (dRemainingBytes <=0 )
			        { // rate has been already generated
				        //printf("removing packet - rate has been reached\n");
				        bKeep = false;
				        bCrop = false;
			        }
			        else if (dRemainingBytes >= uiPacketSize)
			        { // packet can be fully kept
			            //printf("keeping full packet\n");
			            bKeep = true;
			            bCrop = false;
			        }
		            else
		            { // packet should be truncated
			            //printf("cropping packet\n");
		                bKeep = true;
		                bCrop = true;
		                uiShrinkSize = uiPacketSize - (UInt)floor(dRemainingBytes+0.49);
			            dTot += dRemainingBytes;
		            }
      
			    // remove size for target
			    m_aadTargetByteForFrame[uiCurrFrame][uiLayer] -= uiPacketSize;
		        }

		        if( bCrop && uiFGSLayer == 0 )
		        {
			        //NC: keep if uiFGSLayer == 0
			        bKeep = true;
			        bCrop = false;
			        //~NC
		        }
		    }
	    }
    }
    if( bCrop )
    {
      if( uiPacketSize - uiShrinkSize > 13 ) // ten bytes should be enough for the slice headers
      {
        RNOK( pcBinData->decreaseEndPos( uiShrinkSize ) );
#if FGS_ORDER
        pcBinData->data()[0]                    |= 0x80; // set forbidden bit
        pcBinData->data()[pcBinData->size()-1]  |= 0x01; // trailing one
#endif
        totalPackets += (uiPacketSize - uiShrinkSize);
      }
      else
      {
        bKeep = bCrop = false;
      }

    }
	else
		if(bKeep)
		{
			totalPackets += uiPacketSize;
			dTot += uiPacketSize;
		}

    uiNumInput++;
    if( bKeep ) uiNumKept   ++;
    if( bCrop ) uiNumCropped++;

    //============ write and release packet ============
    if( bKeep )
    {
      RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
      RNOK( m_pcWriteBitstream->writePacket( pcBinData ) );
    }
    RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
	
  }

  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );
	
  printf(" totalPackets %4lf \n ", totalPackets);

  printf("\n\nNumber of input packets:  %d\nNumber of output packets: %d (cropped: %d)\n\n", uiNumInput, uiNumKept, uiNumCropped );
  
  printf("total SEI in bitstream: %d \n ",uiTotalSEI);

  return Err::m_nOK;
}

Void Extractor::CalculateMaxRate(UInt uiLayer)
{
	UInt uiNumImage;
	UInt ui;
    for(uiNumImage = 0; uiNumImage < m_auiNbImages[uiLayer]; uiNumImage++)
	  {
		  UInt maxRate = 0;
		  for(ui = 0; ui < MAX_FGS_LAYERS; ui++)
		  {
			  maxRate += (UInt)m_aaadBytesForFrameFGS[uiLayer][uiNumImage][ui];
		  }
		  
		  m_aaadMaxRate[uiLayer][uiNumImage] = maxRate;
	  }
}

Void Extractor::setLevel(UInt                    uiLayer,
                      UInt                    uiLevel,
                      UInt                    uiNumImage)
{
	   m_aaiLevelForFrame[uiLayer][uiNumImage] = uiLevel; 
}

Void Extractor::setMaxRateDS(UInt                    uiNumBytes,
						 UInt				  uiMaxRate,
                      UInt                    uiLayer,
                      UInt                    uiFGSLayer,
					  UInt					  uiNumImage,
					  Bool					   bMaxRate )
{
   //if(uiFGSLayer == 0 && bMaxRate == true)
   //{
	   m_aaadMaxRate[uiLayer][uiNumImage] = (Double)uiMaxRate;
	   //m_aaadBytesForFrameFGS[uiLayer][uiNumImage][uiFGSLayer] += uiNumBytes;
   //}
}

Void Extractor::addPacket(UInt                    uiLayer,
                      UInt                    uiNumImage,
                      UInt                    uiFGSLayer,
                      UInt                    uiNumBytes)
{
  m_aaadBytesForFrameFGS[uiLayer][uiNumImage][uiFGSLayer] += uiNumBytes;
}


ErrVal
Extractor::xSetParameters_DS()
{
  UInt  uiLayer, uiLevel, uiFGSLayer;
  UInt uiNFrames;

  //=========== clear all ===========
  for( uiLayer = 0; uiLayer <  MAX_LAYERS;  uiLayer++ )
  {
      for( uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
      {
	    m_aaadMaxRateForLevel[uiLayer][uiLevel] = 0;
	    for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
	    { 
		      //calculate size of max rate for each level
		    if(m_aaiLevelForFrame[uiLayer][uiNFrames] == uiLevel)
		    {
			    m_aaadMaxRate[uiLayer][uiNFrames] -= m_aaadBytesForFrameFGS[uiLayer][uiNFrames][0];
			    m_aaadMaxRateForLevel[uiLayer][uiLevel] += m_aaadMaxRate[uiLayer][uiNFrames];
			    //initialize target rate for each frame per layer per FGSLayer to -1
			    for(uiFGSLayer = 0; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer++)
	            {
	 	            m_aaadTargetBytesFGS[uiLayer][uiNFrames][uiFGSLayer] = -1;
		        }
		    }
        }
      }
  }
  

#define ERROR(x,t)   {if(x) {::printf("\nERROR:   %s\n",t); assert(0); return Err::m_nERR;} }
#define WARNING(x,t) {if(x) {::printf("\nWARNING: %s\n",t); } }


  const MyList<ExtractorParameter::Point>&          rcExtList   = m_pcExtractorParameter->getExtractionList();
  ROT( rcExtList.size() != 1 );
  MyList<ExtractorParameter::Point>::const_iterator cIter       = rcExtList.begin ();
  MyList<ExtractorParameter::Point>::const_iterator cEnd        = rcExtList.end   ();
  const ExtractorParameter::Point&                  rcExtPoint  = *cIter;
  UInt                                              uiExtLayer  = MSYS_UINT_MAX;
  UInt                                              uiExtLevel  = MSYS_UINT_MAX;
  //----- layer -----
  for( uiLayer = 0; uiLayer <= m_cScalableStreamDescription.getNumberOfLayers(); uiLayer++ )
  {
    if( rcExtPoint.uiWidth  == m_cScalableStreamDescription.getFrameWidth (uiLayer) &&
        rcExtPoint.uiHeight == m_cScalableStreamDescription.getFrameHeight(uiLayer)    )
    {
      uiExtLayer = uiLayer;
      break;
    }
  }
  m_pcExtractorParameter->setLayer(uiExtLayer);
  ERROR( uiExtLayer==MSYS_UINT_MAX, "Spatial resolution of extraction/inclusion point not supported" );
  //--- level ---
  for( uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    if( rcExtPoint.dFrameRate == (Double)(1<<uiLevel)*m_cScalableStreamDescription.getFrameRateUnit() )
    {
      uiExtLevel = uiLevel;
      break;
    }
  }
  m_pcExtractorParameter->setLevel(uiExtLevel);
  ERROR( uiExtLevel==MSYS_UINT_MAX, "Temporal resolution of extraction/inclusion point not supported" );
  ERROR( uiExtLevel>m_cScalableStreamDescription.getMaxLevel(uiExtLayer), "Spatio-temporal resolution of extraction/inclusion point not supported" );
  //--- target number of bytes -----
  Double  dTargetNumExtBytes  = rcExtPoint.dBitRate / 8.0 * 1000.0 / ((Double)(1<<uiExtLevel)*m_cScalableStreamDescription.getFrameRateUnit() ) * (Double)m_cScalableStreamDescription.getNumPictures(uiExtLayer,uiExtLevel);

  //initialization of booleans to indicate if deadsubstream 
  // has to be removed
  Bool bKeepSize = false;//if true, target rate does not include size of dead substreams
						// if false, target rate includes size of dead substreams
  Bool bKeepAllDS = true;
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  //remove dead substream of output layer
	  //if(uiLayer == uiExtLayer)
        //  m_abRemoveMaxRateNAL[uiLayer] = true;
	  //determine if all deadsubstreams are kept or removed
	  if(m_bExtractDeadSubstream[uiLayer])
		  bKeepAllDS = false;
  }

  //calculate minimum size of output stream if dead substreams are kept:
  //minimum size = BL (all layer) + FGS (included layers)
  //if minimum size of output stream is to high compared to target rate
  // dead substreams are thrown away
  if(m_bInInputStreamDS)
  {
	//if(!m_bExtractDeadSubstream[0] || !m_bExtractDeadSubstream[1] )
    if(bKeepAllDS)
	{
		Double dMinOutputSize = 0;
 		for(uiLayer = 0; uiLayer < uiExtLayer; uiLayer ++)
		{
			for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames ++)
			{
				for(uiFGSLayer = 0; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer++)
				{
					if(m_aaiLevelForFrame[uiLayer][uiNFrames] <= uiExtLevel)
						dMinOutputSize += m_aaadBytesForFrameFGS[uiLayer][uiNFrames][uiFGSLayer];
				}
			}
		}
		for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames ++)
		{
			if(m_aaiLevelForFrame[uiExtLayer][uiNFrames] <= uiExtLevel)
				dMinOutputSize += m_aaadBytesForFrameFGS[uiExtLayer][uiNFrames][0];
		}
		if(dMinOutputSize >= dTargetNumExtBytes)
		{
			printf(" cannot keep deadsubstream to generate bitstream \n ");
			  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
			  {
				m_bExtractDeadSubstream[uiLayer] = true;
			  }
			  bKeepSize = true;
		}	
	}
  }//end if(m_bDSInInputStream)
  else
  { //to be sure that we do not want to extract something that is not in the stream
	  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
	  {
		m_bExtractDeadSubstream[uiLayer] = false;
	  }
  }

  //===== get and set required base layer packets ======
  Double  dRemainingBytes     = dTargetNumExtBytes;

  //removed size of dead substreams from target rate:
  //here, we can keep dead substreams in the output stream
  //so now we calculate the remaining bytes to pass the rest
  //of the datas (target rate - size Of Dead substreams)
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  if(m_bExtractDeadSubstream[uiLayer])
	  {
		  if(!bKeepSize && m_bInInputStreamDS)
			dRemainingBytes -= m_aSizeDeadSubstream[uiLayer];
	  }
  }

  //set base layer packets for all layers
  for( uiLayer = 0; uiLayer <= uiExtLayer; uiLayer++ )
  {
      for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
      {
        Int64 i64NALUBytes                    = m_cScalableStreamDescription.getNALUBytes( uiLayer, uiLevel, 0 );
        dRemainingBytes                      -= (Double)i64NALUBytes;
	    for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
	    {
	    	if(m_aaiLevelForFrame[uiLayer][uiNFrames] <= uiExtLevel)
		    {
             m_aaadTargetBytesFGS[uiLayer][uiNFrames][0] = 1;
		    }
	    }
      }
  }

  if( dRemainingBytes < 0.0 )
  {
    WARNING( true, "Bit-rate overflow for extraction/inclusion point" );
    return Err::m_nOK;
  }


  //===== set maximum possible bytes for included layers ======
  Bool bStop = false;
  for( uiLayer = 0; uiLayer <  uiExtLayer; uiLayer++ )
  {
    if(m_bExtractDeadSubstream[uiLayer])
  {
	  //in this case, we want to remove dead substreams from the input stream
	  //for each level, we will try to pass all the datas except the dead substreams
	  //so for each level, we will try to pass at most max rate of the level
	  //if remaining bytes is not enough to pass max rate for each level: each level
	  //will be truncated at a fraction corresponding to remaining bytes and size of FGS level
	  //else if remaining bytes is enough to pass max rate for each level: each level
	  //will be truncated at a fraction corresponding to max rate and size of FGS level
	  for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
    {
     bStop = false;
      for( uiFGSLayer = 1; (uiFGSLayer < MAX_FGS_LAYERS && bStop == false); uiFGSLayer++ )
      {
		  Int64 i64NALUBytes = m_cScalableStreamDescription.getNALUBytes( uiLayer, uiLevel, uiFGSLayer );
		  if(i64NALUBytes < m_aaadMaxRateForLevel[uiLayer][uiLevel])
		  {
			//size of FGS for this level is lower than max rate of this level
              if( (Double)i64NALUBytes <= dRemainingBytes )
              {
				  //size of FGS for this level is lower than remaining bytes
                 dRemainingBytes                      -= (Double)i64NALUBytes;
		         for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
		         {
					 if(m_aaiLevelForFrame[uiLayer][uiNFrames] == uiLevel)
			       {
				     //for each  frame of this level, FGS layer is kept 
                     m_aaadTargetBytesFGS[uiLayer][uiNFrames][uiFGSLayer] = 1;
					 //remove size of FGS of the frame from max rate of the frame
					 m_aaadMaxRate[uiLayer][uiNFrames] -= m_aaadBytesForFrameFGS[uiLayer][uiNFrames][uiFGSLayer];
				   }
				 }
                //removed size of FGS for this level from max rate for this level
				m_aaadMaxRateForLevel[uiLayer][uiLevel] -=  (Double)i64NALUBytes;
			  }
              else
              {
				//size of FGS for this level is higher than remaining bytes
                //====== set fractional FGS layer and exit =====
				Double  dFGSLayer = dRemainingBytes / (Double)i64NALUBytes;
				for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
				{
					if(m_aaiLevelForFrame[uiLayer][uiNFrames] == uiLevel)
					{
						//FGS of this frame from this level is truncated
						m_aaadTargetBytesFGS[uiLayer][uiNFrames][uiFGSLayer] = dFGSLayer;
					}
				}
				return Err::m_nOK;
			  }
		  }
		  else
		  {
			  //size of FGS for this level is higher than max rate of this level
			  //only max rate of this level will be passed
			  if( m_aaadMaxRateForLevel[uiLayer][uiLevel] <= dRemainingBytes )
              {
			     //max rate of this level is lower than remaining bytes
                 dRemainingBytes                      -= (Double)m_aaadMaxRateForLevel[uiLayer][uiLevel];
				 m_aaadMaxRateForLevel[uiLayer][uiLevel] -= m_aaadMaxRateForLevel[uiLayer][uiLevel];
		         for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
		         {
			       if(m_aaiLevelForFrame[uiLayer][uiNFrames] == uiLevel)
			       {
					   if( m_aaadMaxRate[uiLayer][uiNFrames] >= m_aaadBytesForFrameFGS[uiLayer][uiNFrames][uiFGSLayer])
					   {
						   //if max rate of the frame for this level is higher than size of FGS of the frame
						   //FGS is entirely kept 
						   m_aaadTargetBytesFGS[uiLayer][uiNFrames][uiFGSLayer] = 1;
					   }
					   else
					   {
						   //max rate of the frame is lower than size of FGS for the frame
						   //FGS of the frame is truncated
                           Double dFGSLayer = m_aaadMaxRate[uiLayer][uiNFrames]/m_aaadBytesForFrameFGS[uiLayer][uiNFrames][uiFGSLayer];
                           m_aaadTargetBytesFGS[uiLayer][uiNFrames][uiFGSLayer] = dFGSLayer;
					   }
				   }
				 }
				 //we have found the FGS layer corresponding to max rate of this level
				 //no more FGS will be passed for this level
				 bStop = true;
			  }
              else
              {
                //max rate of this level is higher than remaining bytes
			    //FGS of this level will be truncated
                //====== set fractional FGS layer and exit =====
				Double  dFGSLayer = dRemainingBytes / m_aaadMaxRateForLevel[uiLayer][uiLevel];
				for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
				{
					if(m_aaiLevelForFrame[uiLayer][uiNFrames] == uiLevel)
					{
						m_aaadTargetBytesFGS[uiLayer][uiNFrames][uiFGSLayer] = dFGSLayer;
					}
				}
				return Err::m_nOK;
			  }
		  }
	  } //end for uiFGSLayer
	} //end for uiLevel
	 }//if extractDeadSubstream
  else
  {
   //here we want to keep dead substreams in the output stream
    for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
    {
      for( uiFGSLayer = 1; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer++ )
      {
        Int64 i64NALUBytes = m_cScalableStreamDescription.getNALUBytes( uiLayer, uiLevel, uiFGSLayer );
        if( (Double)i64NALUBytes <= dRemainingBytes )
        {
		  //Size of FGS for this level is lower than remaining bytes
		  // FGS of all frames of this level is kept
          dRemainingBytes                      -= (Double)i64NALUBytes;
		  for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
		  {
			  if(m_aaiLevelForFrame[uiLayer][uiNFrames] == uiLevel)
			  {
                  m_aaadTargetBytesFGS[uiLayer][uiNFrames][uiFGSLayer] = 1;
			  }
		  }
        }
        else
        {
		  //Size of FGS for this level is higher than remaining bytes
		  //FGS of all frames of this layer is truncated
          //====== set fractional FGS layer and exit =====
          Double  dFGSLayer = dRemainingBytes / (Double)i64NALUBytes;
		  for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
		  {
			  if(m_aaiLevelForFrame[uiLayer][uiNFrames] == uiLevel)
			     {
				 m_aaadTargetBytesFGS[uiLayer][uiNFrames][uiFGSLayer] = dFGSLayer;
				 }
		  }
          return Err::m_nOK;
        }
      }
    }
  }
  }

  //===== set FGS layer for current layer =====
  for( uiFGSLayer = 1; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer++ )
  {
    Int64 i64FGSLayerBytes = 0;
    for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
    {
      i64FGSLayerBytes += m_cScalableStreamDescription.getNALUBytes( uiExtLayer, uiLevel, uiFGSLayer );
    }
    if( (Double)i64FGSLayerBytes <= dRemainingBytes )
    {
      //Size of FGS of all levels is lower than remaining bytes
	  //FGS of all frames from all levels is kept
      dRemainingBytes -= (Double)i64FGSLayerBytes;
      for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
      {
		 for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
		  {
			  if(m_aaiLevelForFrame[uiLayer][uiNFrames] == uiLevel)
			     {
					m_aaadTargetBytesFGS[uiExtLayer][uiNFrames][uiFGSLayer] = 1;
				 }
		  }
      }
    }
    else
    {
	 //Size of FGs of all levels is higher than remaining bytes
	 //FGS of all frames from all levels is truncated
      Double dFGSLayer = dRemainingBytes / (Double)i64FGSLayerBytes;
      for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
      {
		for(uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
		  {
			  if(m_aaiLevelForFrame[uiLayer][uiNFrames] == uiLevel)
			     {
					m_aaadTargetBytesFGS[uiExtLayer][uiNFrames][uiFGSLayer] = dFGSLayer;
				  }
		  }
      }
      return Err::m_nOK;
    }
  }
  WARNING( dRemainingBytes>0.0, "Bit-rate underflow for extraction/inclusion point" );


#undef ERROR
#undef WARNING

  return Err::m_nOK;
}


Void Extractor::CalculateSizeDeadSubstream()
{
	Double sumFGS = 0.0;
	Double sumMaxRate = 0.0;
	for(UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
	{
		sumFGS = 0.0;
		sumMaxRate = 0.0;
		for(UInt uiNFrames = 0; uiNFrames < MAX_NBFRAMES; uiNFrames++)
		{
            for(UInt uiFGS = 1; uiFGS < MAX_FGS_LAYERS; uiFGS++)
			{	
				sumFGS+= m_aaadBytesForFrameFGS[uiLayer][uiNFrames][uiFGS];
			}
			sumMaxRate += (m_aaadMaxRate[uiLayer][uiNFrames]-m_aaadBytesForFrameFGS[uiLayer][uiNFrames][0]);
		}
		m_aSizeDeadSubstream[uiLayer] = (Int)floor( sumFGS - sumMaxRate );
	}
}

ErrVal
Extractor::xExtractPoints_DS()
{
  UInt  uiNumInput    = 0;
  UInt  uiNumKept     = 0;
  UInt  uiNumCropped  = 0;
  Bool  bKeep         = false;
  Bool  bApplyToNext  = false;
  Bool  bEOS          = false;
  Bool  bCrop         = false;

  UInt  uiLayer       = 0;
  UInt  uiLevel       = 0;
  UInt  uiFGSLayer    = 0;
  UInt  uiPacketSize  = 0;
  UInt  uiShrinkSize  = 0;
  Bool  bNewPicture   = false;
  Bool  bSEIPacket    = true;

  Int currFrame[MAX_LAYERS];
  //count number of frame per layer
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	    currFrame[uiLayer] = 0;
  }

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  while( ! bEOS )
  {
    //=========== get packet ===========
    BinData*  pcBinData;
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }

    //========== get packet description ==========
    h264::SEI::SEIMessage*  pcScalableSEIMessage = 0;
    h264::PacketDescription cPacketDescription;
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEIMessage ) );
    delete pcScalableSEIMessage;

    //============ get packet size ===========
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    uiPacketSize  = 4 + pcBinData->size();
    uiShrinkSize  = 0;

    //============ set parameters ===========
	uiLayer    = cPacketDescription.Layer;
    uiLevel    = cPacketDescription.Level;
    uiFGSLayer = cPacketDescription.FGSLayer;
    bApplyToNext = cPacketDescription.ApplyToNext;
	bNewPicture   = ( ! cPacketDescription.ParameterSet && ! cPacketDescription.ApplyToNext );
	//first packet is an SEI packet
	if (bSEIPacket)
    {
      bNewPicture = false;
    }

	// update frame number
    if (bNewPicture && uiFGSLayer == 0)
    {
      currFrame[uiLayer]++;
    }

	UInt uiCurrFrame = 0;
	Bool bParameterSet = cPacketDescription.ParameterSet;
	if(bParameterSet || bSEIPacket)
	{
		uiCurrFrame = 0;
		bSEIPacket = false;
	}
	else
	{
		if(!bApplyToNext)
			uiCurrFrame = currFrame[uiLayer]-1;//[uiFGSLayer];
		else
			uiCurrFrame = currFrame[uiLayer];
	}
	
    if(cPacketDescription.MaxRateDS != 0 && (m_bExtractDeadSubstream[uiLayer] || 
		uiLayer >= m_pcExtractorParameter->getLayer()||
		m_aaiLevelForFrame[uiLayer][uiCurrFrame] > m_pcExtractorParameter->getLevel()))
		//m_abRemoveMaxRateNAL[uiLayer]))
	{
		//Dead substream SEI NAL that we don't want to keep 
		bKeep = false;
		bCrop = false;
	}
	else
	{
		//look if SPS or PPS nal unit
		UInt eNalUnitType = cPacketDescription.NalUnitType;
		if( eNalUnitType == NAL_UNIT_SPS || eNalUnitType == NAL_UNIT_PPS)
		{
            Bool bRequired = false;
			if(  eNalUnitType == NAL_UNIT_SPS )
			{
				for( UInt layer = 0; layer <= m_pcExtractorParameter->getLayer(); layer ++ )
				{
					if( m_cScalableStreamDescription.m_bSPSRequired[layer][cPacketDescription.SPSid] )
					{
						bRequired = true;
						break;
					}
				}
				bKeep = bRequired;
			}
			else if( eNalUnitType == NAL_UNIT_PPS )
			{
				for( UInt layer = 0; layer <= m_pcExtractorParameter->getLayer(); layer ++ )
				{
					if( m_cScalableStreamDescription.m_bPPSRequired[layer][cPacketDescription.PPSid] )
					{
						bRequired = true;
						break;
					}
				}
				bKeep = bRequired;
			}
		}
		else
		{
			if(m_aaadTargetBytesFGS[uiLayer][uiCurrFrame][uiFGSLayer] == -1)
			{
				//NAL is thrown away
				bKeep  = false;
				bCrop = false;
			}
			if(m_aaadTargetBytesFGS[uiLayer][uiCurrFrame][uiFGSLayer] == 1)
			{
				//NAL is kept
				bKeep = true;
				bCrop = false;
			}
			if(m_aaadTargetBytesFGS[uiLayer][uiCurrFrame][uiFGSLayer] != -1 && 
				m_aaadTargetBytesFGS[uiLayer][uiCurrFrame][uiFGSLayer] != 1)
			{
				//NAL is truncated
				Double dWeight = m_aaadTargetBytesFGS[uiLayer][uiCurrFrame][uiFGSLayer];
				uiShrinkSize        = uiPacketSize - (UInt)ceil( (Double)uiPacketSize * dWeight );
				if(uiPacketSize - uiShrinkSize > 13)
				{
					RNOK( pcBinData->decreaseEndPos( uiShrinkSize ) );
					pcBinData->data()[pcBinData->size()-1]  |= 0x01; // trailing one
					bKeep = true;
					bCrop = true;
					if(uiFGSLayer == 0 && bKeep && bCrop)
					{
						bKeep = true;
						bCrop = false;
					}
				}
				else
				{
					bKeep = false;
					bCrop = false;
				}
			}
		}
	}

    uiNumInput++;
    if( bKeep ) uiNumKept   ++;
    if( bCrop ) uiNumCropped++;


    //============ write and release packet ============
    if( bKeep )
    {
      RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
      RNOK( m_pcWriteBitstream->writePacket( pcBinData ) );
    }
    RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
  }

  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );


  printf("\n\nNumber of input packets:  %d\nNumber of output packets: %d (cropped: %d)\n\n", uiNumInput, uiNumKept, uiNumCropped );

  return Err::m_nOK;
}

//}}Quality level estimation and modified truncation- JVTO044 and m12007






ScalableStreamDescription::ScalableStreamDescription()
: m_bInit     ( false )
, m_bAnalyzed ( false )
{
}

ScalableStreamDescription::~ScalableStreamDescription()
{
}

ErrVal
ScalableStreamDescription::init( h264::SEI::ScalableSei* pcScalableSei )
{
	ROT( m_bInit );

	::memset( m_aaaui64NumNALUBytes, 0x00, MAX_LAYERS*(MAX_DSTAGES+1)*MAX_QUALITY_LEVELS*sizeof(UInt64) );
	::memset( m_aauiNumPictures,		 0x00, MAX_LAYERS*(MAX_DSTAGES+1)										*sizeof(UInt)   );

	::memset( m_auiBitrate,					 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_auiTempLevel,				 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_auiDependencyId,		 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_auiQualityLevel,		 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_adFramerate,				 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_auiFrmWidth,				 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_auiFrmHeight,				 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );

	::memset( m_aaauiScalableLayerId,0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_aaauiBitrate,				 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_aaauiTempLevel,			 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_aaauiDependencyId,	 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_aaauiQualityLevel,	 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_aaadFramerate,			 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_aaauiFrmWidth,			 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );
	::memset( m_aaauiFrmHeight,			 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(UInt)   );

	m_uiScalableNumLayersMinus1 = pcScalableSei->getNumLayersMinus1();
	UInt m_uiMaxWidth;
	UInt m_uiMaxHeight;
	if( pcScalableSei->getDecodingDependencyInfoPresentFlag( m_uiScalableNumLayersMinus1 ) )
	{
		m_uiNumLayers			=	pcScalableSei->getDependencyId( m_uiScalableNumLayersMinus1 ) + 1;
		m_uiMaxDecStages	= pcScalableSei->getTemporalLevel( m_uiScalableNumLayersMinus1 ) + 1;
	}
	if( pcScalableSei->getFrmSizeInfoPresentFlag( m_uiScalableNumLayersMinus1 ) )
	{
		m_uiMaxWidth			= ( pcScalableSei->getFrmWidthInMbsMinus1 ( m_uiScalableNumLayersMinus1 ) + 1 ) << 4;
		m_uiMaxHeight			= ( pcScalableSei->getFrmHeightInMbsMinus1( m_uiScalableNumLayersMinus1 ) + 1 ) << 4; 
	}

	m_bAVCBaseLayer = false;
	m_uiAVCTempResStages = 0;
	m_uiFrameRateUnitDenom = 4;
	m_uiFrameRateUnitNom = 15;

	UInt uiLayer = 0;
	UInt uiLevel = 0;
	UInt uiFGS	 = 0;
	for ( UInt uiScalableLayer = 0; uiScalableLayer <= m_uiScalableNumLayersMinus1; uiScalableLayer++ )
	{
		if( pcScalableSei->getDecodingDependencyInfoPresentFlag( uiScalableLayer ) )
		{
			uiLayer = pcScalableSei->getDependencyId( uiScalableLayer );
			uiLevel = pcScalableSei->getTemporalLevel( uiScalableLayer );
			uiFGS		= pcScalableSei->getQualityLevel( uiScalableLayer );
		}
		
		m_auiFrameWidth [ uiLayer ] = pcScalableSei->getFrmSizeInfoPresentFlag( uiScalableLayer ) ?
																( pcScalableSei->getFrmWidthInMbsMinus1		(uiScalableLayer)+1 ) << 4 : 0;
		m_auiFrameHeight[ uiLayer ] = pcScalableSei->getFrmSizeInfoPresentFlag( uiScalableLayer ) ?
																( pcScalableSei->getFrmHeightInMbsMinus1	(uiScalableLayer)+1 ) << 4 : 0;
		m_auiDecStages	[ uiLayer ] = pcScalableSei->getDecodingDependencyInfoPresentFlag( uiScalableLayer ) ?
																	pcScalableSei->getTemporalLevel( uiScalableLayer )	  : 0;

		m_aaauiScalableLayerId[uiLayer][uiLevel][uiFGS] = uiScalableLayer;
		m_aaauiTempLevel			[uiLayer][uiLevel][uiFGS] = uiLevel;
		m_aaauiDependencyId		[uiLayer][uiLevel][uiFGS] = uiLayer;
		m_aaauiQualityLevel		[uiLayer][uiLevel][uiFGS] = uiFGS;

		m_aaadFramerate				[uiLayer][uiLevel][uiFGS] = pcScalableSei->getFrmRateInfoPresentFlag( uiScalableLayer ) ?
																				pcScalableSei->getAvgFrmRate( uiScalableLayer )/256.0 : 0;
		m_aaauiBitrate				[uiLayer][uiLevel][uiFGS] = pcScalableSei->getBitrateInfoPresentFlag( uiScalableLayer ) ?
																				pcScalableSei->getAvgBitrate( uiScalableLayer ) : 0;
		m_aaauiFrmWidth				[uiLayer][uiLevel][uiFGS] = pcScalableSei->getFrmSizeInfoPresentFlag( uiScalableLayer ) ?
																			( pcScalableSei->getFrmWidthInMbsMinus1		( uiScalableLayer ) + 1 ) << 4 : 0;
		m_aaauiFrmHeight			[uiLayer][uiLevel][uiFGS] = pcScalableSei->getFrmSizeInfoPresentFlag( uiScalableLayer ) ?
																			( pcScalableSei->getFrmHeightInMbsMinus1	( uiScalableLayer ) + 1 ) << 4 : 0;

		m_adFramerate			[ uiScalableLayer ] = m_aaadFramerate			[uiLayer][uiLevel][uiFGS];
		m_auiBitrate			[ uiScalableLayer ] = m_aaauiBitrate			[uiLayer][uiLevel][uiFGS];
		m_auiFrmWidth			[ uiScalableLayer ] = m_aaauiFrmWidth			[uiLayer][uiLevel][uiFGS];
		m_auiFrmHeight		[ uiScalableLayer ] = m_aaauiFrmHeight		[uiLayer][uiLevel][uiFGS];
		m_auiTempLevel		[ uiScalableLayer ] = m_aaauiTempLevel		[uiLayer][uiLevel][uiFGS];
		m_auiDependencyId	[ uiScalableLayer ] = m_aaauiDependencyId [uiLayer][uiLevel][uiFGS];
		m_auiQualityLevel [ uiScalableLayer ] = m_aaauiQualityLevel [uiLayer][uiLevel][uiFGS];

	}

	UInt uiNum, uiIndex;
	for( uiIndex = 0; uiIndex <= uiLayer; uiIndex++ )
	{
		for( uiNum = 0; uiNum < 32; uiNum ++ )
			m_bSPSRequired[uiIndex][uiNum] = false;
		for( uiNum = 0; uiNum < 256; uiNum ++ )
			m_bPPSRequired[uiIndex][uiNum] = false;
	}

	m_bInit = true;
	m_bAnalyzed = false;
	return Err::m_nOK;
}

ErrVal
ScalableStreamDescription::uninit()
{
  m_bInit     = false;
  m_bAnalyzed = false;

  return Err::m_nOK;
}

ErrVal
ScalableStreamDescription::addPacket( UInt  uiNumBytes,
                                      UInt  uiLayer,
                                      UInt  uiLevel,
                                      UInt  uiFGSLayer,
                                      Bool  bNewPicture )
{
  ROF( m_bInit      );
  ROT( m_bAnalyzed  );
  ROF( uiLayer    <  MAX_LAYERS         );
  ROF( uiLevel    <= MAX_DSTAGES        );
  ROF( uiFGSLayer <  MAX_QUALITY_LEVELS );

  m_aaaui64NumNALUBytes[uiLayer][uiLevel][uiFGSLayer] += uiNumBytes;

  if( bNewPicture && uiFGSLayer == 0 )
  {
    m_aauiNumPictures[uiLayer][uiLevel]++;
  }

  return Err::m_nOK;
}


ErrVal
ScalableStreamDescription::analyse()
{
  ROF( m_bInit );


  m_bAnalyzed = true;
  return Err::m_nOK;
}


Void
ScalableStreamDescription::output( FILE* pFile )
{
	printf("\nContained Layers:");
	printf("\n====================\n\n");

	printf( "       Layer" "   Resolution" "   Framerate" "   Bitrate" "      DTQ\n" );
for ( UInt uiScalableLayer = 0; uiScalableLayer <= m_uiScalableNumLayersMinus1; uiScalableLayer++ )
	{
		UInt uiFrameWidth		= m_auiFrmWidth							[uiScalableLayer];
		UInt uiFrameHeight	= m_auiFrmHeight						[uiScalableLayer];
		Double dFrameRate		= m_adFramerate							[uiScalableLayer];
		Double dBitrate			= (Double)m_auiBitrate			[uiScalableLayer];
		UInt uiDependencyId = m_auiDependencyId					[uiScalableLayer];
		UInt uiTempLevel		= m_auiTempLevel						[uiScalableLayer];
		UInt uiQualityLevel	= m_auiQualityLevel					[uiScalableLayer];
		
		printf( "       %3d     %3dx%3d      %7.4lf   %8.2lf     (%d,%d,%d) \n", 
			uiScalableLayer,uiFrameWidth, uiFrameHeight, dFrameRate, dBitrate, uiDependencyId, uiTempLevel, uiQualityLevel );
	}
	printf( "\n\n" );
}



