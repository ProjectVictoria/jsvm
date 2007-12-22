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
#include "MbDecoder.h"
#include "SliceDecoder.h"
#include "H264AVCCommonLib/SliceHeader.h"
#include "DecError.h"

#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/Frame.h"

#include "H264AVCCommonLib/CFMO.h"

H264AVC_NAMESPACE_BEGIN


SliceDecoder::SliceDecoder()
: m_pcMbDecoder       ( 0 )
, m_pcControlMng      ( 0 )
#ifdef SHARP_AVC_REWRITE_OUTPUT
, m_pcRewriteEncoder  ( 0 )
#endif
, m_bInitDone         ( false )
{
}

SliceDecoder::~SliceDecoder()
{
}

ErrVal
SliceDecoder::create( SliceDecoder*& rpcSliceDecoder )
{
  rpcSliceDecoder = new SliceDecoder;
  ROT( NULL == rpcSliceDecoder );
  return Err::m_nOK;
}

ErrVal
SliceDecoder::destroy()
{
  ROT( m_bInitDone );
  delete this;
  return Err::m_nOK;
}

ErrVal
SliceDecoder::init( MbDecoder*        pcMbDecoder,
                    ControlMngIf*     pcControlMng
#ifdef SHARP_AVC_REWRITE_OUTPUT
                    ,RewriteEncoder*  pcRewriteEncoder
#endif
                    )
{
  ROT( m_bInitDone );
  ROF( pcMbDecoder );
  ROF( pcControlMng );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  ROF( pcRewriteEncoder );
#endif

  m_pcMbDecoder       = pcMbDecoder;
  m_pcControlMng      = pcControlMng;
#ifdef SHARP_AVC_REWRITE_OUTPUT
  m_pcRewriteEncoder  = pcRewriteEncoder;
#endif
  m_bInitDone         = true;
  return Err::m_nOK;
}


ErrVal
SliceDecoder::uninit()
{
  ROF( m_bInitDone );

  m_pcMbDecoder       = 0;
  m_pcControlMng      = 0;
#ifdef SHARP_AVC_REWRITE_OUTPUT
  m_pcRewriteEncoder  = 0;
#endif
  m_bInitDone         = false;
  return Err::m_nOK;
}


ErrVal
SliceDecoder::decode( SliceHeader&  rcSH,
                      MbDataCtrl*   pcMbDataCtrl,
                      MbDataCtrl*   pcMbDataCtrlBase,
                      Frame*        pcFrame,
                      Frame*        pcResidual,
                      Frame*        pcBaseLayer,
                      Frame*        pcBaseLayerResidual,
                      RefFrameList* pcRefFrameList0,
                      RefFrameList* pcRefFrameList1,
                      MbDataCtrl*   pcMbDataCtrl0L1,
                      Bool          bReconstructAll )
{
  ROF( m_bInitDone );

  //====== initialization ======
  RNOK( pcMbDataCtrl->initSlice( rcSH, DECODE_PROCESS, true, pcMbDataCtrl0L1 ) );

  const PicType ePicType = rcSH.getPicType();
	if( ePicType != FRAME )
	{
		if( pcFrame )             RNOK( pcFrame->            addFieldBuffer( ePicType ) );
		if( pcResidual )          RNOK( pcResidual->         addFieldBuffer( ePicType ) );
		if( pcBaseLayer )         RNOK( pcBaseLayer->        addFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->addFieldBuffer( ePicType ) );
	}

  //===== loop over macroblocks =====
  UInt uiMbAddress     = rcSH.getFirstMbInSlice();
  UInt uiLastMbAddress = rcSH.getFirstMbInSlice() + rcSH.getNumMbsInSlice() - 1;
  while( uiMbAddress <= uiLastMbAddress )
  {
    MbDataAccess* pcMbDataAccess     = NULL;
    MbDataAccess* pcMbDataAccessBase = NULL;
    UInt          uiMbY, uiMbX;

    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress                            );

    RNOK( pcMbDataCtrl  ->initMb            (  pcMbDataAccess,     uiMbY, uiMbX ) );
    if( pcMbDataCtrlBase )
    {
      RNOK( pcMbDataCtrlBase->initMb        (  pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    RNOK( m_pcControlMng->initMbForDecoding ( *pcMbDataAccess,     uiMbY, uiMbX, false ) );

    RNOK( m_pcMbDecoder ->decode            ( *pcMbDataAccess,
                                              pcMbDataAccessBase,
                                              pcFrame                                  ->getPic( ePicType ),
                                              pcResidual                               ->getPic( ePicType ),
                                              pcBaseLayer         ? pcBaseLayer        ->getPic( ePicType ) : NULL,
                                              pcBaseLayerResidual ? pcBaseLayerResidual->getPic( ePicType ) : NULL,
                                              pcRefFrameList0,
                                              pcRefFrameList1,
                                              bReconstructAll ) );

#ifdef SHARP_AVC_REWRITE_OUTPUT
    RNOK( xRewriteMb( *pcMbDataAccess, pcMbDataAccessBase, uiMbAddress == uiLastMbAddress ) );
#endif

  	if( rcSH.isTrueSlice())
	  {
		  uiMbAddress=rcSH.getFMO()->getNextMBNr(uiMbAddress);
	  }
	  else
	  {
		  uiMbAddress++;
	  }
  }

  if( ePicType!=FRAME )
	{
		if( pcFrame )             RNOK( pcFrame->            removeFieldBuffer( ePicType ) );
		if( pcResidual )          RNOK( pcResidual->         removeFieldBuffer( ePicType ) );
		if( pcBaseLayer )         RNOK( pcBaseLayer->        removeFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->removeFieldBuffer( ePicType ) );
	}

  return Err::m_nOK;
}


ErrVal
SliceDecoder::decodeMbAff( SliceHeader&   rcSH,
                           MbDataCtrl*    pcMbDataCtrl,
                           MbDataCtrl*    pcMbDataCtrlBase,
                           MbDataCtrl*    pcMbDataCtrlBaseField,
                           Frame*         pcFrame,
                           Frame*         pcResidual,
                           Frame*         pcBaseLayer,
                           Frame*         pcBaseLayerResidual,
                           RefFrameList*  pcRefFrameList0,
                           RefFrameList*  pcRefFrameList1,
                           MbDataCtrl*    pcMbDataCtrl0L1,
                           Bool           bReconstructAll )
{
  ROF( m_bInitDone );

  //====== initialization ======
  RNOK( pcMbDataCtrl->initSlice( rcSH, DECODE_PROCESS, true, pcMbDataCtrl0L1 ) );

  RefFrameList acRefFrameList0[2];
  RefFrameList acRefFrameList1[2];

  RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
  RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );

  RefFrameList* apcRefFrameList0[2];
  RefFrameList* apcRefFrameList1[2];

  apcRefFrameList0[0] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
  apcRefFrameList0[1] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
  apcRefFrameList1[0] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
  apcRefFrameList1[1] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];

  Frame* apcFrame            [4] = { NULL, NULL, NULL, NULL };
  Frame* apcResidual         [4] = { NULL, NULL, NULL, NULL };
  Frame* apcBaseLayer        [4] = { NULL, NULL, NULL, NULL };
  Frame* apcBaseLayerResidual[4] = { NULL, NULL, NULL, NULL };

	RNOK( gSetFrameFieldArrays( apcFrame,             pcFrame             ) );
  RNOK( gSetFrameFieldArrays( apcResidual,          pcResidual          ) );
  RNOK( gSetFrameFieldArrays( apcBaseLayer,         pcBaseLayer         ) );
  RNOK( gSetFrameFieldArrays( apcBaseLayerResidual, pcBaseLayerResidual ) );

  //===== loop over macroblocks =====
  UInt uiMbAddress           = rcSH.getFirstMbInSlice();
  const UInt uiLastMbAddress = rcSH.getNumMbsInSlice ()-1;
  for( ; uiMbAddress <= uiLastMbAddress; uiMbAddress+=2 )
  {
    for( UInt eP = 0; eP < 2; eP++ )
    {
      UInt uiMbAddressMbAff = uiMbAddress + eP;
      MbDataAccess* pcMbDataAccess     = NULL;
      MbDataAccess* pcMbDataAccessBase = NULL;
      UInt          uiMbY, uiMbX;

      RefFrameList* pcRefFrameList0F;
      RefFrameList* pcRefFrameList1F;

      rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddressMbAff                     );

      RNOK( pcMbDataCtrl->      initMb       (  pcMbDataAccess,    uiMbY, uiMbX       ) );
      pcMbDataAccess->setFieldMode( pcMbDataAccess->getMbData().getFieldFlag()          );

      if( pcMbDataAccess->getMbPicType()==FRAME && pcMbDataCtrlBase )
      {
        RNOK( pcMbDataCtrlBase->initMb        ( pcMbDataAccessBase, uiMbY, uiMbX  ) );
      }
      else if( pcMbDataAccess->getMbPicType()<FRAME && pcMbDataCtrlBaseField )
      {
        RNOK( pcMbDataCtrlBaseField->initMb   ( pcMbDataAccessBase, uiMbY, uiMbX  ) );
      }

      RNOK( m_pcControlMng->initMbForDecoding( *pcMbDataAccess,    uiMbY, uiMbX, true ) );

      const PicType eMbPicType  = pcMbDataAccess->getMbPicType();
			const UInt    uiLI        = eMbPicType - 1;
      if( FRAME == eMbPicType )
      {
        pcRefFrameList0F = pcRefFrameList0;
        pcRefFrameList1F = pcRefFrameList1;
      }
      else
      {
        pcRefFrameList0F = apcRefFrameList0[eP];
        pcRefFrameList1F = apcRefFrameList1[eP];
      }

      RNOK( m_pcMbDecoder->decode ( *pcMbDataAccess,
                                    pcMbDataAccessBase,
                                    apcFrame            [uiLI],
                                    apcResidual         [uiLI],
                                    apcBaseLayer        [uiLI],
                                    apcBaseLayerResidual[uiLI],
                                    pcRefFrameList0F,
                                    pcRefFrameList1F,
                                    bReconstructAll ) );
    }
  }

  if( pcFrame )             RNOK( pcFrame->            removeFrameFieldBuffer() );
  if( pcResidual )          RNOK( pcResidual->         removeFrameFieldBuffer() );
  if( pcBaseLayer )         RNOK( pcBaseLayer->        removeFrameFieldBuffer() );
  if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->removeFrameFieldBuffer() );

	return Err::m_nOK;
}



#ifdef SHARP_AVC_REWRITE_OUTPUT

ErrVal
SliceDecoder::xRewriteMb( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase, Bool bLastMbInSlice )
{
  ROFRS( m_pcRewriteEncoder->isSliceInProgress(), Err::m_nOK );

  //===== create MbDataAccess for rewriting and copy MB data =====
  MbDataAccess* pcMbDataAccessRewrite = 0;
  UInt          uiMbY                 = rcMbDataAccess.getMbY();
  UInt          uiMbX                 = rcMbDataAccess.getMbX();
  RNOK( m_pcRewriteEncoder->initMb  (   pcMbDataAccessRewrite, uiMbY, uiMbX   ) );
  rcMbDataAccess.getMbTCoeffs().switchLevelCoeffData();
  RNOK( m_pcRewriteEncoder->storeMb (  *pcMbDataAccessRewrite, rcMbDataAccess ) );
  rcMbDataAccess.getMbTCoeffs().switchLevelCoeffData();
#if 1 // not required
  ROT ( pcMbDataAccessRewrite->getMbData().getMbCbp() >= 48 );
#endif

  //===== trace =====
  ETRACE_DECLARE( UInt  uiMbAddress = pcMbDataAccessRewrite->getSH().getMbAddressFromPosition( uiMbY, uiMbX ) );
  ETRACE_DECLARE( Bool m_bTraceEnable = true );
  ETRACE_NEWMB  ( uiMbAddress );

  //===== correct MbMode =====
  pcMbDataAccessRewrite->getMbData().setBLSkipFlag      ( false );
  pcMbDataAccessRewrite->getMbData().setResidualPredFlag( false );
  pcMbDataAccessRewrite->getMbData().setBCBPAll         ( 0 );
  if( pcMbDataAccessRewrite->getMbData().getMbMode() == MODE_SKIP )
  {
    if( ! rcMbDataAccess.getSH().isH264AVCCompatible() || ! rcMbDataAccess.getSH().isBSlice() )
    {
      pcMbDataAccessRewrite->getMbData().setMbMode( MODE_16x16 );
    }
    else 
    {
      pcMbDataAccessRewrite->getMbData().setMbMode( MODE_8x8 );
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag() )
        {
          pcMbDataAccessRewrite->getMbData().setBlkMode( c8x8Idx.b8x8Index(), BLK_8x8 );
        }
        else
        {
          pcMbDataAccessRewrite->getMbData().setBlkMode( c8x8Idx.b8x8Index(), BLK_4x4 );
        }
      }
    }
  }
  else if( pcMbDataAccessRewrite->getMbData().getMbMode() == MODE_8x8 )
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      if( pcMbDataAccessRewrite->getMbData().getBlkMode( c8x8Idx.b8x8Index() ) == BLK_SKIP )
      {
        if( ! rcMbDataAccess.getSH().isH264AVCCompatible() || rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag() )
        {
          pcMbDataAccessRewrite->getMbData().setBlkMode( c8x8Idx.b8x8Index(), BLK_8x8 );
        }
        else
        {
          pcMbDataAccessRewrite->getMbData().setBlkMode( c8x8Idx.b8x8Index(), BLK_4x4 );
        }
      }
    }
  }

  //===== recompute motion vector differences =====
  if( !pcMbDataAccessRewrite->getMbData().isIntra() )
  {
    UInt          uiList;
    Mv            cMv;
    MbMotionData  cMbMotionData[2];

    //----- save motion data and clear mvd's -----
    for( uiList = 0; uiList < 2; uiList++ )
    {
      ListIdx eListIdx = ( uiList == 0 ? LIST_0 : LIST_1 );

      cMbMotionData[uiList].copyFrom( pcMbDataAccessRewrite->getMbMotionData( eListIdx ) );
      pcMbDataAccessRewrite->getMbMvdData   ( eListIdx ).clear();
      pcMbDataAccessRewrite->getMbMotionData( eListIdx ).setMotPredFlag( false );

    }

    //----- re-assign mvd's (this is a sub-optimal implementation) -----
    for( uiList = 0; uiList < 2; uiList++ )
    {
      ListIdx eListIdx = ( uiList == 0 ? LIST_0 : LIST_1 );

      //----- recompute mvd's -----
      for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        m_pcMbDecoder->calcMv( *pcMbDataAccessRewrite, 0 );

        cMv  = cMbMotionData[uiList].getMv( cIdx );
        cMv -= pcMbDataAccessRewrite->getMbMotionData( eListIdx ).getMv( cIdx );
        pcMbDataAccessRewrite->getMbMvdData( eListIdx ).setMv( cMv, cIdx );
      }

      //----- set mvd's for all 4x4 blocks (for CABAC context models) -----
      switch( pcMbDataAccessRewrite->getMbData().getMbMode() )
      {
      case MODE_16x16:
        cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( );
        pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv );
        break;
      case MODE_16x8:
        cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( PART_16x8_0 );
        pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv, PART_16x8_0 );
        cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( PART_16x8_1 );
        pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv, PART_16x8_1 );
        break;
      case MODE_8x16:
        cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( PART_8x16_0 );
        pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv, PART_8x16_0 );
        cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( PART_8x16_1 );
        pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv, PART_8x16_1 );
        break;
      case MODE_8x8:
        {					
          for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
          {
            ParIdx8x8 eParIdx = c8x8Idx.b8x8();

            switch( pcMbDataAccessRewrite->getMbData().getBlkMode( c8x8Idx.b8x8Index() ) )
            {
            case BLK_8x8:
              cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( eParIdx );
              pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv, eParIdx );
              break;
            case BLK_8x4:
              cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( eParIdx, SPART_8x4_0 );
              pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv, eParIdx, SPART_8x4_0 );
              cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( eParIdx, SPART_8x4_1 );
              pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv, eParIdx, SPART_8x4_1 );
              break;
            case BLK_4x8:
              cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( eParIdx, SPART_4x8_0 );
              pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv, eParIdx, SPART_4x8_0 );
              cMv = pcMbDataAccessRewrite->getMbMvdData( eListIdx ).getMv( eParIdx, SPART_4x8_1 );
              pcMbDataAccessRewrite->getMbMvdData(eListIdx).setAllMv( cMv, eParIdx, SPART_4x8_1 );
              break;
            case BLK_4x4:
              break;
            default:
              RERR();
            }
          }
        }
        break;
      default:
        RERR();
      }
    }

#if 1 // not required
    //>>>>> SANITY CHECK >>>>>
    m_pcMbDecoder->calcMv( *pcMbDataAccessRewrite, NULL );
    for( uiList = 0; uiList < 2; uiList++ )
    {
      for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        ListIdx eListIdx = ( uiList == 0 ? LIST_0 : LIST_1 );
        cMv              = cMbMotionData[uiList].getMv( cIdx );
        cMv             -= pcMbDataAccessRewrite->getMbMotionData( eListIdx ).getMv( cIdx );
        if( cMv.getHor() || cMv.getVer() )
        {
          RERR();
        }
      }
    }
    //<<<<< SANITY CHECK <<<<<
#endif
  }

  //===== write macroblock =====
  RNOK( m_pcRewriteEncoder->encodeMb( *pcMbDataAccessRewrite, bLastMbInSlice ) );

  return Err::m_nOK;
}

#endif


H264AVC_NAMESPACE_END
