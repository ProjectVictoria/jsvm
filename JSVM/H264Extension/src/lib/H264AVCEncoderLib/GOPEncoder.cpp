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
#include "GOPEncoder.h"

#include "SliceEncoder.h"
#include "CreaterH264AVCEncoder.h"
#include "ControlMngH264AVCEncoder.h"
#include "RateDistortionIf.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/Sei.h"


#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "H264AVCCommonLib/CFMO.h"

//JVT-U106 Behaviour at slice boundaries{
#include "H264AVCCommonLib/ReconstructionBypass.h"
//JVT-U106 Behaviour at slice boundaries}
// JVT-V068 {
#include "Scheduler.h"
// JVT-V068 }

// JVT-W043 {
#include "RateCtlBase.h"
#include "RateCtlQuadratic.h"
// JVT-W043 }

H264AVC_NAMESPACE_BEGIN



#define FACTOR_22_HP  0.70710678118654752440084436210485  //sqrt( 1.0/ 2.0)
#define FACTOR_22_LP  1.4142135623730950488016887242097   //sqrt( 2.0/ 1.0)
// RWTH Aachen, m11728 (heiko.schwarz@hhi.fhg.de on behalf of Mathias Wien [mathias.wien@rwth-aachen.de])
//#define FACTOR_53_HP  0.81649658092772603273242802490196  //sqrt( 2.0/ 3.0)
//#define FACTOR_53_LP  1.1795356492391770676634011002828   //sqrt(32.0/23.0)
#define FACTOR_53_HP  0.84779124789065851738306954082825  //sqrt(23.0/32.0)
#define FACTOR_53_LP  1.2247448713915890490986420373529   //sqrt( 3.0/ 2.0)

//{{Scaling factor Base Layer-France Telecom R&D (nathalie.cammas@francetelecom.com), JVTO045 and m11876
#define FACTOR_53_HP_BL 1
#define FACTOR_22_HP_BL 1
//}}Scaling factor Base Layer


LayerEncoder::LayerEncoder()
//----- references -----
: m_pcSPS                           ( 0 )
, m_pcPPSLP                         ( 0 )
, m_pcPPSHP                         ( 0 )
, m_pcYuvFullPelBufferCtrl          ( 0 )
, m_pcYuvHalfPelBufferCtrl          ( 0 )
, m_pcPocCalculator                 ( 0 )
, m_pcH264AVCEncoder                ( 0 )
, m_pcSliceEncoder                  ( 0 )
, m_pcNalUnitEncoder                ( 0 )
, m_pcLoopFilter                    ( 0 )
, m_pcQuarterPelFilter              ( 0 )
, m_pcMotionEstimation              ( 0 )
//----- fixed control parameters -----
, m_bTraceEnable                    ( true )
, m_bFrameMbsOnlyFlag               ( true )
, m_uiDependencyId                       ( 0 )
, m_uiScalableLayerId								( 0 )
, m_uiBaseLayerId                   ( MSYS_UINT_MAX )
, m_uiBaseQualityLevel              ( 15 )
, m_uiQualityLevelForPrediction     ( 15 )
, m_uiFrameWidthInMb                ( 0 )
, m_uiFrameHeightInMb               ( 0 )
, m_uiMbNumber                      ( 0 )
, m_uiMaxGOPSize                    ( 0 )
, m_uiDecompositionStages           ( 0 )
, m_uiTemporalResolution            ( 0 )
, m_uiNotCodedStages                ( 0 )
, m_uiFrameDelay                    ( 0 )
, m_uiMaxNumRefFrames               ( 0 )
, m_uiLowPassIntraPeriod            ( 0 )
, m_uiNumMaxIter                    ( 0 )
, m_uiIterSearchRange               ( 0 )
, m_iMaxDeltaQp                     ( 0 )
, m_bH264AVCCompatible              ( true  )
, m_bInterLayerPrediction           ( true  )
, m_bAdaptivePrediction             ( true  )
, m_bHaarFiltering                  ( false )
, m_bBiPredOnly                     ( false )
, m_bWriteSubSequenceSei            ( false )
, m_dBaseQPResidual                 ( 0.0 )
, m_uiFilterIdc                     ( 0 )
, m_iAlphaOffset                    ( 0 )
, m_iBetaOffset                     ( 0 )
, m_bLoadMotionInfo                 ( false )
, m_bSaveMotionInfo                 ( false )
, m_pMotionInfoFile                 ( 0 )
//----- variable control parameters -----
, m_bInitDone                       ( false )
, m_bFirstGOPCoded                  ( false )
, m_uiGOPSize                       ( 0 )
, m_uiFrameCounter                  ( 0 )
, m_uiFrameNum                      ( 0 )
, m_uiIdrPicId											( 0 ) //EIDR 0619
, m_uiGOPNumber                     ( 0 )
//----- frame memories -----
, m_papcFrame                       ( 0 )
, m_papcOrgFrame                    ( 0 )
, m_papcCLRecFrame                  ( 0 )
, m_papcELFrame                     ( 0 )
, m_papcResidual                    ( 0 )
, m_papcSubband                     ( 0 )
, m_pcLowPassBaseReconstruction     ( 0 )
//TMM_WP
, m_bBaseLayerWp                    (false)
//TMM_WP
, m_pcAnchorFrameOriginal           ( 0 )
, m_pcAnchorFrameReconstructed      ( 0 )
, m_pcBaseLayerFrame                ( 0 )
, m_pcBaseLayerResidual             ( 0 )
//----- control data arrays -----
, m_pacControlData                  ( 0 )
, m_pcBaseLayerCtrl                 ( 0 )
, m_pcBaseLayerCtrlField            ( 0 )
, m_pcBaseLayerCtrlEL				        ( 0 )
, m_pacControlDataEL			         	( 0 )
//----- auxiliary buffers -----
, m_uiWriteBufferSize               ( 0 )
, m_pucWriteBuffer                  ( 0 )
//----- PSNR & rate -----
, m_fOutputFrameRate                ( 0.0 )
, m_uiParameterSetBits              ( 0 )
//--- FGS ---
, m_pcResizeParameters              ( 0 )//TMM_ESS
, m_pESSFile                        ( 0 )
// JVT-Q065 EIDR{
, m_iIDRPeriod					        	  ( 0 )
, m_iIDRAccessPeriod		            ( 0 ) //EIDR bug-fix
, m_bBLSkipEnable			          		( false )
// JVT-Q065 EIDR}
, m_bLARDOEnable                    ( false ) //JVT-R057 LA-RDO
, m_uiEssRPChkEnable								( 0 )
, m_uiMVThres												( 20 )
, m_uiNonRequiredWrite				      ( 0 )  //NonRequired JVT-Q066 (06-04-08)
, m_uiPreAndSuffixUnitEnable				      ( 0 ) //JVT-S036 lsj 
, m_uiMMCOBaseEnable						    ( 0 ) //JVT-S036 lsj
// JVT-S054 (ADD) ->
, m_bIroiSliceDivisionFlag          ( false )
, m_uiNumSliceMinus1                ( 0 )
, m_puiFirstMbInSlice               ( 0 )
, m_puiLastMbInSlice                ( 0 )
// JVT-S054 (ADD) <-
//S051{
, m_uiTotalFrame					          ( 0 )
, m_auiFrameBits				          	( NULL )
, m_uiAnaSIP						            ( 0 )
, m_bEncSIP						            	( false )
, m_cInSIPFileName					        ( "none" )
, m_cOutSIPFileName					        ( "none" )
//S051}
//JVT-T054{
, m_uiLayerCGSSNR                   ( 0 )
, m_uiQualityLevelCGSSNR            ( 0 )
, m_uiBaseLayerCGSSNR               ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR        ( 0 )
//JVT-T054}
// JVT-U085 LMI 
, m_bTlevelNestingFlag              (true)
// JVT-U116 W062 LMI 
, m_bTl0DepRepIdxEnable            (false)
//JVT-U106 Behaviour at slice boundaries{
, m_bCIUFlag                       (false)
//JVT-U106 Behaviour at slice boundaries}
, m_bGOPInitialized                ( false )
, m_uiMGSKeyPictureControl( 0 )
, m_bHighestMGSLayer( false )
, m_pcRedundantCtrl (0)//RPIC bug fix
, m_pcRedundant1Ctrl (0)//RPIC bug fix
, m_uiLastCodedFrameIdInGOP ( MSYS_UINT_MAX )
, m_uiLastCodedTemporalId   ( MSYS_UINT_MAX )
{
  ::memset( m_abIsRef,          0x00, sizeof( m_abIsRef           ) );
  ::memset( m_apcFrameTemp,     0x00, sizeof( m_apcFrameTemp      ) );

  m_apabBaseModeFlagAllowedArrays[0] = 0;
  m_apabBaseModeFlagAllowedArrays[1] = 0;

  UInt ui;
  for( ui = 0; ui <= MAX_DSTAGES; ui++ )
  {
    m_auiNumFramesCoded [ui]  = 0;
    m_adPSNRSumY        [ui]  = 0.0;
    m_adPSNRSumU        [ui]  = 0.0;
    m_adPSNRSumV        [ui]  = 0.0;
  }
  m_uiNewlyCodedBits      = 0;
}





LayerEncoder::~LayerEncoder()
{
}





ErrVal
LayerEncoder::create( LayerEncoder*& rpcLayerEncoder )
{
  rpcLayerEncoder = new LayerEncoder;
  ROT( NULL == rpcLayerEncoder );
  return Err::m_nOK;
}





ErrVal
LayerEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}





ErrVal
LayerEncoder::init( CodingParameter*   pcCodingParameter,
                   LayerParameters*   pcLayerParameters,
                   H264AVCEncoder*    pcH264AVCEncoder,
                   SliceEncoder*      pcSliceEncoder,
                   LoopFilter*        pcLoopFilter,
                   PocCalculator*     pcPocCalculator,
                   NalUnitEncoder*    pcNalUnitEncoder,
                   YuvBufferCtrl*     pcYuvFullPelBufferCtrl,
                   YuvBufferCtrl*     pcYuvHalfPelBufferCtrl,
                   QuarterPelFilter*  pcQuarterPelFilter,
                   MotionEstimation*  pcMotionEstimation
           //JVT-U106 Behaviour at slice boundaries{
           ,ReconstructionBypass* pcReconstructionBypass
           //JVT-U106 Behaviour at slice boundaries}
                  // JVT-V068 {
                  ,StatBuf<Scheduler*, MAX_SCALABLE_LAYERS>* apcScheduler
                  // JVT-V068 }
           )
{
  ROF( pcCodingParameter );
  ROF( pcLayerParameters );
  ROF( pcH264AVCEncoder );
  ROF( pcSliceEncoder );
  ROF( pcLoopFilter );
  ROF( pcPocCalculator );
  ROF( pcNalUnitEncoder );
  ROF( pcYuvFullPelBufferCtrl );
  ROF( pcYuvHalfPelBufferCtrl );
  ROF( pcQuarterPelFilter );
  ROF( pcMotionEstimation );
  //JVT-U106 Behaviour at slice boundaries{
  ROF( pcReconstructionBypass );
  //JVT-U106 Behaviour at slice boundaries}

  //----- references -----
  m_pcSPS                   = 0;
  m_pcPPSLP                 = 0;
  m_pcPPSHP                 = 0;
  m_pcYuvFullPelBufferCtrl  = pcYuvFullPelBufferCtrl;
  m_pcYuvHalfPelBufferCtrl  = pcYuvHalfPelBufferCtrl;
  m_pcPocCalculator         = pcPocCalculator;
  m_pcH264AVCEncoder        = pcH264AVCEncoder;
  m_pcSliceEncoder          = pcSliceEncoder;
  m_pcNalUnitEncoder        = pcNalUnitEncoder;
  m_pcLoopFilter            = pcLoopFilter;
  m_pcQuarterPelFilter      = pcQuarterPelFilter;
  m_pcMotionEstimation      = pcMotionEstimation;
  //JVT-U106 Behaviour at slice boundaries{
  m_pcReconstructionBypass  = pcReconstructionBypass;
  //JVT-U106 Behaviour at slice boundaries}
  m_pcLayerParameters       = pcLayerParameters;

  //----- fixed control parameters -----
  m_bInterlaced             = pcLayerParameters->isInterlaced();
  // JVT-V068 {
  m_bEnableHrd              = pcCodingParameter->getEnableNalHRD() || pcCodingParameter->getEnableVclHRD();
  m_apcScheduler            = apcScheduler;
  // JVT-V068 }
  // JVT-W049 {
  m_uiNumberLayersCnt       = pcCodingParameter->getNumberOfLayers();
  // JVT-W049 }
  m_uiDependencyId               = pcLayerParameters->getDependencyId                 ();
  m_uiBaseLayerId           = pcLayerParameters->getBaseLayerId             ();
  m_uiBaseQualityLevel      = pcLayerParameters->getBaseQualityLevel        ();

// JVT-Q065 EIDR{
  m_iIDRPeriod				= pcLayerParameters->getIDRPeriod				();
  m_bBLSkipEnable			= pcLayerParameters->getBLSkipEnable			();
// JVT-Q065 EIDR}
// JVT-U085 LMI
  m_bTlevelNestingFlag      = pcCodingParameter->getTlevelNestingFlag();
// JVT-U116 W062 LMI
  m_bTl0DepRepIdxEnable = pcCodingParameter->getTl0DepRepIdxSeiEnable();

  m_bDiscardable = pcLayerParameters->isDiscardable(); //DS_FIX_FT_09_2007
  m_uiQLDiscardable = pcLayerParameters->getQLDiscardable(); //DS_FIX_FT_09_2007

	m_uiEssRPChkEnable = pcCodingParameter->getEssRPChkEnable();
	m_uiMVThres = pcCodingParameter->getMVThres();

  m_uiQualityLevelForPrediction = 15;

  if( pcCodingParameter->getNumberOfLayers() > pcLayerParameters->getDependencyId() + 1 )
  {
    m_uiQualityLevelForPrediction = (pcLayerParameters + 1)->getBaseQualityLevel();

    if( m_uiQualityLevelForPrediction > 15 )
    {
      ROT( 1 );
      m_uiQualityLevelForPrediction = 15;
    }
  }
  m_uiDecompositionStages   = pcLayerParameters->getDecompositionStages     ();
  m_uiTemporalResolution    = pcLayerParameters->getTemporalResolution      ();
  m_uiNotCodedStages        = pcLayerParameters->getNotCodedStages          ();
  m_uiFrameDelay            = pcLayerParameters->getFrameDelay              ();
  m_uiMaxNumRefFrames       = pcCodingParameter->getNumRefFrames            ();
  m_uiLowPassIntraPeriod    = pcCodingParameter->getIntraPeriodLowPass      ();
  m_uiNumMaxIter            = pcCodingParameter->getMotionVectorSearchParams().getNumMaxIter      ();
  m_uiIterSearchRange       = pcCodingParameter->getMotionVectorSearchParams().getIterSearchRange ();
  m_iMaxDeltaQp             = pcLayerParameters->getMaxAbsDeltaQP           ();
 //  m_bH264AVCCompatible      = pcCodingParameter->getBaseLayerMode           ()  > 0 && m_uiDependencyId == 0;
  m_bH264AVCCompatible      = m_uiDependencyId == 0; //bug-fix suffix
  m_bInterLayerPrediction   = pcLayerParameters->getInterLayerPredictionMode()  > 0;
  m_bAdaptivePrediction     = pcLayerParameters->getInterLayerPredictionMode()  > 1;
  m_bHaarFiltering          = false;
  m_bBiPredOnly             = false;
  m_bForceReOrderingCommands= pcLayerParameters->getForceReorderingCommands ()  > 0;
  m_bWriteSubSequenceSei    = pcCodingParameter->getBaseLayerMode           ()  > 1 && m_uiDependencyId == 0;

  m_bSameResBL              = ( m_uiBaseLayerId != MSYS_UINT_MAX &&
                                pcCodingParameter->getLayerParameters( m_uiBaseLayerId ).getFrameWidth () == pcLayerParameters->getFrameWidth () &&
                                pcCodingParameter->getLayerParameters( m_uiBaseLayerId ).getFrameHeight() == pcLayerParameters->getFrameHeight() &&
                                pcLayerParameters->getResizeParameters().m_iExtendedSpatialScalability == ESS_NONE );
  
  m_bSameResEL              = false;
  {
    UInt uiLId;
    for( uiLId = m_uiDependencyId + 1; uiLId < pcCodingParameter->getNumberOfLayers() && ! m_bSameResEL; uiLId++ )
    {
      LayerParameters& rcEL = pcCodingParameter->getLayerParameters( uiLId );
      if( rcEL.getBaseLayerId() == m_uiDependencyId )
      {
       m_bSameResEL        = ( pcLayerParameters->getFrameWidth () == rcEL.getFrameWidth () &&
                               pcLayerParameters->getFrameHeight() == rcEL.getFrameHeight() &&
                               rcEL.getResizeParameters().m_iExtendedSpatialScalability == ESS_NONE );
      }
    }
  }
  m_bMGS                    = pcCodingParameter->getCGSSNRRefinement    () == 1 && ( m_bSameResBL || m_bSameResEL );
  m_uiEncodeKeyPictures     = pcCodingParameter->getEncodeKeyPictures   ();
  m_uiMGSKeyPictureControl  = pcCodingParameter->getMGSKeyPictureControl();
  m_bHighestMGSLayer        = m_bMGS && !m_bSameResEL;
  m_uiLastCodedFrameIdInGOP = MSYS_UINT_MAX;
  m_uiLastCodedTemporalId   = MSYS_UINT_MAX;

  m_bExplicitQPCascading      = pcLayerParameters->getExplicitQPCascading() != 0;
  for( UInt uiTTL = 0; uiTTL < MAX_TEMP_LEVELS; uiTTL++ )
  {
    m_adDeltaQPTLevel[uiTTL]  = pcLayerParameters->getDeltaQPTLevel( uiTTL );
  }

  //JVT-V079 Low-complexity MB mode decision {
  if ( m_uiDependencyId==0 )
    m_pcSliceEncoder->getMbEncoder()->setLowComplexMbEnable ( m_uiDependencyId, (pcLayerParameters->getLowComplexMbEnable ()==1) );
  else
    m_pcSliceEncoder->getMbEncoder()->setLowComplexMbEnable ( m_uiDependencyId, false );
  //JVT-V079 Low-complexity MB mode decision }

  //JVT-R057 LA-RDO{
  if(pcCodingParameter->getLARDOEnable()!=0)
  {
    static UInt auiPLR[5];
    static UInt aauiSize[5][2];
    static Double dRatio[5][2];
    auiPLR[m_uiDependencyId]      = pcLayerParameters->getPLR                     (); 
    m_bLARDOEnable            = pcCodingParameter->getLARDOEnable()==0? false:true;
    aauiSize[m_uiDependencyId][0]  =pcLayerParameters->getFrameWidth();
    aauiSize[m_uiDependencyId][1]  =pcLayerParameters->getFrameHeight();
    if(m_uiDependencyId==0||pcLayerParameters->getBaseLayerId()==MSYS_UINT_MAX)
    {
      dRatio[m_uiDependencyId][0]=1;
      dRatio[m_uiDependencyId][1]=1;
    }
    else
    {
      dRatio[m_uiDependencyId][0]=(Double)aauiSize[m_uiDependencyId][0]/aauiSize[pcLayerParameters->getBaseLayerId()][0];
      dRatio[m_uiDependencyId][1]=(Double)aauiSize[m_uiDependencyId][1]/aauiSize[pcLayerParameters->getBaseLayerId()][1];
    }
    m_pcSliceEncoder->getMbEncoder()->setRatio(dRatio);
    m_pcSliceEncoder->getMbEncoder()->setPLR(auiPLR);
    pcLayerParameters->setContrainedIntraForLP();
  }
  //JVT-R057 LA-RDO}

  m_uiPreAndSuffixUnitEnable = pcCodingParameter->getPreAndSuffixUnitEnable();//JVT-S036 lsj 
  m_uiMMCOBaseEnable   = pcCodingParameter->getMMCOBaseEnable();  //JVT-S036 lsj

  // TMM_ESS 
  m_pcResizeParameters  = &pcLayerParameters->getResizeParameters();
  m_pESSFile            = 0;
  if( m_pcResizeParameters->m_iExtendedSpatialScalability == ESS_PICT )
  {
    m_pESSFile = fopen( pcLayerParameters->getESSFilename().c_str(), "r" );
    if( !m_pESSFile )
    { 
      printf( "failed to open resize parameter file %s\n", pcLayerParameters->getESSFilename().c_str() );
      ROT(1);
    }
  }

  for( UInt uiStage = 0; uiStage < MAX_DSTAGES; uiStage++ )
  {
    m_adBaseQpLambdaMotion[uiStage] = pcLayerParameters->getQpModeDecision( uiStage );
  }
  m_dBaseQpLambdaMotionLP   = pcLayerParameters->getQpModeDecisionLP          ();
  m_dBaseQPResidual         = pcLayerParameters->getBaseQpResidual            ();

  m_uiFilterIdc             = pcCodingParameter->getLoopFilterParams          ().getFilterIdc       ();
  m_iAlphaOffset            = pcCodingParameter->getLoopFilterParams          ().getAlphaOffset     ();
  m_iBetaOffset             = pcCodingParameter->getLoopFilterParams          ().getBetaOffset      ();
  m_uiInterLayerFilterIdc   = pcCodingParameter->getInterLayerLoopFilterParams().getFilterIdc       ();
  m_iInterLayerAlphaOffset  = pcCodingParameter->getInterLayerLoopFilterParams().getAlphaOffset     ();
  m_iInterLayerBetaOffset   = pcCodingParameter->getInterLayerLoopFilterParams().getBetaOffset      ();

  m_bLoadMotionInfo         = pcLayerParameters->getMotionInfoMode            () == 1;
  m_bSaveMotionInfo         = pcLayerParameters->getMotionInfoMode            () == 2;
  m_pMotionInfoFile         = 0;

  if( m_bLoadMotionInfo )
  {
    m_pMotionInfoFile = ::fopen( pcLayerParameters->getMotionInfoFilename().c_str(), "rb" );
    if( ! m_pMotionInfoFile )
    {
      fprintf( stderr, "\nCANNOT OPEN MOTION INFO FILE \"%s\"\n\n", pcLayerParameters->getMotionInfoFilename().c_str() );
      return Err::m_nERR;
    }
  }
  else if( m_bSaveMotionInfo )
  {
    m_pMotionInfoFile = ::fopen( pcLayerParameters->getMotionInfoFilename().c_str(), "wb" );
    if( ! m_pMotionInfoFile )
    {
      fprintf( stderr, "\nCANNOT OPEN MOTION INFO FILE \"%s\"\n\n", pcLayerParameters->getMotionInfoFilename().c_str() );
      return Err::m_nERR;
    }
  }


  //----- PSNR and rate -----
  m_fOutputFrameRate          = pcLayerParameters->getOutputFrameRate();
  m_uiParameterSetBits        = 0;
  
  UInt ui;
  for( ui = 0; ui <= MAX_DSTAGES; ui++ )
  {
    m_auiNumFramesCoded [ui]  = 0;
    m_adPSNRSumY        [ui]  = 0.0;
    m_adPSNRSumU        [ui]  = 0.0;
    m_adPSNRSumV        [ui]  = 0.0;
  }
  m_uiNewlyCodedBits  = 0;

  // JVT-S054 (ADD) ->
  m_bIroiSliceDivisionFlag = pcLayerParameters->m_bSliceDivisionFlag;
  if (m_bIroiSliceDivisionFlag)
  {
    m_uiNumSliceMinus1 = pcLayerParameters->m_uiNumSliceMinus1;
    if (m_puiFirstMbInSlice != NULL)
      free(m_puiFirstMbInSlice);
    m_puiFirstMbInSlice = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));
    memcpy( m_puiFirstMbInSlice, pcLayerParameters->m_puiFirstMbInSlice, (m_uiNumSliceMinus1+1)*sizeof(UInt) );

    if (m_puiLastMbInSlice != NULL)
      free(m_puiLastMbInSlice);
    m_puiLastMbInSlice = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));
    memcpy( m_puiLastMbInSlice, pcLayerParameters->m_puiLastMbInSlice, (m_uiNumSliceMinus1+1)*sizeof(UInt) );
  }
  // JVT-S054 (ADD) <-

  //S051{
  m_uiTotalFrame	= pcCodingParameter->getTotalFrames();
  m_uiAnaSIP		= pcLayerParameters->getAnaSIP();
  m_cOutSIPFileName	= pcLayerParameters->getOutSIPFileName();
  if(m_uiAnaSIP==1)
    m_bInterLayerPrediction=true;
  if(m_uiAnaSIP==2)
    m_bInterLayerPrediction=m_bAdaptivePrediction=false;

  if(pcCodingParameter->getNumberOfLayers() > m_uiDependencyId+1)
  {
    m_bEncSIP			= pcCodingParameter->getLayerParameters( m_uiDependencyId+1).getEncSIP();
    m_cInSIPFileName	= pcCodingParameter->getLayerParameters( m_uiDependencyId+1).getInSIPFileName();
  }
  //S051}
  
  //JVT-U106 Behaviour at slice boundaries{
  m_bCIUFlag=pcCodingParameter->getCIUFlag()!=0;
  //JVT-U106 Behaviour at slice boundaries}
  {
    Int iMaxNumMb = ( ( pcLayerParameters->getFrameWidth() + 15 ) >> 4 ) * ( ( pcLayerParameters->getFrameHeight() + 15 ) >> 4 );
    m_apabBaseModeFlagAllowedArrays[0] = new Bool [iMaxNumMb];
    m_apabBaseModeFlagAllowedArrays[1] = new Bool [iMaxNumMb];
    ROF( m_apabBaseModeFlagAllowedArrays[0] );
    ROF( m_apabBaseModeFlagAllowedArrays[1] );
  }
  
  m_uiFramesInCompleteGOPsProcessed = 0;
  m_uiMinScalableLayer              = 0;
  for( UInt uiBaseLayerId = 0; uiBaseLayerId < m_uiDependencyId; uiBaseLayerId++ )
  {
    m_uiMinScalableLayer += pcCodingParameter->getLayerParameters( uiBaseLayerId ).getDecompositionStages() -
      pcCodingParameter->getLayerParameters( uiBaseLayerId ).getNotCodedStages     () + 1U;
  }
	//JVT-X046 {
  m_uiSliceMode = pcCodingParameter->getSliceMode();
  m_uiSliceArgument = pcCodingParameter->getSliceArgument();
	if ( m_uiSliceMode == 2 )
	{
		m_uiSliceArgument = m_uiSliceArgument*8;
	}
  //JVT-X046 }

  return Err::m_nOK;
}



__inline UInt downround2powerof2( UInt i ) { UInt r = 1; for( ; (UInt)( 1 << r ) <= i; r++ ); return ( 1 << ( r - 1 ) ); }

ErrVal
LayerEncoder::initParameterSets( const SequenceParameterSet& rcSPS,
                                const PictureParameterSet&  rcPPSLP,
                                const PictureParameterSet&  rcPPSHP )
{
  //===== set references =====
  m_pcSPS                 = &rcSPS;
  m_pcPPSLP               = &rcPPSLP;
  m_pcPPSHP               = &rcPPSHP;

  //===== get and set relevant parameters =====
  m_bFrameMbsOnlyFlag     = rcSPS.getFrameMbsOnlyFlag ();
  UInt  uiMaxDPBSize      = rcSPS.getMaxDPBSize       ();
  m_uiFrameWidthInMb      = rcSPS.getFrameWidthInMbs  ();
  m_uiFrameHeightInMb     = rcSPS.getFrameHeightInMbs ();
  m_uiMbNumber            = m_uiFrameWidthInMb * m_uiFrameHeightInMb;
  m_uiMaxGOPSize          = downround2powerof2( uiMaxDPBSize );

  //===== re-allocate dynamic memory =====
  RNOK( xDeleteData()        );
  RNOK( xCreateData( rcSPS ) );

  //===== initialize some parameters =====
  m_bInitDone             = true;
  m_bFirstGOPCoded        = false;
  m_uiFrameCounter        = 0;
  m_uiFrameNum            = 0;
  m_uiGOPNumber           = 0;
  ::memset( m_abIsRef, 0x00, sizeof( m_abIsRef ) );

  return Err::m_nOK;
}




ErrVal
LayerEncoder::addParameterSetBits( UInt uiParameterSetBits )
{
  m_uiParameterSetBits += uiParameterSetBits;
  return Err::m_nOK;
}




ErrVal
LayerEncoder::xCreateData( const SequenceParameterSet& rcSPS )
{
  UInt uiIndex;


  //========== CREATE FRAME MEMORIES ==========
  ROFS   ( ( m_papcFrame                     = new Frame* [ m_uiMaxGOPSize + 1 ]      ) );
  ROFS   ( ( m_papcOrgFrame                  = new Frame* [ m_uiMaxGOPSize + 1 ]      ) );
  
  if( m_uiQualityLevelForPrediction < 15 )
  {
    ROFS ( ( m_papcCLRecFrame                = new Frame* [ m_uiMaxGOPSize + 1 ]      ) );
  }
  if( m_uiMGSKeyPictureControl && ! m_bHighestMGSLayer )
  {
    ROFS ( ( m_papcELFrame                   = new Frame* [ m_uiMaxGOPSize + 1 ]      ) );
  }
  ROFS   ( ( m_papcResidual                  = new Frame* [ m_uiMaxGOPSize + 1 ]      ) );
  ROFS   ( ( m_papcSubband                   = new Frame* [ m_uiMaxGOPSize + 1 ]      ) );

  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    RNOK( Frame::create( m_papcFrame         [ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
    RNOK  (   m_papcFrame         [ uiIndex ] ->init        () );
    RNOK( Frame::create( m_papcOrgFrame         [ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
    RNOK  (   m_papcOrgFrame         [ uiIndex ] ->init        () );
      
    if( m_papcCLRecFrame )
    {
      RNOK( Frame::create( m_papcCLRecFrame[ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
      RNOK(   m_papcCLRecFrame    [ uiIndex ] ->init        () );
    }
    if( m_papcELFrame )
    {
      RNOK( Frame::create( m_papcELFrame[ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
      RNOK(   m_papcELFrame    [ uiIndex ] ->init        () );
    }
    
    RNOK( Frame::create( m_papcResidual     [ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
    RNOK  (   m_papcResidual      [ uiIndex ] ->init        () );
    RNOK( Frame::create( m_papcSubband      [ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
    RNOK  (   m_papcSubband       [ uiIndex ] ->init        () );
  }
  
  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES;  uiIndex++ )
  {
    RNOK( Frame::create( m_apcFrameTemp      [ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
    RNOK  (   m_apcFrameTemp      [ uiIndex ] ->init        () );
  }
  
  RNOK( Frame::create( m_pcLowPassBaseReconstruction, *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_pcLowPassBaseReconstruction   ->init        () );
  RNOK( Frame::create( m_pcAnchorFrameOriginal      , *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_pcAnchorFrameOriginal         ->init        () );
  RNOK( Frame::create( m_pcAnchorFrameReconstructed , *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_pcAnchorFrameReconstructed    ->init        () );
  RNOK( Frame::create( m_pcBaseLayerFrame           , *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_pcBaseLayerFrame              ->init        () );
  RNOK( Frame::create( m_pcBaseLayerResidual        , *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_pcBaseLayerResidual           ->init        () );
  
  ROFS ( ( m_pbFieldPicFlag = new Bool[ m_uiMaxGOPSize + 1 ] ));

  //========== CREATE MACROBLOCK DATA MEMORIES ==========
  ROFS   ( ( m_pacControlData  = new ControlData[ m_uiMaxGOPSize + 1 ] ) );
  ROFS   ( ( m_pacControlDataEL  = new ControlData[ m_uiMaxGOPSize + 1 ] ) );

  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    MbDataCtrl*  pcMbDataCtrl                = 0;
    ROFS ( (     pcMbDataCtrl                = new MbDataCtrl  () ) );
    RNOK  (       pcMbDataCtrl                ->init            ( rcSPS ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setMbDataCtrl   ( pcMbDataCtrl ) );
    RNOK  (       m_pacControlData[ uiIndex ] .initFGSData      ( m_uiFrameWidthInMb * m_uiFrameHeightInMb ) );
    
    Bool          bLowPass                    = ( ( uiIndex % ( 1 << m_uiDecompositionStages ) ) == 0 );
    SliceHeader*  pcSliceHeader               = 0;
    ROFS ( (     pcSliceHeader               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setSliceHeader  (  pcSliceHeader, FRAME ) );

    if( m_pcLayerParameters->getPAff() )
    {
      ROFRS ( (   pcSliceHeader               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ), Err::m_nERR );
      RNOK  (     m_pacControlData[ uiIndex ] . setSliceHeader  ( pcSliceHeader, BOT_FIELD ) );
    }
    
   MbDataCtrl*   pcMbDataCtrlEL                = 0;
    ROFS ( (     pcMbDataCtrlEL                = new MbDataCtrl  () ) );
    RNOK  (       pcMbDataCtrlEL                ->init            ( rcSPS ) );
    RNOK  (       m_pacControlDataEL[ uiIndex ] . setMbDataCtrl   ( pcMbDataCtrlEL ) );
    RNOK  (       m_pacControlDataEL[ uiIndex ] .initFGSData      ( m_uiFrameWidthInMb * m_uiFrameHeightInMb ) );

   // ICU/ETRI FGS_MOT_USE
    SliceHeader* pcSliceHeaderEL               = 0;
    ROFS ( (     pcSliceHeaderEL               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ) );
    RNOK  (      m_pacControlDataEL[ uiIndex ] . setSliceHeader  (  pcSliceHeaderEL, FRAME ) );
   
    if( m_pcLayerParameters->getPAff() )
    {
      ROFRS ( (   pcSliceHeader               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ), Err::m_nERR );
      RNOK  (     m_pacControlDataEL[ uiIndex ] . setSliceHeader  ( pcSliceHeader, BOT_FIELD ) );
    }

  }
  
  ROFS   ( ( m_pcBaseLayerCtrl = new MbDataCtrl() ) );
  RNOK    (   m_pcBaseLayerCtrl ->init          ( rcSPS ) );
  
  ROFS   ( ( m_pcBaseLayerCtrlField = new MbDataCtrl() ) );
  RNOK    (   m_pcBaseLayerCtrlField ->init          ( rcSPS ) );
  
  ROFS   ( ( m_pcBaseLayerCtrlEL = new MbDataCtrl() ) );
  RNOK    (  m_pcBaseLayerCtrlEL ->init          ( rcSPS ) );

//RPIC bug fix
  ROFS ( (     m_pcRedundantCtrl                = new MbDataCtrl  () ) );
  RNOK  (       m_pcRedundantCtrl                ->init            ( rcSPS ) );
  ROFS ( (     m_pcRedundant1Ctrl                = new MbDataCtrl  () ) );
  RNOK  (       m_pcRedundant1Ctrl                ->init            ( rcSPS ) );
//RPIC bug fix

  //========== CREATE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  UInt  uiNum4x4Blocks        = m_uiFrameWidthInMb * m_uiFrameHeightInMb * 4 * 4;
  m_uiWriteBufferSize         = 3 * ( uiNum4x4Blocks * 4 * 4 );
  ROFS( ( m_pucWriteBuffer   = new UChar [ m_uiWriteBufferSize ] ) );
  ROT ( m_cDownConvert    .init   ( m_uiFrameWidthInMb<<4, m_uiFrameHeightInMb<<4 ) );

  //S051{
  ROFRS( m_auiFrameBits	=	new UInt[m_uiTotalFrame], Err::m_nERR );
  memset( m_auiFrameBits,0,sizeof(UInt)*m_uiTotalFrame);
  if(m_bEncSIP)
  {
    FILE* file=fopen(m_cInSIPFileName.c_str(),"rt");
    if(file==NULL)
    {
      printf("\nCan't open SIP file %s",m_cInSIPFileName.c_str());
      return Err::m_nOK;
    }
    while(!feof(file))
  {
    UInt tmp;
    fscanf(file,"%d",&tmp);
    m_cPocList.push_back(tmp);
  }
    fclose(file);	  
  }
  //S051}

  return Err::m_nOK;
}





ErrVal
LayerEncoder::xDeleteData()
{
  UInt uiIndex;

  //========== DELETE FRAME MEMORIES ==========
  if( m_papcFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcFrame[ uiIndex ] )
      {
    //JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
       m_papcFrame[uiIndex]->uninitChannelDistortion();
    //JVT-R057 LA-RDO}  
        RNOK(   m_papcFrame[ uiIndex ]->uninit() );
        RNOK( m_papcFrame[ uiIndex ]->destroy() );
        m_papcFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcFrame;
    m_papcFrame = 0;
  }
  
  if( m_papcOrgFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcOrgFrame[ uiIndex ] )
      {
        RNOK(   m_papcOrgFrame[ uiIndex ]->uninit() );
        RNOK( m_papcOrgFrame[ uiIndex ]->destroy() );
      }
    }
    delete [] m_papcOrgFrame;
    m_papcOrgFrame = 0;
  }
  
  if( m_papcCLRecFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcCLRecFrame[ uiIndex ] )
      {
        RNOK(   m_papcCLRecFrame[ uiIndex ]->uninit() );
        RNOK( m_papcCLRecFrame[ uiIndex ]->destroy() );
        m_papcCLRecFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcCLRecFrame;
    m_papcCLRecFrame = 0;
  }

  if( m_papcELFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcELFrame[ uiIndex ] )
      {
        RNOK(   m_papcELFrame[ uiIndex ]->uninit() );
        RNOK(   m_papcELFrame[ uiIndex ]->destroy() );
        m_papcELFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcELFrame;
    m_papcELFrame = 0;
  }

  if( m_papcResidual )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcResidual[ uiIndex ] )
      {
        RNOK(   m_papcResidual[ uiIndex ]->uninit() );
        RNOK(   m_papcResidual[ uiIndex ]->destroy() );
        m_papcResidual[ uiIndex ] = 0;
      }
    }
    delete [] m_papcResidual;
    m_papcResidual = 0;
  }
  
  if( m_papcSubband )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcSubband[ uiIndex ] )
      {
        RNOK(   m_papcSubband[ uiIndex ]->uninit() );
        RNOK( m_papcSubband[ uiIndex ]->destroy() );
        m_papcSubband[ uiIndex ] = 0;
      }
    }
    delete [] m_papcSubband;
    m_papcSubband = 0;
  }
  
  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )
  {
    if( m_apcFrameTemp[ uiIndex ] )
    {
      RNOK(   m_apcFrameTemp[ uiIndex ]->uninit() );
      RNOK( m_apcFrameTemp[ uiIndex ]->destroy() );
      m_apcFrameTemp[ uiIndex ] = 0;
    }
  }
  
  if( m_pcLowPassBaseReconstruction )
  {
  // JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
    m_pcLowPassBaseReconstruction->uninitChannelDistortion();
  // JVT-R057 LA-RDO}
    RNOK(   m_pcLowPassBaseReconstruction->uninit() );
    RNOK( m_pcLowPassBaseReconstruction->destroy() );
    m_pcLowPassBaseReconstruction = 0;
  }

  if( m_pcAnchorFrameOriginal )
  {
    RNOK(   m_pcAnchorFrameOriginal->uninit() );
    RNOK( m_pcAnchorFrameOriginal->destroy() );
    m_pcAnchorFrameOriginal = 0;
  }

  if( m_pcAnchorFrameReconstructed )
  {
    RNOK(   m_pcAnchorFrameReconstructed->uninit() );
    RNOK( m_pcAnchorFrameReconstructed->destroy() );
    m_pcAnchorFrameReconstructed = 0;
  }

  if( m_pcBaseLayerFrame )
  {
    RNOK(   m_pcBaseLayerFrame->uninit() );
    RNOK( m_pcBaseLayerFrame->destroy() );
    m_pcBaseLayerFrame = 0;
  }

  if( m_pcBaseLayerResidual )
  {
    RNOK(   m_pcBaseLayerResidual->uninit() );
    RNOK( m_pcBaseLayerResidual->destroy() );
    m_pcBaseLayerResidual = 0;
  }


  //========== DELETE MACROBLOCK DATA MEMORIES (and SLICE HEADER) ==========
  if( m_pacControlData )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      RNOK( m_pacControlData[ uiIndex ].uninitBQData() );
      RNOK( m_pacControlData[ uiIndex ].uninitFGSData() );

      MbDataCtrl*   pcMbDataCtrl  = m_pacControlData[ uiIndex ].getMbDataCtrl  ();
      if( pcMbDataCtrl )
      {
        RNOK( pcMbDataCtrl->uninit() );
      }
      delete pcMbDataCtrl;
      pcMbDataCtrl = 0 ;
      SliceHeader*  pcSliceHeader = m_pacControlData[ uiIndex ].getSliceHeader( FRAME );
      delete pcSliceHeader;
      pcSliceHeader = 0 ;
   
      if( m_pcLayerParameters->getPAff() )
      {
        pcSliceHeader = m_pacControlData[ uiIndex ].getSliceHeader( BOT_FIELD );
      delete pcSliceHeader;
        pcSliceHeader = 0;
      }
      if( m_pbFieldPicFlag )
      {
        delete [] m_pbFieldPicFlag;
        m_pbFieldPicFlag = 0;
      }
   
   
    }
    delete [] m_pacControlData;
    m_pacControlData = 0;
  }
//RPIC bug fix
	  if(m_pcRedundantCtrl)
  {
	  RNOK( m_pcRedundantCtrl->uninit() );
	  delete m_pcRedundantCtrl;
	  m_pcRedundantCtrl = 0 ;
  }
		  if(m_pcRedundant1Ctrl)
  {
	  RNOK( m_pcRedundant1Ctrl->uninit() );
	  delete m_pcRedundant1Ctrl;
	  m_pcRedundant1Ctrl = 0 ;
  }
//RPIC bug fix 
  // ICU/ETRI FGS_MOT_USE
  if( m_pacControlDataEL )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      RNOK( m_pacControlDataEL[ uiIndex ].uninitBQData() );
      RNOK( m_pacControlDataEL[ uiIndex ].uninitFGSData() );

      MbDataCtrl*   pcMbDataCtrlEL  = m_pacControlDataEL[ uiIndex ].getMbDataCtrl  ();
      SliceHeader*  pcSliceHeaderEL = m_pacControlDataEL[ uiIndex ].getSliceHeader ();
      if( pcMbDataCtrlEL )
      {
        RNOK( pcMbDataCtrlEL->uninit() );
      }
      delete pcMbDataCtrlEL;
      delete pcSliceHeaderEL;
    }
    delete [] m_pacControlDataEL;
    m_pacControlDataEL = 0;
  }


  if( m_pcBaseLayerCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->uninit() );
    delete m_pcBaseLayerCtrl;
    m_pcBaseLayerCtrl = 0;
  }

  if( m_pcBaseLayerCtrlField )
  {
    RNOK( m_pcBaseLayerCtrlField->uninit() );
    delete m_pcBaseLayerCtrlField;
    m_pcBaseLayerCtrlField = 0;
  }
  
  // ICU/ETRI FGS_MOT_USE
  if( m_pcBaseLayerCtrlEL )
  {
    RNOK( m_pcBaseLayerCtrlEL->uninit() );
    delete m_pcBaseLayerCtrlEL;
    m_pcBaseLayerCtrlEL = 0;
  }
  

  //========== DELETE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  delete [] m_pucWriteBuffer;
  m_pucWriteBuffer    = 0;
  m_uiWriteBufferSize = 0;

  //S051{
  delete[]	m_auiFrameBits;
  //S051}
  
  return Err::m_nOK;
}





ErrVal
LayerEncoder::uninit()
{
  m_bInitDone  = false;

  xDeleteData();

  if( m_pMotionInfoFile )
  {
    ::fclose( m_pMotionInfoFile );
  }
  if( m_pESSFile )
  {
    ::fclose( m_pESSFile );
  }

  // JVT-S054 (ADD) ->
  if( m_puiFirstMbInSlice )
  {
    free(m_puiFirstMbInSlice);
    m_puiFirstMbInSlice = 0;
  }
  if( m_puiLastMbInSlice )
  {
    free(m_puiLastMbInSlice);
    m_puiLastMbInSlice = 0;
  }
  // JVT-S054 (ADD) <-

  delete [] m_apabBaseModeFlagAllowedArrays[0];
  delete [] m_apabBaseModeFlagAllowedArrays[1];
  m_apabBaseModeFlagAllowedArrays[0] = 0;
  m_apabBaseModeFlagAllowedArrays[1] = 0;

  return Err::m_nOK;
}





ErrVal
LayerEncoder::xFillAndUpsampleFrame( Frame* pcFrame, PicType ePicType, Bool bFrameMbsOnlyFlag )
{
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );
  RNOK( m_pcYuvHalfPelBufferCtrl->initMb() );

  if( ! pcFrame->isHalfPel() )
  {
    XPel* pHPData = NULL;
    RNOK( pcFrame->initHalfPel( pHPData ) );
  }

  RNOK( pcFrame->extendFrame( m_pcQuarterPelFilter, ePicType, bFrameMbsOnlyFlag ) );

  return Err::m_nOK;
}





ErrVal
LayerEncoder::xFillAndExtendFrame( Frame* pcFrame, PicType ePicType, Bool bFrameMbsOnlyFlag )
{
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );

  RNOK( pcFrame->extendFrame( NULL, ePicType, bFrameMbsOnlyFlag ) );

  return Err::m_nOK;
}





ErrVal
LayerEncoder::xMotionEstimation( RefFrameList*    pcRefFrameList0,
                                RefFrameList*    pcRefFrameList1,
                                Frame*        pcOrigFrame,
                                Frame*        pcIntraRecFrame,
                                ControlData&     rcControlData,
                                Bool             bBiPredOnly,
                                UInt             uiNumMaxIter,
                                UInt             uiIterSearchRange,
                                UInt             uiFrameIdInGOP,
                                PicType          ePicType )
{
  MbEncoder*    pcMbEncoder           =  m_pcSliceEncoder->getMbEncoder         ();
  SliceHeader&  rcSliceHeader         = *rcControlData.getSliceHeader           ( ePicType );
  MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
  Frame*        pcBaseLayerFrame      =  rcControlData.getBaseLayerRec          ();
  Frame*        pcBaseLayerResidual   =  rcControlData.getBaseLayerSbb          ();
  MbDataCtrl*   pcBaseLayerCtrl       =  rcControlData.getBaseLayerCtrl         ();
  Bool          bEstimateBase         =  rcSliceHeader.getNoInterLayerPredFlag() && ! pcBaseLayerCtrl;
  Bool          bEstimateMotion       =  rcSliceHeader.getAdaptiveBaseModeFlag() || bEstimateBase;

  // JVT-S054 (ADD)
  MbDataCtrl*     pcMbDataCtrlL1    = xGetMbDataCtrlL1( rcSliceHeader, uiFrameIdInGOP, pcRefFrameList1 );

// JVT-S054 (REPLACE) ->
  //===== copy motion if non-adaptive prediction =====
  if( ! bEstimateMotion )
  {
    ROF ( pcBaseLayerCtrl )

    if (m_bIroiSliceDivisionFlag)
    {
      for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
      {
        rcSliceHeader.setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
        rcSliceHeader.setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
        // JVT-S054 (2) (ADD)
        rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
        UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
        UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
        UInt uiNumMBInSlice;
        //===== initialization =====
        RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );

        //===== loop over macroblocks =====
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
        {
          UInt          uiMbY, uiMbX;
          MbDataAccess* pcMbDataAccess      = 0;
          MbDataAccess* pcMbDataAccessBase  = 0;
          
          rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress            );


          //===== init macroblock =====
          RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
          if    ( pcBaseLayerCtrl )
          {
            RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
          }

          RNOK( pcMbDataCtrl->getMbData(uiMbX, uiMbY).copyMotion( pcBaseLayerCtrl->getMbData(uiMbX, uiMbY), pcMbDataCtrl->getSliceId() ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_0 ).setRefPicIdcs( pcRefFrameList0 ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_1 ).setRefPicIdcs( pcRefFrameList1 ) );
          // <<<< bug fix by heiko.schwarz@hhi.fhg.de
          if( pcBaseLayerFrame ) // the motion data are not just copied, but inferred from the base layer
          {
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_0 ).clear();
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_1 ).clear();
          }
          // >>>> bug fix by heiko.schwarz@hhi.fhg.de
          uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
          uiNumMBInSlice++;
        }
      }
    }
    else
    {
      FMO* pcFMO = rcControlData.getSliceHeader( ePicType )->getFMO(); //TMM
    
      for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
      {
        rcSliceHeader.setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
        rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
        // JVT-S054 (2) (ADD)
        rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
        UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
        UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
        UInt uiNumMBInSlice;
        //===== initialization =====
        RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );

        //===== loop over macroblocks =====
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
        {
          UInt          uiMbY,  uiMbX;
          MbDataAccess* pcMbDataAccess      = 0;
          MbDataAccess* pcMbDataAccessBase  = 0;

           rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress            );

          
          //===== init macroblock =====
          RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
          if    ( pcBaseLayerCtrl )
          {
            RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
          }

          RNOK( pcMbDataCtrl->getMbData(uiMbX, uiMbY).copyMotion( pcBaseLayerCtrl->getMbData(uiMbX, uiMbY), pcMbDataCtrl->getSliceId() ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_0 ).setRefPicIdcs( pcRefFrameList0 ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_1 ).setRefPicIdcs( pcRefFrameList1 ) );
          // <<<< bug fix by heiko.schwarz@hhi.fhg.de
          if( pcBaseLayerFrame ) // the motion data are not just copied, but inferred from the base layer
          {
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_0 ).clear();
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_1 ).clear();
          }
          // >>>> bug fix by heiko.schwarz@hhi.fhg.de
          uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
          uiNumMBInSlice++;
        }
      }
    }
    return Err::m_nOK;
  }

  
  
  if( ePicType!=FRAME )
      {
        if( pcOrigFrame )         RNOK( pcOrigFrame        ->addFieldBuffer( ePicType ) );
        if( pcIntraRecFrame )     RNOK( pcIntraRecFrame    ->addFieldBuffer( ePicType ) );
        if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->addFieldBuffer( ePicType ) );
        if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->addFieldBuffer( ePicType ) );
      }

  
  if (m_bIroiSliceDivisionFlag)
  {
    for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
    {
      rcSliceHeader.setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
      rcSliceHeader.setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
      // JVT-S054 (2) (ADD)
      rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
      UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
      UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
      UInt uiNumMBInSlice;

      //===== initialization =====
      RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
      if( ! m_bLoadMotionInfo )
      {
        RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
        RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
        RNOK( m_pcMotionEstimation->getSW()->initSlice( rcSliceHeader ) );
      }

      
      //===== loop over macroblocks =====
     for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
      {
    MbDataAccess* pcMbDataAccess     = NULL;
    MbDataAccess* pcMbDataAccessBase = NULL;
    UInt          uiMbY, uiMbX;
    Double        dCost              = 0;

    rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress            );

        //===== init macroblock =====
        RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
        if    ( pcBaseLayerCtrl )
        {
          RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
        }

        if( ! m_bLoadMotionInfo )
        {
          //===== initialisation =====
            RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
            RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
            RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

            if( ! rcSliceHeader.getNoInterLayerPredFlag() )
            {
              m_pcSliceEncoder->m_pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[0][uiMbAddress] );
            }

          //===== estimate prediction data =====
          RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
                            pcMbDataAccessBase,
                            *pcRefFrameList0,
                            *pcRefFrameList1,
                             pcBaseLayerFrame    ? pcBaseLayerFrame   ->getPic( ePicType ) : NULL,
                             pcBaseLayerResidual ? pcBaseLayerResidual->getPic( ePicType ) : NULL,
                             *pcOrigFrame                              ->getPic( ePicType ),
                             *pcIntraRecFrame                          ->getPic( ePicType ),
                             bBiPredOnly,
                             uiNumMaxIter,
                             uiIterSearchRange,
                             m_bBLSkipEnable, //JVT-Q065 EIDR
                             rcControlData.getLambda(),
                             dCost,
                             true ) );
// TMM_INTERLACE{
         /* if( m_bSaveMotionInfo )
          {
            //===== save prediction data =====
// saveAll is displaced because the Mvs are Ok but the other data could be modified (mode,...).
// Do it after m_pcSliceEncoder->encodeHighPassPicture
//            RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
          }*/
// TMM_INTERLACE}
        }
        else
        {
          //===== load prediction data =====
            RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
        }
        uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
        uiNumMBInSlice++;
      }
    }
  }
  else
  {
   
    FMO* pcFMO = rcControlData.getSliceHeader( ePicType )->getFMO(); //TMM
 
    for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
    {
      rcSliceHeader.setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
      rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
      // JVT-S054 (2) (ADD)
      rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
      UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
      UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
      UInt uiNumMBInSlice;


      //===== initialization =====
      RefFrameList rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
      if( rcSliceHeader.isMbaffFrame() )
        RNOK( pcMbDataCtrl->initUsedField( rcSliceHeader, rcRefFrameList1 ) );

      //===== initialization =====
      RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
      if( ! m_bLoadMotionInfo )
      {
        RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
        RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
        RNOK( m_pcMotionEstimation->getSW()->initSlice( rcSliceHeader ) );
      }

      //===== loop over macroblocks =====
      //for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
      for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
      {
        MbDataAccess* pcMbDataAccess      = 0;
        MbDataAccess* pcMbDataAccessBase  = 0;

        UInt          uiMbY, uiMbX;
        Double        dCost              = 0;

        rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress     );

        //===== init macroblock =====
        RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
        if    ( pcBaseLayerCtrl )
        {
          RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
          if (rcSliceHeader.getTCoeffLevelPredictionFlag())
            pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );
        }

        if( ! m_bLoadMotionInfo )
        {
          //===== initialisation =====
          RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
          RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
          RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

          if( !rcSliceHeader.getNoInterLayerPredFlag() )
          {
            pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[0][uiMbAddress] );
          }

          RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
                                                  pcMbDataAccessBase,
                                                  *pcRefFrameList0,
                                                  *pcRefFrameList1,
                                                 
                                                  pcBaseLayerFrame    ? pcBaseLayerFrame   ->getPic( ePicType ) : NULL,
                                                  pcBaseLayerResidual ? pcBaseLayerResidual->getPic( ePicType ) : NULL,
                                                  *pcOrigFrame                              ->getPic( ePicType ),
                                                  *pcIntraRecFrame                          ->getPic( ePicType ),
                                                  
                                                  bBiPredOnly,
                                                  uiNumMaxIter,
                                                  uiIterSearchRange,
                                                  m_bBLSkipEnable, //JVT-Q065 EIDR
                                                  rcControlData.getLambda(),
                                                  dCost,
                                                  true ) );
// TMM_INTERLACE{
         /* if( m_bSaveMotionInfo )
          {
            //===== save prediction data =====
           // saveAll is displaced because the Mvs are Ok but the other data could be modified (mode,...).
           // Do it after m_pcSliceEncoder->encodeHighPassPicture
           //            RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
          }*/
// TMM_INTERLACE}
        }
        else
        {
          //===== load prediction data =====
            RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
        }
        uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
        uiNumMBInSlice++;
      }
    }
  }

  if( ePicType!=FRAME )
  {
    if( pcOrigFrame )         RNOK( pcOrigFrame        ->removeFieldBuffer( ePicType ) );
    if( pcIntraRecFrame )     RNOK( pcIntraRecFrame    ->removeFieldBuffer( ePicType ) );
    if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->removeFieldBuffer( ePicType ) );
    if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->removeFieldBuffer( ePicType ) );
  }
  
  return Err::m_nOK;
}




ErrVal
LayerEncoder::xMotionCompensation( Frame*        pcMCFrame,
                                  RefFrameList*    pcRefFrameList0,
                                  RefFrameList*    pcRefFrameList1,
                                  MbDataCtrl*      pcMbDataCtrl,
                                  SliceHeader&     rcSH )
{
  Bool        bCalcMv         = false;
  Bool        bFaultTolerant  = false;
  MbEncoder*  pcMbEncoder     = m_pcSliceEncoder->getMbEncoder();

  RNOK( pcMbDataCtrl        ->initSlice( rcSH, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcMotionEstimation->initSlice( rcSH              ) );

  MbDataCtrl*      pcBaseMbDataCtrl = getBaseMbDataCtrl();

  RefFrameList* apcRefFrameList0[4] = { NULL, NULL, NULL, NULL };
  RefFrameList* apcRefFrameList1[4] = { NULL, NULL, NULL, NULL };
  const PicType ePicType = rcSH.getPicType();
  const Bool    bMbAff   = rcSH.isMbaffFrame   ();
  Frame* apcBLFrame[3] = { 0, 0, m_pcBaseLayerFrame };
  
  if( bMbAff )
  {
    RefFrameList acRefFrameList0[2];
    RefFrameList acRefFrameList1[2];
  
    RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
    RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );
  
    apcRefFrameList0[ TOP_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
    apcRefFrameList0[ BOT_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
    apcRefFrameList1[ TOP_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
    apcRefFrameList1[ BOT_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];
    apcRefFrameList0[     FRAME ] = pcRefFrameList0;
    apcRefFrameList1[     FRAME ] = pcRefFrameList1;

    if( m_pcBaseLayerFrame )
    {
      m_pcBaseLayerFrame->addFrameFieldBuffer();
      apcBLFrame[0] = m_pcBaseLayerFrame->getPic( TOP_FIELD );
      apcBLFrame[1] = m_pcBaseLayerFrame->getPic( BOT_FIELD );
    }
  }
  else
  {
    RNOK( pcMCFrame->addFieldBuffer( ePicType ) );
    apcRefFrameList0[ ePicType ] = pcRefFrameList0;
    apcRefFrameList1[ ePicType ] = pcRefFrameList1;
  }

  //===== loop over macroblocks =====
  const UInt uiMbNumber = rcSH.getMbInPic(); //TMM
  for( UInt uiMbAddress = 0 ; uiMbAddress < uiMbNumber; uiMbAddress++ ) //TMM
  {
    MbDataAccess* pcMbDataAccess = NULL;
    MbDataAccess* pcMbDataAccessBase  = 0;
    UInt          uiMbY, uiMbX;

    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress                                );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX                  ) );
    if    ( pcBaseMbDataCtrl )
    {
      RNOK( pcBaseMbDataCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX, bMbAff          ) );
    RNOK( m_pcYuvHalfPelBufferCtrl->initMb(                 uiMbY, uiMbX, bMbAff          ) );
    RNOK( m_pcMotionEstimation    ->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );

    const PicType eMbPicType = pcMbDataAccess->getMbPicType();
    pcMbDataAccess->setMbDataAccessBase(pcMbDataAccessBase);
    pcMbEncoder->setBaseLayerRec( apcBLFrame[ eMbPicType - 1 ] );

    RNOK( pcMbEncoder->compensatePrediction( *pcMbDataAccess, 
                                              pcMCFrame->getPic( eMbPicType ),
                                             *apcRefFrameList0 [ eMbPicType ],
                                             *apcRefFrameList1 [ eMbPicType ],
                                             bCalcMv, bFaultTolerant) );

  }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xGetConnections( Double&       rdL0Rate,
                              Double&       rdL1Rate,
                              Double&       rdBiRate )
{
  //=== just a guess ===
  if( m_bHaarFiltering )
  {
    rdL0Rate = 1.0;
    rdL1Rate = 0.0;
    rdBiRate = 0.0;
  }
  else if( m_bBiPredOnly )
  {
    rdL0Rate = 0.0;
    rdL1Rate = 0.0;
    rdBiRate = 1.0;
  }
  else
  {
    rdL0Rate = 0.2;
    rdL1Rate = 0.2;
    rdBiRate = 0.6;
  }
  return Err::m_nOK;
}




ErrVal
LayerEncoder::xZeroIntraMacroblocks( Frame*    pcFrame,
                                    ControlData& rcCtrlData, 
                                    PicType      ePicType )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      (ePicType);
  

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  YuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

  const Bool bMbAff = pcSliceHeader->isMbaffFrame();
  if( ePicType!=FRAME )
  {
    RNOK( pcFrame->addFieldBuffer     ( ePicType ) );
  }
  else if( bMbAff )
  {
    RNOK( pcFrame->addFrameFieldBuffer() );
  }

  //===== loop over macroblocks =====
  const UInt uiMbNumber = pcSliceHeader->getMbInPic(); //TMM
  for( UInt uiMbAddress = 0 ; uiMbAddress < uiMbNumber; uiMbAddress++ ) //TMM
  {
    MbDataAccess* pcMbDataAccess = NULL;
    UInt          uiMbY, uiMbX;

    pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress              );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb (                 uiMbY, uiMbX, bMbAff ) );

    if( pcMbDataAccess->getMbData().isIntra() )
    {
      const PicType eMbPicType = pcMbDataAccess->getMbPicType();
      pcFrame->getPic( eMbPicType )->getFullPelYuvBuffer()->loadBuffer( &cZeroMbBuffer );
    }
  }

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
LayerEncoder::xClipIntraMacroblocks( Frame*    pcFrame,
                                    ControlData& rcCtrlData, 
                                    Bool         bClipAll,
                                    PicType      ePicType )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader* pcSliceHeader = rcCtrlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );
  YuvPicBuffer* pcPicBuffer;
  YuvMbBuffer    cMbBuffer;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  const Bool bMbAff = pcSliceHeader->isMbaffFrame();
  if( ePicType!=FRAME )
  {
    RNOK( pcFrame->addFieldBuffer     ( ePicType ) );
  }
  else if( bMbAff )
  {
    RNOK( pcFrame->addFrameFieldBuffer() );
  }

  //===== loop over macroblocks =====
  const UInt uiMbNumber = pcSliceHeader->getMbInPic();//TMM
  for( UInt uiMbAddress = 0 ; uiMbAddress < uiMbNumber; uiMbAddress++ )//TMM
  {
    MbDataAccess* pcMbDataAccess = 0;
    UInt          uiMbY, uiMbX;

    pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress              );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb (                 uiMbY, uiMbX, bMbAff ) );

    if( bClipAll || pcMbDataAccess->getMbData().isIntra() )
    {
      const PicType eMbPicType = pcMbDataAccess->getMbPicType();
      pcPicBuffer = pcFrame->getPic( eMbPicType )->getFullPelYuvBuffer();
      cMbBuffer   .loadBuffer( pcPicBuffer );
      cMbBuffer   .clip      ();
      pcPicBuffer->loadBuffer( &cMbBuffer );
    }
  }
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
LayerEncoder::xInitExtBinDataAccessor( ExtBinDataAccessor& rcExtBinDataAccessor )
{
  ROF( m_pucWriteBuffer );
  m_cBinData.reset          ();
  m_cBinData.set            ( m_pucWriteBuffer, m_uiWriteBufferSize );
  m_cBinData.setMemAccessor ( rcExtBinDataAccessor );

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xAppendNewExtBinDataAccessor( ExtBinDataAccessorList& rcExtBinDataAccessorList,
                                           ExtBinDataAccessor*     pcExtBinDataAccessor )
{
  ROF( pcExtBinDataAccessor );
  ROF( pcExtBinDataAccessor->data() );

  UInt    uiNewSize     = pcExtBinDataAccessor->size();
  UChar*  pucNewBuffer  = new UChar [ uiNewSize ];
  ROF( pucNewBuffer );
  ::memcpy( pucNewBuffer, pcExtBinDataAccessor->data(), uiNewSize * sizeof( UChar ) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor );

  m_cBinData              .reset          ();
  m_cBinData              .set            (  pucNewBuffer, uiNewSize );
  m_cBinData              .setMemAccessor ( *pcNewExtBinDataAccessor );
  rcExtBinDataAccessorList.push_back      (  pcNewExtBinDataAccessor );

  m_cBinData              .reset          ();
  m_cBinData              .setMemAccessor ( *pcExtBinDataAccessor );

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xAddBaseLayerResidual( ControlData& rcControlData,
                                    Frame*    pcFrame,
                                    Bool				 bSubtract,
                                    PicType      ePicType )
{
  ROFRS( rcControlData.getBaseLayerSbb(), Err::m_nOK );

  MbDataCtrl*       pcMbDataCtrl  = rcControlData.getMbDataCtrl       ();
  SliceHeader*  pcSliceHeader     = rcControlData.getSliceHeader( ePicType );
  Frame* pcBLResidual          = rcControlData.getBaseLayerSbb();

  YuvMbBuffer    cBLResBuffer;
  YuvMbBuffer    cMbBuffer;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  const Bool bMbAff = pcSliceHeader->isMbaffFrame();
  if( ePicType!=FRAME )
  {
    RNOK( pcBLResidual->addFieldBuffer     ( ePicType ) );
    RNOK( pcFrame     ->addFieldBuffer     ( ePicType ) );
  }
  else if( bMbAff )
  {
    RNOK( pcBLResidual->addFrameFieldBuffer() );
    RNOK( pcFrame     ->addFrameFieldBuffer() );
  }

  //===== loop over macroblocks =====
  const UInt uiMbNumber = pcSliceHeader->getMbInPic(); //TMM
  for( UInt uiMbAddress = 0 ; uiMbAddress < uiMbNumber; uiMbAddress++ ) //TMM
  {
    MbDataAccess* pcMbDataAccess = NULL;
    UInt          uiMbY, uiMbX;

    pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress              );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb (                 uiMbY, uiMbX, bMbAff ) );

    if( ! pcMbDataAccess->getMbData().isIntra() && pcMbDataAccess->getMbData().getResidualPredFlag() )
    {
      const PicType eMbPicType  = pcMbDataAccess->getMbPicType();
      cMbBuffer   .loadBuffer( pcFrame     ->getPic( eMbPicType )->getFullPelYuvBuffer() );
      cBLResBuffer.loadBuffer( pcBLResidual->getPic( eMbPicType )->getFullPelYuvBuffer() );
      
      if( bSubtract ) cMbBuffer.subtract( cBLResBuffer );
      else            cMbBuffer.add     ( cBLResBuffer );

      RNOK( pcFrame->getPic( eMbPicType )->getFullPelYuvBuffer()->loadBuffer( &cMbBuffer ) );
    }
  }
  if( ePicType!=FRAME )
  {
    RNOK( pcBLResidual->removeFieldBuffer     ( ePicType ) );
    RNOK( pcFrame     ->removeFieldBuffer     ( ePicType ) );
    }
  else if( bMbAff )
  {
    RNOK( pcBLResidual->removeFrameFieldBuffer()           );
    RNOK( pcFrame     ->removeFrameFieldBuffer()           );
  }

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xEncodeLowPassSignal( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
                                   ControlData&             rcControlData,
                                   Frame*               pcOrgFrame,
                                   Frame*                pcFrame,
                                   Frame*                pcResidual,
                                   Frame*                pcPredSignal,
                                   UInt&                   ruiBits,
                                   PicOutputDataList&       rcPicOutputDataList,
                                   PicType                 ePicType )
{
  UInt          uiBits              = 0;
  UInt          uiBitsSEI           = 0;
  Frame*     pcBaseLayerFrame    = rcControlData.getBaseLayerRec ();
  SliceHeader* pcSliceHeader       = rcControlData.getSliceHeader ( ePicType );
  ROF( pcSliceHeader );   
  const Bool   bFrameMbsOnlyFlag   = pcSliceHeader->getSPS().getFrameMbsOnlyFlag();
  
  //----- subsequence SEI -----
  if( m_bWriteSubSequenceSei && m_bH264AVCCompatible )
  {
    RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBitsSEI ) );
  }

  // JVT-S054 (ADD) ->
  if (m_bIroiSliceDivisionFlag)
  {
    for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
    {
      pcSliceHeader->setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
      pcSliceHeader->setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
      // JVT-S054 (2) (ADD)
      pcSliceHeader->setNumMbsInSlice(pcSliceHeader->getFMO()->getNumMbsInSlice(pcSliceHeader->getFirstMbInSlice(), pcSliceHeader->getLastMbInSlice()));

      if(m_uiMMCOBaseEnable  && m_bH264AVCCompatible)  //bug-fix suffix  
      {//JVT-S036 lsj
        UInt iFrameNum =  m_pcLowPassBaseReconstruction->getFrameNum(); 
        RNOK( xSetMmcoBase( *pcSliceHeader, iFrameNum ) );  
      }
//prefix unit{{
    if( m_uiPreAndSuffixUnitEnable ) 
    {
        if ( rcControlData.getSliceHeader( ePicType )->getNalUnitType() == NAL_UNIT_CODED_SLICE|| rcControlData.getSliceHeader( ePicType )->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
        {
          RNOK( xWritePrefixUnit( rcOutExtBinDataAccessorList, *rcControlData.getSliceHeader( ePicType ), uiBits ) );
        }
    }
//prefix unit}}
      //----- init NAL UNIT -----
      RNOK( xInitExtBinDataAccessor                 (  m_cExtBinDataAccessor ) );
      RNOK( m_pcNalUnitEncoder->initNalUnit         ( &m_cExtBinDataAccessor ) );

      //---- write Slice Header -----
      ETRACE_NEWSLICE;
      xAssignSimplePriorityId( pcSliceHeader );
      if( pcSliceHeader->getSliceSkipFlag() )
      {
        UInt uiNumSkipSlices  = pcSliceHeader->getNumMbsInSlice();
        ROF( uiNumSkipSlices  > 0 );
        pcSliceHeader->setNumMbsInSliceMinus1( uiNumSkipSlices - 1 );
      }

      RNOK( m_pcNalUnitEncoder->write               ( *pcSliceHeader ) );

      //----- initialize reference lists -----
      rcControlData.getPrdFrameList( LIST_0 ).reset();
      rcControlData.getPrdFrameList( LIST_1 ).reset();

      //----- encode slice data -----
      if( pcSliceHeader->isIntraSlice() )
      {
        if( pcSliceHeader->isMbaffFrame() )
        {
          RNOK( m_pcSliceEncoder->encodeIntraPictureMbAff( uiBits,
                                                           rcControlData,
                                                           pcOrgFrame,
                                                           pcFrame,
                                                           pcResidual,
                                                           pcBaseLayerFrame,
                                                           pcPredSignal,
                                                           m_uiFrameWidthInMb,
                                                           rcControlData.getLambda() ) );
        }
        else
        {
          RNOK( m_pcSliceEncoder->encodeIntraPicture  ( uiBits,
            rcControlData,
            pcFrame,
            pcResidual,
            pcBaseLayerFrame,
            pcPredSignal,
            m_uiFrameWidthInMb,
            rcControlData.getLambda(),
            ePicType ) );
        }
      }
      else //P_SLICE
      {
        //----- initialize reference lists -----
        //ROF ( pcSliceHeader->getNumRefIdxActive ( LIST_0 ) == 1 );
        ROF ( pcSliceHeader->getNumRefIdxActive ( LIST_1 ) == 0 );

        //===== get reference frame lists =====
        RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );

        RefFrameList  cDPCMRefFrameListBase;
        cDPCMRefFrameListBase.reset();

        if(!m_pcLowPassBaseReconstruction->getUnusedForRef())  // JVT-Q065 EIDR
        {
          if( ePicType==FRAME )
          {
            RNOK( xFillAndUpsampleFrame( m_pcLowPassBaseReconstruction,         ePicType, bFrameMbsOnlyFlag ) );
            RNOK( rcRefFrameList0.add  ( m_pcLowPassBaseReconstruction->getPic( ePicType )                  ) );
          }
          else//ePicType!=FRAME
          {
            RNOK( xGetPredictionListsFieldKey( rcRefFrameList0, pcSliceHeader->getNumRefIdxActive( LIST_0 ), ePicType ) );
          }
        }

        if( pcSliceHeader->isMbaffFrame() )
        {
          RNOK( m_pcSliceEncoder->encodeInterPicturePMbAff( uiBits,
                                                            pcOrgFrame,
                                                            pcFrame,
                                                            pcResidual,
                                                            pcPredSignal,
                                                            rcControlData,
                                                            m_uiFrameWidthInMb,
                                                            rcRefFrameList0,
                                                            cDPCMRefFrameListBase ) );
        }
        else
        {
          RNOK( m_pcSliceEncoder->encodeInterPictureP ( uiBits,
            pcFrame,
            pcResidual,
            pcPredSignal,
            rcControlData,
            m_uiFrameWidthInMb,
            rcRefFrameList0,
            cDPCMRefFrameListBase,
            ePicType ) );
        }

        RNOK( m_pcLowPassBaseReconstruction->uninitHalfPel() );
        RNOK( m_pcLowPassBaseReconstruction->removeFieldBuffer( ePicType ) );
      }

      //----- close NAL UNIT -----
      RNOK( m_pcNalUnitEncoder->closeNalUnit        ( uiBits ) );
      RNOK( xAppendNewExtBinDataAccessor            ( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
      uiBits += 4*8;

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = pcSliceHeader->getNoInterLayerPredFlag() && pcSliceHeader->getFirstMbInSlice() == 0;
      cPicOutputData.iPicType      = pcSliceHeader->getPicType();
      cPicOutputData.Poc           = pcSliceHeader->getPoc                   ();
      cPicOutputData.FrameType[0]  = pcSliceHeader->getSliceType             () == B_SLICE ? 'B' :
                                     pcSliceHeader->getSliceType             () == P_SLICE ? 'P' : 'I';
      cPicOutputData.FrameType[1]  = pcSliceHeader->getUseRefBasePicFlag()            ? 'K' : ' ';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = pcSliceHeader->getLayerCGSSNR           ();
      cPicOutputData.QualityId     = pcSliceHeader->getQualityLevelCGSSNR    ();
      cPicOutputData.TemporalId    = pcSliceHeader->getTemporalId         ();
      cPicOutputData.Qp            = pcSliceHeader->getSliceQp                 ();        
      cPicOutputData.Bits          = uiBits + uiBitsSEI;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;
      // JVT-V068 {
      cPicOutputData.iPicType      = (Int)ePicType;
      if (  m_uiQualityLevelCGSSNR != 0 || m_uiLayerCGSSNR != rcControlData.getSliceHeader()->getDependencyId()  )
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerCGSSNR;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      else
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerId;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      // JVT-V068 }
      rcPicOutputDataList.push_back( cPicOutputData );
      //S051{
      if(m_uiAnaSIP>0)
        m_auiFrameBits[rcControlData.getSliceHeader( ePicType )->getPoc()]=uiBits+uiBitsSEI;  // TMM
      //S051}

      if( ePicType != TOP_FIELD ) { ETRACE_NEWFRAME; }

      if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits + uiBitsSEI;
      }
      else
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits + uiBitsSEI;
      }

      ruiBits = ruiBits + uiBits + uiBitsSEI;
      uiBitsSEI=0;
    }
  }
  else
  {
  // JVT-S054 (ADD) <-
    FMO* pcFMO = pcSliceHeader->getFMO();
    for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)  
    {
			//JVT-X046 {
			rcControlData.m_bSliceGroupAllCoded = false;
			rcControlData.m_uiCurrentFirstMB = pcFMO->getFirstMacroblockInSlice(iSliceGroupID);
			while(!rcControlData.m_bSliceGroupAllCoded)
			{
			if (!m_uiSliceMode)
				rcControlData.m_bSliceGroupAllCoded = true;
			//JVT-X046 }
      pcSliceHeader->setFirstMbInSlice(rcControlData.m_uiCurrentFirstMB);
      pcSliceHeader->setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
      // JVT-S054 (2) (ADD)
      pcSliceHeader->setNumMbsInSlice(pcFMO->getNumMbsInSlice(pcSliceHeader->getFirstMbInSlice(), pcSliceHeader->getLastMbInSlice()));

      if(m_uiMMCOBaseEnable  && m_bH264AVCCompatible)  //bug-fix suffix  
      {//JVT-S036 lsj
        UInt iFrameNum =  m_pcLowPassBaseReconstruction->getFrameNum(); 
        RNOK( xSetMmcoBase( *pcSliceHeader, iFrameNum ) );  
      }
//prefix unit{{
      if( m_uiPreAndSuffixUnitEnable ) 
      {
// TMM {	   
        if ( rcControlData.getSliceHeader( ePicType )->getNalUnitType() == NAL_UNIT_CODED_SLICE|| rcControlData.getSliceHeader( ePicType )->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
        {
          RNOK( xWritePrefixUnit( rcOutExtBinDataAccessorList, *rcControlData.getSliceHeader( ePicType ), uiBits ) );
        }
 // TMM }
      }
//prefix unit}}
      //----- init NAL UNIT -----
      RNOK( xInitExtBinDataAccessor                 (  m_cExtBinDataAccessor ) );
      RNOK( m_pcNalUnitEncoder->initNalUnit         ( &m_cExtBinDataAccessor ) );

      //---- write Slice Header -----
      ETRACE_NEWSLICE;
      xAssignSimplePriorityId( pcSliceHeader );
      if( pcSliceHeader->getSliceSkipFlag() )
      {
        UInt uiMaxMbsInSlice  = ( m_uiSliceMode == 1 ? m_uiSliceArgument : MSYS_UINT_MAX );
        UInt uiNumSkipSlices  = min( uiMaxMbsInSlice, pcSliceHeader->getNumMbsInSlice() );
        ROF( uiNumSkipSlices  > 0 );
        pcSliceHeader->setNumMbsInSliceMinus1( uiNumSkipSlices - 1 );
      }

			//JVT-X046 {
			m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode = m_uiSliceMode;
			m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument = m_uiSliceArgument;
			if (m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode==2)
			{
				//subtract slice header size and 4*8 bytes from Slice Argument
				m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument -= 
					m_pcNalUnitEncoder->xGetBitsWriteBuffer()->getBitsWritten() + 4*8 +20;
			}
			//JVT-X046 }

      RNOK( m_pcNalUnitEncoder->write               ( *pcSliceHeader ) );

      rcControlData.getPrdFrameList( LIST_0 ).reset();
      rcControlData.getPrdFrameList( LIST_1 ).reset();

      //----- encode slice data -----
      if( pcSliceHeader->isIntraSlice() )
      { 
       if( pcSliceHeader->isMbaffFrame() )
       {
        RNOK( m_pcSliceEncoder->encodeIntraPictureMbAff( uiBits,
                                                    rcControlData,
                                                    pcOrgFrame,
                                                    pcFrame,
                                                    pcResidual,
                                                    pcBaseLayerFrame,
                                                    pcPredSignal,
                                                    m_uiFrameWidthInMb,
                                                    rcControlData.getLambda() ) );
       }
       else
       {
       RNOK( m_pcSliceEncoder->encodeIntraPicture  ( uiBits,
                                                     rcControlData,
                                                     pcFrame,
                                                     pcResidual,
                                                     pcBaseLayerFrame,
                                                     pcPredSignal,
                                                     m_uiFrameWidthInMb,
                                                     rcControlData.getLambda(),
                                                     ePicType ) );
        }
      }
      else
      {
        //----- initialize reference lists -----
        ROF ( pcSliceHeader->getNumRefIdxActive ( LIST_0 ) == 1 );
        ROF ( pcSliceHeader->getNumRefIdxActive ( LIST_1 ) == 0 );

        //===== get reference frame lists =====
        RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );

        RefFrameList  cDPCMRefFrameListBase;
        cDPCMRefFrameListBase.reset();

        if(!m_pcLowPassBaseReconstruction->getUnusedForRef())  // JVT-Q065 EIDR
        {
          if( ePicType==FRAME )
          {
                RNOK( xFillAndUpsampleFrame                         ( m_pcLowPassBaseReconstruction,ePicType, bFrameMbsOnlyFlag ) );
                RNOK( rcRefFrameList0.add  ( m_pcLowPassBaseReconstruction->getPic( ePicType )                  ) );
          }
          else//ePicType!=FRAME
          {
            RNOK( xGetPredictionListsFieldKey( rcRefFrameList0, pcSliceHeader->getNumRefIdxActive( LIST_0 ), ePicType ) );
          }
        }

        if( pcSliceHeader->isMbaffFrame() )
        {
          RNOK( m_pcSliceEncoder->encodeInterPicturePMbAff( uiBits,
                                                            pcOrgFrame,
                                                            pcFrame,
                                                            pcResidual,
                                                            pcPredSignal,
                                                            rcControlData,
                                                            m_uiFrameWidthInMb,
                                                            rcRefFrameList0,
                                                            cDPCMRefFrameListBase ) );
        }
        else
        {
          RNOK( m_pcSliceEncoder->encodeInterPictureP ( uiBits,
                                                        pcFrame,
                                                        pcResidual,
                                                        pcPredSignal,
                                                        rcControlData,
                                                        m_uiFrameWidthInMb,
                                                        rcRefFrameList0,
                                                        cDPCMRefFrameListBase,
                                                        ePicType ) );
    
        }

        RNOK( m_pcLowPassBaseReconstruction->uninitHalfPel() );
        RNOK( m_pcLowPassBaseReconstruction->removeFieldBuffer( ePicType ) );
      }

      //----- close NAL UNIT -----
      UInt auiBits[16];
      RNOK( m_pcNalUnitEncoder->closeAndAppendNalUnits( auiBits,
                                                        rcOutExtBinDataAccessorList,
                                                        &m_cExtBinDataAccessor,
                                                        m_cBinData,
                                                        m_pcH264AVCEncoder,
                                                        m_uiQualityLevelCGSSNR,
                                                        m_uiLayerCGSSNR ) );
      uiBits = auiBits[0] + 4*8;


      //JVT-W052
      if(m_pcH264AVCEncoder->getCodingParameter()->getIntegrityCheckSEIEnable()&&m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        if(m_pcH264AVCEncoder->getCodingParameter()->getNumberOfLayers()-1 == pcSliceHeader->getDependencyId() )
        {
          xWriteIntegrityCheckSEI(rcOutExtBinDataAccessorList, m_pcH264AVCEncoder->m_pcIntegrityCheckSEI, uiBits);
        }
      }
      //JVT-W052

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = pcSliceHeader->getNoInterLayerPredFlag() && pcSliceHeader->getFirstMbInSlice() == 0;
      cPicOutputData.iPicType      = pcSliceHeader->getPicType();
      cPicOutputData.Poc           = pcSliceHeader->getPoc                   ();
      cPicOutputData.FrameType[0]  = pcSliceHeader->getSliceType             () == B_SLICE ? 'B' :
                                     pcSliceHeader->getSliceType             () == P_SLICE ? 'P' : 'I';
      cPicOutputData.FrameType[1]  = pcSliceHeader->getUseRefBasePicFlag ()            ? 'K' : ' ';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = pcSliceHeader->getLayerCGSSNR           ();
      cPicOutputData.QualityId     = pcSliceHeader->getQualityLevelCGSSNR    ();
      cPicOutputData.TemporalId    = pcSliceHeader->getTemporalId         ();
      cPicOutputData.Qp            = pcSliceHeader->getSliceQp                 ();        
      cPicOutputData.Bits          = uiBits + uiBitsSEI;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;

      // JVT-V068 {
      cPicOutputData.iPicType      = (Int)ePicType;
      if (  m_uiQualityLevelCGSSNR != 0 || m_uiLayerCGSSNR != rcControlData.getSliceHeader()->getDependencyId()  )
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerCGSSNR;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      else
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerId;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      // JVT-V068 }

      rcPicOutputDataList.push_back( cPicOutputData );

      for( UInt uiFrag = 0; true; )
      {
        if( pcSliceHeader->getSPS().getMGSCoeffStop( uiFrag ) == 16 )
        {
          break;
        }
        uiFrag++;
        cPicOutputData.QualityId++;
        cPicOutputData.uiBaseLayerId      = pcSliceHeader->getLayerCGSSNR();
        cPicOutputData.uiBaseQualityLevel = cPicOutputData.QualityId - 1;
        cPicOutputData.Bits               = auiBits[uiFrag];
        rcPicOutputDataList.push_back( cPicOutputData );
        if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()+uiFrag] += auiBits[uiFrag];
        }
        else
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += auiBits[uiFrag];
        }
        ruiBits += auiBits[uiFrag];
      }

      if( ePicType != TOP_FIELD )
      {
        ETRACE_NEWFRAME;
      }
      if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits + uiBitsSEI;
      }
      else
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits + uiBitsSEI;
      }
      ruiBits = ruiBits + uiBits + uiBitsSEI;
      uiBitsSEI=0;
    }
    
		}//JVT-X046
    
    // JVT-S054 (ADD)
    }
  
    if (( ! pcSliceHeader->getNoInterLayerPredFlag())
      && pcSliceHeader->getSCoeffResidualPredFlag() )
    {
      m_pcSliceEncoder->updatePictureResTransform( rcControlData,m_uiFrameWidthInMb);
    }

    if (pcSliceHeader->getTCoeffLevelPredictionFlag())
    {
      //===== update state prior to deblocking
      m_pcSliceEncoder->updatePictureAVCRewrite( rcControlData, m_uiFrameWidthInMb );
    }

  return Err::m_nOK;
}







ErrVal
LayerEncoder::xEncodeHighPassSignal( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
                                    ControlData&             rcControlData,
                                    Frame*                pcOrgFrame, 
                                    Frame*                pcFrame,
                                    Frame*                pcResidual,
                                    Frame*                pcPredSignal,
                                    UInt&                    ruiBits,
                                    UInt&                    ruiBitsRes,
                                    PicOutputDataList&       rcPicOutputDataList,
                                    PicType                  ePicType )
{
  UInt  uiBitsSEI   = 0; 
  UInt  uiBits      = 0;//prefix unit 
  SliceHeader* pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  //----- Subsequence SEI -----
  if( m_bWriteSubSequenceSei &&   m_bH264AVCCompatible )
  {
    RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBitsSEI ) );
  }

  // JVT-S054 (ADD) ->
  if (m_bIroiSliceDivisionFlag)
  {
    for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
    {
      UInt  uiBitsRes   = 0;
      UInt  uiMbCoded   = 0;

      pcSliceHeader->setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
      pcSliceHeader->setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
      // JVT-S054 (2) (ADD)
      // TMM      
      pcSliceHeader->setNumMbsInSlice(pcSliceHeader->getFMO()->getNumMbsInSlice(rcControlData.getSliceHeader( ePicType )->getFirstMbInSlice(), rcControlData.getSliceHeader(/*chevance TMM*/ ePicType)->getLastMbInSlice()));

      pcSliceHeader->getDecRefBasePicMarking().setAdaptiveRefPicMarkingModeFlag( false );  //JVT-S036 lsj
//prefix unit{{
    if( m_uiPreAndSuffixUnitEnable ) 
    {
        if ( pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE|| pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
        {
          RNOK( xWritePrefixUnit( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBits ) );
        }
      }
//prefix unit}}

      //----- init NAL UNIT -----
      RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
      RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

      //---- write Slice Header -----
      ETRACE_NEWSLICE;
      xAssignSimplePriorityId( pcSliceHeader );
      if( pcSliceHeader->getSliceSkipFlag() )
      {
        UInt uiNumSkipSlices  = pcSliceHeader->getNumMbsInSlice();
        ROF( uiNumSkipSlices  > 0 );
        pcSliceHeader->setNumMbsInSliceMinus1( uiNumSkipSlices - 1 );
      }

      RNOK( m_pcNalUnitEncoder->write( *pcSliceHeader ) );

      //----- write slice data -----
      if( pcSliceHeader->isMbaffFrame() )
      {
      RNOK( m_pcSliceEncoder->encodeHighPassPictureMbAff ( uiMbCoded, 
                                                          uiBits, 
                                                          *rcControlData.getSliceHeader            (),
                                                          pcOrgFrame,
                                                          pcFrame,
                                                          pcResidual,
                                                          pcPredSignal,
                                                          rcControlData.getBaseLayerSbb           (),
                                                          rcControlData.getBaseLayerRec           (),
                                                          rcControlData.getMbDataCtrl             (),
                                                          rcControlData.getBaseLayerCtrl          (),
                                                          rcControlData.getBaseLayerCtrlField     (),
                                                          m_uiFrameWidthInMb,
                                                          rcControlData.getLambda                 (),
                                                          m_iMaxDeltaQp ) );
      }
  else
      {
        RNOK( m_pcSliceEncoder->encodeHighPassPicture ( uiMbCoded, 
                                                        uiBits,
                                                       *pcSliceHeader,
                                                        pcOrgFrame,																									     
                                                        pcFrame,
                                                        pcResidual,
                                                        pcPredSignal,
                                                        rcControlData.getBaseLayerSbb(),
                                                        rcControlData.getBaseLayerRec(),
                                                        rcControlData.getMbDataCtrl(),
                                                        rcControlData.getBaseLayerCtrl(),
                                                        m_uiFrameWidthInMb,
                                                        rcControlData.getLambda(),
                                                        m_iMaxDeltaQp,
                                                        ePicType ) );
      }
// TMM_INTERLACE {
// saveAll is deplaced here. All data are correct.
// save prediction data for B pictures 
      if( m_bSaveMotionInfo )
      {
        MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
        //===== loop over macroblocks =====
        UInt uiMbAddress       = pcSliceHeader->getFirstMbInSlice();
        UInt uiLastMbAddress   = pcSliceHeader->getLastMbInSlice();
        UInt uiNumMBInSlice;
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
        {
          MbDataAccess* pcMbDataAccess     = NULL;
          UInt          uiMbY, uiMbX;

          pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress            );

          //===== init macroblock =====
          RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );

          //===== save prediction data =====
          RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
          uiMbAddress = pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress);
          uiNumMBInSlice++;
        }
      }
// TMM_INTERLACE }
     
      //----- close NAL UNIT -----
      RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

      //----- update -----
      RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
      uiBits += 4*8;

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = rcControlData.getSliceHeader( ePicType )->getNoInterLayerPredFlag() && rcControlData.getSliceHeader( ePicType )->getFirstMbInSlice() == 0;
      cPicOutputData.iPicType      = ePicType;
      cPicOutputData.Poc           = rcControlData.getSliceHeader( ePicType )->getPoc                  ();
      cPicOutputData.FrameType[0]  = rcControlData.getSliceHeader( ePicType )->getSliceType            () == B_SLICE ? 'B' :
      rcControlData.getSliceHeader( ePicType )->getSliceType                                           () == P_SLICE ? 'P' : 'I';
      cPicOutputData.FrameType[1]  = rcControlData.getSliceHeader( ePicType )->getUseRefBasePicFlag()            ? 'K' : ' ';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = rcControlData.getSliceHeader( ePicType )->getLayerCGSSNR          ();
      cPicOutputData.QualityId     = rcControlData.getSliceHeader( ePicType )->getQualityLevelCGSSNR   ();
      cPicOutputData.TemporalId    = rcControlData.getSliceHeader( ePicType )->getTemporalId        ();
      cPicOutputData.Qp            = rcControlData.getSliceHeader( ePicType )->getSliceQp                ();        
      cPicOutputData.Bits          = uiBits + uiBitsSEI;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;
      // JVT-V068 {
      cPicOutputData.iPicType      = (Int)ePicType;
      if (  m_uiQualityLevelCGSSNR != 0 || m_uiLayerCGSSNR != rcControlData.getSliceHeader()->getDependencyId()  )
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerCGSSNR;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      else
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerId;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      // JVT-V068 }

      rcPicOutputDataList.push_back( cPicOutputData );

      if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits + uiBitsSEI;
      }
      else
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits + uiBitsSEI;
      }

    //S051{
    if(m_uiAnaSIP>0)
        m_auiFrameBits[rcControlData.getSliceHeader( ePicType )->getPoc()]=uiBits+uiBitsSEI; // TMM
    //S051}
      ruiBits     += uiBits+uiBitsSEI;
      ruiBitsRes  += uiBitsRes;
      uiBitsSEI =0;
    }
  }
  else
  {
  // JVT-S054 (ADD) <-
    FMO* pcFMO = rcControlData.getSliceHeader( ePicType )->getFMO();
    
    for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
    {
      uiBits      = 0;
      UInt  uiBitsRes   = 0;
      UInt  uiMbCoded   = 0;

      pcSliceHeader->setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
      pcSliceHeader->setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
      pcSliceHeader->setNumMbsInSlice (pcFMO->getNumMbsInSlice(rcControlData.getSliceHeader( ePicType )->getFirstMbInSlice(), rcControlData.getSliceHeader( ePicType )->getLastMbInSlice()));
      rcControlData.getSliceHeader( ePicType )->getDecRefPicMarking().setAdaptiveRefPicMarkingModeFlag( false );  //JVT-S036 lsj
   
//prefix unit{{
    if( m_uiPreAndSuffixUnitEnable ) 
    {
        if ( pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE|| pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
        {
          RNOK( xWritePrefixUnit( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBits ) );
        }
      }
//prefix unit}}
      //----- init NAL UNIT -----
      RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
      RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

      //---- write Slice Header -----
      ETRACE_NEWSLICE;
      xAssignSimplePriorityId( pcSliceHeader );
      if( pcSliceHeader->getSliceSkipFlag() )
      {
        UInt uiNumSkipSlices  = pcSliceHeader->getNumMbsInSlice();
        ROF( uiNumSkipSlices  > 0 );
        pcSliceHeader->setNumMbsInSliceMinus1( uiNumSkipSlices - 1 );
      }

      RNOK( m_pcNalUnitEncoder->write( *pcSliceHeader ) );

      //----- write slice data -----
      if( pcSliceHeader->isMbaffFrame() )
      {
      RNOK( m_pcSliceEncoder->encodeHighPassPictureMbAff ( uiMbCoded, 
                                                      uiBits, 
                                                      *rcControlData.getSliceHeader            (),
                                                      pcOrgFrame,
                                                      pcFrame,
                                                      pcResidual,
                                                      pcPredSignal,
                                                      rcControlData.getBaseLayerSbb           (),
                                                      rcControlData.getBaseLayerRec           (),
                                                      rcControlData.getMbDataCtrl             (),
                                                      rcControlData.getBaseLayerCtrl          (),
                                                      rcControlData.getBaseLayerCtrlField     (),
                                                      m_uiFrameWidthInMb,
                                                      rcControlData.getLambda                 (),
                                                      m_iMaxDeltaQp ) );
      }
  else
  {
    RNOK( m_pcSliceEncoder->encodeHighPassPicture ( uiMbCoded, 
                                                    uiBits,
                                                   *pcSliceHeader,
                                                    pcOrgFrame,
                                                    pcFrame,
                                                    pcResidual,
                                                    pcPredSignal,
                                                    rcControlData.getBaseLayerSbb(),
                                                    rcControlData.getBaseLayerRec(),
                                                    rcControlData.getMbDataCtrl(),
                                                    rcControlData.getBaseLayerCtrl(),
                                                    m_uiFrameWidthInMb,
                                                    rcControlData.getLambda(),
                                                    m_iMaxDeltaQp,
                                                    ePicType ) );
  }
   
// TMM_INTERLACE {
// saveAll is deplaced here. All data are correct.
// save prediction data for B pictures 
      if( m_bSaveMotionInfo )
      {
        MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
        //===== loop over macroblocks =====
        UInt uiMbAddress       = pcSliceHeader->getFirstMbInSlice();
        UInt uiLastMbAddress   = pcSliceHeader->getLastMbInSlice();
        UInt uiNumMBInSlice;
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
        {
          MbDataAccess* pcMbDataAccess     = NULL;
          UInt          uiMbY, uiMbX;
     
          pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress            );

          //===== init macroblock =====
          RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );

          //===== save prediction data =====
          RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
          uiMbAddress = pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress);
          uiNumMBInSlice++;
        }
      }
// TMM_INTERLACE }
     
      //----- close NAL UNIT -----
      UInt auiBits[16];
      RNOK( m_pcNalUnitEncoder->closeAndAppendNalUnits( auiBits, rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor, m_cBinData, NULL, 0, 0 ) );
      uiBits = auiBits[0] + 4*8;

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = rcControlData.getSliceHeader( ePicType )->getNoInterLayerPredFlag() && rcControlData.getSliceHeader( ePicType )->getFirstMbInSlice() == 0;
      cPicOutputData.iPicType      = ePicType;
      cPicOutputData.Poc           = rcControlData.getSliceHeader( ePicType )->getPoc                  ();
      cPicOutputData.FrameType[0]  = rcControlData.getSliceHeader( ePicType )->getSliceType            () == B_SLICE ? 'B' :
      rcControlData.getSliceHeader( ePicType )->getSliceType                                           () == P_SLICE ? 'P' : 'I';
      cPicOutputData.FrameType[1]  = rcControlData.getSliceHeader( ePicType )->getUseRefBasePicFlag()            ? 'K' : ' ';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = rcControlData.getSliceHeader( ePicType )->getLayerCGSSNR          ();
      cPicOutputData.QualityId     = rcControlData.getSliceHeader( ePicType )->getQualityLevelCGSSNR   ();
      cPicOutputData.TemporalId    = rcControlData.getSliceHeader( ePicType )->getTemporalId        ();
      cPicOutputData.Qp            = rcControlData.getSliceHeader( ePicType )->getSliceQp                ();        
      cPicOutputData.Bits          = uiBits + uiBitsSEI;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;
      // JVT-V068 {
      cPicOutputData.iPicType      = (Int)ePicType;
      if (  m_uiQualityLevelCGSSNR != 0 || m_uiLayerCGSSNR != rcControlData.getSliceHeader()->getDependencyId()  )
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerCGSSNR;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      else
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerId;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      // JVT-V068 }
      rcPicOutputDataList.push_back( cPicOutputData );

      for( UInt uiFrag = 0; true; )
      {
        if( pcSliceHeader->getSPS().getMGSCoeffStop( uiFrag ) == 16 )
        {
          break;
        }
        uiFrag++;
        cPicOutputData.QualityId++;
        cPicOutputData.uiBaseLayerId      = pcSliceHeader->getLayerCGSSNR();
        cPicOutputData.uiBaseQualityLevel = cPicOutputData.QualityId - 1;
        cPicOutputData.Bits               = auiBits[uiFrag];
        rcPicOutputDataList.push_back( cPicOutputData );
        if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()+uiFrag] += auiBits[uiFrag];
        }
        else
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += auiBits[uiFrag];
        }
        ruiBits += auiBits[uiFrag];
      }

      if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits+uiBitsSEI;
      }
      else
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits+uiBitsSEI;
      }

      ruiBits     += uiBits+uiBitsSEI;
      ruiBitsRes  += uiBitsRes;
      uiBitsSEI =0;
    }
  // JVT-S054 (ADD)
  }

  if ( !pcSliceHeader->getNoInterLayerPredFlag()
    && pcSliceHeader->getSCoeffResidualPredFlag() )
  {
    m_pcSliceEncoder->updatePictureResTransform( rcControlData,m_uiFrameWidthInMb);
  }

  if (pcSliceHeader->getTCoeffLevelPredictionFlag())
  {
    //===== update state prior to deblocking
    m_pcSliceEncoder->updatePictureAVCRewrite( rcControlData, m_uiFrameWidthInMb );
  }

  ETRACE_NEWFRAME;

  return Err::m_nOK;
}




ErrVal
LayerEncoder::xInitGOP( PicBufferList&  rcPicBufferInputList )
{
  //========== INITIALIZE DECOMPOSITION STRUCTURES ==========
  m_uiGOPSize                         = rcPicBufferInputList.size ();
// TMM {
  Bool bUncompleteCOP = false;
  if( m_pcLayerParameters[ m_uiDependencyId ].getPAff() == 1 )
  {
    if( m_uiDecompositionStages != (UInt)floor( log10( (Double)m_uiGOPSize ) / log10( 2.0 ) ) )
    {
      bUncompleteCOP = true;
    }
  }
// TMM }
  PicBufferList::iterator cInputIter  = rcPicBufferInputList.begin();
  UInt                    uiFrame     = 0;

  if( m_pbFieldPicFlag )
  {
    ::memset( m_pbFieldPicFlag, false, sizeof(Bool)*m_uiGOPSize );
  }

  if( m_bFirstGOPCoded )
  {
    ASSERT( uiFrame==0 );
    m_papcOrgFrame      [ uiFrame ]->copyAll          ( m_pcAnchorFrameOriginal );
    m_papcFrame					[ uiFrame ]->copyAll          ( m_pcAnchorFrameOriginal );
    m_papcFrame         [ uiFrame ]->setFrameIdInGop  ( uiFrame );
    m_papcFrame         [ uiFrame ]->copyPicParameters(*m_pcAnchorFrameOriginal );
    if( m_papcCLRecFrame )
    {
      m_papcCLRecFrame  [ uiFrame ]->copyPicParameters(*m_pcAnchorFrameOriginal );
    }
    if( m_papcELFrame )
    {
      m_papcELFrame     [ uiFrame ]->copyPicParameters(*m_pcAnchorFrameOriginal );
    }
    uiFrame++;
  }
  else
  {
    m_uiGOPSize--;
  }
  for( ; uiFrame <= m_uiGOPSize; uiFrame++, cInputIter++ )
  {
    if( m_pcResizeParameters->m_iExtendedSpatialScalability == ESS_PICT )
    {
      RNOK( m_pcResizeParameters->readPictureParameters( m_pESSFile, ! m_bInterlaced ) );
    }

    Int         iPoc        = ( m_bInterlaced ? m_uiFrameCounter++ << ( m_uiTemporalResolution+1 ) : m_uiFrameCounter++ << ( m_uiTemporalResolution ) );
    PicBuffer*  pcPicBuffer = *cInputIter;
    
    m_papcOrgFrame      [ uiFrame ]->load             ( pcPicBuffer );
    m_papcFrame					[ uiFrame ]->load             ( pcPicBuffer );
    m_papcFrame         [ uiFrame ]->setFrameIdInGop  ( uiFrame );
    m_papcFrame					[ uiFrame ]->setPoc           ( iPoc );
    m_papcFrame         [ uiFrame ]->setPicParameters ( *m_pcResizeParameters );
    if( m_papcCLRecFrame )
    {
      m_papcCLRecFrame  [ uiFrame ]->setPoc           ( iPoc );
      m_papcCLRecFrame  [ uiFrame ]->setPicParameters ( *m_pcResizeParameters );
    }
    if( m_papcELFrame )
    {
      m_papcELFrame     [ uiFrame ]->setPoc           ( iPoc );
      m_papcELFrame     [ uiFrame ]->setPicParameters ( *m_pcResizeParameters );
    }

    m_pacControlData[ uiFrame ].setSpatialScalability( false );
  }
  if( m_uiGOPSize == ( 1 << m_uiDecompositionStages ) ) // store original anchor frame
  {
    RNOK( m_pcAnchorFrameOriginal ->copyAll           ( m_papcFrame[ m_uiGOPSize ] ) );
    m_pcAnchorFrameOriginal       ->copyPicParameters (*m_papcFrame[ m_uiGOPSize ] );
    m_pcAnchorFrameReconstructed  ->copyPicParameters (*m_papcFrame[ m_uiGOPSize ] );
  }

  //========== INITIALIZE SLICE HEADERS (the decomposition structure is determined at this point) ==========
  if( ! m_bFirstGOPCoded )
  {
    xPAffDecision( 0 );
    switch( m_pcLayerParameters->getPAff() )
    {
    case 0:
      RNOK( xInitSliceHeader  ( 0, 0, FRAME     ) );
      break;

    case 1:
      RNOK( xInitSliceHeader  ( 0, 0, TOP_FIELD ) );
      RNOK( xInitSliceHeader  ( 0, 0, BOT_FIELD ) );
      break;

    case 2:
      if( ! m_pbFieldPicFlag[0] )
      {
        RNOK( xInitSliceHeader( 0, 0, FRAME     ) );
      }
      else 
      {
        RNOK( xInitSliceHeader( 0, 0, TOP_FIELD ) );
        RNOK( xInitSliceHeader( 0, 0, BOT_FIELD ) );
      }
      break;

    default:
      AF ();
      break;
    }
  }
  else
  {
    //----- copy frame_num of anchor frame -> needed for RPLR command setting -----
    m_pacControlData[0].getSliceHeader()->setFrameNum( m_cLPFrameNumList.front()  );
    if( m_pcLayerParameters->getPAff() )
    {
      m_pacControlData[0].getSliceHeader( BOT_FIELD )->setFrameNum( m_cLPFrameNumList.front() );
    }
  }
  UInt uiTemporalLevel;
  for( uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages; uiTemporalLevel++ )
  {
    UInt      uiStep    = ( 1 << ( m_uiDecompositionStages - uiTemporalLevel ) );
    for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGOPSize; uiFrameId += ( uiStep << 1 ) )
    {
      xPAffDecision( uiFrameId );
      switch( m_pcLayerParameters->getPAff() )
      { 
      case 0:
        RNOK( xInitSliceHeader  ( uiTemporalLevel, uiFrameId, FRAME     ) );
        break;

      case 1:
        RNOK( xInitSliceHeader  ( uiTemporalLevel, uiFrameId, TOP_FIELD ) );
        RNOK( xInitSliceHeader  ( uiTemporalLevel, uiFrameId, BOT_FIELD ) );
        break;

      case 2:
        if( ! m_pbFieldPicFlag[uiFrameId] )
        {
          RNOK( xInitSliceHeader( uiTemporalLevel, uiFrameId, FRAME     ) );
        }
        else
        {
          RNOK( xInitSliceHeader( uiTemporalLevel, uiFrameId, TOP_FIELD ) );
          RNOK( xInitSliceHeader( uiTemporalLevel, uiFrameId, BOT_FIELD ) );
        }
        break;

      default:
        AOT( 1 );
        break;
      }
    }
  }

// JVT-Q065 EIDR{
 for( uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages; uiTemporalLevel++ )
  {
    UInt      uiStep    = ( 1 << ( m_uiDecompositionStages - uiTemporalLevel ) );
    for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGOPSize; uiFrameId += ( uiStep << 1 ) )
    {
      SliceHeader* pcSliceHeader = m_pacControlData[uiFrameId].getSliceHeader();
      if(pcSliceHeader->getIdrFlag())
      {
        for( uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++)
        {
          if(m_pacControlData[uiFrame].getSliceHeader()->getTemporalId() < pcSliceHeader->getTemporalId() || (m_pacControlData[uiFrame].getSliceHeader()->getTemporalId() == pcSliceHeader->getTemporalId()&& uiFrame < uiFrameId))
          {
            //bug-fix shenqiu EIDR}
            m_papcFrame[uiFrame]->setUnusedForRef(true);
            m_papcOrgFrame[uiFrame]->setUnusedForRef(true);
            if(m_papcCLRecFrame)
            {
              m_papcCLRecFrame[uiFrame]->setUnusedForRef(true);
            }
            if(m_papcELFrame)
            {
              m_papcELFrame[uiFrame]->setUnusedForRef(true);
            }
          }
        }
      }
    }
  }

  //bug-fix shenqiu EIDR{
  if((m_iIDRPeriod != 0) && (m_papcFrame[m_uiGOPSize]->getPoc() % m_iIDRPeriod > 0))//EIDR 0619
  {
    m_bBLSkipEnable = true;
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

  //===== RPLR and MMCO commands =====
   for( uiFrame = m_bFirstGOPCoded ? 1 : 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    switch( m_pcLayerParameters->getPAff() )
    { 
    case 0:
      RNOK( xInitReordering  ( uiFrame, FRAME     ) );
      break;

    case 1:
      RNOK( xInitReordering  ( uiFrame, TOP_FIELD, bUncompleteCOP ) );
      RNOK( xInitReordering  ( uiFrame, BOT_FIELD ) );
      break;

    case 2:
      if( ! m_pbFieldPicFlag[uiFrame] )
      {    
        RNOK( xInitReordering( uiFrame, FRAME     ) );
      }
      else 
      {
        RNOK( xInitReordering( uiFrame, TOP_FIELD ) );
        RNOK( xInitReordering( uiFrame, BOT_FIELD ) );
  }
      break;

    default:
      AF();
      break;
    }
  }

  //========== INITIALIZE SCALING FACTORS ==========
  for( uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    m_pacControlData[ uiFrame ].clear();
    m_pacControlData[ uiFrame ].setScalingFactor( 1.0 );
    MbDataCtrl* pcMbDataCtrl = m_pacControlData[ uiFrame ].getMbDataCtrl();
    RNOK( pcMbDataCtrl->reset () );
    RNOK( pcMbDataCtrl->clear () );
  }
  
  m_uiNewlyCodedBits += m_uiParameterSetBits;
  if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
  {
    m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][0][0] += m_uiParameterSetBits;
  }
  else
  {
    m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][0][0] += m_uiParameterSetBits;
  }

  m_uiLastCodedFrameIdInGOP = MSYS_UINT_MAX;
  m_uiLastCodedTemporalId   = MSYS_UINT_MAX;

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitBitCounts()
{
  UInt  uiLowerLayerBits             = m_pcH264AVCEncoder->getNewBits( m_uiBaseLayerId );
  m_uiNewlyCodedBits                += uiLowerLayerBits;
  return Err::m_nOK;
}


ErrVal
LayerEncoder::getBaseLayerStatus( Bool&   bExists,
                                  Bool&   bMotion,
                                  PicType ePicType,
                                  UInt		uiTemporalId )
{
  bExists = false;
  bMotion = false;

  if( m_uiLastCodedFrameIdInGOP != MSYS_UINT_MAX && m_uiLastCodedTemporalId == uiTemporalId )
  {
    bExists = true;
    bMotion = bExists || !m_bH264AVCCompatible;
  }

  //S051{
  if(m_bEncSIP && m_uiLastCodedFrameIdInGOP != MSYS_UINT_MAX && m_uiLastCodedTemporalId == uiTemporalId )
  {
    Int iPoc = m_pacControlData[m_uiLastCodedTemporalId].getSliceHeader( ePicType )->getPoc( ePicType );
    if(xSIPCheck(iPoc))
    {
      bExists = false;
      bMotion = bExists || !m_bH264AVCCompatible;
    }
  }
  //S051}
  
  return Err::m_nOK;
}

ErrVal
LayerEncoder::getBaseLayerDataAvailability( Frame*&       pcFrame,
                                            Frame*&       pcResidual,
                                            MbDataCtrl*&  pcMbDataCtrl,
                                            Bool          bMotion,
                                            PicType       ePicType,
                                            UInt		      uiTemporalId )
{
  pcFrame      = NULL;
  pcResidual   = NULL;
  pcMbDataCtrl = NULL;

  if( m_uiLastCodedFrameIdInGOP != MSYS_UINT_MAX && m_uiLastCodedTemporalId == uiTemporalId )
  {
    SliceHeader* pcSliceHeader = m_pacControlData[ m_uiLastCodedFrameIdInGOP ].getSliceHeader( ePicType );
    ROF( pcSliceHeader );

    pcFrame       = m_papcSubband   [m_uiLastCodedFrameIdInGOP];
    pcResidual    = m_papcResidual  [m_uiLastCodedFrameIdInGOP];

    if( ! bMotion )
    {                                                                     // HS -> that's the correct place for it
      m_pacControlData[ m_uiLastCodedFrameIdInGOP ].activateMbDataCtrlForQpAndCbp( false ); //    -> need correct CBP values for loop filter of next layer (residual prediction)
    }
    //    -> it cannot be reset after this assignment !!!
    pcMbDataCtrl  = m_pacControlData[m_uiLastCodedFrameIdInGOP].getMbDataCtrl();        //    -> it is reset in ControlData::clear at the beginning of the next GOP

    if( pcMbDataCtrl )
    {
      pcMbDataCtrl->setSliceHeader( pcSliceHeader );
    }
  }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::getBaseLayerData( SliceHeader&  rcELSH,
                                Frame*&       pcFrame,
                                Frame*&       pcResidual,
                                MbDataCtrl*&  pcMbDataCtrl,
                                MbDataCtrl*&  pcMbDataCtrlEL,		// ICU/ETRI FGS_MOT_USE
                                Bool          bSpatialScalability,
                                Bool          bMotion,
                                PicType       ePicType,
                                UInt	      	uiTemporalId )
{
  UInt  uiPos   = MSYS_UINT_MAX;
  pcFrame       = 0;
  pcResidual    = 0;
  pcMbDataCtrl  = 0;
  pcMbDataCtrlEL = 0;

  if( m_uiLastCodedFrameIdInGOP != MSYS_UINT_MAX && m_uiLastCodedTemporalId == uiTemporalId )
  {
    SliceHeader* pcSliceHeader = m_pacControlData[ m_uiLastCodedFrameIdInGOP ].getSliceHeader( ePicType );
    ROF( pcSliceHeader );

    pcFrame       = m_papcSubband   [m_uiLastCodedFrameIdInGOP];
    pcResidual    = m_papcResidual  [m_uiLastCodedFrameIdInGOP];

    if( !bMotion )                                                    // HS -> that's the correct place for it
      m_pacControlData[m_uiLastCodedFrameIdInGOP].activateMbDataCtrlForQpAndCbp( false ); //    -> need correct CBP values for loop filter of next layer (residual prediction)
    //    -> it cannot be reset after this assignment !!!
    pcMbDataCtrl  = m_pacControlData[m_uiLastCodedFrameIdInGOP].getMbDataCtrl();        //    -> it is reset in ControlData::clear at the beginning of the next GOP

    // ICU/ETRI FGS_MOT_USE
    pcMbDataCtrlEL  = m_pacControlDataEL[m_uiLastCodedFrameIdInGOP].getMbDataCtrl();        //    -> it is reset in ControlData::clear at the beginning of the next GOP

    uiPos = m_uiLastCodedFrameIdInGOP;

    if( pcMbDataCtrl )
    {
      pcMbDataCtrl->setSliceHeader( pcSliceHeader );
    }				
  }


  if( bSpatialScalability )
  {
    RNOK( m_apcFrameTemp[0]->copy( pcFrame, ePicType ) );
  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
    m_apcFrameTemp[0]->setChannelDistortion(pcFrame);
  //JVT-R057 LA-RDO}
    pcFrame = m_apcFrameTemp[0];

    SliceHeader* pcSliceHeader = m_pacControlData[ uiPos ].getSliceHeader( ePicType );
    ROF( pcSliceHeader );
    RNOK( m_pcLoopFilter->process(*pcSliceHeader, pcFrame, pcResidual, m_pacControlData[uiPos].getMbDataCtrl(), 
                                   &rcELSH.getInterLayerDeblockingFilterParameter(), 
                                   m_pacControlData[uiPos].getSpatialScalability() ) );  
  }
  
  return Err::m_nOK;
}


ErrVal
LayerEncoder::xGetListSizes( UInt  uiTemporalLevel,
                            UInt  uiFrameIdInGOP,
                            UInt  auiPredListSize[2] )
{
  //----- get delay decomposition stages -----
  UInt  uiDelayDecompositionStages = 0;
  for( ; m_uiFrameDelay >> uiDelayDecompositionStages; uiDelayDecompositionStages++ );
  uiDelayDecompositionStages = min( m_uiDecompositionStages, uiDelayDecompositionStages );


  //----- loop over prediction and update steps -----
  for( UInt uiLevel = uiTemporalLevel; uiLevel <= m_uiDecompositionStages; uiLevel++ )
  {
    //----- get parameters base GOP size and cut-off frame id -----
    UInt  uiBaseLevel       = m_uiDecompositionStages - uiLevel;
    UInt  uiFrameIdLevel    = uiFrameIdInGOP >> uiBaseLevel;
    UInt  uiBaseGOPSize     = ( 1 << uiDelayDecompositionStages ) >> min( uiBaseLevel, uiDelayDecompositionStages );
    UInt  uiCutOffFrame     = max( 0, Int( uiBaseGOPSize - ( m_uiFrameDelay >> uiBaseLevel ) - 1 ) );

    if( uiLevel == uiTemporalLevel )
    {
      //=========== PREDICTION LIST SIZES ==========
      auiPredListSize[0]    = ( uiFrameIdLevel + 1 ) >> 1;
      UInt  uiFrameIdWrap   = ( uiFrameIdLevel % uiBaseGOPSize );
      if( uiFrameIdWrap > uiCutOffFrame )
      {
        auiPredListSize[1]  = ( uiBaseGOPSize - uiFrameIdWrap + 1 ) >> 1;
      }
      else
      {
        auiPredListSize[1]  = ( uiCutOffFrame - uiFrameIdWrap + 1 ) >> 1;
      }

      auiPredListSize[0]    = min( m_uiMaxNumRefFrames, auiPredListSize[0] );
      auiPredListSize[1]    = min( m_uiMaxNumRefFrames, auiPredListSize[1] );

      //----- take into account actual GOP size -----
      {
        UInt  uiMaxL1Size   = ( ( m_uiGOPSize >> uiBaseLevel ) + 1 - uiFrameIdLevel ) >> 1;
        auiPredListSize[1]  = min( uiMaxL1Size,         auiPredListSize[1] );
      }

      if( m_bHaarFiltering )
      {
        auiPredListSize[1]  = 0;
      }
    }
  }

  //----- check intra -----
  if( m_uiLowPassIntraPeriod != MSYS_UINT_MAX )
  {
    UInt  uiCurrFrame   = (   m_uiGOPNumber                << m_uiDecompositionStages ) + uiFrameIdInGOP;
    UInt  uiIntraPeriod = ( ( m_uiLowPassIntraPeriod + 1 ) << m_uiDecompositionStages );
    if( ( uiCurrFrame % uiIntraPeriod ) == 0 )
    {
      auiPredListSize[0] = 0;
      auiPredListSize[1] = 0;
    }
  }

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xSetBaseLayerData( UInt    uiFrameIdInGOP,
                                 PicType ePicType )
{
  ControlData&  rcControlData       = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*  pcSliceHeader       = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );
  UInt          uiTemporalId        = pcSliceHeader->getTemporalId();
  UInt          uiBaseLayerId       = m_uiBaseLayerId;
  UInt          uiBaseLayerIdMotion = m_uiBaseLayerId;

  if( ! m_bInterLayerPrediction )
  {
    pcSliceHeader->setBaseLayerId           ( MSYS_UINT_MAX );
    pcSliceHeader->setNoInterLayerPredFlag( true );
    //pcSliceHeader->setBaseQualityLevel      ( 0 );
    pcSliceHeader->setAdaptiveBaseModeFlag( false );
    // JVT-U160 LMI
    pcSliceHeader->setAdaptiveResidualPredictionFlag (false);
    rcControlData .setBaseLayer             ( MSYS_UINT_MAX, MSYS_UINT_MAX );
    return Err::m_nOK;
  }

  RNOK( m_pcH264AVCEncoder->getBaseLayerStatus( uiBaseLayerId, uiBaseLayerIdMotion, m_uiDependencyId, ePicType, uiTemporalId ) );

  Bool  bAdaptive = m_bAdaptivePrediction;
  if(   bAdaptive )
  {
    if( uiBaseLayerId != MSYS_UINT_MAX         && // when base layer available
        pcSliceHeader->getTemporalId() == 0 && // only for TL=0 pictures
        m_bMGS                                 && // only when MGS
        m_bSameResBL                           && // only when not lowest MGS layer (QL=0)
        m_uiEncodeKeyPictures   >  0            ) // only when MGS key pictures
    {
      bAdaptive = false;
    }
  }

  m_pcResizeParameters->updatePicParameters( m_papcFrame[ uiFrameIdInGOP ]->getPicParameters( ePicType ) );
  if( uiBaseLayerId != uiBaseLayerIdMotion && ( bAdaptive || m_pcResizeParameters->getSpatialResolutionChangeFlag() ) )
  {
    uiBaseLayerIdMotion = uiBaseLayerId;
  }

  pcSliceHeader->setBaseLayerId( uiBaseLayerId );
  pcSliceHeader->setRefLayer( m_uiBaseLayerCGSSNR, m_uiBaseQualityLevelCGSSNR );
  pcSliceHeader->setNoInterLayerPredFlag( uiBaseLayerId == MSYS_UINT_MAX );
  Bool bSendSkipSlices = ( uiBaseLayerId != MSYS_UINT_MAX && m_pcLayerParameters->getSliceSkip() != 0 && pcSliceHeader->getTemporalId() >= m_pcLayerParameters->getSliceSkipTLevelStart() );
  pcSliceHeader->setSliceSkipFlag( bSendSkipSlices );
  Bool bAdaptivePredFlag = ( uiBaseLayerId != MSYS_UINT_MAX ? bAdaptive : false );
  pcSliceHeader->setAdaptiveBaseModeFlag          ( bAdaptivePredFlag );
  pcSliceHeader->setAdaptiveMotionPredictionFlag  ( bAdaptivePredFlag );
  pcSliceHeader->setAdaptiveResidualPredictionFlag( bAdaptivePredFlag );
  if( ! bAdaptivePredFlag )
  {
    pcSliceHeader->setDefaultBaseModeFlag           ( ! pcSliceHeader->getNoInterLayerPredFlag() );
    pcSliceHeader->setDefaultMotionPredictionFlag   ( ! pcSliceHeader->getNoInterLayerPredFlag() );
    pcSliceHeader->setDefaultResidualPredictionFlag ( ! pcSliceHeader->getNoInterLayerPredFlag() );
  }
  rcControlData.setBaseLayer( uiBaseLayerId, uiBaseLayerIdMotion );

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xInitSliceHeader( UInt uiTemporalLevel,
                               UInt    uiFrameIdInGOP,
                               PicType ePicType )
{
  SliceHeader* pcSliceHeader = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader( ePicType );
  ROF( pcSliceHeader );


  //===== get maximum sizes of prediction and update lists ( decomposition structure ) =====
  UInt  auiPredListSize             [2];
  RNOK( xGetListSizes( uiTemporalLevel, uiFrameIdInGOP, auiPredListSize ) );


  //===== get slice header parameters =====
  NalRefIdc     eNalRefIdc;

  if ( uiFrameIdInGOP == 0 || uiFrameIdInGOP == ( 1 << m_uiDecompositionStages ) )
    eNalRefIdc = NAL_REF_IDC_PRIORITY_HIGHEST;
  else
    eNalRefIdc = NalRefIdc( min( 2, max( 0, (Int)( m_uiDecompositionStages - m_uiNotCodedStages     - uiTemporalLevel ) ) ) );

  NalUnitType   eNalUnitType  = ( m_bH264AVCCompatible ? ( uiFrameIdInGOP ? NAL_UNIT_CODED_SLICE : NAL_UNIT_CODED_SLICE_IDR ) : NAL_UNIT_CODED_SLICE_SCALABLE );
  Bool          bIdr          = false;

  if( (m_papcFrame[uiFrameIdInGOP]->getPoc() == 0) || (m_iIDRPeriod != 0 && m_papcFrame[uiFrameIdInGOP]->getPoc() % m_iIDRPeriod == 0 ) )
  {
    bIdr                = true;
    auiPredListSize[0]  = 0;
    auiPredListSize[1]  = 0;
    if( m_bH264AVCCompatible )
    {
      eNalUnitType = NAL_UNIT_CODED_SLICE_IDR;
    }
  }

  SliceType     eSliceType      = ( auiPredListSize[1] ? B_SLICE : auiPredListSize[0] ? P_SLICE : I_SLICE );
  Bool          bUseBaseRep     = ( eNalRefIdc == NAL_REF_IDC_PRIORITY_HIGHEST ) ? 1 : 0;
 if( ePicType!=FRAME && eSliceType==B_SLICE )
  {
    // two reference coded fields for one reference coded frame
    auiPredListSize[0] <<= 1;
    auiPredListSize[1] <<= 1;
    eNalRefIdc = NAL_REF_IDC_PRIORITY_HIGH;
  }
// TMM {
  if( ePicType!=FRAME && eSliceType==P_SLICE )
  {
    if( eNalRefIdc == NAL_REF_IDC_PRIORITY_LOWEST )
    {
      eNalRefIdc = NAL_REF_IDC_PRIORITY_HIGH;
    }
  }
// TMM }

  if( ePicType==BOT_FIELD && eSliceType==I_SLICE )
  {
    eSliceType = P_SLICE;
    auiPredListSize[0] = 1;
    bIdr = false;
    if( eNalUnitType==NAL_UNIT_CODED_SLICE_IDR )
    {
      eNalUnitType = NAL_UNIT_CODED_SLICE;
    }
  }

  if( bUseBaseRep )
  {
    bUseBaseRep =
      ( m_uiEncodeKeyPictures >= 1 && m_bMGS ) || // when MGS and KeyPicMode > 0
      ( m_uiEncodeKeyPictures == 2           );   // always when  KeyPicMode > 1
  }

  //===== set simple slice header parameters =====
  pcSliceHeader->setNalRefIdc             ( eNalRefIdc );
  pcSliceHeader->setNalUnitType           ( eNalUnitType );
  pcSliceHeader->setIdrFlag               ( bIdr );
  pcSliceHeader->setNoInterLayerPredFlag  ( ! m_bInterLayerPrediction );
  pcSliceHeader->setDependencyId          ( m_uiDependencyId );
  pcSliceHeader->setTemporalId            ( uiTemporalLevel );
  pcSliceHeader->setOutputFlag            ( true );
  pcSliceHeader->setDiscardableFlag       ( m_bDiscardable );
  pcSliceHeader->setQLDiscardable         ( m_uiQLDiscardable );
  pcSliceHeader->setPriorityId            ( 0	);
  
  if( ! pcSliceHeader->getSPS().getFrameMbsOnlyFlag() )
  {
    pcSliceHeader->setFieldPicFlag              ( m_pbFieldPicFlag[ uiFrameIdInGOP ] );
    if( m_pbFieldPicFlag[ uiFrameIdInGOP ] )
    {
      pcSliceHeader->setBottomFieldFlag         ( ePicType==BOT_FIELD );
    }
  }

  m_uiMbNumber = m_uiFrameWidthInMb*m_uiFrameHeightInMb;
  m_uiMbNumber = pcSliceHeader->getFieldPicFlag() ? m_uiMbNumber>>1 : m_uiMbNumber;
  
//EIDR 0619{
  if( pcSliceHeader->getIdrFlag() )
  {
    m_uiFrameNum = 0; 
    pcSliceHeader->setIdrPicId                    ( m_uiIdrPicId                     ); 
  }
//EIDR 0619}
  pcSliceHeader->setFirstMbInSlice              ( 0                     );
  pcSliceHeader->setLastMbInSlice               ( m_uiMbNumber - 1      );
  pcSliceHeader->setSliceType                   ( eSliceType            );
  pcSliceHeader->setFrameNum                    ( m_uiFrameNum          );
//  pcSliceHeader->setIdrPicId                    ( 0                     ); //EIDR 0619
  pcSliceHeader->setDirectSpatialMvPredFlag     ( true                  );
  pcSliceHeader->setUseRefBasePicFlag   ( bUseBaseRep           );
  pcSliceHeader->setStoreRefBasePicFlag   ( bUseBaseRep           );
  pcSliceHeader->setNumRefIdxActiveOverrideFlag ( false                 );
  pcSliceHeader->setCabacInitIdc                ( 0                     );
  pcSliceHeader->setSliceHeaderQp               ( 0                     );
  // Currently hard-coded
  pcSliceHeader->setNumMbsInSlice               ( m_uiMbNumber          );
//JVT-T054{
  pcSliceHeader->setLayerCGSSNR                 (m_uiLayerCGSSNR);
  pcSliceHeader->setQualityLevelCGSSNR          (m_uiQualityLevelCGSSNR);
  pcSliceHeader->setBaseLayerCGSSNR                 (m_uiBaseLayerCGSSNR);
  pcSliceHeader->setBaseQualityLevelCGSSNR          (m_uiBaseQualityLevelCGSSNR);
//JVT-T054}

//JVT-Q054 Red. Picture {
  if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag())
  {
    pcSliceHeader->setRedundantPicCnt( 0 );
  }
// JVT-Q054 Red. Picture }

  //JVT-U106 Behaviour at slice boundaries{
  pcSliceHeader->setConstrainedIntraResamplingFlag(m_bCIUFlag);
  //JVT-U106 Behaviour at slice boundaries{

  //===== set prediction and update list sizes =====
  {
    //--- prediction ---
    pcSliceHeader->setNumRefIdxActive( LIST_0, 0 );
    pcSliceHeader->setNumRefIdxActive( LIST_1, 0 );

    UInt  uiMaxLists = ( eSliceType == B_SLICE ? 2 : eSliceType == P_SLICE ? 1 : 0 );
    for( UInt uiList = 0; uiList < uiMaxLists; uiList++ )
    {
      ListIdx eListIdx  = ListIdx( uiList );
      ROF( auiPredListSize[ uiList ] );

      pcSliceHeader->setNumRefIdxActive( eListIdx, auiPredListSize[ uiList ] );
      if( pcSliceHeader->getPPS().getNumRefIdxActive( eListIdx ) != auiPredListSize[ uiList ] )
      {
        pcSliceHeader->setNumRefIdxActiveOverrideFlag( true );
      }
    }
  }

  //===== de-blocking filter parameters =====
  if( pcSliceHeader->getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    UInt uiFilterIdc = m_uiFilterIdc;
    if( ( eNalUnitType == NAL_UNIT_CODED_SLICE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR ) && uiFilterIdc > 2 )
    {
      if( uiFilterIdc == 5 )  uiFilterIdc = 2;
      else                    uiFilterIdc = 0;
    }
    pcSliceHeader->getDeblockingFilterParameter().setDisableDeblockingFilterIdc ( uiFilterIdc         );
    pcSliceHeader->getDeblockingFilterParameter().setSliceAlphaC0Offset         ( 2 * m_iAlphaOffset  );
    pcSliceHeader->getDeblockingFilterParameter().setSliceBetaOffset            ( 2 * m_iBetaOffset   );
  }
  if( pcSliceHeader->getSPS().getInterlayerDeblockingPresent() )
  {
    pcSliceHeader->getInterLayerDeblockingFilterParameter().setDisableDeblockingFilterIdc( m_uiInterLayerFilterIdc );
    pcSliceHeader->getInterLayerDeblockingFilterParameter().setSliceAlphaC0Offset        ( 2 * m_iInterLayerAlphaOffset );
    pcSliceHeader->getInterLayerDeblockingFilterParameter().setSliceBetaOffset           ( 2 * m_iInterLayerBetaOffset );
  }

   //EIDR bug-fix
  if(pcSliceHeader->getIdrFlag() && m_iIDRPeriod == m_iIDRAccessPeriod)
  pcSliceHeader->setInIDRAccess(true);
  
  //===== set remaining slice header parameters =====
  RNOK( m_pcPocCalculator->setPoc( *pcSliceHeader, m_papcFrame[uiFrameIdInGOP]->getPoc() ) );
//EIDR 0619{ 
  m_papcFrame[uiFrameIdInGOP]->setPoc( *pcSliceHeader );
  if( m_papcCLRecFrame )
  {
    m_papcCLRecFrame[uiFrameIdInGOP]->setPoc( *pcSliceHeader );
  }
  if( m_papcELFrame )
  {
    m_papcELFrame[ uiFrameIdInGOP ]->setPoc( *pcSliceHeader );
  }
  if( pcSliceHeader->getIdrFlag() )
  {
    m_papcFrame[uiFrameIdInGOP]->setIdrPicId(m_uiIdrPicId);
    m_uiIdrPicId  = ( m_uiIdrPicId + 1 ) % 3;
  }
//EIDR 0619}


  if( ! pcSliceHeader->getNoInterLayerPredFlag() )
  {
    m_pcResizeParameters->updatePicParameters   ( m_papcFrame[uiFrameIdInGOP]->getPicParameters( ePicType ) );
    pcSliceHeader->setScaledRefLayerLeftOffset  ( m_pcResizeParameters->getLeftFrmOffset  () / 2 );
    pcSliceHeader->setScaledRefLayerTopOffset   ( m_pcResizeParameters->getTopFrmOffset   () / 2 );
    pcSliceHeader->setScaledRefLayerRightOffset ( m_pcResizeParameters->getRightFrmOffset () / 2 );
    pcSliceHeader->setScaledRefLayerBottomOffset( m_pcResizeParameters->getBotFrmOffset   () / 2 );
    pcSliceHeader->setRefLayerChromaPhaseX      ( m_pcResizeParameters->m_iRefLayerChromaPhaseX );
    pcSliceHeader->setRefLayerChromaPhaseY      ( m_pcResizeParameters->m_iRefLayerChromaPhaseY );
  }

  //===== update some parameters =====
  if( eNalRefIdc && ePicType!=TOP_FIELD )
  {
    m_uiFrameNum = ( m_uiFrameNum + 1 ) % ( 1 << pcSliceHeader->getSPS().getLog2MaxFrameNum() );
  }

  pcSliceHeader->setSliceGroupChangeCycle(1);
  pcSliceHeader->FMOInit();


  //S051{
  if(m_bEncSIP)
  {
    if(xSIPCheck(pcSliceHeader->getPoc()))
      pcSliceHeader->setDiscardableFlag( true );
  }
  //S051}
 
  return Err::m_nOK;
}


Frame*
LayerEncoder::getMGSLPRec()
{
  ROFRS( m_bMGS && m_bSameResEL, 0 );
  return m_pcLowPassBaseReconstruction;
}


Frame*
LayerEncoder::xGetRefFrame( Frame**   papcRefFrameList,
                           UInt         uiRefIndex,
                           RefListUsage eRefListUsage )
{
  if( eRefListUsage <= (Int)m_uiMGSKeyPictureControl &&
      m_papcELFrame                                  && 
     !m_papcELFrame[ uiRefIndex ]->isUnvalid()         )
  {
    return  m_papcELFrame   [ uiRefIndex ];
  }
  return    papcRefFrameList[ uiRefIndex ];
}


ErrVal
LayerEncoder::xClearELPics()
{
  ROFRS( m_papcELFrame, Err::m_nOK );

  for( UInt uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    m_papcELFrame[uiIndex]->setUnvalid();
  }
  return Err::m_nOK;
}

ErrVal
LayerEncoder::xUpdateELPics()
{
  ROFRS( m_papcELFrame, Err::m_nOK );

  for( UInt uiIndex = 0; uiIndex <= m_uiGOPSize; uiIndex++ )
  {
    if( m_papcELFrame[uiIndex]->isUnvalid      () &&
       !m_papcELFrame[uiIndex]->getUnusedForRef()   )
    {
      Int             iPoc  = m_papcFrame[uiIndex] ->getPoc     ();
      const Frame* pcELPic  = m_pcH264AVCEncoder   ->getELRefPic( m_uiDependencyId, iPoc );
      if( pcELPic )
      {
        RNOK( m_papcELFrame[uiIndex]->copy( const_cast<Frame*>( pcELPic ), FRAME ) );
        RNOK( xFillAndUpsampleFrame( m_papcELFrame[uiIndex], FRAME, m_bFrameMbsOnlyFlag ) );
        m_papcELFrame[uiIndex]->setValid();
      }
    }
  }
  return Err::m_nOK;
}


Frame* 
LayerEncoder::getRefPic( Int iPoc )
{
  Frame* pcRefPic = 0;

  //===== determine frameId in GOP =====
  UInt  uiFrameIdInGOP = MSYS_UINT_MAX;
  for(  uiFrameIdInGOP = 0; uiFrameIdInGOP <= m_uiGOPSize; uiFrameIdInGOP++ )
  {
    if( m_papcFrame[uiFrameIdInGOP]->getPoc() == iPoc )
    {
      break;
    }
  }
  AOT(  uiFrameIdInGOP > m_uiGOPSize );

  if( m_abCoded       [uiFrameIdInGOP]                                  &&
     !m_papcFrame     [uiFrameIdInGOP]->getUnusedForRef ()              &&
      m_pacControlData[uiFrameIdInGOP] .getSliceHeader  ()->getNalRefIdc() )
  {
    if( m_papcCLRecFrame )  pcRefPic  = m_papcCLRecFrame[uiFrameIdInGOP];
    else                    pcRefPic  = m_papcFrame     [uiFrameIdInGOP];
  }

  return pcRefPic;
}




ErrVal
LayerEncoder::xInitReordering( UInt    uiFrameIdInGOP,
                              PicType ePicType
                              , Bool  bUncompleteCOP //TMM
                              )
{
  SliceHeader* pcSliceHeader = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader( ePicType );
  ROF( pcSliceHeader );


  //===== set RPLR and MMCO =====
  if( pcSliceHeader->getTemporalId() == 0 )
  {
    if( ePicType!=FRAME && m_pacControlData[ uiFrameIdInGOP ].getSliceHeader( TOP_FIELD )->getIdrFlag() )
    {
      m_cLPFrameNumList.clear();
      m_cLPFrameNumList.push_front( pcSliceHeader->getFrameNum() );
      return Err::m_nOK;
    }
    else
    { 
    //===== low-pass frames =====
    RNOK( xSetRplrAndMmco( *pcSliceHeader ) );
  }
  }
  else if( m_uiDecompositionStages - pcSliceHeader->getTemporalId() >= m_uiNotCodedStages     )
  {
    UIntList cFrameNumList;
    UIntList cFieldNumList;
    pcSliceHeader->getDecRefPicMarking().clear( false, false );

    if( bUncompleteCOP )
    {
      RNOK( xSetMmcoFld( *pcSliceHeader ) );
    }

    if( pcSliceHeader->getSliceType() == B_SLICE )
    {
      // LIST_0
      if( ePicType==FRAME )
      {
      if( m_bForceReOrderingCommands || pcSliceHeader->getNumRefIdxActive( LIST_0 ) > 1 )
      {
        RNOK( xGetFrameNumList( *pcSliceHeader,                       cFrameNumList, LIST_0, uiFrameIdInGOP ) );
          RNOK( xSetRplr         ( pcSliceHeader->getRefPicListReordering( LIST_0 ), cFrameNumList, pcSliceHeader->getFrameNum(), ePicType ) );
      }
      else
      {
        pcSliceHeader->getRefPicListReordering( LIST_0 ).clear( false );
      }
      }
      else//ePicType!=FRAME
      {
       //TMM if( pcSliceHeader->getNalRefIdc() && pcSliceHeader->getNumRefIdxActive( LIST_0 ) > 1 )
        if( m_bForceReOrderingCommands || pcSliceHeader->getNumRefIdxActive( LIST_0 ) > 1 )
       {
          RNOK( xGetFieldNumList (*pcSliceHeader,                          cFieldNumList, LIST_0, uiFrameIdInGOP                     ) );
          RNOK( xSetRplr         ( pcSliceHeader->getRefPicListReordering( LIST_0 ), cFieldNumList, pcSliceHeader->getFrameNum()*2+1, ePicType ) );
      }
      else
      {
        pcSliceHeader->getRefPicListReordering( LIST_0 ).clear( false );
      }
      }  
      // LIST_1
      if( ePicType==FRAME )
      {
      if( m_bForceReOrderingCommands )
      {
        RNOK( xGetFrameNumList( *pcSliceHeader,                       cFrameNumList, LIST_1, uiFrameIdInGOP ) );
          RNOK( xSetRplr        ( pcSliceHeader->getRefPicListReordering( LIST_1 ), cFrameNumList, pcSliceHeader->getFrameNum(), ePicType ) );
      }
      else
      {
        pcSliceHeader->getRefPicListReordering( LIST_1 ).clear( false );
      }
    }
      else//ePicType!=FRAME
      {
        pcSliceHeader->getRefPicListReordering( LIST_1 ).clear( false );
      }
    }
    else
    {
      ROF( pcSliceHeader->getSliceType() == P_SLICE );

      if( ePicType==FRAME )
      {
      RNOK( xGetFrameNumList( *pcSliceHeader,                       cFrameNumList, LIST_0, uiFrameIdInGOP ) );
        RNOK( xSetRplr          ( pcSliceHeader->getRefPicListReordering( LIST_0 ), cFrameNumList, pcSliceHeader->getFrameNum(),     ePicType ) );
      }
      else//ePicType!=FRAME
      {
        RNOK( xGetFieldNumList  (*pcSliceHeader,                          cFieldNumList, LIST_0, uiFrameIdInGOP                     ) );
        RNOK( xSetRplr          ( pcSliceHeader->getRefPicListReordering( LIST_0 ), cFieldNumList, pcSliceHeader->getFrameNum()*2+1, ePicType ) );
      }

      pcSliceHeader->getRefPicListReordering( LIST_1 ).clear( false );
    }
  }

  return Err::m_nOK;
}






ErrVal
LayerEncoder::xSetScalingFactors()
{
  for( UInt uiLevel = 0; uiLevel < m_uiDecompositionStages; uiLevel++ )
  {
    RNOK( xSetScalingFactors( uiLevel ) );
  }
  return Err::m_nOK;
}


ErrVal
LayerEncoder::xSetScalingFactors( UInt uiBaseLevel )
{
  Double  adRateL0 [( 1 << MAX_DSTAGES )];
  Double  adRateL1 [( 1 << MAX_DSTAGES )];
  Double  adRateBi [( 1 << MAX_DSTAGES )];

  Double  dScalingBase    = m_pacControlData[0].getScalingFactor();
  Double  dScalingLowPass = 0.0;
  Int     iLowPassSize    = ( m_uiGOPSize >> uiBaseLevel );
  Int     iFrame;


  //===== get connection data =====
  for( iFrame = 1; iFrame <= iLowPassSize; iFrame += 2 )
  {
    RNOK( xGetConnections( adRateL0[iFrame], adRateL1[iFrame], adRateBi[iFrame] ) );
  }


  //===== get low-pass scaling =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame += 2 )
  {
    Double  dScalLPCurr = 1.0;

    if( iFrame > 0 )
    {
      if( ( iFrame + 1 ) < iLowPassSize )
      {
        dScalLPCurr = ( adRateBi[iFrame-1] + adRateBi[iFrame+1] ) * ( FACTOR_53_LP - 1.0 ) / 2.0 +
                      ( adRateL1[iFrame-1] + adRateL0[iFrame+1] ) * ( FACTOR_22_LP - 1.0 ) / 2.0 + 1.0;
      }
      else
      {
        dScalLPCurr = ( adRateBi[iFrame-1] / 2.0 ) * ( FACTOR_53_LP - 1.0 ) +
                      ( adRateL1[iFrame-1]       ) * ( FACTOR_22_LP - 1.0 ) + 1.0;
      }
    }
    else
    {
      if( iLowPassSize )
      {
        dScalLPCurr = ( adRateBi[iFrame+1] / 2.0 ) * ( FACTOR_53_LP - 1.0 ) +
                      ( adRateL0[iFrame+1]       ) * ( FACTOR_22_LP - 1.0 ) + 1.0;
      }
    }

    dScalingLowPass += dScalLPCurr;
  }
  dScalingLowPass /= (Double)( 1 + ( iLowPassSize >> 1 ) );

  //{{Scaling factor Base Layer-France Telecom R&D (nathalie.cammas@francetelecom.com), JVTO045 and m11876
  Double dFactor53;
  Double dFactor22;
#if SCALING_FACTOR_HACK
  // heiko.schwarz@hhi.fhg.de: This is a bad hack for ensuring that the
  // closed-loop config files work and use identical scaling factor as
  // the old version. The non-update scaling factors don't work and shall
  // be completely removed in future versions.
  if( m_uiDependencyId == 0 && m_uiFrameWidthInMb <= 11 )
#else
  if( false )
#endif
  {
    dFactor53 = FACTOR_53_HP_BL;
    dFactor22 = FACTOR_22_HP_BL;
  }
  else
  {
    dFactor53 = FACTOR_53_HP;
    dFactor22 = FACTOR_22_HP;
  }
  //}}Scaling factor Base Layer
  

  //===== get high-pass scaling and set scaling factors =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame++ )
  {
    Double dScal = dScalingBase;

    if( iFrame % 2 )
    {
      //===== high-pass pictures =====
      //{{Scaling factor Base Layer-France Telecom R&D (nathalie.cammas@francetelecom.com), JVTO045 and m11876
      dScal *= ( adRateBi[iFrame]                    ) * ( dFactor53 - 1.0 ) +
               ( adRateL0[iFrame] + adRateL1[iFrame] ) * ( dFactor22 - 1.0 ) + 1.0;
      //}}Scaling factor Base Layer
    }
    else
    {
      //===== low-pass pictures =====
      dScal *= dScalingLowPass;
    }
    m_pacControlData[ iFrame << uiBaseLevel ].setScalingFactor( dScal );
  }

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xClearBufferExtensions()
{
  for( UInt uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    RNOK( m_papcFrame   [uiFrame]->uninitHalfPel() );
    RNOK( m_papcFrame   [ uiFrame ]->removeFrameFieldBuffer() );
    RNOK( m_papcResidual[uiFrame]->uninitHalfPel() );
    RNOK( m_papcResidual[ uiFrame ]->removeFrameFieldBuffer() );

    if( m_papcCLRecFrame )
    {
      RNOK( m_papcCLRecFrame[uiFrame]->uninitHalfPel() );
      RNOK( m_papcCLRecFrame[ uiFrame ]->removeFrameFieldBuffer() );
    }
  }
  return Err::m_nOK;
}



ErrVal
LayerEncoder::xGetPredictionLists( RefFrameList& rcRefList0,
                                  RefFrameList& rcRefList1,
                                  UInt          uiBaseLevel,
                                  UInt          uiFrame,
                                  RefListUsage  eRefListUsage,
                                  Bool          bHalfPel )
{
  rcRefList0.reset();
  rcRefList1.reset();

  const UInt    uiFrameIdInGOP    = ( uiFrame << uiBaseLevel );
  SliceHeader*  pcSliceHeader     = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader( FRAME );
  ROF( pcSliceHeader );
  const Bool    bFrameMbsOnlyFlag = pcSliceHeader->getSPS().getFrameMbsOnlyFlag();
  UInt          uiList0Size     = pcSliceHeader->getNumRefIdxActive( LIST_0 );
  UInt          uiList1Size     = pcSliceHeader->getNumRefIdxActive( LIST_1 );

  //===== list 0 =====
  {
   Int iFrameId;
   for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
    {
      Frame* pcFrame = xGetRefFrame( m_papcFrame, iFrameId << uiBaseLevel, eRefListUsage );
    if(!pcFrame->getUnusedForRef())  // JVT-Q065 EIDR
    {
    //----- create half-pel buffer -----
  
      if( ! pcFrame->isExtended() )
    {
        if( bHalfPel ) { RNOK( xFillAndUpsampleFrame ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame   ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
    }

    RNOK( rcRefList0.add( pcFrame ) );
    //bug-fix shenqiu EIDR{
    if(uiList0Size > 0)
    {
    uiList0Size--;
    }
    else
    {
      rcRefList0.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
    }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ) ; iFrameId += 2 )
  {
    Frame* pcFrame = xGetRefFrame( m_papcFrame, iFrameId << uiBaseLevel, eRefListUsage );

    //----- create half-pel buffer -----
    if(!pcFrame->getUnusedForRef())
    {
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel ) { RNOK( xFillAndUpsampleFrame ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame   ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
      }

      RNOK( rcRefList0.add( pcFrame ) );
      
      if(uiList0Size > 0)
      {
      uiList0Size--;
    }
      else
      {
        rcRefList0.decActive();
      }
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

  ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    Int iFrameId;
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ); iFrameId += 2 )
    {
      Frame* pcFrame = xGetRefFrame( m_papcFrame, iFrameId << uiBaseLevel, eRefListUsage );

      //----- create half-pel buffer -----
    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR{
    {
    if( ! pcFrame->isExtended() )
    {
       if( bHalfPel ) { RNOK( xFillAndUpsampleFrame ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame   ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }

    }

    RNOK( rcRefList1.add( pcFrame ) );
    //bug-fix shenqiu EIDR{
    if(uiList1Size > 0)
    {
    uiList1Size--;
    }
    else
    {
      rcRefList1.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
    }
// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
  {
    Frame* pcFrame = xGetRefFrame( m_papcFrame, iFrameId << uiBaseLevel, eRefListUsage );

    if(!pcFrame->getUnusedForRef())
    {
      if( ! pcFrame->isExtended() )
      {
         if( bHalfPel ) { RNOK( xFillAndUpsampleFrame ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame   ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }

      }

      RNOK( rcRefList1.add( pcFrame ) );

      if(uiList1Size > 0)
      {
      uiList1Size--;
    }
      else
      {
        rcRefList1.decActive();
      }
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}
  ROT( uiList1Size );
  }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  if( rcRefList1.getSize() >= 2 && rcRefList0.getSize() == rcRefList1.getSize()) 
  {
    Bool bSwitch = true;
    for( UInt uiPos = 0; uiPos < rcRefList0.getSize(); uiPos++ )
    {
      if( rcRefList0.getEntry(uiPos) != rcRefList1.getEntry(uiPos) )
      {
        bSwitch = false;
        break;
      }
    }
    if( bSwitch )
    {
      rcRefList1.switchFirst();
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xGetPredictionListsField( RefFrameList& rcRefList0,
                                       RefFrameList& rcRefList1,
                                       UInt          uiBaseLevel,
                                       UInt          uiFrame,
                                       RefListUsage  eRefListUsage,
                                       Bool          bHalfPel,
                                       PicType       eCurrentPicType )
{
  rcRefList0.reset();
  rcRefList1.reset();

  const UInt    uiFrameIdInGOP    = ( uiFrame << uiBaseLevel );
  SliceHeader*  pcSliceHeader     = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader( eCurrentPicType );
  ROF( pcSliceHeader );
  const Bool    bFrameMbsOnlyFlag = pcSliceHeader->getSPS().getFrameMbsOnlyFlag();

  RefFrameList rcTmpList0;
  RefFrameList rcTmpList1;
  rcTmpList0.reset();
  rcTmpList1.reset();
  
  Int iFrameId;

  if( eCurrentPicType == BOT_FIELD )
  {
    Frame* pcFrame = m_apcFrameTemp[ 5 ];
    RNOK( rcTmpList0.add( pcFrame ) );
    pcFrame->setFrameNum( m_pacControlData[  uiFrame << uiBaseLevel ].getSliceHeader()->getFrameNum() );
  }

  //===== temp list 0 =====
  for( iFrameId = Int( uiFrame-1 ); iFrameId >= 0; iFrameId -= 2 )
  {
    Frame* pcFrame = xGetRefFrame( m_papcFrame, iFrameId << uiBaseLevel, eRefListUsage );
    RNOK( rcTmpList0.add( pcFrame ) );
    //TMM
    pcFrame->setFrameNum( m_pacControlData[ iFrameId << uiBaseLevel ].getSliceHeader()->getFrameNum() );
  }

  //===== temp list 1 =====
  for( iFrameId = Int( uiFrame+1 ); iFrameId <= Int( m_uiGOPSize >> uiBaseLevel ); iFrameId += 2 )
  {
    Frame* pcFrame = xGetRefFrame( m_papcFrame, iFrameId << uiBaseLevel, eRefListUsage );
    RNOK( rcTmpList1.add( pcFrame ) );
    pcFrame->setFrameNum( m_pacControlData[ iFrameId << uiBaseLevel ].getSliceHeader()->getFrameNum() ); //TMM
  }

  const PicType eOppositePicType = ( eCurrentPicType==TOP_FIELD ? BOT_FIELD : TOP_FIELD );
  UInt uiList0Size = pcSliceHeader->getNumRefIdxActive( LIST_0 );
  UInt uiList1Size = pcSliceHeader->getNumRefIdxActive( LIST_1 );
  UInt uiCurrentParityIndex  = 0;

  if( eCurrentPicType == BOT_FIELD )
  {
       uiCurrentParityIndex  = 1;
  }

  UInt uiOppositeParityIndex = 0;
  //===== list 0 =====
  while( ( uiCurrentParityIndex < rcTmpList0.getSize() || uiOppositeParityIndex < rcTmpList0.getSize() ) && uiList0Size )
  {
    if( eCurrentPicType == BOT_FIELD )
    {
    //--- opposite parity ---
    while( uiOppositeParityIndex < rcTmpList0.getSize() && uiList0Size )
    {
      Frame* pcFrame = rcTmpList0[ ++uiOppositeParityIndex ];
      if( !pcFrame->getUnusedForRef() )  // JVT-Q065 EIDR
      {
        //----- create half-pel buffer -----
        if( ! pcFrame->isExtended() )
        {
          if( bHalfPel ) { RNOK( xFillAndUpsampleFrame( pcFrame, eOppositePicType, bFrameMbsOnlyFlag ) ); }
          else           { RNOK( xFillAndExtendFrame  ( pcFrame, eOppositePicType, bFrameMbsOnlyFlag ) ); }
        }

        RNOK( rcRefList0.add( pcFrame->getPic( eOppositePicType ) ) );
        //bug-fix shenqiu EIDR{
        if( uiList0Size > 0 )
        {
          uiList0Size--;
        }
        else
        {
          rcRefList0.decActive();
        }
        //bug-fix shenqiu EIDR}
      }
      break;
    }
    //--- current parity ---
    while( uiCurrentParityIndex < rcTmpList0.getSize() && uiList0Size )
    {
      Frame* pcFrame = rcTmpList0[ ++uiCurrentParityIndex ];
      if( !pcFrame->getUnusedForRef() )  // JVT-Q065 EIDR
      {
        //----- create half-pel buffer -----
        if( ! pcFrame->isExtended() )
        {
          if( bHalfPel ) { RNOK( xFillAndUpsampleFrame( pcFrame, eCurrentPicType, bFrameMbsOnlyFlag ) ); }
          else           { RNOK( xFillAndExtendFrame  ( pcFrame, eCurrentPicType, bFrameMbsOnlyFlag ) ); }
        }

        RNOK( rcRefList0.add( pcFrame->getPic( eCurrentPicType ) ) );
        //bug-fix shenqiu EIDR{
        if( uiList0Size > 0 )
        {
          uiList0Size--;
        }
        else
        {
          rcRefList0.decActive();
        }
        //bug-fix shenqiu EIDR}
      }
      break;
    }
    }
    else
    {
    //--- current parity ---
    while( uiCurrentParityIndex < rcTmpList0.getSize() && uiList0Size )
    {
      Frame* pcFrame = rcTmpList0[ ++uiCurrentParityIndex ];
      if(!pcFrame->getUnusedForRef())  // JVT-Q065 EIDR
      {
       //----- create half-pel buffer -----
       if( ! pcFrame->isExtended() )
       {
        if( bHalfPel ) { RNOK( xFillAndUpsampleFrame( pcFrame, eCurrentPicType, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame  ( pcFrame, eCurrentPicType, bFrameMbsOnlyFlag ) ); }
       }
      
      RNOK( rcRefList0.add( pcFrame->getPic( eCurrentPicType ) ) );
        //bug-fix shenqiu EIDR{
        if(uiList0Size > 0)
        {
        uiList0Size--;
        }
        else
        {
          rcRefList0.decActive();
        }
        //bug-fix shenqiu EIDR}
      }
      break;
    }
    //--- opposite parity ---
    while( uiOppositeParityIndex < rcTmpList0.getSize() && uiList0Size )
    {
      Frame* pcFrame = rcTmpList0[ ++uiOppositeParityIndex ];
       if(!pcFrame->getUnusedForRef())  // JVT-Q065 EIDR
      {
      //----- create half-pel buffer -----
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel ) { RNOK( xFillAndUpsampleFrame( pcFrame, eOppositePicType, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame  ( pcFrame, eOppositePicType, bFrameMbsOnlyFlag ) ); }
      }
      
      RNOK( rcRefList0.add( pcFrame->getPic( eOppositePicType ) ) );
      //bug-fix shenqiu EIDR{
      if(uiList0Size > 0)
      {
      uiList0Size--;
      }
      else
      {
        rcRefList0.decActive();
      }
      //bug-fix shenqiu EIDR}
      }
      break;
    }
    }
  }

  ROT( uiList0Size );

  //===== list 1 =====
  uiCurrentParityIndex  = 0;
  uiOppositeParityIndex = 0;
  while( ( uiCurrentParityIndex < rcTmpList1.getSize() || uiOppositeParityIndex < rcTmpList1.getSize() ) && uiList1Size )
  {
    //--- current parity ---
    while( uiCurrentParityIndex < rcTmpList1.getSize() && uiList1Size )
    {
      Frame* pcFrame = rcTmpList1[ ++uiCurrentParityIndex ];

      if(!pcFrame->getUnusedForRef())  // JVT-Q065 EIDR
      {
      //----- create half-pel buffer -----
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel ) { RNOK( xFillAndUpsampleFrame( pcFrame, eCurrentPicType, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame  ( pcFrame, eCurrentPicType, bFrameMbsOnlyFlag ) ); }
      }

      RNOK( rcRefList1.add( pcFrame->getPic( eCurrentPicType ) ) );
      //bug-fix shenqiu EIDR{
    if(uiList1Size > 0)
    {
    uiList1Size--;
    }
    else
    {
      rcRefList1.decActive();
    }
    //bug-fix shenqiu EIDR}
      }
      break;
    }
    //--- opposite parity ---
    while( uiOppositeParityIndex < rcTmpList1.getSize() && uiList1Size )
    {
      Frame* pcFrame = rcTmpList1[ ++uiOppositeParityIndex ];

      if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR{
      {
      //----- create half-pel buffer -----
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel ) { RNOK( xFillAndUpsampleFrame( pcFrame, eOppositePicType, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame  ( pcFrame, eOppositePicType, bFrameMbsOnlyFlag ) ); }
      }

      RNOK( rcRefList1.add( pcFrame->getPic( eOppositePicType ) ) );
      //bug-fix shenqiu EIDR{
    if(uiList1Size > 0)
    {
    uiList1Size--;
    }
    else
    {
      rcRefList1.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
      break;
    }
  }

  ROT( uiList1Size );


  return Err::m_nOK;
}

ErrVal
LayerEncoder::xGetCLRecPredictionLists( RefFrameList& rcRefList0,
                                       RefFrameList& rcRefList1,
                                       UInt          uiBaseLevel,
                                       UInt          uiFrame,
                                       RefListUsage  eRefListUsage,
                                       Bool          bHalfPel )
{
  rcRefList0.reset();
  rcRefList1.reset();

  const UInt   uiFrameIdInGOP    = ( uiFrame << uiBaseLevel );
  SliceHeader* pcSliceHeader     = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader( FRAME );
  ROF( pcSliceHeader );
  const Bool   bFrameMbsOnlyFlag = pcSliceHeader->getSPS().getFrameMbsOnlyFlag();
  UInt          uiList0Size     = pcSliceHeader->getNumRefIdxActive( LIST_0 );
  UInt          uiList1Size     = pcSliceHeader->getNumRefIdxActive( LIST_1 );

  //===== list 0 =====
  {
    Int iFrameId;
  for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
    {
      Frame* pcFrame = xGetRefFrame( m_papcCLRecFrame, iFrameId << uiBaseLevel, eRefListUsage );

    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR
    {
    if( ! pcFrame->isExtended() )
    {
        if( bHalfPel ) { RNOK( xFillAndUpsampleFrame ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame   ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
    }

    RNOK( rcRefList0.add( pcFrame ) );
    //bug-fix shenqiu EIDR{
    if(uiList0Size > 0)
    {
    uiList0Size--;
    }
    else
    {
      rcRefList0.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
    }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ); iFrameId += 2 )
  {
    Frame* pcFrame = xGetRefFrame( m_papcCLRecFrame, iFrameId << uiBaseLevel, eRefListUsage );

    if(!pcFrame->getUnusedForRef())
    {
      //----- create half-pel buffer -----
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel ) { RNOK( xFillAndUpsampleFrame ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame   ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
      }

      RNOK( rcRefList0.add( pcFrame ) );
      if(uiList0Size > 0)
      {
      uiList0Size--;
    }
      else
      {
        rcRefList0.decActive();
      }
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}
  ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    Int iFrameId;
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ); iFrameId += 2 )
    {
      Frame* pcFrame = xGetRefFrame( m_papcCLRecFrame, iFrameId << uiBaseLevel, eRefListUsage );

    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR
    {
    //----- create half-pel buffer -----
    if( ! pcFrame->isExtended() )
    {
       if( bHalfPel ) { RNOK( xFillAndUpsampleFrame ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame   ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
  
    }

    RNOK( rcRefList1.add( pcFrame ) );
    //bug-fix shenqiu EIDR{
    if(uiList1Size > 0)
    {
    uiList1Size--;
    }
    else
    {
      rcRefList1.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
    }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
  {
    Frame* pcFrame = xGetRefFrame( m_papcCLRecFrame, iFrameId << uiBaseLevel, eRefListUsage );

    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR
    {
      if( ! pcFrame->isExtended() )
      {
         if( bHalfPel ) { RNOK( xFillAndUpsampleFrame ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
        else           { RNOK( xFillAndExtendFrame   ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
  
    }

      RNOK( rcRefList1.add( pcFrame ) );
      if(uiList1Size > 0)
    {
        uiList1Size--;
    }
      else
      {
        rcRefList1.decActive();
      }
  }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

  ROT( uiList1Size );
  }
// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  if( rcRefList1.getSize() >= 2 && rcRefList0.getSize() == rcRefList1.getSize()) 
  {
    Bool bSwitch = true;
    for( UInt uiPos = 0; uiPos < rcRefList0.getSize(); uiPos++ )
    {
      if( rcRefList0.getEntry(uiPos) != rcRefList1.getEntry(uiPos) )
      {
        bSwitch = false;
        break;
      }
    }
    if( bSwitch )
    {
      rcRefList1.switchFirst();
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}
  return Err::m_nOK;
}




ErrVal
LayerEncoder::xInitBaseLayerData( ControlData&  rcControlData, 
                                  UInt          uiBaseLevel,
                                  UInt          uiFrame,
                                  Bool          bMotion,
                                  RefFrameList* pcRefFrameList0,
                                  RefFrameList* pcRefFrameList1,
                                  PicType       ePicType )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  rcControlData.setBaseLayerSbb ( 0 );
  rcControlData.setBaseLayerCtrl( 0 );
  rcControlData.setBaseLayerCtrlField( 0 );
  
  Frame*     pcBaseFrame         = 0;
  Frame*     pcBaseResidual      = 0;
  MbDataCtrl*   pcBaseDataCtrl      = 0;
  MbDataCtrl*   pcBaseDataCtrlEL	  = 0;

  Bool          bBaseDataAvailable  = false;

  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  if( rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX )
  {
     RNOK( m_pcH264AVCEncoder->getBaseLayerDataAvailability( pcBaseFrame,
                                                             pcBaseResidual,
                                                             pcBaseDataCtrl,
                                                             rcControlData.getBaseLayerIdMotion      (),
                                                             bMotion,
                                                             bBaseDataAvailable,
                                                             ePicType,
                                                             pcSliceHeader->getTemporalId() ) );
                                     
        // Get resampling mode & obtain again with same resolution interlaced-to-progressive check
    if( bBaseDataAvailable )
    {
      m_pcResizeParameters->m_iLevelIdc                  = m_uiLevelIdc;//jzxu, 02Nov2007
      m_pcResizeParameters->m_bFrameMbsOnlyFlag          = pcSliceHeader->getSPS().getFrameMbsOnlyFlag();
      m_pcResizeParameters->m_bFieldPicFlag              = pcSliceHeader->getFieldPicFlag();
      m_pcResizeParameters->m_bBotFieldFlag              = pcSliceHeader->getBottomFieldFlag();
      m_pcResizeParameters->m_bIsMbAffFrame              = pcSliceHeader->isMbaffFrame();
      m_pcResizeParameters->m_bRefLayerFrameMbsOnlyFlag  = pcBaseDataCtrl->getSliceHeader()->getSPS().getFrameMbsOnlyFlag();
      m_pcResizeParameters->m_bRefLayerFieldPicFlag      = pcBaseDataCtrl->getSliceHeader()->getFieldPicFlag();;
      m_pcResizeParameters->m_bRefLayerIsMbAffFrame      = pcBaseDataCtrl->getSliceHeader()->isMbaffFrame();
      m_pcResizeParameters->m_bRefLayerBotFieldFlag      = pcBaseDataCtrl->getSliceHeader()->getBottomFieldFlag();
      rcControlData.setSpatialScalability( m_pcResizeParameters->getSpatialResolutionChangeFlag() );

      RNOK( m_pcH264AVCEncoder->getBaseLayerData( *pcSliceHeader,
                                                  pcBaseFrame,
                                                  pcBaseResidual,
                                                  pcBaseDataCtrl,
                                                  pcBaseDataCtrlEL,
                                                  m_pcResizeParameters->getSpatialResolutionChangeFlag(),
                                                  rcControlData.getBaseLayerIdMotion(),
                                                  bMotion,
                                                  ePicType,
                                                  pcSliceHeader->getTemporalId() ) );
    }
  }


  //===== motion data =====
  if( pcBaseDataCtrl )
  {
    if( pcBaseDataCtrlEL )
    {
      pcBaseDataCtrlEL->setSliceHeader( pcBaseDataCtrl->getSliceHeader() );
      pcBaseDataCtrl = pcBaseDataCtrlEL;
    }
    pcSliceHeader->setSCoeffResidualPredFlag( m_pcResizeParameters );
    RNOK( m_pcBaseLayerCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );
    RNOK( m_pcBaseLayerCtrl->upsampleMotion( pcSliceHeader, m_pcResizeParameters, pcBaseDataCtrl, pcRefFrameList0, pcRefFrameList1, m_pcResizeParameters->m_bFieldPicFlag, (m_uiEssRPChkEnable!=0), m_uiMVThres ) );
    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );

    if( m_pcResizeParameters->m_bIsMbAffFrame )
    {
      RNOK( m_pcBaseLayerCtrlField->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );
      RNOK( m_pcBaseLayerCtrlField->upsampleMotion( pcSliceHeader, m_pcResizeParameters, pcBaseDataCtrl, pcRefFrameList0, pcRefFrameList1, true, (m_uiEssRPChkEnable!=0), m_uiMVThres ) );
      rcControlData.setBaseLayerCtrlField( m_pcBaseLayerCtrlField );
    }

//TMM_WP
    SliceHeader *pcSliceHeaderCurr, *pcSliceHeaderBase;
    pcSliceHeaderCurr = rcControlData.getSliceHeader(ePicType);
    pcSliceHeaderBase = pcBaseDataCtrl->getSliceHeader();
    
    //indicates whether the base layer use wp or not
    m_bBaseLayerWp = pcSliceHeaderBase->getPPS().getWeightedPredFlag();

    pcSliceHeaderCurr->setLumaLog2WeightDenom( pcSliceHeaderBase->getLumaLog2WeightDenom() );
    pcSliceHeaderCurr->setChromaLog2WeightDenom( pcSliceHeaderBase->getChromaLog2WeightDenom() );
    pcSliceHeaderCurr->getPredWeightTable( LIST_0 ).copy( pcSliceHeaderBase->getPredWeightTable( LIST_0 ) );
    pcSliceHeaderCurr->getPredWeightTable( LIST_1 ).copy( pcSliceHeaderBase->getPredWeightTable( LIST_1 ) );
//TMM_WP
  }


  //===== residual data =====
  if( bBaseDataAvailable )
  {
    RNOK( m_pcBaseLayerResidual->residualUpsampling( pcBaseResidual, m_cDownConvert, m_pcResizeParameters, pcBaseDataCtrl ) );
    rcControlData.setBaseLayerSbb( m_pcBaseLayerResidual );
  }


  //==== reconstructed (intra) data =====
  if( bBaseDataAvailable )
  {
    Frame*  pcTempBaseFrame = m_apcFrameTemp[0];
    Frame*  pcTempFrame     = m_apcFrameTemp[1];
    RNOK( m_pcBaseLayerFrame->intraUpsampling( pcBaseFrame, pcTempBaseFrame, pcTempFrame, m_cDownConvert, m_pcResizeParameters, 
                                               pcBaseDataCtrl, m_pcBaseLayerCtrl, m_pcBaseLayerCtrlField,
                                               m_pcReconstructionBypass, m_bCIUFlag, m_apabBaseModeFlagAllowedArrays[0], m_apabBaseModeFlagAllowedArrays[1] ) );
    m_pcSliceEncoder->setIntraBLFlagArrays( m_apabBaseModeFlagAllowedArrays );
    rcControlData.setBaseLayerRec( m_pcBaseLayerFrame );
    
  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
    m_pcBaseLayerFrame->setChannelDistortion(pcBaseFrame);
  //JVT-R057 LA-RDO}
  }

  setMCResizeParameters(m_pcResizeParameters);

  return Err::m_nOK;
}

Void LayerEncoder::setMCResizeParameters   (ResizeParameters*				resizeParameters)
{
  m_pcMotionEstimation->setResizeParameters(resizeParameters);
} 


ErrVal
LayerEncoder::xInitControlDataMotion( UInt          uiBaseLevel,
                                      UInt          uiFrame,
                                      Bool          bMotionEstimation,
                                      PicType       ePicType )
{
  UInt            uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&    rcControlData   = m_pacControlData[uiFrameIdInGOP];
  SliceHeader*    pcSliceHeader   = rcControlData.getSliceHeader  ( ePicType );
  ROF( pcSliceHeader );
  Double          dScalFactor     = rcControlData.getScalingFactor();  
  Double          dQpPredData     = m_adBaseQpLambdaMotion[ uiBaseLevel ] - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    dQpPredData = m_adBaseQpLambdaMotion[ uiBaseLevel ] = pcGenericRC->m_pcJSVMParams->qp;
  }
  // JVT-W043 }
  if( m_bExplicitQPCascading )
  {
    UInt  uiTLevel               = m_uiDecompositionStages - uiBaseLevel;
    dQpPredData                  = m_adBaseQpLambdaMotion[ uiBaseLevel ] + m_adDeltaQPTLevel[ uiTLevel ];
  }
  Double          dLambda           = 0.85 * pow( 2.0, min( 52.0, dQpPredData ) / 3.0 - 4.0 );
  Int             iQp               = max( MIN_QP, min( MAX_QP, (Int)floor( dQpPredData + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp( iQp );
  rcControlData. setLambda       ( dLambda );

  if( bMotionEstimation )
  {
    RefFrameList* pcRefFrameList0 = &rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList* pcRefFrameList1 = &rcControlData.getPrdFrameList( LIST_1 );
    m_pcResizeParameters->updatePicParameters( m_papcFrame[ uiFrameIdInGOP ]->getPicParameters( ePicType ) );
    RNOK( xInitBaseLayerData( rcControlData,uiBaseLevel, uiFrame, true, pcRefFrameList0, pcRefFrameList1, ePicType) ); 
  }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitControlDataLowPass( UInt           uiFrameIdInGOP, 
                                       UInt           uiBaseLevel,
                                       UInt           uiFrame,
                                       PicType        ePicType )
{
  ControlData&  rcControlData = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader* pcSliceHeader  = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );
  
  Double        dScalFactor   = rcControlData.getScalingFactor();
  Double        dQP           = m_dBaseQpLambdaMotionLP - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  if( m_bExplicitQPCascading )
  {
    dQP                       = m_dBaseQpLambdaMotionLP + m_adDeltaQPTLevel[ 0 ];
  }
  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    dQP = m_dBaseQpLambdaMotionLP = pcGenericRC->m_pcJSVMParams->qp;
  }
  // JVT-W043 }
  Double        dLambda         = 0.85 * pow( 2.0, min( 52.0, dQP ) / 3.0 - 4.0 );
  dQP                           = m_dBaseQPResidual - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  if( m_bExplicitQPCascading )
  {
    dQP                         = m_dBaseQPResidual + m_adDeltaQPTLevel[ 0 ];
  }
  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    dQP = m_dBaseQPResidual = pcGenericRC->m_pcJSVMParams->qp;
  }
  // JVT-W043 }
  Int           iQP             = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp ( iQP );
  rcControlData. setLambda        ( dLambda );

//TMM_WP
  rcControlData.getPrdFrameList ( LIST_0 ).reset();
  rcControlData.getPrdFrameList ( LIST_1 ).reset();
  RNOK( rcControlData.getPrdFrameList ( LIST_0 ).add  ( m_pcLowPassBaseReconstruction ) );
  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );

  m_pcResizeParameters->updatePicParameters( m_papcFrame[ uiFrameIdInGOP ]->getPicParameters( ePicType ) );
  RNOK( xInitBaseLayerData( rcControlData,uiBaseLevel,uiFrame, false, &rcRefFrameList0, &rcRefFrameList1, ePicType ) );

  /* el & has correspongding bl frame & bl does wp then use bl wts */
  if((m_uiDependencyId > 0 && m_bBaseLayerWp && 
      rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX) )
  {
      /* use same wts as bl */
      pcSliceHeader->setBasePredWeightTableFlag(true);
  }
  else
  {
      /* call function to calculate the weights */
      m_pcSliceEncoder->xSetPredWeights( *pcSliceHeader, 
                                         m_papcFrame[uiFrameIdInGOP],
                                         rcRefFrameList0,
                                         rcRefFrameList1
                                         //m_pcLowPassBaseReconstruction,                              
                                         //NULL,
                                         );
      pcSliceHeader->setBasePredWeightTableFlag(false);
  }

  rcControlData.getPrdFrameList ( LIST_0 ).reset();
//TMM_WP

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitControlDataHighPass( UInt          uiFrameIdInGOP,
                                        UInt          uiBaseLevel,
                                        UInt          uiFrame,
                                        PicType       ePicType )  
{
  ControlData&  rcControlData = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );
  
  Double        dScalFactor   = rcControlData.getScalingFactor();
  Double        dQP           = m_dBaseQPResidual - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  if( m_bExplicitQPCascading )
  {
    UInt  uiTLevel            = m_uiDecompositionStages - uiBaseLevel;
    dQP                       = m_dBaseQPResidual + m_adDeltaQPTLevel[ uiTLevel ];
  }
  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    dQP = m_dBaseQPResidual = pcGenericRC->m_pcJSVMParams->qp;
  }
  // JVT-W043 }
  Double        dLambda         = 0.85 * pow( 2.0, min( 52.0, dQP ) / 3.0 - 4.0 );
  Int           iQP             = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp ( iQP );
  rcControlData. setLambda        ( dLambda );

  RefFrameList* pcRefFrameList0 = &rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList* pcRefFrameList1 = &rcControlData.getPrdFrameList( LIST_1 );
  m_pcResizeParameters->updatePicParameters( m_papcFrame[ uiFrameIdInGOP ]->getPicParameters( ePicType ) );
  RNOK( xInitBaseLayerData( rcControlData, uiBaseLevel, uiFrame, false, pcRefFrameList0, pcRefFrameList1, ePicType ) );

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xMotionEstimationFrame( UInt uiBaseLevel, UInt uiFrame, PicType ePicType ) //TMM
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  Frame*     pcIntraRecFrame = m_apcFrameTemp  [0];
  
  const Bool bFieldCoded = m_pbFieldPicFlag[ uiFrameIdInGOP ] ; 

  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );

    //===== get reference frame lists =====
    if( m_papcCLRecFrame )
    {
        if( ! bFieldCoded )
        {
          RNOK( xGetCLRecPredictionLists( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_MOTION_ESTIMATION, true ) );
        }
        else/*bFieldCoded*/
        {
          AF( );
        }
    }
    else
    {
      if( ! bFieldCoded )
      {
        RNOK( xGetPredictionLists     ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_MOTION_ESTIMATION, true ) );
      }
      else/*bFieldCoded*/
      {
       RNOK( xGetPredictionListsField( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_MOTION_ESTIMATION, true, ePicType ) );
      }
    }

    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;
      pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP;
      pcGenericRC->m_pcJSVMParams->type = B_SLICE;
      pcJSVMParams->CurrGopLevel = pcGenericRC->getCurrGopLevel( pcGenericRC->m_pcJSVMParams->current_frame_number );
      pcGenericRC->m_pcJSVMParams->nal_reference_idc = (pcJSVMParams->CurrGopLevel == 0) ? 0 : 1;
      pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
    }
    // JVT-W043 }

    //===== set lambda and QP =====
    RNOK( xInitControlDataMotion  ( uiBaseLevel, uiFrame, true, ePicType ) );

  //S051{
  m_pcSliceEncoder->setUseBDir(true);
  if(m_bEncSIP)
  {
      if(m_bH264AVCCompatible||!rcControlData.getSliceHeader( ePicType )->getDirectSpatialMvPredFlag())
    {
      int				pos				  = xGetMbDataCtrlL1Pos( *rcControlData.getSliceHeader( ePicType ), rcRefFrameList1 );
      if(pos!=-1)
      {
      SliceHeader* pcSliceHeaderL1     = m_pacControlData[pos].getSliceHeader  ( ePicType );
      if(xSIPCheck(pcSliceHeaderL1->getPoc()))
        m_pcSliceEncoder->setUseBDir(false);
    }
  }
}
//S051}

//TMM_WP    
  /* el & has correspongding bl frame & bl uses wp then use bl wts */
  if((m_uiDependencyId > 0 && m_bBaseLayerWp &&
      rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX) )
  {
      /* use same wts as bl */
      pcSliceHeader->setBasePredWeightTableFlag(true);
  }
  else
  {
      /* call function to calculate the weights */
      m_pcSliceEncoder->xSetPredWeights( *pcSliceHeader, 
                                         pcFrame,
                                         rcRefFrameList0,
                                         rcRefFrameList1
                                         );
      pcSliceHeader->setBasePredWeightTableFlag(false);
  }
//TMM_WP
  // JVT-R057 LA-RDO{ 
  if(m_bLARDOEnable)
  {
    pcFrame->initChannelDistortion();
    m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
  }
// JVT-R057 LA-RDO}
  
  //===== motion estimation =====
    if( pcSliceHeader->isMbaffFrame() )
    {
      RNOK( xMotionEstimationMbAff( &rcRefFrameList0, 
                                    &rcRefFrameList1,
                                    pcFrame, 
                                    pcIntraRecFrame, 
                                    rcControlData,
                                    m_bBiPredOnly, 
                                    m_uiNumMaxIter, 
                                    m_uiIterSearchRange,
                                    uiFrameIdInGOP ) );
    }
  else
    {
    RNOK( xMotionEstimation     ( &rcRefFrameList0, 
                                &rcRefFrameList1,
                                pcFrame, 
                                pcIntraRecFrame, 
                                rcControlData,
                                m_bBiPredOnly, 
                                m_uiNumMaxIter, 
                                m_uiIterSearchRange, 
                                uiFrameIdInGOP ,
                                ePicType ) );
    }															
  return Err::m_nOK;
}


ErrVal
LayerEncoder::xDecompositionFrame( UInt uiBaseLevel, UInt uiFrame, PicType ePicType ) //TMM
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  Frame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
  Frame*     pcMCFrame       = m_apcFrameTemp  [0];
 
  const Bool bFieldCoded =  m_pbFieldPicFlag[ uiFrameIdInGOP ] ;
  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
  //===== get reference frame lists =====
  if( m_papcCLRecFrame )
  {
      if( ! bFieldCoded )
      {
        RNOK( xGetCLRecPredictionLists( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_GET_RESIDUAL, false ) );
      }
      else/*bFieldCoded*/
      {
        AF();
      }
  }
  else
  {
      if( ! bFieldCoded )
      {	
        RNOK( xGetPredictionLists     ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_GET_RESIDUAL, false ) );
      }
      else/*bFieldCoded*/
      {
        RNOK( xGetPredictionListsField( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_GET_RESIDUAL, false, ePicType ) );
      }
  }

  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;
    pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP;
    pcGenericRC->m_pcJSVMParams->type = B_SLICE;
    pcJSVMParams->CurrGopLevel = pcGenericRC->getCurrGopLevel( pcGenericRC->m_pcJSVMParams->current_frame_number );
    pcGenericRC->m_pcJSVMParams->nal_reference_idc = (pcJSVMParams->CurrGopLevel == 0) ? 0 : 1;
    pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
  }
  // JVT-W043 }

  //===== set lambda and QP =====// warnnig false in 7.8
    RNOK( xInitControlDataMotion( uiBaseLevel, uiFrame, true, ePicType ) );

  setBaseMbDataCtrl(rcControlData.getBaseLayerCtrl());

  //===== prediction =====
    RNOK( xMotionCompensation  ( pcMCFrame, &rcRefFrameList0, &rcRefFrameList1, rcControlData.getMbDataCtrl(), *pcSliceHeader ) );
    RNOK( pcFrame->prediction  ( pcMCFrame, pcFrame,        ePicType ) );

  //===== set residual =====
    RNOK( pcResidual->copy     ( pcFrame                  , ePicType ) );
    RNOK( xZeroIntraMacroblocks( pcResidual, rcControlData, ePicType ) );

    return Err::m_nOK;
}


ErrVal
LayerEncoder::xCompositionFrame( UInt uiBaseLevel, UInt uiFrame, PicBufferList& rcPicBufferInputList, PicType ePicType ) //TMM
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  Frame*     pcResidual     = m_papcResidual  [ uiFrameIdInGOP ];
  Frame*     pcMCFrame       = m_apcFrameTemp  [0];
  
  const Bool bFieldCoded = m_pbFieldPicFlag[ uiFrameIdInGOP ] ; 

  setBaseMbDataCtrl(rcControlData.getBaseLayerCtrl());

  SliceHeader*  pcSliceHeader = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader( ePicType );
  ROF( pcSliceHeader );
    
  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;
    pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP;
    pcGenericRC->m_pcJSVMParams->type = B_SLICE;
    pcJSVMParams->CurrGopLevel = pcGenericRC->getCurrGopLevel( pcGenericRC->m_pcJSVMParams->current_frame_number );
    pcGenericRC->m_pcJSVMParams->nal_reference_idc = (pcJSVMParams->CurrGopLevel == 0) ? 0 : 1;
    pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
  }
  // JVT-W043 }

  //--- highest FGS reference for closed-loop coding ---
  Frame*     pcCLRecFrame       = 0;
  Frame*     pcCLRecResidual    = 0;
  RefFrameList  acCLRecRefFrameList[2];
  if( m_papcCLRecFrame )
    {
     pcCLRecFrame       = m_papcCLRecFrame   [uiFrameIdInGOP];
     pcCLRecResidual    = m_apcFrameTemp     [2];
 
      //===== get reference frame lists =====
       if( ! bFieldCoded )
       {
         RNOK( xGetCLRecPredictionLists       ( acCLRecRefFrameList[0], acCLRecRefFrameList[1], uiBaseLevel, uiFrame, RLU_RECONSTRUCTION ) );
       }
       else/*bFieldCoded*/
       {
         AF();
       }
 
      RNOK( pcCLRecResidual->copy( pcCLRecFrame                  , ePicType ) );
      RNOK( xZeroIntraMacroblocks( pcCLRecResidual, rcControlData, ePicType ) );
 
      //===== prediction =====
      RNOK( xMotionCompensation         ( pcMCFrame, &acCLRecRefFrameList[0], &acCLRecRefFrameList[1],
                                         rcControlData.getMbDataCtrl(), *pcSliceHeader ) );
      RNOK( pcCLRecFrame->inversePrediction( pcMCFrame, pcCLRecFrame, ePicType ) );
    }

    //===== get reference frame lists =====
    RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );

    if( ! bFieldCoded )
    {
      RNOK( xGetPredictionLists         ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_RECONSTRUCTION, false ) );
    }
    else/*bFieldCoded*/
    {
      RNOK( xGetPredictionListsField  ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_RECONSTRUCTION, false, ePicType ) );
    }

    //===== prediction =====
    RNOK( xMotionCompensation         ( pcMCFrame, &rcRefFrameList0, &rcRefFrameList1,
                                      rcControlData.getMbDataCtrl(), *pcSliceHeader ) );

    RNOK( pcFrame ->inversePrediction ( pcMCFrame, pcFrame, ePicType ) );
   //--
    //----- store non-deblocked signal for inter-layer prediction -----
    RNOK( m_papcSubband[uiFrameIdInGOP]->copy( pcFrame, ePicType ) );

    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_papcSubband[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
      if(m_papcCLRecFrame)
        m_papcCLRecFrame[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
    }
    // JVT-R057 LA-RDO}

    if( ! pcSliceHeader->getNoInterLayerPredFlag() )
    {
      m_pcSliceEncoder->updateBaseLayerResidual( rcControlData, m_uiFrameWidthInMb );
    }

    //===== de-blocking =====
    RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcFrame, pcResidual, rcControlData.getMbDataCtrl(), 0, rcControlData.getSpatialScalability() ) );  

    //--- highest FGS reference for closed-loop coding ---
    if( pcCLRecFrame )
    {
      RNOK( pcCLRecFrame->removeFrameFieldBuffer() );
      RNOK( rcControlData.activateMbDataCtrlForQpAndCbp( true ) );
      RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcCLRecFrame, pcCLRecResidual, rcControlData.getMbDataCtrl(), 0, rcControlData.getSpatialScalability() ) );  
    }

    if( ePicType == TOP_FIELD )
    {
      RNOK( m_apcFrameTemp[ 5 ]->uninitHalfPel    ()                    );
      RNOK( m_apcFrameTemp[ 5 ]->removeFieldBuffer( ePicType )          );
      RNOK( m_apcFrameTemp[ 5 ]->copy             ( pcFrame, ePicType ) );
    }

  return Err::m_nOK;
}

//--


ErrVal
LayerEncoder::xEncodeKeyPicture( Bool&               rbKeyPicCoded,
                                 UInt                uiFrame,
                                 AccessUnitData&     rcAccessUnitData,
                                 PicOutputDataList&  rcPicOutputDataList )
{
  rbKeyPicCoded         = false;
  UInt   uiFrameIdInGOP = uiFrame << m_uiDecompositionStages;
  ROTRS( uiFrameIdInGOP > m_uiGOPSize, Err::m_nOK );

  m_abCoded[uiFrameIdInGOP] = true;

  //===== check for first GOP =====
  if( uiFrame == 0 && m_uiGOPNumber )
  {
    //====== don't code first anchor picture if it was coded within the last GOP =====
    //bug-fix shenqiu EIDR{
    m_pcAnchorFrameReconstructed->setUnusedForRef(m_papcFrame[ uiFrameIdInGOP ]->getUnusedForRef());
    //bug-fix shenqiu EIDR}
      RNOK( m_papcFrame[ uiFrameIdInGOP ] ->copyAll( m_pcAnchorFrameReconstructed  ) );
    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_papcFrame[uiFrameIdInGOP]->copyChannelDistortion(m_pcLowPassBaseReconstruction);
      m_papcSubband[uiFrameIdInGOP]->setChannelDistortion(m_papcFrame[uiFrameIdInGOP]);
    }
    // JVT-R057 LA-RDO}
    return Err::m_nOK;
  }
  //JVT-W051 {
  if ( rbKeyPicCoded == 0 )
  {
    Int iPicType=(m_pbFieldPicFlag[ uiFrameIdInGOP ]?TOP_FIELD:FRAME);
    PicType ePicType               = PicType( iPicType );
    ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
    SliceHeader* pcSliceHeader           = rcControlData.getSliceHeader( ePicType );
    m_uiProfileIdc      = pcSliceHeader->getSPS().getProfileIdc();
    m_uiLevelIdc        = pcSliceHeader->getSPS().getLevelIdc();
    m_bConstraint0Flag  = pcSliceHeader->getSPS().getConstrainedSet0Flag();
    m_bConstraint1Flag  = pcSliceHeader->getSPS().getConstrainedSet1Flag();
    m_bConstraint2Flag  = pcSliceHeader->getSPS().getConstrainedSet2Flag();
    m_bConstraint3Flag  = pcSliceHeader->getSPS().getConstrainedSet3Flag();
  }
  //JVT-W051 }

  rbKeyPicCoded                         = true;
  UInt          uiBits          = 0;
  ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
  MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl ();
  Frame*     pcFrame         = m_papcFrame     [ uiFrameIdInGOP ];
  Frame*     pcResidual      = m_papcResidual  [ uiFrameIdInGOP ];
  Frame*     pcPredSignal    = m_apcFrameTemp  [ 0 ];
  Frame*     pcBLRecFrame    = m_apcFrameTemp  [ 1 ];

  const Bool bFieldCoded =  m_pbFieldPicFlag[ uiFrameIdInGOP ] ; 

  for( Int iPicType=(bFieldCoded?TOP_FIELD:FRAME); iPicType<=(bFieldCoded?BOT_FIELD:FRAME); iPicType++ )
  {
    const PicType ePicType               = PicType( iPicType );
    RNOK( xSetBaseLayerData( uiFrameIdInGOP, ePicType ) );

    SliceHeader* pcSliceHeader           = rcControlData.getSliceHeader( ePicType );
    //JVT-W047
    m_bOutputFlag = pcSliceHeader->getOutputFlag();
    //JVT-W047
    ROF( pcSliceHeader );
  
    ExtBinDataAccessorList& rcOutputList    = rcAccessUnitData.getNalUnitList();
    ExtBinDataAccessorList& rcRedOutputList = rcAccessUnitData.getRedNalUnitList();

    m_pcLowPassBaseReconstruction->setUnusedForRef(m_papcFrame[0]->getUnusedForRef());  // JVT-Q065 EIDR

    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;
      pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP;
      pcGenericRC->m_pcJSVMParams->type = 
        ( uiFrame > 0 && (uiFrame % pcJSVMParams->m_uiIntraPeriod) != 0 ) ? 
        ( ( pcGenericRC->m_pcJSVMParams->current_frame_number % m_uiGOPSize == 0 ) ? P_SLICE : B_SLICE ) : I_SLICE;
      pcGenericRC->m_pcJSVMParams->nal_reference_idc = 1;
      pcGenericRC->m_pcJSVMParams->number = pcGenericRC->m_pcJSVMParams->current_frame_number;

      pcQuadraticRC->rc_init_pict(1, 0, 1, 1.0F);
      if ( pcJSVMParams->m_uiIntraPeriod != 1 )
        pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
      else
        pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC1(0);
    }
    // JVT-W043 }

    //===== initialize =====
    RNOK( xInitControlDataLowPass ( uiFrameIdInGOP, m_uiDecompositionStages-1, uiFrame, ePicType ) );

    //NonRequired JVT-Q066 (06-04-08){{
    if(m_uiDependencyId != 0 && m_uiNonRequiredWrite != 0)
    {
      if( pcSliceHeader->getNoInterLayerPredFlag() || m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId() > 1 || pcSliceHeader->getRefLayerQualityId() != 3 )
      {
        rcAccessUnitData.CreatNonRequiredSei();
      }
      xSetNonRequiredSEI(pcSliceHeader, rcAccessUnitData.getNonRequiredSei());
      if(m_uiNonRequiredWrite == 2 && rcAccessUnitData.getNonRequiredSei() != NULL)
      {
        xWriteNonRequiredSEI(rcOutputList, rcAccessUnitData.getNonRequiredSei(), uiBits);
      }
    }
    //NonRequired JVT-Q066 (06-04-08)}}

    // JVT-V068 HRD {
    if ( pcSliceHeader->getIdrFlag() && m_bEnableHrd )
    {
      SEI::BufferingPeriod *pcBufferingPeriodSei = NULL;
      for (UInt uiLayer = 0; uiLayer < m_pcSPS->getVUI()->getNumLayers(); uiLayer++)
      {
        UInt uiDependencyId = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getDependencyID();
        UInt uiTemporalLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getTemporalId();
        UInt uiQualityLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getQualityId();
        UInt uiIndex = (uiDependencyId<<7)+(uiTemporalLevel<<4)+uiQualityLevel;
        RNOK( m_apcScheduler->get(uiIndex)->createBufferingSei( pcBufferingPeriodSei, m_pcParameterSetMng, (uiDependencyId<<4)+uiQualityLevel ) );
        if ( uiTemporalLevel == m_pcSPS->getVUI()->getNumTemporalLevels() - 1 && uiDependencyId == 0 && uiQualityLevel == 0 )
        {
          // AVC compatible layer
          SEI::MessageList  cSEIMessageList;
          cSEIMessageList.push_back(pcBufferingPeriodSei);
          RNOK( xWriteSEI(rcOutputList,cSEIMessageList,uiBits) );
        }
        else
        {
          RNOK( xWriteNestingSEIforHrd(rcOutputList, pcBufferingPeriodSei, uiDependencyId, uiQualityLevel, uiTemporalLevel, uiBits) );
        }
      }
      ETRACE_LAYER(0);
    }

    if ( m_bEnableHrd )
    {
      SEI::PicTiming *pcPicTimingSei = NULL;
      for (UInt uiLayer = 0; uiLayer < m_pcSPS->getVUI()->getNumLayers(); uiLayer++)
      {
        UInt uiDependencyId = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getDependencyID();
        UInt uiTemporalLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getTemporalId();
        UInt uiQualityLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getQualityId();
        UInt uiIndex = (uiDependencyId<<7)+(uiTemporalLevel<<4)+uiQualityLevel;
        RNOK( m_apcScheduler->get(uiIndex)->createTimingSei( pcPicTimingSei, m_pcSPS->getVUI(), 0, *pcSliceHeader, bFieldCoded, uiLayer ) );
        if ( uiTemporalLevel == m_pcSPS->getVUI()->getNumTemporalLevels() - 1 && uiDependencyId == 0 && uiQualityLevel == 0 )
        {
          // AVC compatible layer
          SEI::MessageList  cSEIMessageList;
          cSEIMessageList.push_back(pcPicTimingSei);
          RNOK( xWriteSEI(rcOutputList,cSEIMessageList,uiBits) );
        }
        else
        {
          RNOK( xWriteNestingSEIforHrd(rcOutputList, pcPicTimingSei, uiDependencyId, uiQualityLevel, uiTemporalLevel, uiBits ) );
        }
      }
      ETRACE_LAYER(0);
    }
    // JVT-V068 HRD }
    // JVT-W049 {
    if((pcSliceHeader->getPPS().getEnableRedundantKeyPicCntPresentFlag())&&(pcSliceHeader->getDependencyId()==0))
    {
      RNOK(xWriteRedundantKeyPicSEI ( rcOutputList, uiBits ));
    }
    // JVT-W049 }

    //===== base layer encoding =====
    // JVT-Q054 Red. Picture {
    //===== primary picture coding =====
    if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
    {
      pcSliceHeader->setRedundantPicCnt( 0 );	// set redundant_pic_cnt to 0 for primary coded picture
      if( rcControlData.getBaseLayerSbb() )
  			m_apcFrameTemp[9]->copy((rcControlData.getBaseLayerSbb()),ePicType);//RPIC bug fix
    }
    // JVT-Q054 Red. Picture }
    RNOK( pcBLRecFrame->copy      ( pcFrame, ePicType ) );

    // JVT-R057 LA-RDO{ 
    if(m_bLARDOEnable)
    {
      pcFrame->initChannelDistortion();
      m_pcLowPassBaseReconstruction->initChannelDistortion();
      if( uiFrame == 0 && m_uiGOPNumber==0 )
      {
        pcFrame->zeroChannelDistortion();
        m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(NULL);
      }
      else
      {
        m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_pcLowPassBaseReconstruction);
      }
      pcBLRecFrame->setChannelDistortion(pcFrame);

    }
    // JVT-R057 LA-RDO}

    if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits;
    }
    else
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits;
    }

    RNOK( xEncodeLowPassSignal    ( rcOutputList,
                                  rcControlData,
                                  pcFrame,
                                  pcBLRecFrame,
                                  pcResidual,
                                  pcPredSignal,
                                  uiBits,
                                  rcPicOutputDataList,
                                  ePicType ) );
    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcQuadraticRC->rc_update_pict_frame( uiBits );
      pcQuadraticRC->rc_update_pict( uiBits );
      /* update quadratic R-D model */
      if ( (pcGenericRC->m_pcJSVMParams->type == P_SLICE || 
        (pcGenericRC->m_pcJSVMParams->RCUpdateMode == RC_MODE_1 && pcGenericRC->m_pcJSVMParams->current_frame_number) ) 
        && pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag )
        pcQuadraticRC->updateRCModel();
      else if ( (pcGenericRC->m_pcJSVMParams->type == P_SLICE || 
        (pcGenericRC->m_pcJSVMParams->RCUpdateMode == RC_MODE_1 && pcGenericRC->m_pcJSVMParams->current_frame_number) ) 
        && !(pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag) 
        && pcGenericRC->m_iNoGranularFieldRC == 0 )
        pcQuadraticRC->updateRCModel();
    }
    // JVT-W043 }
    // JVT-Q054 Red. Picture {
    if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
    {
      //RPIC bug fix {
			MbDataCtrl* pcMbDataCtrlTemp=m_pacControlData[uiFrameIdInGOP].getMbDataCtrl();
			m_pcRedundantCtrl->reset();
      m_pacControlData[uiFrameIdInGOP].setMbDataCtrl(m_pcRedundantCtrl);
			MbDataCtrl* pcBaseLayerCtrlTemp=m_pacControlData[uiFrameIdInGOP].getBaseLayerCtrl();
			m_pcRedundant1Ctrl->reset();
      m_pacControlData[uiFrameIdInGOP].setMbDataCtrl(m_pcRedundant1Ctrl);
			m_apcFrameTemp[6]->copy(pcBLRecFrame,ePicType);
			m_apcFrameTemp[7]->copy(pcResidual,ePicType);
			m_apcFrameTemp[8]->copy(pcPredSignal,ePicType); 
      if( rcControlData.getBaseLayerSbb() )
      {
        m_apcFrameTemp[10]->copy((rcControlData.getBaseLayerSbb()),ePicType);
        rcControlData.getBaseLayerSbb()->copy(m_apcFrameTemp[9],ePicType);
      }
      //RPIC bug fix }
      // in current version, slice repetition is supported for each primary coded slice
      UInt  uiRedundantPicNum = 1;  // number of redundant pictures for each primary coded picture
      UInt  uiRedundantPicCnt = 0;
      for ( uiRedundantPicCnt = 1; uiRedundantPicCnt <= uiRedundantPicNum; uiRedundantPicCnt++)
      {
        pcSliceHeader->setRedundantPicCnt( uiRedundantPicCnt );
  
        RNOK( pcBLRecFrame->copy      ( pcFrame,ePicType ) );
        RNOK( xEncodeLowPassSignal    ( rcRedOutputList,
                                         rcControlData,
                                         pcFrame,
                                         pcBLRecFrame,
                                         pcResidual,
                                         pcPredSignal,
                                         uiBits,
                                        rcPicOutputDataList,
                                         ePicType ) );
      }
      pcSliceHeader->setRedundantPicCnt( 0 );
      //RPIC bug fix {
      m_pacControlData[uiFrameIdInGOP].setMbDataCtrl(pcMbDataCtrlTemp);
			m_pacControlData[uiFrameIdInGOP].setBaseLayerCtrl(pcBaseLayerCtrlTemp);
			pcBLRecFrame->copy(m_apcFrameTemp[6],ePicType);
			pcResidual->copy(m_apcFrameTemp[7],ePicType);
			pcPredSignal->copy(m_apcFrameTemp[8],ePicType); 
      if( rcControlData.getBaseLayerSbb() )
  			rcControlData.getBaseLayerSbb()->copy(m_apcFrameTemp[10],ePicType);
      //RPIC bug fix }
    }
    // JVT-Q054 Red. Picture }


    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(NULL);
      pcBLRecFrame->setChannelDistortion(NULL);
    }
    // JVT-R057 LA-RDO}
  
    m_uiNewlyCodedBits += uiBits;
    m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] ++;
    if( ePicType==BOT_FIELD ) // counted twice
    m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] --;
    
    m_dFrameBits += uiBits;//JVT-W051 

    if( ! pcSliceHeader->getNoInterLayerPredFlag() )
    {
      m_pcSliceEncoder->updateBaseLayerResidual( rcControlData, m_uiFrameWidthInMb );
    }

    //===== deblock and store picture for prediction of following low-pass frames =====
    //ROF( pcSliceHeader->getNumRefIdxActive( LIST_0 ) == ( pcSliceHeader->isIntraSlice() ? 0 : 1 ) );
    ROF( pcSliceHeader->getNumRefIdxActive( LIST_1 ) == 0 );
  
    //----- store for inter-layer prediction (non-deblocked version) -----
    RNOK( m_papcSubband[ uiFrameIdInGOP ]->copy( pcBLRecFrame, ePicType ) );
  
    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_pcLowPassBaseReconstruction->copyChannelDistortion(pcFrame);
      m_papcSubband[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
    }
    // JVT-R057 LA-RDO}
      
    //----- de-blocking -----
    RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcBLRecFrame, pcResidual, pcMbDataCtrl, 0, rcControlData.getSpatialScalability() ) );  

    //----- store for prediction of following low-pass pictures -----
    ROF( pcSliceHeader->getNalRefIdc() );
    RNOK( m_pcLowPassBaseReconstruction->copy( pcBLRecFrame, ePicType ) );
    m_pcLowPassBaseReconstruction->setPoc( *pcSliceHeader );
    m_pcLowPassBaseReconstruction->setFrameNum(pcSliceHeader->getFrameNum());  //JVT-S036 lsj
    m_pcLowPassBaseReconstruction->copyPicParameters( *m_papcFrame[uiFrameIdInGOP] );

    // at least the same as the base layer
    RNOK( rcControlData.saveMbDataQpAndCbp() );

    RNOK( pcFrame->copy( pcBLRecFrame, ePicType ) );

    if( m_papcCLRecFrame )
    {
      RNOK( m_papcCLRecFrame[ uiFrameIdInGOP ]->copy( pcFrame, ePicType ) );
      // JVT-R057 LA-RDO{
      if(m_bLARDOEnable&&m_papcCLRecFrame)
         m_papcCLRecFrame[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
      // JVT-R057 LA-RDO}
    }
  }

  RNOK( xUpdateLowPassRec() );

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xUpdateLowPassRec()
{
  ROFRS( m_bMGS && m_bSameResBL,  Err::m_nOK );
  ROFRS( m_uiEncodeKeyPictures,   Err::m_nOK );

  Frame* pcLPRec  = m_pcH264AVCEncoder->getLowPassRec( m_uiDependencyId );
  ROF  (    pcLPRec );

  Bool bUU4R = m_pcLowPassBaseReconstruction->getUnusedForRef();
  RNOK( m_pcLowPassBaseReconstruction->copy( pcLPRec, FRAME ) );
  m_pcLowPassBaseReconstruction->setUnusedForRef( bUU4R );

  RNOK( xFillAndExtendFrame( m_pcLowPassBaseReconstruction, FRAME, m_pcSPS->getFrameMbsOnlyFlag() ) );

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xEncodeNonKeyPicture( UInt               uiBaseLevel,
                                    UInt               uiFrame,
                                    AccessUnitData&    rcAccessUnitData,
                                    PicOutputDataList& rcPicOutputDataList,
                                    PicType            ePicType )
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  UInt          uiBitsRes       = 0;

  Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  Frame*     pcOrgFrame      = m_papcOrgFrame [uiFrameIdInGOP];
  Frame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
  Frame*     pcPredSignal    = m_apcFrameTemp  [0];
  Frame*     pcBLRecFrame    = m_apcFrameTemp  [1];
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
 
  const Bool bFieldCoded    =  m_pbFieldPicFlag[ uiFrameIdInGOP ] ;
  m_abCoded[uiFrameIdInGOP] = true;
  UInt uiBits          = 0;

  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP; 
    pcJSVMParams->CurrGopLevel = pcGenericRC->getCurrGopLevel( pcGenericRC->m_pcJSVMParams->current_frame_number );
    pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;    
    pcGenericRC->m_pcJSVMParams->type = 
        ( uiFrame > 0 && (uiFrame % pcJSVMParams->m_uiIntraPeriod) != 0 ) ? 
        ( ( pcGenericRC->m_pcJSVMParams->current_frame_number % m_uiGOPSize == 0 ) ? P_SLICE : B_SLICE ) : I_SLICE;
    pcGenericRC->m_pcJSVMParams->nal_reference_idc = (pcJSVMParams->CurrGopLevel == 0) ? 0 : 1;
    pcGenericRC->m_pcJSVMParams->number = pcGenericRC->m_pcJSVMParams->current_frame_number;

    pcQuadraticRC->rc_init_pict(1, 0, 1, 1.0F);
    pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
  }
  // JVT-W043 }
  SliceHeader*  pcSliceHeader          = rcControlData.getSliceHeader( ePicType );
  //JVT-W047
  m_bOutputFlag = pcSliceHeader->getOutputFlag();
  //JVT-W047
  ROF( pcSliceHeader );

  ExtBinDataAccessorList& rcOutputList    = rcAccessUnitData.getNalUnitList();
  ExtBinDataAccessorList& rcRedOutputList = rcAccessUnitData.getRedNalUnitList();

  RNOK( xInitControlDataHighPass( uiFrameIdInGOP,uiBaseLevel,uiFrame, ePicType  ) );

  //NonRequired JVT-Q066 (06-04-08){{
  if(m_uiDependencyId != 0 && m_uiNonRequiredWrite != 0)
  {
      if( pcSliceHeader->getNoInterLayerPredFlag() || m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId() > 1 || pcSliceHeader->getRefLayerQualityId() != 3 )
      {
        rcAccessUnitData.CreatNonRequiredSei();
      }
      xSetNonRequiredSEI(pcSliceHeader, rcAccessUnitData.getNonRequiredSei()); 
      if(m_uiNonRequiredWrite == 2 && rcAccessUnitData.getNonRequiredSei() != NULL)
      {
        xWriteNonRequiredSEI(rcOutputList, rcAccessUnitData.getNonRequiredSei(), uiBits);
      }
  }
    //NonRequired JVT-Q066 (06-04-08)}}

  // JVT-V068 HRD {
    if ( m_bEnableHrd )
    {
      SEI::PicTiming *pcPicTimingSei = NULL;
      for (UInt uiLayer = 0; uiLayer < m_pcSPS->getVUI()->getNumLayers(); uiLayer++)
      {
        UInt uiTemporalLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getTemporalId();
        UInt uiDependencyId = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getDependencyID();
        UInt uiQualityLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getQualityId();
        UInt uiIndex = (uiDependencyId<<7)+(uiTemporalLevel<<4)+uiQualityLevel;
        if (uiTemporalLevel >= m_pcSPS->getVUI()->getNumTemporalLevels() - uiBaseLevel - 1 )
        {
          RNOK( m_apcScheduler->get(uiIndex)->createTimingSei( pcPicTimingSei, m_pcSPS->getVUI(), 0, *pcSliceHeader, bFieldCoded, uiLayer ) );
          if ( uiTemporalLevel == m_pcSPS->getVUI()->getNumTemporalLevels() - 1 && uiDependencyId == 0 && uiQualityLevel == 0 )
          {
            // AVC compatible layer
            SEI::MessageList  cSEIMessageList;
            cSEIMessageList.push_back(pcPicTimingSei);
            RNOK( xWriteSEI(rcOutputList,cSEIMessageList,uiBits) );
          }
          else
          {
            RNOK( xWriteNestingSEIforHrd(rcOutputList, pcPicTimingSei, uiDependencyId, uiQualityLevel, uiTemporalLevel, uiBits ) );
          }
          RNOK( addParameterSetBits ( uiBits ) );
        }
      }
      ETRACE_LAYER(0);
    }
  //JVT-V068 HRD }

    //===== base layer encoding =====
    // JVT-Q054 Red. Picture {
    if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
    {
      pcSliceHeader->setRedundantPicCnt( 0 );	// set redundant_pic_cnt to 0 for primary coded picture
    }

    if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits;
    }
    else
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits;
    }

    // JVT-Q054 Red. Picture }
    {
      RNOK( pcBLRecFrame->copy      ( pcFrame,   ePicType  ) );
   
      //JVT-R057 LA-RDO{
      //Bug_Fix JVT-R057 0806{
      if(m_bLARDOEnable)
      {
        pcBLRecFrame->setChannelDistortion(pcFrame);
        m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
      }
      //Bug_Fix JVT-R057 0806}
      //JVT-R057 LA-RDO}
      RNOK( xEncodeHighPassSignal   ( rcOutputList,
                                      rcControlData,
                                      pcOrgFrame, 
                                      pcBLRecFrame,
                                      pcResidual,
                                      pcPredSignal,
                                      uiBits, 
                                      uiBitsRes, rcPicOutputDataList,
                                      ePicType ) );
    }
    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcQuadraticRC->rc_update_pict_frame( uiBits );
      pcQuadraticRC->rc_update_pict( uiBits );
      if ( pcGenericRC->m_pcJSVMParams->type == P_SLICE && pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag )
        pcQuadraticRC->updateRCModel();
      else if ( pcGenericRC->m_pcJSVMParams->type == P_SLICE && !(pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag)
        && pcGenericRC->m_iNoGranularFieldRC == 0 )
        pcQuadraticRC->updateRCModel();
    }
    // JVT-W043 }
    //JVT-Q054 Red. Picture {
    //JVT-W049 {
    if ( (pcSliceHeader->getPPS().getRedundantPicCntPresentFlag())&& (!pcSliceHeader->getPPS().getRedundantKeyPicCntPresentFlag()))
    //if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
    //JVT-W049 }
    {
      // currently only slice repetition is supported for each primary coded slice
      UInt  uiRedundantPicNum = 1;  // number of redundant picture for each primary coded picture
      UInt  uiRedundantPicCnt = 0;
      for ( uiRedundantPicCnt = 1; uiRedundantPicCnt <= uiRedundantPicNum; uiRedundantPicCnt++)
      {
          pcSliceHeader->setRedundantPicCnt( uiRedundantPicCnt );
       
          RNOK( pcBLRecFrame->copy      ( pcFrame,      ePicType  ) );
         
          //JVT-R057 LA-RDO{
          if(m_bLARDOEnable)
          {
            pcBLRecFrame->setChannelDistortion(pcFrame);
            m_pcSliceEncoder->getMbEncoder()->setFrameEcEp( m_papcFrame[(uiFrame-1)<<uiBaseLevel] );
          }
          //JVT-R057 LA-RDO}
          RNOK( xEncodeHighPassSignal   ( rcRedOutputList,
                                          rcControlData,
                                          pcOrgFrame, 
                                          pcBLRecFrame,
                                          pcResidual,
                                          pcPredSignal,
                                          uiBits, 
                                          uiBitsRes, rcPicOutputDataList,
                                          ePicType  ) );
        
      }
      pcSliceHeader->setRedundantPicCnt( 0 );
    }
    // JVT-Q054 Red. Picture }

    m_uiNewlyCodedBits += uiBits;
    m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] ++;
    if( ePicType==BOT_FIELD ) // counted twice
      m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] --;

    m_dFrameBits += uiBits;//JVT-W051 

    // at least the same as the base layer
    RNOK( rcControlData.saveMbDataQpAndCbp() );

    RNOK( pcFrame->copy( pcBLRecFrame, ePicType ) );

    RNOK( m_papcSubband[ uiFrameIdInGOP ]->copy( pcFrame, ePicType ) );


    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_papcSubband[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
    }
    // JVT-R057 LA-RDO}

    if( m_papcCLRecFrame )
    {
      RNOK( m_papcCLRecFrame[ uiFrameIdInGOP ]->copy( pcFrame, ePicType ) );
      // JVT-R057 LA-RDO{
      if(m_bLARDOEnable)
        m_papcCLRecFrame[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
      // JVT-R057 LA-RDO}
    }
 
  return Err::m_nOK;
}



ErrVal
LayerEncoder::xOutputPicData( PicOutputDataList& rcPicOutputDataList )
{
  while( rcPicOutputDataList.size() )
  {
    PicOutputData cPicOutputData = rcPicOutputDataList.popFront();
    if( cPicOutputData.YPSNR )
    {
      printf("%2s %5d: %2s   T%1d L%1d Q%-2d  QP%3d   Y%8.4lf  U%8.4lf  V%8.4lf  %8d bit\n",
        cPicOutputData.FirstPicInAU ? "AU" : "  ",
        cPicOutputData.Poc,
        cPicOutputData.FrameType,
        cPicOutputData.TemporalId,
        cPicOutputData.DependencyId,
        cPicOutputData.QualityId,
        cPicOutputData.Qp,
        cPicOutputData.YPSNR,
        cPicOutputData.UPSNR,
        cPicOutputData.VPSNR,
        cPicOutputData.Bits );
    }
    else
    {
      printf("%2s %5d: %2s   T%1d L%1d Q%-2d  QP%3d                                    %8d bit\n",
        cPicOutputData.FirstPicInAU ? "AU" : "  ",
        cPicOutputData.Poc,
        cPicOutputData.FrameType,
        cPicOutputData.TemporalId,
        cPicOutputData.DependencyId,
        cPicOutputData.QualityId,
        cPicOutputData.Qp,
        cPicOutputData.Bits );
    }
  }
  return Err::m_nOK;
}



ErrVal
LayerEncoder::xStoreReconstruction( PicBufferList&  rcPicBufferOutputList )
{
  PicBufferList::iterator cOutputIter = rcPicBufferOutputList.begin();
  for( UInt uiIndex = (m_bFirstGOPCoded?1:0); uiIndex <= (m_uiGOPSize >> m_uiNotCodedStages    ); uiIndex++, cOutputIter++ )
  {
    PicBuffer*  pcPicBuffer = *cOutputIter;
    if( m_papcCLRecFrame )
    {
      RNOK( m_papcCLRecFrame[uiIndex<<m_uiNotCodedStages    ]->store( pcPicBuffer ) );
    }
    else
    {
      RNOK( m_papcFrame[uiIndex<<m_uiNotCodedStages    ]->store( pcPicBuffer ) );
    }
  }
  return Err::m_nOK;
}




ErrVal
LayerEncoder::initGOP( AccessUnitData& rcAccessUnitData,
                       PicBufferList&  rcPicBufferInputList )
{
  ROT ( m_bGOPInitialized );
  RNOK( xInitGOP              ( rcPicBufferInputList ) );
  RNOK( xSetScalingFactors    () );
  RNOK( xClearELPics          () );
  ::memset( m_abCoded, 0x00, sizeof( m_abCoded ) );
  if  ( m_bFirstGOPCoded )
  {
    //==== copy picture zero =====
    PicOutputDataList cPicOutputDataList;
    Bool              bPicCoded = false;
    RNOK( xEncodeKeyPicture ( bPicCoded, 0, rcAccessUnitData, cPicOutputDataList ) );
    ROT ( bPicCoded );
  }
  m_bGOPInitialized = true;
  return Err::m_nOK;
}



//JVT-X046 {
ErrVal
LayerEncoder::xEncodeHighPassSignalSlices         ( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
													  RefFrameList*    pcRefFrameList0,
													  RefFrameList*    pcRefFrameList1,
													  Frame*        pcOrigFrame,
													  Frame*        pcIntraRecFrame,
													  Frame*        pcMCFrame,
													  Frame*        pcResidual,
													  Frame*        pcPredSignal,
													  ControlData&     rcControlData,
													  Bool             bBiPredOnly,
													  UInt             uiNumMaxIter,
													  UInt             uiIterSearchRange,
													  UInt             uiFrameIdInGOP,
													  PicType          ePicType,
													  UInt&            ruiBits,
													  UInt&            ruiBitsRes,
													  PicOutputDataList&       rcPicOutputDataList,
													  Frame*		   pcFrame,
													  Frame*		   pcBLRecFrame,
														UInt           uiBaseLevel )
{
	Bool        bCalcMv         = false;
	Bool        bFaultTolerant  = false;
	MbEncoder*    pcMbEncoder           =  m_pcSliceEncoder->getMbEncoder         ();
	SliceHeader&  rcSliceHeader         = *rcControlData.getSliceHeader           ( ePicType );
	SliceHeader&  rcSH					= rcSliceHeader;
	MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
	Frame*     pcBaseLayerFrame      =  rcControlData.getBaseLayerRec          ();
	Frame*     pcBaseLayerResidual   =  rcControlData.getBaseLayerSbb          ();
	MbDataCtrl*   pcBaseLayerCtrl       =  rcControlData.getBaseLayerCtrl         ();
	Bool          bEstimateBase         =  rcSliceHeader.getNoInterLayerPredFlag() && ! pcBaseLayerCtrl;
	Bool          bEstimateMotion       =  rcSliceHeader.getAdaptiveBaseModeFlag() || bEstimateBase;
	// JVT-S054 (ADD)
	MbDataCtrl*     pcMbDataCtrlL1    = xGetMbDataCtrlL1( rcSliceHeader, uiFrameIdInGOP, pcRefFrameList1 );

	RNOK( pcMbDataCtrl        ->initSlice( rcSH, PRE_PROCESS, false, NULL ) );
	RNOK( m_pcMotionEstimation->initSlice( rcSH              ) );

	RefFrameList* apcRefFrameList0[4] = { NULL, NULL, NULL, NULL };
	RefFrameList* apcRefFrameList1[4] = { NULL, NULL, NULL, NULL };
	const Bool    bMbAff   = rcSH.isMbaffFrame   ();


	UInt  uiBitsSEI   = 0;
	SliceHeader* pcSliceHeader = rcControlData.getSliceHeader( ePicType );
	ROF( pcSliceHeader );

	//----- Subsequence SEI -----
	if( m_bWriteSubSequenceSei &&   m_bH264AVCCompatible )
	{
		RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBitsSEI ) );
	}


	if( bMbAff )
	{
    RefFrameList acRefFrameList0[2];
		RefFrameList acRefFrameList1[2];

		RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
		RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );

		apcRefFrameList0[ TOP_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
		apcRefFrameList0[ BOT_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
		apcRefFrameList1[ TOP_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
		apcRefFrameList1[ BOT_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];
		apcRefFrameList0[     FRAME ] = pcRefFrameList0;
		apcRefFrameList1[     FRAME ] = pcRefFrameList1;
	}
	else
	{
		RNOK( pcMCFrame->addFieldBuffer( ePicType ) );
		apcRefFrameList0[ ePicType ] = pcRefFrameList0;
		apcRefFrameList1[ ePicType ] = pcRefFrameList1;
	}


	if( ! bEstimateMotion )
	{
		ROF ( pcBaseLayerCtrl )
		FMO* pcFMO = rcControlData.getSliceHeader()->getFMO();
		rcControlData.m_uiCurrentFirstMB=pcFMO->getFirstMacroblockInSlice(0);
		rcControlData.m_bSliceGroupAllCoded=false;
		for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
		{
			rcControlData.m_uiCurrentFirstMB = pcFMO->getFirstMacroblockInSlice(iSliceGroupID);
			while(!rcControlData.m_bSliceGroupAllCoded)
			{
				if (!m_uiSliceMode)
{
					rcControlData.m_bSliceGroupAllCoded = true;
				}
				UInt  muiBits      = 0;
				UInt  muiBitsRes   = 0;
				UInt  muiMbCoded   = 0;
				UInt  uiBits       = 0;

				rcSliceHeader.setFirstMbInSlice(rcControlData.m_uiCurrentFirstMB);
				rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
				// JVT-S054 (2) (ADD)
				rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
				UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
				UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
				UInt uiNumMBInSlice;



				rcControlData.getSliceHeader()->getDecRefPicMarking().setAdaptiveRefPicMarkingModeFlag( false );  //JVT-S036 lsj

				//prefix unit{{
				if( m_uiPreAndSuffixUnitEnable ) 
				{
					if ( pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE|| pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
					{
						RNOK( xWritePrefixUnit( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBits ) );
					}
				}
				//prefix unit}}

				//----- init NAL UNIT -----
				RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
				RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

				//---- write Slice Header -----
				ETRACE_NEWSLICE;
				xAssignSimplePriorityId( pcSliceHeader );
        if( pcSliceHeader->getSliceSkipFlag() )
        {
          UInt uiMaxMbsInSlice  = ( m_uiSliceMode == 1 ? m_uiSliceArgument : MSYS_UINT_MAX );
          UInt uiNumSkipSlices  = min( uiMaxMbsInSlice, pcSliceHeader->getNumMbsInSlice() );
          ROF( uiNumSkipSlices  > 0 );
          pcSliceHeader->setNumMbsInSliceMinus1( uiNumSkipSlices - 1 );
        }

        RNOK( m_pcNalUnitEncoder->write( *pcSliceHeader ) );

				m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode = m_uiSliceMode;
				m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument = m_uiSliceArgument;
				if (m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode==2)
				{
					m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument -= 
						m_pcNalUnitEncoder->xGetBitsWriteBuffer()->getBitsWritten() + 4*8 +20;
				}


				//===== initialization =====
				RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );


				ROF( m_pcSliceEncoder->m_bInitDone );

				//RNOK( pcMbDataCtrl  ->initSlice         ( *pcSliceHeader, PRE_PROCESS, false, NULL ) );
				RNOK( m_pcSliceEncoder->m_pcControlMng->initSliceForCoding( *pcSliceHeader ) );

				//====== initialization ======
				YuvMbBuffer  cZeroBuffer;
				cZeroBuffer.setAllSamplesToZero();
				uiBits = m_pcSliceEncoder->m_pcMbCoder->getBitCount();

				m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone=false;
				//===== loop over macroblocks =====
				//for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
				for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
				{
					ETRACE_NEWMB( uiMbAddress );
					bool bCoded;

					MbDataAccess* pcMbDataAccess      = 0;
					MbDataAccess* pcMbDataAccessBase  = 0;

					UInt          uiMbY, uiMbX;
					Double        dCost              = 0;

					rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress     );

					//===== init macroblock =====
					RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
					if    ( pcBaseLayerCtrl )
					{
						RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
					}		
          
          
					RNOK( pcMbDataCtrl->getMbData(uiMbX, uiMbY).copyMotion( pcBaseLayerCtrl->getMbData(uiMbX, uiMbY), pcMbDataCtrl->getSliceId() ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_0 ).setRefPicIdcs( apcRefFrameList0[ pcMbDataAccess->getMbPicType() ] ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_1 ).setRefPicIdcs( apcRefFrameList1[ pcMbDataAccess->getMbPicType() ] ) );

          // <<<< bug fix by heiko.schwarz@hhi.fhg.de
					if( pcBaseLayerFrame ) // the motion data are not just copied, but inferred from the base layer
					{
						pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_0 ).clear();
						pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_1 ).clear();
					}
					// >>>> bug fix by heiko.schwarz@hhi.fhg.de

					const PicType eMbPicType = pcMbDataAccess->getMbPicType();
					pcMbDataAccess->setMbDataAccessBase(pcMbDataAccessBase);

					pcMCFrame->getFullPelYuvBuffer()->getYuvBufferCtrl().initMb(uiMbY,uiMbX,0);
					
          pcMbEncoder->setBaseLayerRec( pcBaseLayerFrame ? pcBaseLayerFrame->getPic( ePicType ) : NULL );

          RNOK( pcMbEncoder->compensatePrediction( *pcMbDataAccess, 
						pcMCFrame->getPic( eMbPicType ),
						*apcRefFrameList0 [ eMbPicType ],
						*apcRefFrameList1 [ eMbPicType ],
						bCalcMv, bFaultTolerant ) );						

					RNOK( pcOrigFrame->predictionSlices  ( pcOrigFrame,pcMCFrame,  uiMbY, uiMbX) );
						
					RNOK( pcResidual->copyMb(pcOrigFrame, uiMbY, uiMbX) );						
					RNOK( pcBLRecFrame->copyMb(pcOrigFrame,uiMbY,uiMbX) );
					if( pcMbDataAccess->getMbData().isIntra() )
					{
						YuvMbBuffer cZeroMbBuffer;
						cZeroMbBuffer.setAllSamplesToZero();
						pcResidual->getPic( pcMbDataAccess->getMbPicType() )->getFullPelYuvBuffer()->loadBuffer( &cZeroMbBuffer );
					}

					RNOK( m_pcSliceEncoder->m_pcControlMng  ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX, false, false ) );

					if (rcControlData.getBaseLayerCtrl())
					{
						RNOK( rcControlData.getBaseLayerCtrl()->initMb  ( pcMbDataAccessBase, uiMbY, uiMbX ) );
					}
					pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );

					if( pcMbDataAccess->getMbData().isIntra() )
					{
						dCost = 0;
            if( ! pcSliceHeader->getNoInterLayerPredFlag() )
            {
              m_pcSliceEncoder->m_pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[0][uiMbAddress] );
            }

            pcMbDataAccess->getMbData().setQp( pcMbDataAccess->getSH().getSliceQp() );

						RNOK( m_pcSliceEncoder->m_pcMbEncoder ->encodeIntra   ( *pcMbDataAccess, 
							pcMbDataAccessBase, 
							pcBLRecFrame                  ->getPic( ePicType ), 
							pcBLRecFrame                  ->getPic( ePicType ), 
							pcResidual               ->getPic( ePicType ), 
							rcControlData.getBaseLayerRec() ? rcControlData.getBaseLayerRec()->getPic( ePicType ) : NULL, 
							pcPredSignal             ->getPic( ePicType ), 
							rcControlData.getLambda(), 
							dCost ) );

						uiNumMBInSlice++;
						muiMbCoded++;
						if (m_uiSliceMode==1)
						{
							if (uiNumMBInSlice >= m_uiSliceArgument)
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder   ->encode        ( *pcMbDataAccess, 
									pcMbDataAccessBase, 
									true, 
									true ) );
								rcControlData.m_uiCurrentFirstMB = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
								rcSliceHeader.setNumMbsInSlice(uiNumMBInSlice);
								if (rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1)
									rcControlData.m_bSliceGroupAllCoded = true;
								break;
							}
							else
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder   ->encode        ( *pcMbDataAccess, 
									pcMbDataAccessBase, 
									(uiMbAddress == uiLastMbAddress ), 
									true ) );
							}
						}
						else if (m_uiSliceMode==2)
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder   ->encode        ( *pcMbDataAccess, 
								pcMbDataAccessBase, 
								(uiMbAddress == uiLastMbAddress ), 
								true ) );
							if ((rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1) && (!m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone))
								rcControlData.m_bSliceGroupAllCoded = true;
							if (m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone)
							{
								rcControlData.m_uiCurrentFirstMB = uiMbAddress;
								MbData* tempMbData;
								tempMbData=pcMbDataCtrl->xGetMbData(rcControlData.m_uiCurrentFirstMB);
								tempMbData->setSliceId( 0 );																	
								break;
							}
						}
						else
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder   ->encode        ( *pcMbDataAccess, 
								pcMbDataAccessBase, 
								(uiMbAddress == uiLastMbAddress ), 
								true ) );
						}					
					}
					else
					{
            pcMbDataAccess->getMbData().setQp( pcMbDataAccess->getSH().getSliceQp() );

						m_pcSliceEncoder->m_pcTransform->setClipMode( false );
						RNOK( m_pcSliceEncoder->m_pcMbEncoder ->encodeResidual  ( *pcMbDataAccess, 
							pcFrame->getPic( ePicType ), 
							pcBLRecFrame   ->getPic( ePicType ), 
							pcResidual->getPic( ePicType ), 
							rcControlData.getBaseLayerSbb() ? rcControlData.getBaseLayerSbb()->getPic( ePicType ) : NULL,
							bCoded, 
							rcControlData.getLambda(), 
							m_iMaxDeltaQp ) );

						if( ! pcMbDataAccess->getSH().getNoInterLayerPredFlag() && ! pcMbDataAccess->getSH().getAdaptiveBaseModeFlag() )
						{
							pcMbDataAccess->getMbData().setBLSkipFlag( true );
						}

						m_pcSliceEncoder->m_pcTransform->setClipMode( true );

						uiNumMBInSlice++;

						if (m_uiSliceMode==1)
						{
							if (uiNumMBInSlice >= m_uiSliceArgument)
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, true, true ) );
								rcControlData.m_uiCurrentFirstMB = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
								rcSliceHeader.setNumMbsInSlice(uiNumMBInSlice);
								if (rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1)
									rcControlData.m_bSliceGroupAllCoded = true;
								break;									
							}
							else
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
							}
						}
						else if (m_uiSliceMode==2)
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
							if ((rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1) && (!m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone))
								rcControlData.m_bSliceGroupAllCoded = true;
							if (m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone)
							{
								rcControlData.m_uiCurrentFirstMB = uiMbAddress;
								MbData* tempMbData;
								tempMbData=pcMbDataCtrl->xGetMbData(rcControlData.m_uiCurrentFirstMB);
								tempMbData->setSliceId( 0 );
								RNOK( pcOrigFrame->inversepredictionSlices  ( pcOrigFrame,pcMCFrame,  uiMbY, uiMbX) );									
								break;
							}
						}
						else
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
						}

						if( bCoded )
						{
							muiMbCoded++;
						}

						RNOK( pcPredSignal->getFullPelYuvBuffer()->loadBuffer( &cZeroBuffer ) );
					}
										
					uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);

					if (uiMbAddress==-1)
					{
						rcControlData.m_bSliceGroupAllCoded=true;
						break;
					}					
				}

				muiBits += m_pcSliceEncoder->m_pcMbCoder->getBitCount() - uiBits;

				//----- close NAL UNIT -----
				UInt auiBits[16];
				RNOK( m_pcNalUnitEncoder->closeAndAppendNalUnits( auiBits, rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor, m_cBinData, NULL, 0, 0 ) );
				muiBits = auiBits[0] + 4*8;

				PicOutputData cPicOutputData;
				cPicOutputData.FirstPicInAU  = rcControlData.getSliceHeader()->getNoInterLayerPredFlag() && rcControlData.getSliceHeader()->getFirstMbInSlice() == 0;
        cPicOutputData.iPicType      = rcControlData.getSliceHeader()->getPicType();
				cPicOutputData.Poc           = rcControlData.getSliceHeader()->getPoc                  ();
				cPicOutputData.FrameType[0]  = rcControlData.getSliceHeader()->getSliceType            () == B_SLICE ? 'B' :
				rcControlData.getSliceHeader()->getSliceType            () == P_SLICE ? 'P' : 'I';
				cPicOutputData.FrameType[1]  = rcControlData.getSliceHeader()->getUseRefBasePicFlag()            ? 'K' : ' ';
				cPicOutputData.FrameType[2]  = '\0';
				cPicOutputData.DependencyId  = rcControlData.getSliceHeader()->getLayerCGSSNR          ();
				cPicOutputData.QualityId     = rcControlData.getSliceHeader()->getQualityLevelCGSSNR   ();
				cPicOutputData.TemporalId    = rcControlData.getSliceHeader()->getTemporalId        ();
				cPicOutputData.Qp            = rcControlData.getSliceHeader()->getSliceQp                ();        
				cPicOutputData.Bits          = muiBits + uiBitsSEI;
				cPicOutputData.YPSNR         = 0.0;
				cPicOutputData.UPSNR         = 0.0;
				cPicOutputData.VPSNR         = 0.0;
				rcPicOutputDataList.push_back( cPicOutputData );

        if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += muiBits+uiBitsSEI;
        }
        else
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += muiBits+uiBitsSEI;
        }

        ruiBits     += muiBits+uiBitsSEI;
				ruiBitsRes  += muiBitsRes;
				uiBitsSEI =0;
			}
		}

    if ( !pcSliceHeader->getNoInterLayerPredFlag()
      && pcSliceHeader->getSCoeffResidualPredFlag() )
    {
      m_pcSliceEncoder->updatePictureResTransform( rcControlData,m_uiFrameWidthInMb);
    }

    if (pcSliceHeader->getTCoeffLevelPredictionFlag())
    {
      //===== update state prior to deblocking
      m_pcSliceEncoder->updatePictureAVCRewrite( rcControlData, m_uiFrameWidthInMb );
    }

		return Err::m_nOK;
	}

	if( ePicType!=FRAME )
  {
		if( pcOrigFrame )         RNOK( pcOrigFrame        ->addFieldBuffer( ePicType ) );
		if( pcIntraRecFrame )     RNOK( pcIntraRecFrame    ->addFieldBuffer( ePicType ) );
		if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->addFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->addFieldBuffer( ePicType ) );
	}


	FMO* pcFMO = rcControlData.getSliceHeader()->getFMO();
	rcControlData.m_uiCurrentFirstMB=pcFMO->getFirstMacroblockInSlice(0);	

	for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
	{
		rcControlData.m_bSliceGroupAllCoded=false;
		rcControlData.m_uiCurrentFirstMB = pcFMO->getFirstMacroblockInSlice(iSliceGroupID);
		while(!rcControlData.m_bSliceGroupAllCoded)
		{
			if (!m_uiSliceMode)
			{
				rcControlData.m_bSliceGroupAllCoded = true;
			}
			UInt  muiBits      = 0;
			UInt  muiBitsRes   = 0;
			UInt  muiMbCoded   = 0;
			UInt  uiBits       = 0;

			rcSliceHeader.setFirstMbInSlice(rcControlData.m_uiCurrentFirstMB);
			rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
			// JVT-S054 (2) (ADD)
			rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
			UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
			UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
			UInt uiNumMBInSlice;



			rcControlData.getSliceHeader()->getDecRefPicMarking().setAdaptiveRefPicMarkingModeFlag( false );  //JVT-S036 lsj

			//prefix unit{{
			if( m_uiPreAndSuffixUnitEnable ) 
			{
				if ( pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE|| pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
				{
					RNOK( xWritePrefixUnit( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBits ) );
				}
			}
			//prefix unit}}

			//----- init NAL UNIT -----
			RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
			RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

			//---- write Slice Header -----
			ETRACE_NEWSLICE;
			xAssignSimplePriorityId( pcSliceHeader );
      if( pcSliceHeader->getSliceSkipFlag() )
      {
        UInt uiMaxMbsInSlice  = ( m_uiSliceMode == 1 ? m_uiSliceArgument : MSYS_UINT_MAX );
        UInt uiNumSkipSlices  = min( uiMaxMbsInSlice, pcSliceHeader->getNumMbsInSlice() );
        ROF( uiNumSkipSlices  > 0 );
        pcSliceHeader->setNumMbsInSliceMinus1( uiNumSkipSlices - 1 );
      }

			RNOK( m_pcNalUnitEncoder->write( *pcSliceHeader ) );

			m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode = m_uiSliceMode;
			m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument = m_uiSliceArgument;
			if (m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode==2)
			{
				m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument -= 
					m_pcNalUnitEncoder->xGetBitsWriteBuffer()->getBitsWritten() + 4*8 +20;
			}

			//===== initialization =====
			RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
			if( ! m_bLoadMotionInfo )
			{
				RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
				RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
				RNOK( m_pcMotionEstimation->getSW()->initSlice( rcSliceHeader ) );
  }


			ROF( m_pcSliceEncoder->m_bInitDone );

			RNOK( m_pcSliceEncoder->m_pcControlMng->initSliceForCoding( *pcSliceHeader ) );

			//====== initialization ======
			YuvMbBuffer  cZeroBuffer;
			cZeroBuffer.setAllSamplesToZero();
			uiBits = m_pcSliceEncoder->m_pcMbCoder->getBitCount();

			m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone=false;
			//===== loop over macroblocks =====
			//for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
			for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
			{
				ETRACE_NEWMB( uiMbAddress );
				bool bCoded;

				MbDataAccess* pcMbDataAccess      = 0;
				MbDataAccess* pcMbDataAccessBase  = 0;

				UInt          uiMbY, uiMbX;
				Double        dCost              = 0;

				rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress     );

				//===== init macroblock =====
				RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
				if    ( pcBaseLayerCtrl )
				{
					RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
				}

				if( ! m_bLoadMotionInfo )
				{
					//===== initialisation =====
					RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
					RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
					RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

          if( ! pcSliceHeader->getNoInterLayerPredFlag() )
          {
            pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[0][uiMbAddress] );
          }

          RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
						pcMbDataAccessBase,
						*pcRefFrameList0,
						*pcRefFrameList1,

						pcBaseLayerFrame    ? pcBaseLayerFrame   ->getPic( ePicType ) : NULL,
						pcBaseLayerResidual ? pcBaseLayerResidual->getPic( ePicType ) : NULL,
						*pcOrigFrame                              ->getPic( ePicType ),
						*pcIntraRecFrame                          ->getPic( ePicType ),

						bBiPredOnly,
						uiNumMaxIter,
						uiIterSearchRange,
						m_bBLSkipEnable, //JVT-Q065 EIDR
						rcControlData.getLambda(),
						dCost,
						true ) );

					const PicType eMbPicType = pcMbDataAccess->getMbPicType();
					pcMbDataAccess->setMbDataAccessBase(pcMbDataAccessBase);

					pcMCFrame->getFullPelYuvBuffer()->getYuvBufferCtrl().initMb(uiMbY,uiMbX,0);
					
          pcMbEncoder->setBaseLayerRec( pcBaseLayerFrame ? pcBaseLayerFrame->getPic( ePicType ) : NULL );

					RNOK( pcMbEncoder->compensatePrediction( *pcMbDataAccess, 
						pcMCFrame->getPic( eMbPicType ),
						*apcRefFrameList0 [ eMbPicType ],
						*apcRefFrameList1 [ eMbPicType ],
						bCalcMv, bFaultTolerant ) );						

					RNOK( pcOrigFrame->predictionSlices  ( pcOrigFrame,pcMCFrame,  uiMbY, uiMbX) );

					// store the new prediction frame that is obtained by invoking the special MC 
					RNOK( pcResidual->copyMb(pcOrigFrame, uiMbY, uiMbX) );						
					RNOK( pcBLRecFrame->copyMb(pcOrigFrame,uiMbY,uiMbX) );
					if( pcMbDataAccess->getMbData().isIntra() )
					{
						YuvMbBuffer cZeroMbBuffer;
						cZeroMbBuffer.setAllSamplesToZero();
						pcResidual->getPic( pcMbDataAccess->getMbPicType() )->getFullPelYuvBuffer()->loadBuffer( &cZeroMbBuffer );
					}

					RNOK( m_pcSliceEncoder->m_pcControlMng  ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX, false, false ) );

					if (rcControlData.getBaseLayerCtrl())
					{
						RNOK( rcControlData.getBaseLayerCtrl()->initMb  ( pcMbDataAccessBase, uiMbY, uiMbX ) );
					}
					pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );

					
					if( pcMbDataAccess->getMbData().isIntra() )
					{
						dCost = 0;
            if( ! pcSliceHeader->getNoInterLayerPredFlag() )
            {
              m_pcSliceEncoder->m_pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[0][uiMbAddress] );
            }

            pcMbDataAccess->getMbData().setQp( pcMbDataAccess->getSH().getSliceQp() );

						RNOK( m_pcSliceEncoder->m_pcMbEncoder ->encodeIntra   ( *pcMbDataAccess, 
							pcMbDataAccessBase, 
							pcBLRecFrame                  ->getPic( ePicType ), 
							pcBLRecFrame                  ->getPic( ePicType ), 
							pcResidual               ->getPic( ePicType ), 
							rcControlData.getBaseLayerRec() ? rcControlData.getBaseLayerRec()->getPic( ePicType ) : NULL, 
							pcPredSignal             ->getPic( ePicType ), 
							rcControlData.getLambda(), 
							dCost ) );

						uiNumMBInSlice++;
						muiMbCoded++;
						if (m_uiSliceMode==1)
						{
							if (uiNumMBInSlice >= m_uiSliceArgument)
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder   ->encode        ( *pcMbDataAccess, 
									pcMbDataAccessBase, 
									true, 
									true ) );
								rcControlData.m_uiCurrentFirstMB = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
								rcSliceHeader.setNumMbsInSlice(uiNumMBInSlice);
								if (rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1)
									rcControlData.m_bSliceGroupAllCoded = true;
								break;
							}
							else
  {
								RNOK( m_pcSliceEncoder->m_pcMbCoder   ->encode        ( *pcMbDataAccess, 
									pcMbDataAccessBase, 
									(uiMbAddress == uiLastMbAddress ), 
									true ) );
							}
						}
						else if (m_uiSliceMode==2)
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder   ->encode        ( *pcMbDataAccess, 
								pcMbDataAccessBase, 
								(uiMbAddress == uiLastMbAddress ), 
								true ) );
							if ((rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1) && (!m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone))
								rcControlData.m_bSliceGroupAllCoded = true;
							if (m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone)
							{
								rcControlData.m_uiCurrentFirstMB = uiMbAddress;
								MbData* tempMbData;
								tempMbData=pcMbDataCtrl->xGetMbData(rcControlData.m_uiCurrentFirstMB);
								tempMbData->setSliceId( 0 );																	
								break;
							}
						}
						else
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder   ->encode        ( *pcMbDataAccess, 
								pcMbDataAccessBase, 
								(uiMbAddress == uiLastMbAddress ), 
								true ) );
						}					
					}
					else
					{
            pcMbDataAccess->getMbData().setQp( pcMbDataAccess->getSH().getSliceQp() );

						m_pcSliceEncoder->m_pcTransform->setClipMode( false );
						RNOK( m_pcSliceEncoder->m_pcMbEncoder ->encodeResidual  ( *pcMbDataAccess, 
							pcFrame->getPic( ePicType ), 
							pcBLRecFrame   ->getPic( ePicType ), 
							pcResidual->getPic( ePicType ), 
							rcControlData.getBaseLayerSbb() ? rcControlData.getBaseLayerSbb()->getPic( ePicType ) : NULL,
							bCoded, 
							rcControlData.getLambda(), 
							m_iMaxDeltaQp ) );

						if( !pcMbDataAccess->getSH().getNoInterLayerPredFlag() && ! pcMbDataAccess->getSH().getAdaptiveBaseModeFlag() && pcMbDataAccessBase->getMbData().getInCropWindowFlag() )
						{
							ROF( pcMbDataAccess->getMbData().getResidualPredFlag() );
							pcMbDataAccess->getMbData().setBLSkipFlag( true );
						}

						m_pcSliceEncoder->m_pcTransform->setClipMode( true );

						uiNumMBInSlice++;

						if (m_uiSliceMode==1)
						{
							if (uiNumMBInSlice >= m_uiSliceArgument)
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, true, true ) );
								rcControlData.m_uiCurrentFirstMB = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
								rcSliceHeader.setNumMbsInSlice(uiNumMBInSlice);
								if (rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1)
									rcControlData.m_bSliceGroupAllCoded = true;
								break;									
							}
							else
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
							}
						}
						else if (m_uiSliceMode==2)
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
							if ((rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1) && (!m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone))
								rcControlData.m_bSliceGroupAllCoded = true;
							if (m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone)
							{
								rcControlData.m_uiCurrentFirstMB = uiMbAddress;
								MbData* tempMbData;
								tempMbData=pcMbDataCtrl->xGetMbData(rcControlData.m_uiCurrentFirstMB);
								tempMbData->setSliceId( 0 );
								RNOK( pcOrigFrame->inversepredictionSlices  ( pcOrigFrame,pcMCFrame,  uiMbY, uiMbX) );									
								break;
							}
						}
						else
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
						}
						if( bCoded )
						{
							muiMbCoded++;
						}

						RNOK( pcPredSignal->getFullPelYuvBuffer()->loadBuffer( &cZeroBuffer ) );
					}
				}
				else
				{
					//===== load prediction data =====
					RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
				}
									
				uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);

				if (uiMbAddress==-1)
				{
					rcControlData.m_bSliceGroupAllCoded=true;
					break;
				}					
			}

			muiBits += m_pcSliceEncoder->m_pcMbCoder->getBitCount() - uiBits;

			//----- close NAL UNIT -----
			UInt auiBits[16];
            RNOK( m_pcNalUnitEncoder->closeAndAppendNalUnits( auiBits, rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor, m_cBinData, NULL, 0, 0 ) );
			muiBits = auiBits[0] + 4*8;

			PicOutputData cPicOutputData;
			cPicOutputData.FirstPicInAU  = rcControlData.getSliceHeader()->getNoInterLayerPredFlag() && rcControlData.getSliceHeader()->getFirstMbInSlice() == 0;
      cPicOutputData.iPicType      = rcControlData.getSliceHeader()->getPicType();
			cPicOutputData.Poc           = rcControlData.getSliceHeader()->getPoc                  ();
			cPicOutputData.FrameType[0]  = rcControlData.getSliceHeader()->getSliceType            () == B_SLICE ? 'B' :
			rcControlData.getSliceHeader()->getSliceType            () == P_SLICE ? 'P' : 'I';
			cPicOutputData.FrameType[1]  = rcControlData.getSliceHeader()->getUseRefBasePicFlag()            ? 'K' : ' ';
			cPicOutputData.FrameType[2]  = '\0';
			cPicOutputData.DependencyId  = rcControlData.getSliceHeader()->getLayerCGSSNR          ();
			cPicOutputData.QualityId     = rcControlData.getSliceHeader()->getQualityLevelCGSSNR   ();
			cPicOutputData.TemporalId    = rcControlData.getSliceHeader()->getTemporalId        ();
			cPicOutputData.Qp            = rcControlData.getSliceHeader()->getSliceQp                ();        
			cPicOutputData.Bits          = muiBits + uiBitsSEI;
			cPicOutputData.YPSNR         = 0.0;
			cPicOutputData.UPSNR         = 0.0;
			cPicOutputData.VPSNR         = 0.0;
			rcPicOutputDataList.push_back( cPicOutputData );

      if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += muiBits+uiBitsSEI;
      }
      else
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += muiBits+uiBitsSEI;
      }

      ruiBits     += muiBits+uiBitsSEI;
			ruiBitsRes  += muiBitsRes;
			uiBitsSEI =0;
		}
	}

	if( ePicType!=FRAME )
	{
		if( pcOrigFrame )         RNOK( pcOrigFrame        ->removeFieldBuffer( ePicType ) );
		if( pcIntraRecFrame )     RNOK( pcIntraRecFrame    ->removeFieldBuffer( ePicType ) );
		if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->removeFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->removeFieldBuffer( ePicType ) );
	}

  if ( !pcSliceHeader->getNoInterLayerPredFlag()
    && pcSliceHeader->getSCoeffResidualPredFlag() )
  {
    m_pcSliceEncoder->updatePictureResTransform( rcControlData,m_uiFrameWidthInMb);
  }

  if (pcSliceHeader->getTCoeffLevelPredictionFlag())
  {
    //===== update state prior to deblocking
    m_pcSliceEncoder->updatePictureAVCRewrite( rcControlData, m_uiFrameWidthInMb );
  }

	ETRACE_NEWFRAME;
	return Err::m_nOK;
}

ErrVal 
LayerEncoder::xEncodeNonKeyPictureSlices( UInt                uiBaseLevel, 
                                          UInt                uiFrame,
                                          AccessUnitData&     rcAccessUnitData,
							                            PicBufferList&			rcPicBufferInputList,
							                            PicOutputDataList&  rcPicOutputDataList )
{
	UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
	UInt          uiBits          = 0;
	UInt          uiBitsRes       = 0;
	ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
	Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
	Frame*     pcIntraRecFrame = m_apcFrameTemp  [0];
	Frame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
	Frame*     pcMCFrame       = m_apcFrameTemp  [1];
	Frame*     pcOrgFrame      = m_papcOrgFrame [uiFrameIdInGOP];
	Frame*     pcPredSignal    = m_apcFrameTemp  [2];
	Frame*     pcBLRecFrame    = m_apcFrameTemp  [3];
	
	const Bool bFieldCoded = m_pbFieldPicFlag[ uiFrameIdInGOP ] ; 

	m_abCoded[uiFrameIdInGOP] = true;

	for( Int iPicType=(bFieldCoded?TOP_FIELD:FRAME); iPicType<=(bFieldCoded?BOT_FIELD:FRAME); iPicType++ )
	{
		const PicType ePicType      = PicType( iPicType );
		SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
		ROF( pcSliceHeader );

		ExtBinDataAccessorList& rcOutputList  = rcAccessUnitData.getNalUnitList ();

		RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
		RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );

		//===== get reference frame lists =====
		if( m_papcCLRecFrame )
		{
			if( ! bFieldCoded )
			{
				RNOK( xGetCLRecPredictionLists( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_MOTION_ESTIMATION, true ) );
			}
			else/*bFieldCoded*/
			{
				AF( );
			}
		}
		else
		{
			if( ! bFieldCoded )
			{
				RNOK( xGetPredictionLists     ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, RLU_MOTION_ESTIMATION, true ) );
			}
			else/*bFieldCoded*/
			{
				AF( );
			}
		}
		//===== set lambda and QP =====
		RNOK( xInitControlDataMotion  ( uiBaseLevel, uiFrame, true, ePicType ) );
    
    {
      //----- set slice header QP (residual QP) ----
      Double  dQP = m_dBaseQPResidual - 6.0 * log10( rcControlData.getScalingFactor() ) / log10( 2.0 );
      if( m_bExplicitQPCascading )
      {
        dQP = m_dBaseQPResidual + m_adDeltaQPTLevel[ m_uiDecompositionStages - uiBaseLevel ];
      }
      Int iQP = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );
      pcSliceHeader->setSliceHeaderQp( iQP );
    }

		//NonRequired JVT-Q066 (06-04-08){{
		if(m_uiDependencyId != 0 && m_uiNonRequiredWrite != 0)
		{
			if( pcSliceHeader->getNoInterLayerPredFlag() || m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId() > 1 || pcSliceHeader->getRefLayerQualityId() != 3 )
			{
				rcAccessUnitData.CreatNonRequiredSei();
			}
			xSetNonRequiredSEI(pcSliceHeader, rcAccessUnitData.getNonRequiredSei()); 
			if(m_uiNonRequiredWrite == 2 && rcAccessUnitData.getNonRequiredSei() != NULL)
			{
				xWriteNonRequiredSEI(rcOutputList, rcAccessUnitData.getNonRequiredSei(), uiBits);
			}
		}
		//NonRequired JVT-Q066 (06-04-08)}}

		//===== base layer encoding =====
		// JVT-Q054 Red. Picture {
		if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
		{
			pcSliceHeader->setRedundantPicCnt( 0 );	// set redundant_pic_cnt to 0 for primary coded picture
		}
		// JVT-Q054 Red. Picture }
		
		//S051{
		m_pcSliceEncoder->setUseBDir(true);
		if(m_bEncSIP)
		{
			if(m_bH264AVCCompatible||!rcControlData.getSliceHeader()->getDirectSpatialMvPredFlag())
			{
				int				pos				  = xGetMbDataCtrlL1Pos( *rcControlData.getSliceHeader( ePicType ), rcRefFrameList1 );
				if(pos!=-1)
				{
					SliceHeader* pcSliceHeaderL1     = m_pacControlData[pos].getSliceHeader  ( ePicType );
					if(xSIPCheck(pcSliceHeaderL1->getPoc()))
						m_pcSliceEncoder->setUseBDir(false);
				}
			}
		}
		//S051}

		//TMM_WP    
		/* el & has correspongding bl frame & bl uses wp then use bl wts */
		if((m_uiDependencyId > 0 && m_bBaseLayerWp &&
			rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX) )
		{
			/* use same wts as bl */
			pcSliceHeader->setBasePredWeightTableFlag(true);
		}
		else
		{
			/* call function to calculate the weights */
			m_pcSliceEncoder->xSetPredWeights( *pcSliceHeader, 
				pcFrame,
				rcRefFrameList0,
				rcRefFrameList1
				//rcRefFrameList0.getEntry(0),
				//rcRefFrameList1.getEntry(0),
				);
			pcSliceHeader->setBasePredWeightTableFlag(false);
		}
		//TMM_WP
		// JVT-R057 LA-RDO{ 
		if(m_bLARDOEnable)
		{
			pcFrame->initChannelDistortion();
			m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
		}
		// JVT-R057 LA-RDO}

		//JVT-R057 LA-RDO{
		//Bug_Fix JVT-R057 0806{
		if(m_bLARDOEnable)
		{
			pcBLRecFrame->setChannelDistortion(pcFrame);
			m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
		}
		//Bug_Fix JVT-R057 0806}
		//JVT-R057 LA-RDO}

		RNOK(xEncodeHighPassSignalSlices(rcOutputList,&rcRefFrameList0,&rcRefFrameList1,pcFrame,
				pcIntraRecFrame,pcMCFrame,pcResidual,pcPredSignal,rcControlData,
				m_bBiPredOnly,m_uiNumMaxIter,m_uiIterSearchRange,uiFrameIdInGOP,ePicType,
				uiBits,uiBitsRes,rcPicOutputDataList,pcOrgFrame,pcBLRecFrame,uiBaseLevel) );

		m_uiNewlyCodedBits += uiBits;
		m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] ++;
		if( ePicType==BOT_FIELD ) // counted twice
			m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] --;

		// at least the same as the base layer
		RNOK( rcControlData.saveMbDataQpAndCbp() );
    
		RNOK( pcFrame->copy( pcBLRecFrame, ePicType ) );
		RNOK( m_papcSubband[ uiFrameIdInGOP ]->copy( pcFrame, ePicType ) );
		// JVT-R057 LA-RDO{
		if(m_bLARDOEnable)
			m_papcSubband[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
		// JVT-R057 LA-RDO}
		if( m_papcCLRecFrame )
		{
			RNOK( m_papcCLRecFrame[ uiFrameIdInGOP ]->copy( pcFrame, ePicType ) );
			// JVT-R057 LA-RDO{
			if(m_bLARDOEnable)
				m_papcCLRecFrame[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
			// JVT-R057 LA-RDO}
		}
	}

	return Err::m_nOK;
}
//JVT-X046 }
ErrVal
LayerEncoder::process( UInt             uiAUIndex,
                       AccessUnitData&  rcAccessUnitData,
                       PicBufferList&   rcPicBufferInputList,
                       PicBufferList&   rcPicBufferOutputList,
                       PicBufferList&   rcPicBufferUnusedList,
                       ParameterSetMng* pcParameterSetMng )
{
  //JVT-W051 {
  m_dFrameBits  = 0.0;
  m_dAvgBitrate = 0.0;
  //JVT-W051 }
  ROTRS ( rcPicBufferInputList.empty(),              Err::m_nOK );
  ROTRS ( rcPicBufferInputList.size () <= uiAUIndex, Err::m_nOK );
  ROF   ( m_bGOPInitialized );
// JVT-V068 HRD {
  m_pcParameterSetMng = pcParameterSetMng;
// JVT-V068 HRD }

  //===== init some parameters =====
  UInt  uiCurrIdx = 0;
  Bool  bPicCoded = false; 
  g_nLayer        = m_uiQualityLevelCGSSNR;
  ETRACE_LAYER(     g_nLayer );
  if( m_bLARDOEnable )
  {
    m_pcSliceEncoder->getMbEncoder()->setLARDOEnable( m_bLARDOEnable );
    m_pcSliceEncoder->getMbEncoder()->setLayerID    ( m_uiDependencyId    );
  }
  // JVT-W043 {
  if ( bRateControlEnable )
  {
    pcJSVMParams->m_uiLayerId = m_uiDependencyId;
  }
  // JVT-W043 }
  RNOK( xInitBitCounts() );

  //===== update higher layer pictures =====
  RNOK( xUpdateELPics() );

	//JVT-X046 {
  m_pcSliceEncoder->m_uiSliceMode = m_uiSliceMode;
  m_pcSliceEncoder->m_uiSliceArgument = m_uiSliceArgument;
  //JVT-X046 }

  //===== encode key pictures =====
  for( UInt uiKeyFrame = ( m_bFirstGOPCoded ? 1 : 0 ); uiKeyFrame <= ( m_uiGOPSize >> m_uiDecompositionStages ) && ! bPicCoded; uiKeyFrame++, uiCurrIdx++ )
  {
    if( uiAUIndex == uiCurrIdx )
    {
      PicOutputDataList cPicOutputDataList;

      RNOK( xEncodeKeyPicture     ( bPicCoded,               uiKeyFrame, rcAccessUnitData,      cPicOutputDataList ) );
      ROF ( bPicCoded );
      RNOK( xCalculateAndAddPSNR  ( m_uiDecompositionStages, uiKeyFrame, rcPicBufferInputList,  cPicOutputDataList ) );
      // JVT-V068 {
      xCalculateTiming(cPicOutputDataList, uiKeyFrame );
      // JVT-V068 }
      RNOK( xOutputPicData        (                                                             cPicOutputDataList ) );
      RNOK( xClearBufferExtensions() );
      if  ( uiKeyFrame == 1 )
      {
        RNOK( m_pcAnchorFrameReconstructed->copyAll( m_papcFrame[ m_uiGOPSize ] ) );
      }
      m_uiLastCodedFrameIdInGOP = uiKeyFrame << m_uiDecompositionStages;
      m_uiLastCodedTemporalId   = 0;
    }
  }


  //===== encode non-key pictures =====
  for( Int iLevel = (Int)m_uiDecompositionStages - 1; iLevel >= (Int)m_uiNotCodedStages     && ! bPicCoded; iLevel-- )
  {
    for( UInt uiFrame = 1; uiFrame <= ( m_uiGOPSize >> iLevel ) && ! bPicCoded; uiFrame += 2, uiCurrIdx++ )
    {
      if( uiAUIndex == uiCurrIdx )
      {
        PicOutputDataList cPicOutputDataList;
        UInt          uiFrameIdInGOP  = uiFrame << iLevel;
        const Bool bFieldCoded = m_pbFieldPicFlag[ uiFrameIdInGOP ] ; 
        for( Int iPicType=(bFieldCoded?TOP_FIELD:FRAME); iPicType<=(bFieldCoded?BOT_FIELD:FRAME); iPicType++ )
        {
          const PicType ePicType = PicType( iPicType );
          RNOK( xSetBaseLayerData( uiFrameIdInGOP, ePicType ) );
					//JVT-X046 {
					if ( m_uiSliceMode )
					{
						RNOK( xEncodeNonKeyPictureSlices( iLevel, uiFrame, rcAccessUnitData, rcPicBufferInputList, cPicOutputDataList  ) );
						RNOK( xCompositionFrame     ( iLevel, uiFrame, rcPicBufferInputList,                     ePicType ) );
					}				
					else
					{
					//JVT-X046 }
          RNOK( xMotionEstimationFrame( iLevel, uiFrame,                                           ePicType ) );
          RNOK( xDecompositionFrame   ( iLevel, uiFrame,                                           ePicType ) );
          RNOK( xEncodeNonKeyPicture  ( iLevel, uiFrame, rcAccessUnitData,     cPicOutputDataList, ePicType ) );
          RNOK( xCompositionFrame     ( iLevel, uiFrame, rcPicBufferInputList,                     ePicType ) );
					}
        }

        RNOK( xCalculateAndAddPSNR  ( iLevel, uiFrame, rcPicBufferInputList, cPicOutputDataList ) );
        // JVT-V068 {
        xCalculateTiming(cPicOutputDataList, uiFrame );
        // JVT-V068 }
        RNOK( xOutputPicData        (                                        cPicOutputDataList ) );
        RNOK( xClearBufferExtensions() );
        bPicCoded = true;
        m_uiLastCodedFrameIdInGOP = uiFrameIdInGOP;
        m_uiLastCodedTemporalId   = m_uiDecompositionStages - (UInt)iLevel;
      }
    }
  }


  //===== finish GOP =====
  if( uiAUIndex == rcPicBufferInputList.size() - 1 )
  {
    m_bGOPInitialized   = false;

    RNOK( xStoreReconstruction( rcPicBufferOutputList ) );
    RNOK( xFinishGOP          ( rcPicBufferInputList,
                                rcPicBufferOutputList,
                                rcPicBufferUnusedList ) );
  }

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xFinishGOP( PicBufferList& rcPicBufferInputList,
                         PicBufferList& rcPicBufferOutputList,
                         PicBufferList& rcPicBufferUnusedList )
{
  UInt  uiLowPassSize = m_uiGOPSize >> m_uiNotCodedStages    ;

  while( rcPicBufferOutputList.size() > uiLowPassSize + ( m_bFirstGOPCoded ? 0 : 1 ) )
  {
    PicBuffer*  pcPicBuffer = rcPicBufferOutputList.popBack();
    rcPicBufferUnusedList.push_back( pcPicBuffer );
  }

  //--- highest FGS layer for closed-loop coding ----
  // move last LP frame to the beginning -> for next GOP
  if( m_papcCLRecFrame && m_uiGOPSize == (1<<m_uiDecompositionStages) ) // full GOP
  {
    Frame*  pcTempFrame         = m_papcCLRecFrame[0];
    m_papcCLRecFrame[0]            = m_papcCLRecFrame[m_uiGOPSize];
    m_papcCLRecFrame[m_uiGOPSize]  = pcTempFrame;
  }

  //===== update parameters =====
  m_uiParameterSetBits  = 0;
  m_bFirstGOPCoded      = true;
  m_uiGOPNumber        ++;

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xCalculateAndAddPSNR( UInt               uiStage,
                                   UInt               uiFrame,
                                   PicBufferList&     rcPicBufferInputList,
                                   PicOutputDataList& rcPicOutputDataList )
{
  //===== initialize buffer control =====
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );

  const YuvBufferCtrl::YuvBufferParameter&  cBufferParam  = m_pcYuvFullPelBufferCtrl->getBufferParameter( FRAME );
  PicBufferList::iterator                   cIter           = rcPicBufferInputList.begin();
  UInt                                      uiFrameIdInGOP  = uiFrame << uiStage;
  PicBuffer*                                pcPicBuffer     = 0;
  Frame*                                 pcFrame         = ( m_papcCLRecFrame ? m_papcCLRecFrame : m_papcFrame )[uiFrameIdInGOP];
  //TMM_INTERLACE{
  const PicType                             ePicType        = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader()->getPicType();
  const Int                                 iPoc            = (ePicType == TOP_FIELD) ? m_pacControlData[ uiFrameIdInGOP ].getSliceHeader(BOT_FIELD)->getPoc() : m_pacControlData[ uiFrameIdInGOP ].getSliceHeader(FRAME)->getPoc();
//TMM_INTERLACE} 
  Double                                    dYPSNR          = 0.0;
  Double                                    dUPSNR          = 0.0;
  Double                                    dVPSNR          = 0.0;


  //===== get correct picture buffer =====
  for( UInt uiIndex = uiFrameIdInGOP - ( m_bFirstGOPCoded ? 1 : 0 ); uiIndex; uiIndex--, cIter++ )
  {
    ROT( cIter == rcPicBufferInputList.end() );    
  }
  pcPicBuffer = *cIter;
  ROF  ( pcPicBuffer );

  //===== calculate PSNR =====
  {
    Pel*  pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbLum();
    XPel* pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbLumAddr();
    Int   iStride   = cBufferParam.getStride();
    Int   iWidth    = cBufferParam.getWidth ();
    Int   iHeight   = cBufferParam.getHeight();
    UInt  uiSSDY    = 0;
    UInt  uiSSDU    = 0;
    UInt  uiSSDV    = 0;
    Int   x, y;


    for( y = 0; y < iHeight; y++ )
    {
      for( x = 0; x < iWidth; x++ )
      {
        Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
        uiSSDY   += iDiff * iDiff;
      }
      pPelOrig += iStride;
      pPelRec  += iStride;
    }

    iHeight >>= 1;
    iWidth  >>= 1;
    iStride >>= 1;
    pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbCb();
    pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbCbAddr();

    for( y = 0; y < iHeight; y++ )
    {
      for( x = 0; x < iWidth; x++ )
      {
        Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
        uiSSDU   += iDiff * iDiff;
      }
      pPelOrig += iStride;
      pPelRec  += iStride;
    }

    pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbCr();
    pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbCrAddr();

    for( y = 0; y < iHeight; y++ )
    {
      for( x = 0; x < iWidth; x++ )
      {
        Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
        uiSSDV   += iDiff * iDiff;
      }
      pPelOrig += iStride;
      pPelRec  += iStride;
    }


    UInt uiMbNumber = cBufferParam.getWidth () * cBufferParam.getHeight() / 16 / 16 ;
    Double fRefValueY = 255.0 * 255.0 * 16.0 * 16.0 * (Double)uiMbNumber;
    Double fRefValueC = fRefValueY / 4.0;
    dYPSNR            = ( uiSSDY ? 10.0 * log10( fRefValueY / (Double)uiSSDY ) : 99.99 );
    dUPSNR            = ( uiSSDU ? 10.0 * log10( fRefValueC / (Double)uiSSDU ) : 99.99 );
    dVPSNR            = ( uiSSDV ? 10.0 * log10( fRefValueC / (Double)uiSSDV ) : 99.99 );
  }

  //===== add PSNR =====
  for( UInt uiLevel = m_uiDecompositionStages - uiStage; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    m_adPSNRSumY[ uiLevel ] += dYPSNR;
    m_adPSNRSumU[ uiLevel ] += dUPSNR;
    m_adPSNRSumV[ uiLevel ] += dVPSNR;
  }

  //===== output PSNR =====
  PicOutputData& rcPicOutputData = rcPicOutputDataList.back();
// TMM
  ROF( rcPicOutputData.Poc == iPoc );
  rcPicOutputData.YPSNR = dYPSNR;
  rcPicOutputData.UPSNR = dUPSNR;
  rcPicOutputData.VPSNR = dVPSNR;

  return Err::m_nOK;
}


ErrVal
LayerEncoder::finish( UInt&    ruiNumCodedFrames,
                     Double&  rdOutputRate,
                     Double   aaadOutputFramerate[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS],
                     Double   aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] )
{
  ROFRS( m_auiNumFramesCoded[0], Err::m_nOK );

  UInt  uiStage;
  UInt  uiMaxStage        = m_uiDecompositionStages - m_uiNotCodedStages    ;
  // Bug fix: yiliang.bao@nokia.com
  // uiMaxStage is unsigned, it has a problem when uiMaxStage == 0,
  // uiMaxStage - 1 will result in a large number
  UInt  uiMinStage        = 0; //bugfix replace

  Char  acResolution[10];
//bugfix delete

  sprintf( acResolution, "%dx%d", 16*m_uiFrameWidthInMb, 16*m_uiFrameHeightInMb );

  //===== set final sum of bits and average PSNR's =====
  for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ )
  {
    if( uiStage  )
    {
      m_auiNumFramesCoded   [uiStage] += m_auiNumFramesCoded        [uiStage-1];
    }
    if( m_auiNumFramesCoded [uiStage] )
    {
      m_adPSNRSumY          [uiStage] /= (Double)m_auiNumFramesCoded[uiStage];
      m_adPSNRSumU          [uiStage] /= (Double)m_auiNumFramesCoded[uiStage];
      m_adPSNRSumV          [uiStage] /= (Double)m_auiNumFramesCoded[uiStage];
    }
  }

  static Double aaadCurrBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];	

  if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() == 1 )
  {
    UInt uiLayerIdCGSSNR = getLayerCGSSNR();
    UInt uiBaseLayerIdCGSSNR = MSYS_UINT_MAX;
    UInt uiBaseQualityLevelCGSSNR = 0;
    if( uiLayerIdCGSSNR ) //may have dependent layer
    for( Int iLayer = (Int)m_uiDependencyId; iLayer >= 0; iLayer-- )
    {
      UInt uiLayer = iLayer;
      if( m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getLayerCGSSNR() == uiLayerIdCGSSNR
        && m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getQualityLevelCGSSNR() == 0 ) 
      {
        uiBaseLayerIdCGSSNR = m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getBaseLayerCGSSNR();
        uiBaseQualityLevelCGSSNR = m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getBaseQualityLevelCGSSNR();
        break;
      }
    }
    for( UInt uiLevel = 0; uiLevel < MAX_TEMP_LEVELS; uiLevel++ )
    {
      Double dBits = 0;
      if( uiLevel == 0 ) //TL = 0
      {
        for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel++ )
        {
          if( uiBaseLayerIdCGSSNR < MAX_LAYERS ) //not base layer
          {
            if( uiQualityLevel ) // D > 0, T = 0, Q != 0
              dBits = aaadCurrBits[uiLayerIdCGSSNR][uiLevel][uiQualityLevel-1];
            else // D > 0, T = 0, Q = 0
            {
              ROT( m_uiBaseQualityLevel >= MAX_QUALITY_LEVELS );
              dBits = aaadCurrBits[uiBaseLayerIdCGSSNR][uiLevel][uiBaseQualityLevelCGSSNR];
            }
          }
          dBits += aaauidSeqBits[uiLayerIdCGSSNR][uiLevel][uiQualityLevel];
          aaadCurrBits[uiLayerIdCGSSNR][uiLevel][uiQualityLevel] = dBits;
        }
      }
      else //TL != 0
      {
        for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel++ )
        {
          dBits = 0;
          if( uiBaseLayerIdCGSSNR < MAX_LAYERS ) //not base layer
          {
            ROT( uiBaseQualityLevelCGSSNR >= MAX_QUALITY_LEVELS );
            dBits = aaadCurrBits[uiBaseLayerIdCGSSNR][uiLevel][uiBaseQualityLevelCGSSNR];
          }
          for( UInt uiTIndex = 0; uiTIndex <= uiLevel; uiTIndex++ )
          {
            for( UInt uiFIndex = 0; uiFIndex <= uiQualityLevel; uiFIndex++ )
            {
              dBits += aaauidSeqBits[uiLayerIdCGSSNR][uiTIndex][uiFIndex];
            }
          }
          aaadCurrBits[uiLayerIdCGSSNR][uiLevel][uiQualityLevel] = dBits;
        }
      } 
    } 
    if( getLayerCGSSNR() == 0 )
    {
      printf( " \n\n\nSUMMARY:\n" );
      printf( "                      " "  bitrate " "   Min-bitr" "   Y-PSNR" "   U-PSNR" "   V-PSNR\n" );
      printf( "                      " " ---------" " ----------" " --------" " --------" " --------\n" );    
    }
  }
  else
  {
  for( UInt uiLevel = 0; uiLevel < MAX_TEMP_LEVELS; uiLevel++ )
  {
    Double dBits = 0;
    if( uiLevel == 0 ) //TL = 0
    {
      for( UInt uiFGS = 0; uiFGS < MAX_QUALITY_LEVELS; uiFGS++ )
      {
        if( m_uiBaseLayerId < MAX_LAYERS ) //not base layer
        {
// BUG_FIX liuhui{
          if( uiFGS ) // D > 0, T = 0, Q != 0
            dBits = aaadCurrBits[m_uiDependencyId][uiLevel][uiFGS-1];
          else // D > 0, T = 0, Q = 0
          {
            ROT( m_uiBaseQualityLevel >= MAX_QUALITY_LEVELS );
            dBits = aaadCurrBits[m_uiBaseLayerId][uiLevel][m_uiBaseQualityLevel];
//bugfix delete
          }
// BUG_FIX liuhui}
        }
//bugfix delete
        dBits += aaauidSeqBits[m_uiDependencyId][uiLevel][uiFGS];
        aaadCurrBits[m_uiDependencyId][uiLevel][uiFGS] = dBits;
      }
    }
    else //TL != 0
    {
      for( UInt uiFGS = 0; uiFGS < MAX_QUALITY_LEVELS; uiFGS++ )
      {
        dBits = 0;
        if( m_uiBaseLayerId < MAX_LAYERS ) //not base layer
        {
          ROT( m_uiBaseQualityLevel >= MAX_QUALITY_LEVELS );
// BUG_FIX liuhui{
          dBits = aaadCurrBits[m_uiBaseLayerId][uiLevel][m_uiBaseQualityLevel];
//bugfix delete
// BUG_FIX liuhui}
        }
        for( UInt uiTIndex = 0; uiTIndex <= uiLevel; uiTIndex++ )
        {
          for( UInt uiFIndex = 0; uiFIndex <= uiFGS; uiFIndex++ )
          {
            dBits += aaauidSeqBits[m_uiDependencyId][uiTIndex][uiFIndex];
          }
        }
        aaadCurrBits[m_uiDependencyId][uiLevel][uiFGS] = dBits;
      }
    } 
  }	

// BUG_FIX liuhui{
  if( m_uiDependencyId == 0 )
  {
    printf( " \n\n\nSUMMARY:\n" );
    printf( "                      " "  bitrate " "   Min-bitr" "   Y-PSNR" "   U-PSNR" "   V-PSNR\n" );
    printf( "                      " " ---------" " ----------" " --------" " --------" " --------\n" );
  }
// BUG_FIX liuhui}
  }

  for( uiStage = uiMinStage; uiStage <= uiMaxStage; uiStage++ )
  {
    Double  dFps    = m_fOutputFrameRate / (Double)( 1 << ( uiMaxStage - uiStage ) );
    Double  dScale  = dFps / (Double)m_auiNumFramesCoded[uiStage];

    Double dBitrate = aaadCurrBits[m_uiDependencyId][uiStage][0] * dScale;

    if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() == 1 )
    {
      UInt uiStart = 0;
      UInt uiStop  = getQualityLevelCGSSNR() + m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( m_uiDependencyId).getNumberOfQualityLevelsCGSSNR();
      for( UInt uiQLL = uiStart; uiQLL < uiStop; uiQLL++ )
      {
        aaadOutputFramerate[getLayerCGSSNR()][uiStage][uiQLL] = dFps;
      }
      dBitrate = aaadCurrBits[getLayerCGSSNR()][uiStage][uiStop-1] * dScale; // using highest quality layer as output
      for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel ++ )
        m_pcH264AVCEncoder->setBitrateRep( getLayerCGSSNR(), uiStage, uiQualityLevel, aaadCurrBits[getLayerCGSSNR()][uiStage][uiQualityLevel]*dScale );
    }
    else
    {
      aaadOutputFramerate[m_uiDependencyId][uiStage][0] = dFps;
      for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel ++ )
      {
        m_pcH264AVCEncoder->setBitrateRep( m_uiDependencyId, uiStage, uiQualityLevel, aaadCurrBits[m_uiDependencyId][uiStage][uiQualityLevel]*dScale );
      }
    }

    //JVT-W051 {
    if ( uiStage == uiMaxStage )
    {
      m_dAvgBitrate = dBitrate;
    }
    //JVT-W051 }
    static Double adMinBitrate[MAX_LAYERS][MAX_TEMP_LEVELS];
    if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() == 1 )
    {
      UInt uiLayerIdCGSSNR = getLayerCGSSNR();
      UInt uiBaseLayerIdCGSSNR = MSYS_UINT_MAX;
      if( uiLayerIdCGSSNR ) //may have dependent layer
      for( Int iLayer = (Int)m_uiDependencyId; iLayer >= 0; iLayer-- )
      {
        UInt uiLayer = iLayer;
        if( m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getLayerCGSSNR() == uiLayerIdCGSSNR
          && m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getQualityLevelCGSSNR() == 0 ) 
        {
          uiBaseLayerIdCGSSNR = m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getBaseLayerCGSSNR();
          break;
        }
      }

      if( uiLayerIdCGSSNR == 0 )
      {
        adMinBitrate[uiLayerIdCGSSNR][uiStage] = aaadCurrBits[uiLayerIdCGSSNR][uiStage][0] * dScale;
      }
      else //D!=0
      {
        if( adMinBitrate[uiBaseLayerIdCGSSNR][uiStage] ) //base layer with the same TL exists
        {
          adMinBitrate[uiLayerIdCGSSNR][uiStage] = adMinBitrate[uiBaseLayerIdCGSSNR][uiStage];
          for(UInt uiTIndex = 0; uiTIndex <= uiStage; uiTIndex++)
          {  
            adMinBitrate[uiLayerIdCGSSNR][uiStage] += aaauidSeqBits[uiLayerIdCGSSNR][uiTIndex][0] * dScale;
          }			  
        }
        else // base layer non-exists
        {
          if(adMinBitrate[uiLayerIdCGSSNR][0] == 0.0) // first time for layer, uiStage = 0
          {
            if( uiBaseLayerIdCGSSNR == 0 && adMinBitrate[uiBaseLayerIdCGSSNR][0] == 0.0 ) //AVC-COMPATIBLE
            {
              UInt uiTL;
              for( uiTL = 0; uiTL < MAX_TEMP_LEVELS; uiTL++) //search minimum base layer bitrate
                if( adMinBitrate[uiBaseLayerIdCGSSNR][uiTL] )
                  break;
              adMinBitrate[uiLayerIdCGSSNR][uiStage] = aaauidSeqBits[uiLayerIdCGSSNR][uiStage][0]*dScale +
                ( adMinBitrate[uiBaseLayerIdCGSSNR][uiTL]*m_auiNumFramesCoded[uiTL]/m_auiNumFramesCoded[uiStage] ) / ( 1 << ( uiTL - uiStage ) );
            }
          }
          else //high layer without corresponding TL in base layer
          {
            adMinBitrate[uiLayerIdCGSSNR][uiStage] = aaauidSeqBits[uiLayerIdCGSSNR][uiStage][0]*dScale + 
              adMinBitrate[uiLayerIdCGSSNR][uiStage-1]*m_auiNumFramesCoded[uiStage-1]/m_auiNumFramesCoded[uiStage]*2;	
          }//if(adMinBitrate[m_uiBaseLayerId][uiStage]) //base layer exist for same TL
        }
      }
      printf( " %9s @ %7.4lf" " %10.4lf" " %10.4lf" " %8.4lf" " %8.4lf" " %8.4lf" "\n",
      acResolution,
      dFps,
      dBitrate / 1000.0,
      adMinBitrate[uiLayerIdCGSSNR][uiStage] / 1000.0,
      m_adPSNRSumY    [uiStage],
      m_adPSNRSumU    [uiStage],
      m_adPSNRSumV    [uiStage] );    
    }
    else 
    {
    if( m_uiDependencyId == 0 )
    {
      adMinBitrate[m_uiDependencyId][uiStage] = aaadCurrBits[m_uiDependencyId][uiStage][0] * dScale;
    }
    else //D!=0
    {
      if( adMinBitrate[m_uiBaseLayerId][uiStage] ) //base layer with the same TL exists
      {
        adMinBitrate[m_uiDependencyId][uiStage] = adMinBitrate[m_uiBaseLayerId][uiStage];
        for(UInt uiTIndex = 0; uiTIndex <= uiStage; uiTIndex++)
        {  
          adMinBitrate[m_uiDependencyId][uiStage] += aaauidSeqBits[m_uiDependencyId][uiTIndex][0] * dScale;
        }			  
      }
      else // base layer non-exists
      {
        if(adMinBitrate[m_uiDependencyId][0] == 0.0) // first time for layer, uiStage = 0
        {
          if( m_uiBaseLayerId == 0 && adMinBitrate[m_uiBaseLayerId][0] == 0.0 ) //AVC-COMPATIBLE
          {
            UInt uiTL;
            for( uiTL = 0; uiTL < MAX_TEMP_LEVELS; uiTL++) //search minimum base layer bitrate
              if( adMinBitrate[m_uiBaseLayerId][uiTL] )
                break;
            adMinBitrate[m_uiDependencyId][uiStage] = aaauidSeqBits[m_uiDependencyId][uiStage][0]*dScale +
              ( adMinBitrate[m_uiBaseLayerId][uiTL]*m_auiNumFramesCoded[uiTL]/m_auiNumFramesCoded[uiStage] ) / ( 1 << ( uiTL - uiStage ) );
          }
        }
        else //high layer without corresponding TL in base layer
        {
          adMinBitrate[m_uiDependencyId][uiStage] = aaauidSeqBits[m_uiDependencyId][uiStage][0]*dScale + 
            adMinBitrate[m_uiDependencyId][uiStage-1]*m_auiNumFramesCoded[uiStage-1]/m_auiNumFramesCoded[uiStage]*2;	
        }//if(adMinBitrate[m_uiBaseLayerId][uiStage]) //base layer exist for same TL
      }
    }

    printf( " %9s @ %7.4lf" " %10.4lf" " %10.4lf" " %8.4lf" " %8.4lf" " %8.4lf" "\n",
      acResolution,
      dFps,
      dBitrate / 1000.0,
      adMinBitrate[m_uiDependencyId][uiStage] / 1000.0,
      m_adPSNRSumY    [uiStage],
      m_adPSNRSumU    [uiStage],
      m_adPSNRSumV    [uiStage] );

    }
  }

  ruiNumCodedFrames = m_auiNumFramesCoded[uiMaxStage];
  rdOutputRate      = m_fOutputFrameRate;

  //S051{
  if(m_uiAnaSIP>0&&m_cOutSIPFileName.length())
  { 
    FILE* file=fopen(m_cOutSIPFileName.c_str(),"wt");
    
    if(file==NULL)
      return Err::m_nOK;

    for(UInt poc=0;poc<m_uiTotalFrame;poc++)
    {
      if(m_auiFrameBits[poc]!=0)
        fprintf(file,"%d ",m_auiFrameBits[poc]);
    }
    fclose(file);
  }
  //S051}
  
  return Err::m_nOK;
}


ErrVal
LayerEncoder::xSetRplrAndMmco( SliceHeader& rcSH )
{
  if( rcSH.getFieldPicFlag() )
  {
    RNOK( xSetRplrAndMmcoFld( rcSH ) );
    return Err::m_nOK;
  }

  // clear L1 RPLR buffer
  rcSH.getRefPicListReordering(LIST_1).clear( false );

  // clear MMCO buffer
  rcSH.getDecRefPicMarking().clear( false );

  const UInt uiCurrFrameNr = rcSH.getFrameNum();

  // leave if IDR
  if( rcSH.getIdrFlag() )
  {
    m_cLPFrameNumList.clear();
    m_cLPFrameNumList.push_front( uiCurrFrameNr );
//EIDR 0619{ 
    rcSH.getRefPicListReordering(LIST_0).clear( false );
//EIDR 0619}
    return Err::m_nOK;
  }

  // generate RPLR commands
  AOT( m_cLPFrameNumList.size() < rcSH.getNumRefIdxActive( LIST_0 ) );
  UIntList            cTempList;
  UIntList::iterator  iter = m_cLPFrameNumList.begin();
  for( UInt n = 0; n < rcSH.getNumRefIdxActive( LIST_0 ); n++ )
  {
    cTempList.push_back( *iter++ );
  }
  RNOK( xSetRplr( rcSH.getRefPicListReordering( LIST_0 ), cTempList, uiCurrFrameNr, FRAME ) );

  // calculate number of MMCO commands
  const UInt  uiMaxFrameNumber  = ( 1 << rcSH.getSPS().getLog2MaxFrameNum() );
  const Int   iDiffA             = m_cLPFrameNumList.front() - uiCurrFrameNr;
  UInt        uiDiffA            = ( uiMaxFrameNumber - iDiffA ) % uiMaxFrameNumber;

  // generate mmco commands for inter b frames
  UInt uiPos = 0;
  while( --uiDiffA )
  {
    rcSH.getDecRefPicMarking().set( uiPos++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiffA-1 ) );
  }

  // generate mmco command for high-pass frame
  UInt uiNeedLowPassBefore = max( 1, rcSH.getNumRefIdxActive( LIST_0 ) );
  if( m_cLPFrameNumList.size() > uiNeedLowPassBefore )
  {
    const Int iDiffB   = m_cLPFrameNumList.popBack() - uiCurrFrameNr;
    UInt      uiDiffB  = ( uiMaxFrameNumber - iDiffB ) % uiMaxFrameNumber;
    rcSH.getDecRefPicMarking().set( uiPos++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiffB-1 ) );
  }

  // end of command list
  if ( uiPos )
  {
  rcSH.getDecRefPicMarking().set( uiPos, MmcoCommand( MMCO_END) );

  rcSH.getDecRefPicMarking().setAdaptiveRefPicMarkingModeFlag( true );
  }
  else
  rcSH.getDecRefPicMarking().setAdaptiveRefPicMarkingModeFlag( false );

  // insert frame_num
  m_cLPFrameNumList.push_front( uiCurrFrameNr );

  return Err::m_nOK;
}

ErrVal        
LayerEncoder::xSetRplrAndMmcoFld( SliceHeader& rcSH )
{
  // clear L1 RPLR buffer
  rcSH.getRefPicListReordering( LIST_1 ).clear( false );

  // clear MMCO buffer
  rcSH.getDecRefPicMarking().clear( false );

  const UInt    uiCurrFrameNr = rcSH.getFrameNum();
  const PicType ePicType      = rcSH.getPicType ();

  // leave if IDR
  if( rcSH.getIdrFlag() )
  {
    m_cLPFrameNumList.clear();
    m_cLPFrameNumList.push_front( uiCurrFrameNr );
//EIDR 0619{ 
    rcSH.getRefPicListReordering(LIST_0).clear( false );
//EIDR 0619}
    return Err::m_nOK;
  }

//TMM
  AOT( m_cLPFrameNumList.size() < rcSH.getNumRefIdxActive( LIST_0 ) );

  UIntList cTmpNumList;
  if( ePicType==BOT_FIELD && rcSH.getNalRefIdc() )
  {
    cTmpNumList.push_back( uiCurrFrameNr*2 );
  }

  UIntList::iterator iter = m_cLPFrameNumList.begin();
  while( iter != m_cLPFrameNumList.end() )
  {
    cTmpNumList.push_back( (*iter)*2+1 );	
    cTmpNumList.push_back( (*iter)*2   );
    *iter++;
  }

  UIntList           cPicNumList;
  UIntList::iterator iter1 = cTmpNumList.begin();
  for( UInt n = 0; n < rcSH.getNumRefIdxActive( LIST_0 ); n++ )
  {
    cPicNumList.push_back( *iter1++ );
  }

  // RPLR 
  RNOK( xSetRplr( rcSH.getRefPicListReordering( LIST_0 ), cPicNumList, uiCurrFrameNr*2+1, ePicType ) );

  // MMCO 
  if( ePicType==TOP_FIELD )
  {
    // calculate number of MMCO commands
    const UInt  uiMaxFrameNumber = ( 1 << rcSH.getSPS().getLog2MaxFrameNum() );
    const Int   iDiffA            = m_cLPFrameNumList.front() - uiCurrFrameNr;
    UInt        uiDiffA           = ( uiMaxFrameNumber - iDiffA ) % uiMaxFrameNumber;

    // generate MMCO commands for inter B_SLICE
    UInt uiPos = 0;
    while( --uiDiffA )
    {
      rcSH.getDecRefPicMarking().set( uiPos++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiffA*2-1 ) );
      rcSH.getDecRefPicMarking().set( uiPos++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiffA*2   ) );
    }

    // generate MMCO command for high-pass frame
    if( m_cLPFrameNumList.size() > 1)
    {
      const Int iDiffB  = m_cLPFrameNumList.popBack() - uiCurrFrameNr;
      UInt      uiDiffB = ( uiMaxFrameNumber - iDiffB ) % uiMaxFrameNumber;
      rcSH.getDecRefPicMarking().set( uiPos++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiffB*2-1 ) );
      rcSH.getDecRefPicMarking().set( uiPos++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiffB*2   ) );
    }

    // end of command list
    rcSH.getDecRefPicMarking().set( uiPos, MmcoCommand( MMCO_END ) );
    rcSH.getDecRefPicMarking().setAdaptiveRefPicMarkingModeFlag( true );
  }
  else
  {
    // insert frame_num
    m_cLPFrameNumList.push_front( uiCurrFrameNr );
  }

  return Err::m_nOK;
}

// TMM {
ErrVal        
LayerEncoder::xSetMmcoFld( SliceHeader& rcSH )
{
  // clear MMCO buffer
  rcSH.getDecRefPicMarking().clear( false );

  const UInt    uiCurrFrameNr = rcSH.getFrameNum();

  ASSERT( ! rcSH.getIdrFlag() );

  if( uiCurrFrameNr - m_cLPFrameNumList.front() == 1 << m_uiDecompositionStages )
  {
    // calculate number of MMCO commands
    const UInt  uiMaxFrameNumber = ( 1 << rcSH.getSPS().getLog2MaxFrameNum() );
    const Int   iDiff            = m_cLPFrameNumList.front() - uiCurrFrameNr;
    UInt        uiDiff           = ( uiMaxFrameNumber - iDiff ) % uiMaxFrameNumber;

    // generate MMCO commands for inter B_SLICE
    UInt uiPos = 0;
    while( --uiDiff )
    {
      rcSH.getDecRefPicMarking().set( uiPos++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiff*2-1 ) );
      rcSH.getDecRefPicMarking().set( uiPos++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiff*2   ) );
    }

    // end of command list
    rcSH.getDecRefPicMarking().set( uiPos, MmcoCommand( MMCO_END ) );
    rcSH.getDecRefPicMarking().setAdaptiveRefPicMarkingModeFlag( true );
  }

  return Err::m_nOK;
}
// TMM  }

ErrVal
LayerEncoder::xSetRplr( RefPicListReOrdering&  rcRplrBuffer,
                       UIntList    cPicNumList,
                       UInt        uiCurrPicNr,
                       PicType     ePicType )
{
  rcRplrBuffer.clear();
  
  if( cPicNumList.empty() )
  {
    rcRplrBuffer.setRefPicListReorderingFlag( false );
    return Err::m_nOK;
  }

  UIntList::iterator  iter            = cPicNumList.begin();
  UInt                uiCurrReorderNr = uiCurrPicNr;
  UInt                uiCount         = 0;
  Int                 iSum            = 0;
  Bool                bNeg            = false;
  Bool                bPos            = false;
  
  for( ; iter != cPicNumList.end(); iter++ )
  {
    Int  iDiff = *iter - uiCurrReorderNr;
    AOF( iDiff );

    if( iDiff < 0 )
    {
      Int iVal  = -iDiff - 1;
      rcRplrBuffer.set( uiCount++, RplrCommand( RPLR_NEG, iVal) );
      bNeg      = true;
      iSum     += iVal;
    }
    else
    {
      Int iVal  =  iDiff - 1;
      rcRplrBuffer.set( uiCount++, RplrCommand( RPLR_POS, iVal) );
      bPos      = true;
      iSum     += iVal;
    }
    uiCurrReorderNr = *iter;
  }
  rcRplrBuffer.set( uiCount++, RplrCommand( RPLR_END ) );
  rcRplrBuffer.setRefPicListReorderingFlag( true );

  if( ePicType==FRAME && iSum == 0 && ( bPos == ! bNeg ) && ! m_bForceReOrderingCommands )
  {
    rcRplrBuffer.clear( false );
  }

  return Err::m_nOK;
}


//JVT-S036 lsj start
ErrVal
LayerEncoder::xSetMmcoBase( SliceHeader& pcSliceHeader, UInt iNum )
{
  SliceHeader& rcSH = pcSliceHeader;
  rcSH.getDecRefBasePicMarking().clear( false );
    
  // leave if idr
    if( rcSH.getIdrFlag() )
  {
    return Err::m_nOK;
  }
  //generate mmco commands
  if( rcSH.getUseRefBasePicFlag() )
  {
    UInt uiPos = 0;
  //bug-fix 11/16/06{{
  Int   iDiff             =  iNum - rcSH.getFrameNum(); 
  const UInt  uiMaxFrameNumber  = ( 1 << rcSH.getSPS().getLog2MaxFrameNum() );
  UInt        uiDiff            = ( uiMaxFrameNumber - iDiff ) % uiMaxFrameNumber;

  rcSH.getDecRefBasePicMarking().set( uiPos++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiff-1 ) );
  //bug-fix 11/16/06}}
    
    rcSH.getDecRefBasePicMarking().set( uiPos, MmcoCommand( MMCO_END) );
    rcSH.getDecRefBasePicMarking().setAdaptiveRefPicMarkingModeFlag( true );

  }
  
  return Err::m_nOK;
}
//JVT-S036 lsj end


ErrVal
LayerEncoder::xWriteSEI( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit )
{
  UInt uiBit = 0;
  Bool m_bWriteSEI = true; 
  if( m_bWriteSEI )
  {
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

    SEI::MessageList cSEIMessageList;
    SEI::SubSeqInfo* pcSubSeqInfo;
    RNOK( SEI::SubSeqInfo::create( pcSubSeqInfo ) );

    const UInt uiSubSeqLayer         = /*m_uiDecompositionStages+1-*/rcSH.getTemporalId();
    const Bool bFirstRefPicFlag      = rcSH.getNalRefIdc() && ! m_abIsRef[uiSubSeqLayer];
    const Bool bLeadingNonRefPicFlag = (0 == rcSH.getNalRefIdc());

    m_abIsRef[uiSubSeqLayer] |= (0 != rcSH.getNalRefIdc());

    cSEIMessageList.push_back( pcSubSeqInfo );
    RNOK( pcSubSeqInfo->init( uiSubSeqLayer, 0, bFirstRefPicFlag, bLeadingNonRefPicFlag ) );

    RNOK( m_pcNalUnitEncoder->write( cSEIMessageList ) );

    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );
    RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
    uiBit += 4*8;
    ruiBit += uiBit;
  }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xWritePrefixUnit( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit )
{
  UInt uiBit = 0;
  Bool m_bWriteSuffixUnit = true; 
  if( m_bWriteSuffixUnit )
  {
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

    NalUnitType eNalUnitType = rcSH.getNalUnitType();
    UInt eLayerId = rcSH.getDependencyId();
      
    rcSH.setDependencyId( 0 );

    if( eNalUnitType == NAL_UNIT_CODED_SLICE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
    {
      rcSH.setNalUnitType( NAL_UNIT_PREFIX );
    }
    else
    {
      return Err::m_nERR;
    }
    ETRACE_HEADER( "PREFIX UNIT" );

    RNOK( m_pcNalUnitEncoder->writePrefix( rcSH ) ); 

    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );
    RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
    uiBit += 4*8;
    ruiBit += uiBit;

    rcSH.setNalUnitType( eNalUnitType );
    rcSH.setDependencyId( eLayerId );
  }

  return Err::m_nOK;
}
//prefix unit}}
//JVT-S036 lsj end

//NonRequired JVT-Q066 (06-04-08){{
ErrVal 
LayerEncoder::xSetNonRequiredSEI(SliceHeader* pcSliceHeader, SEI::NonRequiredSei* pcNonRequiredSei)
{
  if( pcSliceHeader->getNoInterLayerPredFlag() || m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId() > 1 )
  {
    if(pcNonRequiredSei->getNumInfoEntriesMinus1() != MSYS_UINT_MAX)
      pcNonRequiredSei->setNumInfoEntriesMinus1(pcNonRequiredSei->getNumInfoEntriesMinus1()+1);
    else
      pcNonRequiredSei->setNumInfoEntriesMinus1(0);
    pcNonRequiredSei->setEntryDependencyId(pcNonRequiredSei->getNumInfoEntriesMinus1(), m_uiDependencyId);

    UInt temp = 0;
    UInt i = pcNonRequiredSei->getNumInfoEntriesMinus1();
    UInt j = 0;

    if(pcSliceHeader->getNoInterLayerPredFlag())
      temp = m_uiDependencyId + 1;
    else
      temp = m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId();

    while(temp > 1)
    {
      if(pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) != MSYS_UINT_MAX)
        pcNonRequiredSei->setNumNonRequiredPicsMinus1(i,pcNonRequiredSei->getNumNonRequiredPicsMinus1(i)+16);
      else
        pcNonRequiredSei->setNumNonRequiredPicsMinus1(i, 15);

      for(UInt k = 0; j <= pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) ; k++, j++)
      {
        pcNonRequiredSei->setNonNonRequiredPicDependencyId(i, j, m_uiDependencyId + 1 - temp);
        pcNonRequiredSei->setNonNonRequiredPicQulityLevel(i, j, k);
        pcNonRequiredSei->setNonNonRequiredPicFragmentOrder(i, j, 0);
      }
      temp--;
    }
  }
  else if(pcSliceHeader->getRefLayerQualityId() != 15)
  {
    if(pcNonRequiredSei->getNumInfoEntriesMinus1() != MSYS_UINT_MAX)
      pcNonRequiredSei->setNumInfoEntriesMinus1(pcNonRequiredSei->getNumInfoEntriesMinus1()+1);
    else
      pcNonRequiredSei->setNumInfoEntriesMinus1(0);
    pcNonRequiredSei->setEntryDependencyId(pcNonRequiredSei->getNumInfoEntriesMinus1(), m_uiDependencyId);

    UInt i = pcNonRequiredSei->getNumInfoEntriesMinus1();

    pcNonRequiredSei->setNumNonRequiredPicsMinus1(i, 15 - pcSliceHeader->getRefLayerQualityId() - 1);

    for(UInt j = 0; j <= pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) ; j++)
    {
      pcNonRequiredSei->setNonNonRequiredPicDependencyId(i, j, pcSliceHeader->getRefLayerDependencyId());
      pcNonRequiredSei->setNonNonRequiredPicQulityLevel(i, j, pcSliceHeader->getRefLayerQualityId() + j + 1);
      pcNonRequiredSei->setNonNonRequiredPicFragmentOrder(i, j, 0);
    }
  }
  return Err::m_nOK;
}


//JVT-W052
ErrVal
LayerEncoder::xWriteIntegrityCheckSEI(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::SEIMessage *pcSEIMessage, UInt &ruiBits)
{
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );

  SEI::MessageList cSEIMessageList;
  cSEIMessageList.push_back(pcSEIMessage);
  RNOK( m_pcNalUnitEncoder->write(cSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;
  return Err::m_nOK;
}
//JVT-W052

ErrVal
LayerEncoder::xWriteNonRequiredSEI( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SEI::NonRequiredSei* pcNonRequiredSei, UInt& ruiBit )
{
  UInt uiBit = 0;

  RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

  SEI::MessageList cSEIMessageList;
  cSEIMessageList.push_back( pcNonRequiredSei );

  RNOK( m_pcNalUnitEncoder->write( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );

  ROF( &m_cExtBinDataAccessor );
  ROF( m_cExtBinDataAccessor.data() );
  UInt    uiNewSize     = m_cExtBinDataAccessor.size();
  UChar*  pucNewBuffer  = new UChar [ uiNewSize ];
  ROF( pucNewBuffer );
  ::memcpy( pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof( UChar ) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor );
  m_cBinData              .reset          ();
  m_cBinData              .set            (  pucNewBuffer, uiNewSize );
  m_cBinData              .setMemAccessor ( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front     (  pcNewExtBinDataAccessor );
  m_cBinData              .reset          ();
  m_cBinData              .setMemAccessor ( m_cExtBinDataAccessor );

  uiBit += 4*8;
  ruiBit += uiBit;

  return Err::m_nOK;
}


// JVT-V068 HRD {
ErrVal
LayerEncoder::xWriteSEI(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::MessageList& rcSEIMessageList, UInt &ruiBits)
{
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );

  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );
  RNOK( m_pcNalUnitEncoder->write(rcSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xWriteNestingSEIforHrd(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::SEIMessage *pcSEIMessage, UInt uiDependencyId, UInt uiQualityLevel, UInt uiTemporalLevel, UInt &ruiBits)
{
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );

  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );

  SEI::ScalableNestingSei* pcScalableNestingSei;
  RNOK( SEI::ScalableNestingSei::create(pcScalableNestingSei) );

  //===== set message =====
  pcScalableNestingSei->setAllPicturesInAuFlag( 0 );
  pcScalableNestingSei->setNumPictures( 1 );
  pcScalableNestingSei->setDependencyId( 0, uiDependencyId );
  pcScalableNestingSei->setQualityLevel( 0, uiQualityLevel );
  pcScalableNestingSei->setTemporalId( uiTemporalLevel );

  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back( pcScalableNestingSei );  
  cSEIMessageList.push_back( pcSEIMessage );
  RNOK( m_pcNalUnitEncoder->writeScalableNestingSei(cSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xWriteSEIforAVCCompatibleHrd(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::SEIMessage* pcSEIMessage, UInt &ruiBits)
{
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );

  SEI::MessageList cSEIMessageList;
  cSEIMessageList.push_back(pcSEIMessage);
  RNOK( m_pcNalUnitEncoder->write(cSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xCalculateTiming( PicOutputDataList&  rcPicOutputDataList, UInt uiFrame )
{
  if ( !m_bEnableHrd ) return Err::m_nOK;

  UInt uiFrameIdInGOP = uiFrame >> m_uiDecompositionStages;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  PicOutputDataList::iterator iter;
  for (iter = rcPicOutputDataList.begin(); iter != rcPicOutputDataList.end(); iter++)
  {
    UInt uiBaseQualityLevel;
    UInt uiBaseLayerId;
    UInt uiBitCounts = iter->Bits;

    SliceHeader* pcSH = rcControlData.getSliceHeader( PicType( iter->iPicType ) );

    uiBaseQualityLevel = iter->uiBaseQualityLevel;
    uiBaseLayerId = iter->uiBaseLayerId;
    
    if ( iter->DependencyId == uiBaseLayerId )
    {
      uiBaseQualityLevel = iter->QualityId > (Int) uiBaseQualityLevel ? uiBaseQualityLevel : iter->QualityId - 1;
    }
    else if ( uiBaseLayerId!=MSYS_UINT_MAX && uiBaseQualityLevel!=MSYS_UINT_MAX )
    {
      while(!m_apcScheduler->get((uiBaseLayerId<<7)+ ((iter->TemporalId)<<4) + uiBaseQualityLevel) && (uiBaseQualityLevel--) );
    }

    if ( uiBaseLayerId!=MSYS_UINT_MAX && uiBaseQualityLevel!=MSYS_UINT_MAX )
    {
      for (UInt uiTmp = 0; uiTmp <= uiBaseQualityLevel; uiTmp++)
        uiBitCounts += m_apcScheduler->get((uiBaseLayerId<<7)+(iter->TemporalId<<4)+uiTmp)->getLayerBits();
    }

    for (UInt uiTemporalId=iter->TemporalId; uiTemporalId<=m_uiDecompositionStages; uiTemporalId++)
    {
      m_apcScheduler->get(((iter->DependencyId)<<7)+(uiTemporalId<<4)+(iter->QualityId))->setLayerBits(iter->QualityId == 0 ? uiBitCounts : iter->Bits); // if layer's quality_id == 0, store the sum of bits in all reference layers, otherwise, only store the bits of current layer.
      m_apcScheduler->get(((iter->DependencyId)<<7)+(uiTemporalId<<4)+(iter->QualityId))->calculateTiming(uiBitCounts, uiBitCounts, pcSH->getIdrFlag(), pcSH->getFieldPicFlag());
    }
  }
  return Err::m_nOK;
}
// JVT-V068 HRD }

//JVT-W049 {
ErrVal
LayerEncoder::xWriteRedundantKeyPicSEI ( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, UInt &ruiBits )
{
  //===== create message =====
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );
  SEI::RedundantPicSei* pcRedundantPicSEI;
  RNOK( SEI::RedundantPicSei::create( pcRedundantPicSEI ) );
  //===== set message =====
  UInt uiInputLayers  = m_uiNumberLayersCnt;
  UInt uiNumdIdMinus1 = uiInputLayers - 1;
  UInt uiNumqIdMinus1 = 0;//may be changed
  UInt uiRedundantPicCntMinus1 = 0;//may be changed
  pcRedundantPicSEI->setNumDIdMinus1( uiNumdIdMinus1 );
  for( UInt i = 0; i <= uiNumdIdMinus1; i++ ) 
  {
    pcRedundantPicSEI->setDependencyId ( i, i );
    pcRedundantPicSEI->setNumQIdMinus1 ( i, uiNumqIdMinus1 );
    for( UInt j = 0; j <= uiNumqIdMinus1; j++ ) 
    {
      pcRedundantPicSEI->setQualityId ( i, j, j );
      pcRedundantPicSEI->setNumRedundantPicsMinus1 ( i, j, uiRedundantPicCntMinus1 );
      for( UInt k = 0; k <= uiRedundantPicCntMinus1; k++ ) 
      {
        pcRedundantPicSEI->setRedundantPicCntMinus1 ( i, j, k, k );
        Bool bPicMatchFlag = true;//may be changed
        pcRedundantPicSEI->setPicMatchFlag ( i, j, k, bPicMatchFlag );
        if ( bPicMatchFlag == false )
        {
           Bool bMbTypeMatchFlag = true;//may be changed
           Bool bMotionMatchFlag = true;//may be changed
           Bool bResidualMatchFlag = true;//may be changed
           Bool bIntraSamplesMatchFlag = true;//may be changed
           pcRedundantPicSEI->setMbTypeMatchFlag ( i, j, k, bMbTypeMatchFlag ); 
                 pcRedundantPicSEI->setMotionMatchFlag ( i, j, k, bMotionMatchFlag );
                   pcRedundantPicSEI->setResidualMatchFlag ( i, j, k, bResidualMatchFlag );
                   pcRedundantPicSEI->setIntraSamplesMatchFlag ( i, j, k, bIntraSamplesMatchFlag );
                }
      }
    }
  }              

  //===== write message =====
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back( pcRedundantPicSEI );  
  RNOK( m_pcNalUnitEncoder->write(cSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;

  return Err::m_nOK;
}
//JVT-W049 }

ErrVal
LayerEncoder::xGetFrameNumList( SliceHeader& rcSH, UIntList& rcFrameNumList, ListIdx eLstIdx, UInt uiCurrBasePos )
{
  rcFrameNumList.clear();

  const UInt uiLevel   = rcSH.getTemporalId  ();
  const UInt uiMaxSize = rcSH.getNumRefIdxActive( eLstIdx );
  ROF( uiMaxSize );

  //===== list 1 =====
  if( eLstIdx == LIST_1 )
  {
    for( Int i = (Int)uiCurrBasePos+1; i <= (Int)m_uiGOPSize; i++ )
    {
    if( m_pacControlData[i].getSliceHeader()->getTemporalId() < uiLevel && !m_papcFrame[i]->getUnusedForRef())  // JVT-Q065 EIDR
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }

// JVT-Q065 EIDR{
  if(rcFrameNumList.size() < uiMaxSize)
  {
    for( Int i = uiCurrBasePos-1; i >= 0; i-- )
    {
      if( m_pacControlData[i].getSliceHeader()->getTemporalId() < uiLevel && !m_papcFrame[i]->getUnusedForRef())
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
  }
// JVT-Q065 EIDR}
  }
  else //===== list 0 =====
  {
    for( Int i = uiCurrBasePos-1; i >= 0; i-- )
    {
    if( m_pacControlData[i].getSliceHeader()->getTemporalId() < uiLevel && !m_papcFrame[i]->getUnusedForRef()) // JVT-Q065 EIDR
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
// JVT-Q065 EIDR{
  if(rcFrameNumList.size() < uiMaxSize)
  {
    for( Int i = (Int)uiCurrBasePos+1; i <= (Int)m_uiGOPSize; i++ )
    {
      if( m_pacControlData[i].getSliceHeader()->getTemporalId() < uiLevel && !m_papcFrame[i]->getUnusedForRef())
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
    }
// JVT-Q065 EIDR}
  }

  ROF( rcFrameNumList.size() == uiMaxSize );

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xGetFieldNumList( SliceHeader& rcSH, UIntList& rcFieldNumList, ListIdx eLstIdx, UInt uiCurrBasePos )
{
  rcFieldNumList.clear();

  const UInt uiLevel   = rcSH.getTemporalId  ();
  const UInt uiMaxSize = rcSH.getNumRefIdxActive( eLstIdx );
  ROF( uiMaxSize );

  //===== list 1 =====
  if( eLstIdx == LIST_1 )
  {
    for( UInt i = uiCurrBasePos+1; i <= m_uiGOPSize; i++ )
    {
      if( m_pacControlData[i].getSliceHeader()->getTemporalId() < uiLevel && !m_papcFrame[i]->getUnusedForRef())  // JVT-Q065 EIDR
         {
        //--- current parity ---
        rcFieldNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum()*2+1 );
        if( rcFieldNumList.size() == uiMaxSize )
        {
          break;
        }
        //--- opposite parity ---
        rcFieldNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum()*2   );
        if( rcFieldNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }

    // JVT-Q065 EIDR{
  if(rcFieldNumList.size() < uiMaxSize)
  {
    for( Int i = uiCurrBasePos-1; i >= 0; i-- )
    {
      if( m_pacControlData[i].getSliceHeader()->getTemporalId() < uiLevel && !m_papcFrame[i]->getUnusedForRef())
      {
        //--- current parity ---
        rcFieldNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum()*2+1 );
        if( rcFieldNumList.size() == uiMaxSize )
        {
          break;
        }
        //--- opposite parity ---
        rcFieldNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum()*2   );
        if( rcFieldNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
  }
// JVT-Q065 EIDR}
 }
  //===== list 0 =====
  else
  {
// TMM {
    if( rcSH.getPicType() == BOT_FIELD )
    {
      //--- opposite parity ---
      rcFieldNumList.push_back( m_pacControlData[ uiCurrBasePos ].getSliceHeader()->getFrameNum()*2 );
    }
    if( rcFieldNumList.size() < uiMaxSize )
// TMM }
    for( Int i = uiCurrBasePos-1; i >= 0; i-- )
    {
      if( m_pacControlData[i].getSliceHeader()->getTemporalId() < uiLevel && !m_papcFrame[i]->getUnusedForRef()) // JVT-Q065 EIDR
     {
        //--- current parity ---
        rcFieldNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum()*2+1 );
        if( rcFieldNumList.size() == uiMaxSize )
        {
          break;
        }
        //--- opposite parity ---
        rcFieldNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum()*2   );
        if( rcFieldNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }

    // JVT-Q065 EIDR{
  if(rcFieldNumList.size() < uiMaxSize)
  {
    for( Int i = (Int)uiCurrBasePos+1; i <= (Int)m_uiGOPSize; i++ )
    {
      if( m_pacControlData[i].getSliceHeader()->getTemporalId() < uiLevel && !m_papcFrame[i]->getUnusedForRef())
      {
        //--- current parity ---
        rcFieldNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum()*2+1 );
        if( rcFieldNumList.size() == uiMaxSize )
        {
          break;
        }
        //--- opposite parity ---
        rcFieldNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum()*2   );
        if( rcFieldNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
    }
// JVT-Q065 EIDR}


  }

  ROF( rcFieldNumList.size() == uiMaxSize );

  return Err::m_nOK;
}


MbDataCtrl*
LayerEncoder::xGetMbDataCtrlL1( SliceHeader& rcSH, UInt uiCurrBasePos, RefFrameList* pcRefFrameList1 )
{
  ROFRS( pcRefFrameList1, 0 );
  ROFRS( pcRefFrameList1->getActive(), 0 );

  UInt    uiLevel   = rcSH.getTemporalId();
  PicType ePicType  = rcSH.getPicType();
  Frame*  pcFrameL1 = (*pcRefFrameList1)[1];
  Int     iPoc1     = pcFrameL1->getPoc();

  for( UInt i = 1; i <= m_uiGOPSize; i++ )
  {
    if( m_pacControlData[i].getSliceHeader( ePicType ) 
      && uiLevel > m_pacControlData[i].getSliceHeader( ePicType )->getTemporalId()
      && m_pacControlData[i].getSliceHeader( ePicType )->getPoc() == iPoc1 )
    {
      return m_pacControlData[i].getMbDataCtrl();
    }
  }
/*
  const UInt uiLevel   = rcSH.getTemporalId();
  const PicType ePicType = rcSH.getPicType      ();

  for( UInt i = uiCurrBasePos+1; i <= m_uiGOPSize; i++ )
  {
    if( m_pacControlData[i].getSliceHeader( ePicType ) && uiLevel > m_pacControlData[i].getSliceHeader( ePicType )->getTemporalId() )
    {
      return m_pacControlData[i].getMbDataCtrl();
    }
  }
*/
  return 0;
}

Void
LayerEncoder::xAssignSimplePriorityId( SliceHeader* pcSliceHeader )
{
    // Lookup simple priority ID from mapping table (J. Ridge, Y-K. Wang @ Nokia)
    Bool bFound = false;
//JVT-S036 lsj start
 //   for ( UInt uiSimplePriId = 0; uiSimplePriId < (1 << PRI_ID_BITS); uiSimplePriId++ )
  //  {
   //     UInt uiLayer, uiTempLevel, uiQualLevel;
   //     m_pcSPS->getSimplePriorityMap( uiSimplePriId, uiTempLevel, uiLayer, uiQualLevel );
  //      if ( pcSliceHeader->getTemporalId() == uiTempLevel && m_uiDependencyId == uiLayer && pcSliceHeader->getQualityId() == uiQualLevel )
    //    {
            pcSliceHeader->setPriorityId ( 0 ); //lsj The syntax element is not used by the decoding process specified in this Recommendation
            bFound = true;
      //      break;
    //    }
   // }
//JVT-S036 lsj end
    //AOF( bFound );
}



ErrVal
LayerEncoder::xGetPredictionListsFieldKey( RefFrameList& rcRefList0,
                                          UInt          uiList0Size,
                                          PicType       eCurrentPicType )
{
  rcRefList0.reset();
  const PicType eOppositePicType = ( eCurrentPicType == TOP_FIELD ? BOT_FIELD : TOP_FIELD );

  if( eCurrentPicType==BOT_FIELD )
  {
    RNOK( xFillAndUpsampleFrame( m_pcLowPassBaseReconstruction,         TOP_FIELD, false        ) );
    RNOK( rcRefList0.add       ( m_pcLowPassBaseReconstruction->getPic( TOP_FIELD )             ) );
    uiList0Size--;
  }

  //--- current parity ---
  if( uiList0Size )
  {
    RNOK( xFillAndUpsampleFrame( m_pcLowPassBaseReconstruction,         eCurrentPicType, false  ) );
    RNOK( rcRefList0.add       ( m_pcLowPassBaseReconstruction->getPic( eCurrentPicType )       ) );
    uiList0Size--;
  }
  //--- opposite parity ---
  if( uiList0Size )
  {
    RNOK( xFillAndUpsampleFrame( m_pcLowPassBaseReconstruction,         eOppositePicType, false ) );
    RNOK( rcRefList0.add       ( m_pcLowPassBaseReconstruction->getPic( eOppositePicType )      ) );
    uiList0Size--;
  }

  return Err::m_nOK;
}

Void 
LayerEncoder::xPAffDecision( UInt uiFrame )
{
  switch( m_pcLayerParameters->getPAff() )
  {
  case 0:
    m_pbFieldPicFlag[ uiFrame ] = false;

    break;

  case 1:
    m_pbFieldPicFlag[ uiFrame ] = true;
    break;

  case 2:
    m_pbFieldPicFlag[ uiFrame ] = ( rand()%2 ) ? true : false;
    break;

  default:
    AOT( 1 );
    break;
  }
}


//S051{
Bool LayerEncoder:: xSIPCheck	(UInt Poc)
{
  if(Poc==0)               //There seems to be  a bug in decoder if we can 
    return false;        //discard picture with Poc=0. So here I forbid Poc=0 
  if(std::find(m_cPocList.begin(),m_cPocList.end(),Poc)!=m_cPocList.end())
    return true;				
  return false;
}

int LayerEncoder::xGetMbDataCtrlL1Pos( const SliceHeader& rcSH, RefFrameList& rcRefFrameList1 )
{
  ROFRS( rcRefFrameList1.getActive(), 0 );

  UInt    uiLevel   = rcSH.getTemporalId();
  PicType ePicType  = rcSH.getPicType();
  Frame*  pcFrameL1 = (rcRefFrameList1)[1];
  Int     iPoc1     = pcFrameL1->getPoc();

  for( UInt i = 1; i <= m_uiGOPSize; i++ )
  {
    if( m_pacControlData[i].getSliceHeader( ePicType ) 
      && uiLevel > m_pacControlData[i].getSliceHeader( ePicType )->getTemporalId()
      && m_pacControlData[i].getSliceHeader( ePicType )->getPoc() == iPoc1 )
    {
      return i;
    }
  }
  /*
  const UInt uiLevel   = rcSH.getTemporalId();
  for( UInt i = uiCurrBasePos+1; i <= m_uiGOPSize; i++ )
  {
    if( m_pacControlData[i].getSliceHeader() && uiLevel > m_pacControlData[i].getSliceHeader()->getTemporalId() )
    {
    return i;
    }
  }
  */
  return -1;
}
//S051}


// TMM_INTERLACE{
ErrVal
LayerEncoder::xMotionEstimationMbAff( RefFrameList*    pcRefFrameList0,
                                     RefFrameList*    pcRefFrameList1,
                                     Frame*        pcOrigFrame,
                                     Frame*        pcIntraRecFrame,
                                     ControlData&     rcControlData,
                                     Bool             bBiPredOnly,
                                     UInt             uiNumMaxIter,
                                     UInt             uiIterSearchRange,
                                     UInt             uiFrameIdInGOP )
{
  MbEncoder*    pcMbEncoder							=  m_pcSliceEncoder->getMbEncoder         ();
  SliceHeader&  rcSliceHeader						= *rcControlData.getSliceHeader           ( FRAME );
  MbDataCtrl*   pcMbDataCtrl						=  rcControlData.getMbDataCtrl            ();
  Frame*     pcBaseLayerFrame				=  rcControlData.getBaseLayerRec          ();
  Frame*     pcBaseLayerResidual	    =  rcControlData.getBaseLayerSbb          ();
  MbDataCtrl*   pcBaseLayerCtrl					=  rcControlData.getBaseLayerCtrl         ();
  MbDataCtrl*   pcBaseLayerCtrlField    =  rcControlData.getBaseLayerCtrlField    ();
  Bool          bEstimateBase						=  rcSliceHeader.getNoInterLayerPredFlag() && ! pcBaseLayerCtrl;
  Bool          bEstimateMotion					=  rcSliceHeader.getAdaptiveBaseModeFlag() || bEstimateBase;

  // JVT-S054 (ADD)
  MbDataCtrl*     pcMbDataCtrlL1    = xGetMbDataCtrlL1( rcSliceHeader, uiFrameIdInGOP, pcRefFrameList1 );
  
  //===== copy motion if non-adaptive prediction =====
  if( ! bEstimateMotion )
  {
    ROF ( pcBaseLayerCtrl );

    RefFrameList cRefListL0Top, cRefListL1Top, cRefListL0Bot, cRefListL1Bot; 
    RNOK( gSetFrameFieldLists( cRefListL0Top, cRefListL0Bot, *pcRefFrameList0 ) );
    RNOK( gSetFrameFieldLists( cRefListL1Top, cRefListL1Bot, *pcRefFrameList1 ) );
    RefFrameList* apcRefFrameList0[4] = {0, &cRefListL0Top, &cRefListL0Bot, pcRefFrameList0 };
    RefFrameList* apcRefFrameList1[4] = {0, &cRefListL1Top, &cRefListL1Bot, pcRefFrameList1 };

    if (m_bIroiSliceDivisionFlag)
    {
      for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
      {
        rcSliceHeader.setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
        rcSliceHeader.setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
        // JVT-S054 (2) (ADD)
        rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
        UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
        UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
        UInt uiNumMBInSlice;
        //===== initialization =====
        RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );

        //===== loop over macroblocks =====
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
        {
          UInt          uiMbY, uiMbX;
          MbDataAccess* pcMbDataAccess      = 0;
          MbDataAccess* pcMbDataAccessBase  = 0;
          
          rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress            );


          //===== init macroblock =====
          RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
          if    ( pcBaseLayerCtrl )
          {
            RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
          }

          RNOK( pcMbDataCtrl->getMbData(uiMbX, uiMbY).copyMotion( pcBaseLayerCtrl->getMbData(uiMbX, uiMbY), pcMbDataCtrl->getSliceId() ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_0 ).setRefPicIdcs( apcRefFrameList0[ pcMbDataAccess->getMbPicType() ] ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_1 ).setRefPicIdcs( apcRefFrameList1[ pcMbDataAccess->getMbPicType() ] ) );
          // <<<< bug fix by heiko.schwarz@hhi.fhg.de
          if( pcBaseLayerFrame ) // the motion data are not just copied, but inferred from the base layer
          {
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_0 ).clear();
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_1 ).clear();
          }
          // >>>> bug fix by heiko.schwarz@hhi.fhg.de
          uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
          uiNumMBInSlice++;
        }
      }
    }
    else
    {
        FMO* pcFMO = rcControlData.getSliceHeader( FRAME )->getFMO(); //TMM
 
      for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
      {
        rcSliceHeader.setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
        rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
        // JVT-S054 (2) (ADD)
        rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
        UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
        UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
        UInt uiNumMBInSlice;
        //===== initialization =====
        RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );

        //===== loop over macroblocks =====
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
        {
          UInt          uiMbY,  uiMbX;
          MbDataAccess* pcMbDataAccess      = 0;
          MbDataAccess* pcMbDataAccessBase  = 0;

           rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress            );

          
          //===== init macroblock =====
          RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
          if    ( pcBaseLayerCtrl )
          {
            RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
          }

          RNOK( pcMbDataCtrl->getMbData(uiMbX, uiMbY).copyMotion( pcBaseLayerCtrl->getMbData(uiMbX, uiMbY), pcMbDataCtrl->getSliceId() ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_0 ).setRefPicIdcs( apcRefFrameList0[ pcMbDataAccess->getMbPicType() ] ) );
          RNOK( pcMbDataAccess->getMbMotionData( LIST_1 ).setRefPicIdcs( apcRefFrameList1[ pcMbDataAccess->getMbPicType() ] ) );
          // <<<< bug fix by heiko.schwarz@hhi.fhg.de
          if( pcBaseLayerFrame ) // the motion data are not just copied, but inferred from the base layer
          {
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_0 ).clear();
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_1 ).clear();
          }
          // >>>> bug fix by heiko.schwarz@hhi.fhg.de
          uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
          uiNumMBInSlice++;
        }
      }
    }
    return Err::m_nOK;
  }

  if (m_bIroiSliceDivisionFlag)
  {
    for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
    {
      rcSliceHeader.setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
      rcSliceHeader.setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
      // JVT-S054 (2) (ADD)
      rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
      UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
      UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
      UInt uiNumMBInSlice;

      //===== initialization =====
      RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
        if( ! m_bLoadMotionInfo )
        {
          RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
          RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
        }
        RefFrameList acRefFrameList0    [2];
        RefFrameList acRefFrameList1    [2];

        Frame* apcOrigFrame        [4] = { NULL, NULL, NULL, NULL };
        Frame* apcIntraRecFrame    [4] = { NULL, NULL, NULL, NULL };
        Frame* apcBaseLayerFrame   [4] = { NULL, NULL, NULL, NULL };
        Frame* apcBaseLayerResidual[4] = { NULL, NULL, NULL, NULL };

        RNOK( gSetFrameFieldArrays( apcOrigFrame,         pcOrigFrame         ) );
        RNOK( gSetFrameFieldArrays( apcIntraRecFrame,     pcIntraRecFrame     ) );
        RNOK( gSetFrameFieldArrays( apcBaseLayerFrame,    pcBaseLayerFrame    ) );
        RNOK( gSetFrameFieldArrays( apcBaseLayerResidual, pcBaseLayerResidual ) );

        RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
        RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );

        RefFrameList* apcRefFrameList0[4];
        RefFrameList* apcRefFrameList1[4];

        apcRefFrameList0[0] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
        apcRefFrameList0[1] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
        apcRefFrameList1[0] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
        apcRefFrameList1[1] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];
        apcRefFrameList0[2] = apcRefFrameList0[3] = pcRefFrameList0;
        apcRefFrameList1[2] = apcRefFrameList1[3] = pcRefFrameList1;

        YuvMbBuffer acIntYuvMbBuffer[2];

        MbDataBuffer acMbData[2];
        Bool   abSkipModeAllowed[4] = {true,true,true,true};
        IntMbTempData cMbTempDataBase;

        //===== loop over macroblocks =====
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress; uiMbAddress+=2 )
        {
          Double adCost[2]  = {0,0};
          for( Int eP = 0; eP < 4; eP++ )
          {
            MbDataAccess* pcMbDataAccess     = NULL;
            MbDataAccess* pcMbDataAccessBase = NULL;
            Double        dCost = 0;
            UInt          uiMbY, uiMbX;

            const Bool    bField = (eP < 2);
            const UInt    uiMbAddressMbAff = uiMbAddress+(eP%2);

            rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddressMbAff );

            //===== init macroblock =====
            RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );

            if  (eP < 2 && pcBaseLayerCtrlField)  // field case
            {
              RNOK( pcBaseLayerCtrlField         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
            }
            else if ( eP >= 2 && pcBaseLayerCtrl)  //frame case
            {
              RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
            }
          
            if( 0 == eP)
            {
              abSkipModeAllowed[1] = pcMbDataAccess->getDefaultFieldFlag(); // do not move
              abSkipModeAllowed[3] = ! abSkipModeAllowed[1];
            }

            if( ! m_bLoadMotionInfo )
            {
              pcMbDataAccess->setFieldMode( eP < 2 );
              //===== initialisation =====
              RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX, true ) );
              RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX, true ) );
              RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

              if( ! rcSliceHeader.getNoInterLayerPredFlag() )
              {
                Int iFieldMode = ( eP < 2 ? 1 : 0 );
                m_pcSliceEncoder->m_pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[iFieldMode][uiMbAddressMbAff] );
              }

              //===== estimate prediction data =====
              RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
                                                      pcMbDataAccessBase,
                                                      *apcRefFrameList0    [eP],
                                                      *apcRefFrameList1    [eP],
                                                      apcBaseLayerFrame   [eP],
                                                      apcBaseLayerResidual[eP],
                                                      *apcOrigFrame        [eP],
                                                      *apcIntraRecFrame    [eP],
                                                      bBiPredOnly,
                                                      uiNumMaxIter,
                                                      uiIterSearchRange,
                                                      m_bBLSkipEnable, //JVT-Q065 EIDR
                                                      rcControlData.getLambda(),
                                                      dCost,
                                                      abSkipModeAllowed   [eP]) );

// TMM_INTERLACE {
              /*if( m_bSaveMotionInfo )
              {
                //===== save prediction data =====
           // saveAll is displaced because the Mvs are Ok but the other data could be modified (mode,...).
           // Do it after m_pcSliceEncoder->encodeHighPassPicture
           //            RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
              }*/
// TMM_INTERLACE }
            }
            else
            {
              //===== load prediction data =====
              RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
            }

            if( adCost[eP>>1] != DOUBLE_MAX )
            {
              adCost [eP>>1] += dCost;
            }
            if( bField )
            {
              acMbData[eP].copy( pcMbDataAccess->getMbData() );
             // TMM_INTERLACE
             (&acIntYuvMbBuffer[eP])->loadBuffer( apcIntraRecFrame[eP]->getFullPelYuvBuffer() );
           }
          }
          Bool bFieldMode = adCost[0] < adCost[1];

          if ( m_bLoadMotionInfo )
          {
            char ch;
            fread( &ch, sizeof(char), 1, m_pMotionInfoFile );
            bFieldMode = ( ch == 1 );
          }
          if ( m_bSaveMotionInfo )
          {
            char ch = ( bFieldMode == true );
            fwrite( &ch, sizeof(char), 1, m_pMotionInfoFile );
          }

#ifdef RANDOM_MBAFF
          bFieldMode = gBoolRandom();
#endif

          if( bFieldMode )
          {
            // copy field modes back 
            UInt          uiMbY, uiMbX;
            MbDataAccess* pcMbDataAccess = NULL;
            rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress          );
            RNOK( pcMbDataCtrl      ->initMb      ( pcMbDataAccess,     uiMbY, uiMbX ) );
            pcMbDataAccess->getMbData().copy( acMbData[0] );
            apcIntraRecFrame[0]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBuffer[0] );

            pcMbDataAccess = NULL;
            rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress+1        );
            RNOK( pcMbDataCtrl      ->initMb      ( pcMbDataAccess,     uiMbY, uiMbX ) );
            pcMbDataAccess->getMbData().copy( acMbData[1] );
            apcIntraRecFrame[1]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBuffer[1] );
          }
           //uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
           uiNumMBInSlice++;
        }
     }
    }
    else
    {
    FMO* pcFMO = rcControlData.getSliceHeader( FRAME )->getFMO(); //TMM
   
      for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
      {
        rcSliceHeader.setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
        rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
        // JVT-S054 (2) (ADD)
        rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
        UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
        UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
        UInt uiNumMBInSlice;
        //===== initialization =====
      RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
        if( ! m_bLoadMotionInfo )
        {
          RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
          RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
        }
        RefFrameList acRefFrameList0    [2];
        RefFrameList acRefFrameList1    [2];

        Frame* apcOrigFrame        [4] = { NULL, NULL, NULL, NULL };
        Frame* apcIntraRecFrame    [4] = { NULL, NULL, NULL, NULL };
        Frame* apcBaseLayerFrame   [4] = { NULL, NULL, NULL, NULL };
        Frame* apcBaseLayerResidual[4] = { NULL, NULL, NULL, NULL };

        RNOK( gSetFrameFieldArrays( apcOrigFrame,         pcOrigFrame         ) );
        RNOK( gSetFrameFieldArrays( apcIntraRecFrame,     pcIntraRecFrame     ) );
        RNOK( gSetFrameFieldArrays( apcBaseLayerFrame,    pcBaseLayerFrame    ) );
        RNOK( gSetFrameFieldArrays( apcBaseLayerResidual, pcBaseLayerResidual ) );

        RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
        RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );

        RefFrameList* apcRefFrameList0[4];
        RefFrameList* apcRefFrameList1[4];

        apcRefFrameList0[0] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
        apcRefFrameList0[1] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
        apcRefFrameList1[0] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
        apcRefFrameList1[1] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];
        apcRefFrameList0[2] = apcRefFrameList0[3] = pcRefFrameList0;
        apcRefFrameList1[2] = apcRefFrameList1[3] = pcRefFrameList1;

        YuvMbBuffer acIntYuvMbBuffer[2];

        MbDataBuffer acMbData[2];
        Bool   abSkipModeAllowed[4] = {true,true,true,true};
        IntMbTempData cMbTempDataBase;

        //===== loop over macroblocks =====
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress; uiMbAddress+=2 )
        {
          Double adCost[2]  = {0,0};
          for( Int eP = 0; eP < 4; eP++ )
          {
            MbDataAccess* pcMbDataAccess     = NULL;
            MbDataAccess* pcMbDataAccessBase = NULL;
            Double        dCost = 0;
            UInt          uiMbY, uiMbX;

            const Bool    bField = (eP < 2);
            const UInt    uiMbAddressMbAff = uiMbAddress+(eP%2);

            rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddressMbAff );

            //===== init macroblock =====
            RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );

            if  (eP < 2 && pcBaseLayerCtrlField)  // field case
            {
              RNOK( pcBaseLayerCtrlField         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
            }
            else if ( eP >= 2 && pcBaseLayerCtrl)  //frame case
            {
              RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
            }
          
            if( 0 == eP)
            {
              abSkipModeAllowed[1] = pcMbDataAccess->getDefaultFieldFlag(); // do not move
              abSkipModeAllowed[3] = ! abSkipModeAllowed[1];
            }
            else if( eP%2 == 1 )
            {
              const MbData&	rcTopMb = pcMbDataAccess->getMbDataComplementary();
              if ( rcTopMb.getSkipFlag() && ( pcMbDataAccess->getDefaultFieldFlag() != rcTopMb.getFieldFlag() ) )
                abSkipModeAllowed[eP] = false;
            }

            if( ! m_bLoadMotionInfo )
            {
              pcMbDataAccess->setFieldMode( eP < 2 );
              //===== initialisation =====
              RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX, true ) );
              RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX, true ) );
              RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

              if( ! rcSliceHeader.getNoInterLayerPredFlag() )
              {
                Int iFieldMode = ( eP < 2 ? 1 : 0 );
                m_pcSliceEncoder->m_pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[iFieldMode][uiMbAddressMbAff] );
              }

              //===== estimate prediction data =====
              RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
                                                      pcMbDataAccessBase,
                                                      *apcRefFrameList0    [eP],
                                                      *apcRefFrameList1    [eP],
                                                      apcBaseLayerFrame   [eP],
                                                      apcBaseLayerResidual[eP],
                                                      *apcOrigFrame        [eP],
                                                      *apcIntraRecFrame    [eP],
                                                      bBiPredOnly,
                                                      uiNumMaxIter,
                                                      uiIterSearchRange,
                                                      m_bBLSkipEnable, //JVT-Q065 EIDR
                                                      rcControlData.getLambda(),
                                                      dCost,
                                                      abSkipModeAllowed   [eP]) );

// TMM_INTERLACE {
              /*if( m_bSaveMotionInfo )
              {
                //===== save prediction data =====
           // saveAll is displaced because the Mvs are Ok but the other data could be modified (mode,...).
           // Do it after m_pcSliceEncoder->encodeHighPassPicture
           //            RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
              }*/
// TMM_INTERLACE }
            }
            else
            {
              //===== load prediction data =====
              RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
            }

            if( adCost[eP>>1] != DOUBLE_MAX )
            {
              adCost [eP>>1] += dCost;
            }
            if( bField )
            {
              acMbData[eP].copy( pcMbDataAccess->getMbData() );
            //TMM_INTERLACE
            (&acIntYuvMbBuffer[eP])->loadBuffer( apcIntraRecFrame[eP]->getFullPelYuvBuffer() );
            }
          }
          Bool bFieldMode = adCost[0] < adCost[1];

          if ( m_bLoadMotionInfo )
          {
            char ch;
            fread( &ch, sizeof(char), 1, m_pMotionInfoFile );
            bFieldMode = ( ch == 1 );
          }
          if ( m_bSaveMotionInfo )
          {
            char ch = ( bFieldMode == true );
            fwrite( &ch, sizeof(char), 1, m_pMotionInfoFile );
          }

      #ifdef RANDOM_MBAFF
          bFieldMode = gBoolRandom();
      #endif

          if( bFieldMode )
          {
            // copy field modes back 
            UInt          uiMbY, uiMbX;
            MbDataAccess* pcMbDataAccess = NULL;
            rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress          );
            RNOK( pcMbDataCtrl      ->initMb      ( pcMbDataAccess,     uiMbY, uiMbX ) );
            pcMbDataAccess->getMbData().copy( acMbData[0] );
            apcIntraRecFrame[0]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBuffer[0] );

            pcMbDataAccess = NULL;
            rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress+1        );
            RNOK( pcMbDataCtrl      ->initMb      ( pcMbDataAccess,     uiMbY, uiMbX ) );
            pcMbDataAccess->getMbData().copy( acMbData[1] );
            apcIntraRecFrame[1]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBuffer[1] );
          }
           //uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
           uiNumMBInSlice++;
        }
      }
      }
  return Err::m_nOK;
}

// TMM_INTERLACE}
//JVT-U106 Behaviour at slice boundaries}
H264AVC_NAMESPACE_END


