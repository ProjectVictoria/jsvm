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
#include "ControlMngH264AVCDecoder.h"


H264AVC_NAMESPACE_BEGIN

ControlMngH264AVCDecoder::ControlMngH264AVCDecoder()
: m_pcMbDataCtrl          ( NULL )
, m_pcUvlcReader          ( NULL )
, m_pcMbParser            ( NULL )
, m_pcMotionCompensation  ( NULL )
, m_pcCabacReader         ( NULL )
, m_pcSampleWeighting     ( NULL )
, m_uiCurrLayer           ( MSYS_UINT_MAX )
{
  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_auiMbXinFrame           [uiLayer] = 0;
    m_auiMbYinFrame           [uiLayer] = 0;
    m_apcLayerDecoder         [uiLayer] = NULL;
    m_apcYuvFullPelBufferCtrl [uiLayer] = NULL;
		m_uiInitialized           [uiLayer] = false;
  }
}


ControlMngH264AVCDecoder::~ControlMngH264AVCDecoder()
{
}


ErrVal
ControlMngH264AVCDecoder::init( UvlcReader*          pcUvlcReader,
                                MbParser*            pcMbParser,
                                MotionCompensation*  pcMotionCompensation,
                                YuvBufferCtrl*       apcYuvFullPelBufferCtrl [MAX_LAYERS],
                                CabacReader*         pcCabacReader,
                                SampleWeighting*     pcSampleWeighting,
                                LayerDecoder*        apcLayerDecoder         [MAX_LAYERS] )
{ 
  ROF( pcUvlcReader );
  ROF( pcMbParser );
  ROF( pcMotionCompensation );
  ROF( pcCabacReader );
  ROF( pcSampleWeighting );

  m_uiCurrLayer           = MSYS_UINT_MAX;
  m_pcUvlcReader          = pcUvlcReader; 
  m_pcMbParser            = pcMbParser; 
  m_pcMotionCompensation  = pcMotionCompensation; 
  m_pcCabacReader         = pcCabacReader; 
  m_pcSampleWeighting     = pcSampleWeighting; 

  ROF( apcLayerDecoder );
  ROF( apcYuvFullPelBufferCtrl );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    ROF( apcLayerDecoder        [uiLayer] );
    ROF( apcYuvFullPelBufferCtrl[uiLayer] );

    m_apcLayerDecoder         [uiLayer] = apcLayerDecoder         [uiLayer];
    m_apcYuvFullPelBufferCtrl [uiLayer] = apcYuvFullPelBufferCtrl [uiLayer];
  }
  
  return Err::m_nOK;
}


ErrVal
ControlMngH264AVCDecoder::uninit()
{
  return Err::m_nOK;
}


ErrVal
ControlMngH264AVCDecoder::create( ControlMngH264AVCDecoder*& rpcControlMngH264AVCDecoder )
{
  rpcControlMngH264AVCDecoder = new ControlMngH264AVCDecoder;
  ROF( rpcControlMngH264AVCDecoder );
  return Err::m_nOK;
}


ErrVal
ControlMngH264AVCDecoder::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal
ControlMngH264AVCDecoder::initMbForParsing( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  UInt uiMbY, uiMbX;

  uiMbY = uiMbIndex         / m_auiMbXinFrame[m_uiCurrLayer];
  uiMbX = uiMbIndex - uiMbY * m_auiMbXinFrame[m_uiCurrLayer];

  RNOK( m_pcMbDataCtrl                          ->initMb( rpcMbDataAccess, uiMbY, uiMbX ) );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX, false ) );

  return Err::m_nOK;
}

ErrVal
ControlMngH264AVCDecoder::initMbForDecoding( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  RNOK( m_pcMbDataCtrl                          ->initMb( rpcMbDataAccess, uiMbY, uiMbX                   ) );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX, bMbAff           ) );
  RNOK( m_pcMotionCompensation                  ->initMb(                  uiMbY, uiMbX, *rpcMbDataAccess ) ) ;

  return Err::m_nOK;
}

ErrVal
ControlMngH264AVCDecoder::initMbForDecoding( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX, bMbAff         ) );
  RNOK( m_pcMotionCompensation                  ->initMb( uiMbY, uiMbX, rcMbDataAccess ) ) ;

  return Err::m_nOK;
}

ErrVal
ControlMngH264AVCDecoder::initMbForFiltering( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  

  RNOK( m_pcMbDataCtrl                          ->initMb( rpcMbDataAccess, uiMbY, uiMbX           ) );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX, bMbAff         ) );

  return Err::m_nOK;
}


ErrVal
ControlMngH264AVCDecoder::initMbForFiltering( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff  )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb( uiMbY, uiMbX, bMbAff ) );
  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initSlice0( SliceHeader *rcSH )
{
  UInt  uiLayer = rcSH->getDependencyId();

  ROTRS( m_uiInitialized[uiLayer], Err::m_nOK );
  m_auiMbXinFrame[uiLayer]  = rcSH->getSPS().getFrameWidthInMbs   ();
  m_auiMbYinFrame[uiLayer]  = rcSH->getSPS().getFrameHeightInMbs  ();

  UInt uiSizeX = m_auiMbXinFrame  [uiLayer] << 4;
  UInt uiSizeY = m_auiMbYinFrame  [uiLayer] << 4;

  RNOK( m_apcYuvFullPelBufferCtrl [uiLayer]->initSlice( uiSizeY, uiSizeX, YUV_Y_MARGIN, YUV_X_MARGIN ) );

  RNOK( xInitESS( rcSH ) );
	m_uiInitialized[uiLayer] = true;

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initSPS( SequenceParameterSet& rcSequenceParameterSet, UInt  uiLayer )
{
  m_auiMbXinFrame[uiLayer]  = rcSequenceParameterSet.getFrameWidthInMbs   ();
  m_auiMbYinFrame[uiLayer]  = rcSequenceParameterSet.getFrameHeightInMbs  ();
  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::xInitESS( SliceHeader* pcSliceHeader )
{
  UInt uiLayer = pcSliceHeader->getDependencyId();

  if( pcSliceHeader->getQualityId() == 0)
  {
    pcSliceHeader->getSPS().getResizeParameters(&m_ResizeParameter[uiLayer]);
  }

  if( ! pcSliceHeader->getNoInterLayerPredFlag() )
  {
    UInt  uiBaseLayer = pcSliceHeader->getRefLayerDependencyId();

    ResizeParameters * curr = 0;
    if(pcSliceHeader->getQualityId() == 0)
    {
      curr = &m_ResizeParameter[uiLayer];
    }

    curr->m_iInWidth  = m_auiMbXinFrame  [uiBaseLayer] << 4;
    curr->m_iInHeight = m_auiMbYinFrame  [uiBaseLayer] << 4;

    bool is_crop_aligned = (curr->m_iPosX%16 == 0) && (curr->m_iPosY%16 == 0);
    if      ((curr->m_iInWidth == curr->m_iOutWidth) && (curr->m_iInHeight == curr->m_iOutHeight) &&
             is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
      curr->m_iSpatialScalabilityType = SST_RATIO_1;
    else if ((curr->m_iInWidth*2 == curr->m_iOutWidth) && (curr->m_iInHeight*2 == curr->m_iOutHeight) &&
             is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
      curr->m_iSpatialScalabilityType = SST_RATIO_2;
    else 
      curr->m_iSpatialScalabilityType = SST_RATIO_X;

    if ( curr->m_iExtendedSpatialScalability == ESS_NONE && curr->m_iSpatialScalabilityType > SST_RATIO_2 )
    {
      printf("\nControlMngH264AVCDecoder::initEES() - use of Extended Spatial Scalability not signaled\n");
      return Err::m_nERR;
    }

    if(pcSliceHeader->getQualityId() == 0)
    {
      m_apcLayerDecoder[uiLayer]->setResizeParameters(&m_ResizeParameter[uiLayer]);
    }

    if (curr->m_iExtendedSpatialScalability == ESS_SEQ)
    {
      printf("Extended Spatial Scalability - crop win: origin=(%3d,%3d) - size=(%3d,%3d)\n\n",
             curr->m_iPosX,curr->m_iPosY,curr->m_iOutWidth,curr->m_iOutHeight);
    }
    else if (curr->m_iExtendedSpatialScalability == ESS_PICT)
    {
      printf("Extended Spatial Scalability - crop win by picture\n\n");
    }
    
  }
	return Err::m_nOK;
}


ErrVal
ControlMngH264AVCDecoder::initSliceForReading( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getDependencyId();

  MbSymbolReadIf* pcMbSymbolReadIf;
  
  if( rcSH.getPPS().getEntropyCodingModeFlag() )
  {
    pcMbSymbolReadIf = m_pcCabacReader;
  }
  else
  {
    pcMbSymbolReadIf = m_pcUvlcReader;
  }

	if ( rcSH.isTrueSlice())
	{
		RNOK( pcMbSymbolReadIf->startSlice( rcSH ) );
	}
  RNOK( m_pcMbParser->initSlice( pcMbSymbolReadIf ) );

  return Err::m_nOK;
}


ErrVal
ControlMngH264AVCDecoder::initSliceForDecoding( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getDependencyId();

  RNOK( m_pcMotionCompensation->initSlice( rcSH ) );
  RNOK( m_pcSampleWeighting->initSlice( rcSH ) );

  return Err::m_nOK;
}


ErrVal
ControlMngH264AVCDecoder::initSliceForFiltering( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getDependencyId();
  return Err::m_nOK;
}


ErrVal
ControlMngH264AVCDecoder::finishSlice( const SliceHeader& rcSH, Bool& rbPicDone, Bool& rbFrameDone )
{
  rbPicDone     = m_pcMbDataCtrl->isPicDone( rcSH );
  rbFrameDone   = m_pcMbDataCtrl->isFrameDone( rcSH );
  m_uiCurrLayer = MSYS_UINT_MAX;
  
  return Err::m_nOK;
}

H264AVC_NAMESPACE_END
