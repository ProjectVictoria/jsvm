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





#if !defined(AFX_LOOPFILTER_H__1860BB4C_C677_487A_A81F_0BD39DA40284__INCLUDED_)
#define AFX_LOOPFILTER_H__1860BB4C_C677_487A_A81F_0BD39DA40284__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN

class ControlMngIf;
class IntYuvPicBuffer;
class IntFrame;

class ReconstructionBypass;

class H264AVCCOMMONLIB_API LoopFilter
{
  enum Dir
  {
    VER = 0,
    HOR
  };

public:
  enum LFMode
  {
    LFM_DEFAULT_FILTER    = 0,
    LFM_NO_INTER_FILTER   = 1,
    LFM_NO_INTRAINTER     = 2,
    LFM_EXTEND_INTRA_SUR  = 4,
    LFM_NO_FILTER         = 8
  };

  typedef struct
  {
    UChar ucAlpha;
    UChar aucClip[5];
  }AlphaClip;

  typedef struct
  {
    Pel* pFlt;
    Int iOffset;
    Int iIndexA;
    Int iIndexB;
    UChar ucBs;
    Bool bLum;
  }FilterParameter;

  static const UChar g_aucBetaTab[52]; // leszek
  static const AlphaClip g_acAlphaClip[52]; // leszek

  static const UChar g_aucTcTab_H241RCDO[52];
  static const UChar g_aucBetaTab_H241RCDO[52];

protected:
	LoopFilter();
	virtual ~LoopFilter();

public:
  static ErrVal create( LoopFilter*& rpcLoopFilter );
  ErrVal destroy();

  Void   setRCDOSliceHeader( SliceHeader* pcSliceHeader = 0 )
  {
    m_pcRCDOSliceHeader = pcSliceHeader;
  }
  ErrVal process        ( SliceHeader& rcSH, IntFrame* pcIntFrame = NULL/*, IntYuvPicBuffer* pcHighpassYuvBuffer = NULL,*/
							            ,bool  bAllSliceDone= false);
  ErrVal process        ( SliceHeader&        rcSH,
                          IntFrame*           pcFrame,
                          MbDataCtrl*         pcMbDataCtrlMot,
                          MbDataCtrl*         pcMbDataCtrlRes,
                          UInt                uiMbInRow,
                          RefFrameList*       pcRefFrameList0,
                          RefFrameList*       pcRefFrameList1,
						  bool				  bAllSliceDone,
                          bool                spatial_scalable_flg);  // SSUN@SHARP

  ErrVal init( ControlMngIf*          pcControlMngIf,
               ReconstructionBypass*  pcReconstructionBypass,
               Bool                   bEncoder
               );

  ErrVal uninit();

  Void setFilterMode( LFMode eLFMode = LFM_DEFAULT_FILTER ) { m_eLFMode = eLFMode; }

  // Hanke@RWTH
  Void setHighpassFramePointer( IntFrame* pcHighpassFrame = NULL ) { m_pcHighpassFrame = pcHighpassFrame; }

private:

  UChar xCheckMvDataB( const MbData& rcQMbData, const LumaIdx cQIdx, const MbData& rcPMbData, const LumaIdx cPIdx, const Short sHorMvThr, const Short sVerMvThr );
  UChar xCheckMvDataP( const MbData& rcQMbData, const LumaIdx cQIdx, const MbData& rcPMbData, const LumaIdx cPIdx, const Short sHorMvThr, const Short sVerMvThr );

  __inline ErrVal xFilterMb       ( const MbDataAccess& rcMbDataAccess, bool enhancedLayerFlag ); //V032, added enhanced layer indicator
  __inline ErrVal xFilterMb_RCDO  ( const MbDataAccess& rcMbDataAccess );
  __inline ErrVal xFilterMbFast ( const MbDataAccess& rcMbDataAccess, bool enhancedLayerFlag); //V032, added enhanced layer indicator
	ErrVal xGetFilterStrengthFast ( const MbDataAccess& rcMbDataAccess, const Int iFilterIdc );
  __inline UInt xGetHorFilterStrengthFast( const MbData& rcMbDataCurr,
		                                       const MbData& rcMbDataAbove,
																					 LumaIdx       cIdx,
																					 Bool          bAboveIntra,
																					 Bool          bCheckMv,
																					 Bool          bCoded,
																					 const Short   sVerMvThr,
																					 Int           iIntraStrength);
  __inline UInt xGetVerFilterStrengthFast( const MbData& rcMbDataCurr,
		                                       const MbData& rcMbDataLeft,
																					 LumaIdx       cIdx,
																					 Bool          bLeftIntra,
																					 Bool          bCheckMv,
																					 Bool          bCoded,
																					 const Short   sVerMvThr);

  __inline Void xFilter( Pel* pFlt, const Int& iOffset, const Int& iIndexA, const Int& iIndexB, const UChar& ucBs, const Bool& bLum );

  __inline UInt xGetHorFilterStrength( const MbDataAccess& rcMbDataAccess, LumaIdx cIdx, Int iFilterIdc);
  __inline UInt xGetVerFilterStrength( const MbDataAccess& rcMbDataAccess, LumaIdx cIdx, Int iFilterIdc);
  __inline ErrVal xLumaHorFiltering  ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);
  __inline ErrVal xLumaVerFiltering  ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);
  __inline ErrVal xChromaHorFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);
  __inline ErrVal xChromaVerFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);

  __inline Void xFilter_H241RCDO( Pel*  pFlt, const Int& iOffset, const Int& iIndexA, const Int& iIndexB, const Int& blk_pos, const Bool& bLum );
  __inline Void xFilter_H241RCDO( XPel* pFlt, const Int& iOffset, const Int& iIndexA, const Int& iIndexB, const Int& blk_pos, const Bool& bLum );
  
  __inline ErrVal xLumaHorFiltering_H241RCDO  ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);
  __inline ErrVal xLumaVerFiltering_H241RCDO  ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);
  __inline ErrVal xChromaHorFiltering_H241RCDO( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);
  __inline ErrVal xChromaVerFiltering_H241RCDO( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);

  __inline ErrVal xLumaHorFiltering_H241RCDO  ( const MbDataAccess& rcMbDataAccessRes,
                                                const DFP&          rcDFP,
                                                IntYuvPicBuffer*    pcYuvBuffer,
												                        const MbDataAccess* rcMbDataAccessMot, //HZL added
                                                RefFrameList*       rcRefFrameList0,   //HZL added
                                                RefFrameList*       rcRefFrameList1 ); //HZL added
  __inline ErrVal xLumaVerFiltering_H241RCDO  ( const MbDataAccess& rcMbDataAccessRes,
                                                const DFP&          rcDFP,
                                                IntYuvPicBuffer*    pcYuvBuffer,
												                        const MbDataAccess* rcMbDataAccessMot, //HZL added
                                                RefFrameList*       rcRefFrameList0,   //HZL added
                                                RefFrameList*       rcRefFrameList1 ); //HZL added												  
  __inline ErrVal xChromaHorFiltering_H241RCDO( const MbDataAccess& rcMbDataAccessRes,
                                                const DFP&          rcDFP,
                                                IntYuvPicBuffer*    pcYuvBuffer );
  __inline ErrVal xChromaVerFiltering_H241RCDO( const MbDataAccess& rcMbDataAccessRes,
                                                const DFP&          rcDFP,
                                                IntYuvPicBuffer*    pcYuvBuffer );

  __inline Void   xFilter                       ( XPel*               pFlt,
                                                  const Int&          iOffset,
                                                  const Int&          iIndexA,
                                                  const Int&          iIndexB,
                                                  const UChar&        ucBs,
                                                  const Bool&         bLum );
  __inline ErrVal xFilterMb                     ( MbDataAccess* rcMbDataAccessMot,
                                                  MbDataAccess* rcMbDataAccessRes,
                                                  IntYuvPicBuffer*    pcYuvBuffer,
                                                  RefFrameList*       pcRefFrameList0,
                                                  RefFrameList*       pcRefFrameList1,
                                                  bool                spatial_scalable_flg,
									                        			  bool				  enhancedLayerFlag ); //V032 of FSL, added enhanced layer indicator
  __inline ErrVal xFilterMb_RCDO                ( const MbDataAccess* rcMbDataAccessMot,
                                                  const MbDataAccess* rcMbDataAccessRes,
                                                  IntYuvPicBuffer*    pcYuvBuffer,
                                                  RefFrameList*       pcRefFrameList0,
                                                  RefFrameList*       pcRefFrameList1,
                                                  bool                spatial_scalable_flg);  // SSUN@SHARP

  ErrVal xRecalcCBP( MbDataAccess &rcMbDataAccess )
  {
    UInt uiCbp = 0;
    UInt uiStart = 0;
    UInt uiStop  = 16;
    if( rcMbDataAccess.getMbData().isTransformSize8x8() )
    {
      const UChar *pucScan = rcMbDataAccess.getMbData().getFieldFlag() ? g_aucFieldScan64 : g_aucFrameScan64;
      uiStart <<= 2;
      uiStop  <<= 2;
      for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        TCoeff *piCoeff = rcMbDataAccess.getMbTCoeffs().get8x8( cIdx );
        for( UInt ui = uiStart; ui < uiStop; ui++ )
        {
          if( m_bEncoder ? piCoeff[pucScan[ui]].getLevel() : piCoeff[pucScan[ui]].getCoeff() )
          {
            uiCbp |= 0x33 << cIdx.b8x8();
            break;
          }
        }
      }
    }
    else
    {
      const UChar *pucScan = rcMbDataAccess.getMbData().getFieldFlag() ? g_aucFieldScan : g_aucFrameScan;
      for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        TCoeff *piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );
        for( UInt ui = uiStart; ui < uiStop; ui++ )
        {
          if( m_bEncoder ? piCoeff[pucScan[ui]].getLevel() : piCoeff[pucScan[ui]].getCoeff() )
          {
            uiCbp |= 1<<cIdx.b4x4();
            break;
          }
        }
      }
      if( rcMbDataAccess.getMbData().isIntra16x16() )
      {
        uiCbp = uiCbp ? 0xFFFF : 0;
      }
    }
    rcMbDataAccess.getMbData().setAndConvertMbExtCbp( uiCbp );
    return Err::m_nOK;
  }

  __inline ErrVal xLumaHorFiltering             ( const MbDataAccess& rcMbDataAccessRes,
                                                  const DFP&          rcDFP,
                                                  IntYuvPicBuffer*    pcYuvBuffer );
  __inline ErrVal xLumaVerFiltering             ( const MbDataAccess& rcMbDataAccessRes,
                                                  const DFP&          rcDFP,
                                                  IntYuvPicBuffer*    pcYuvBuffer );
  __inline ErrVal xChromaHorFiltering           ( const MbDataAccess& rcMbDataAccessRes,
                                                  const DFP&          rcDFP,
                                                  IntYuvPicBuffer*    pcYuvBuffer );
  __inline ErrVal xChromaVerFiltering           ( const MbDataAccess& rcMbDataAccessRes,
                                                  const DFP&          rcDFP,
                                                  IntYuvPicBuffer*    pcYuvBuffer );

  UChar           xCheckMvDataP_RefIdx          ( const MbData&       rcQMbData,
                                                  const LumaIdx       cQIdx,
                                                  const MbData&       rcPMbData,
                                                  const LumaIdx       cPIdx,
                                                  const Short         sHorMvThr,
                                                  const Short         sVerMvThr,
                                                  RefFrameList&       rcRefFrameList0,
                                                  RefFrameList&       rcRefFrameList1  );
  UChar           xCheckMvDataB_RefIdx          ( const MbData&       rcQMbData,
                                                  const LumaIdx       cQIdx,
                                                  const MbData&       rcPMbData,
                                                  const LumaIdx       cPIdx,
                                                  const Short         sHorMvThr,
                                                  const Short         sVerMvThr,
                                                  RefFrameList&       rcRefFrameList0,
                                                  RefFrameList&       rcRefFrameList1 );
  __inline UInt   xGetHorFilterStrength_RefIdx  ( const MbDataAccess* pcMbDataAccessMot,
                                                  const MbDataAccess* pcMbDataAccessRes,
                                                  LumaIdx             cIdx,
                                                  Int                 iFilterIdc,
                                                  RefFrameList*       pcRefFrameList0,
                                                  RefFrameList*       pcRefFrameList1,
                                                  bool                spatial_scalable_flg );
  __inline UInt   xGetVerFilterStrength_RefIdx  ( const MbDataAccess* pcMbDataAccessMot,
                                                  const MbDataAccess* pcMbDataAccessRes,
                                                  LumaIdx             cIdx,
                                                  Int                 iFilterIdc,
                                                  RefFrameList*       pcRefFrameList0,
                                                  RefFrameList*       pcRefFrameList1,
                                                  bool                spatial_scalable_flg );

protected:
  // Hanke@RWTH
  IntFrame*        m_pcHighpassFrame;
  IntYuvPicBuffer* m_pcHighpassYuvBuffer;

  UChar m_aucBs[4];

  ControlMngIf*    m_pcControlMngIf;
  FrameUnit*       m_pcRecFrameUnit;
  UChar            m_aaaucBs[2][4][4];
  IntYuvPicBuffer* m_apcIntYuvBuffer[4];
  LFMode           m_eLFMode;
  ReconstructionBypass*         m_pcReconstructionBypass;

  UChar           m_aucBsHorTop[4];
  UChar           m_aucBsVerBot[4];
  Bool            m_bVerMixedMode;
  Bool            m_bHorMixedMode;
  Bool            m_bAddEdge;

  Bool          m_bRCDO;
  SliceHeader*  m_pcRCDOSliceHeader; // only for SPS RCDO deblocking flag
  Bool          m_bEncoder;

protected:

  template <UInt uiLum> Void xFilterTempl( FilterParameter& rcFilterParameter )
  {
    const Int iAlpha = g_acAlphaClip[ rcFilterParameter.iIndexA ].ucAlpha;

    Int P0 = rcFilterParameter.pFlt[-rcFilterParameter.iOffset];
    Int Q0 = rcFilterParameter.pFlt[       0];

    Int iDelta = Q0 - P0;
    Int iAbsDelta  = abs( iDelta  );

    AOF_DBG( rcFilterParameter.ucBs );

    ROFVS( iAbsDelta < iAlpha );


    const Int iBeta = g_aucBetaTab [ rcFilterParameter.iIndexB ];

    Int P1  = rcFilterParameter.pFlt[-2*rcFilterParameter.iOffset];
    Int Q1  = rcFilterParameter.pFlt[   rcFilterParameter.iOffset];

    ROFVS( (abs(P0 - P1) < iBeta) && (abs(Q0 - Q1) < iBeta) );

    if( rcFilterParameter.ucBs < 4 )
    {
      Int C0 = g_acAlphaClip[ rcFilterParameter.iIndexA ].aucClip[rcFilterParameter.ucBs];

      if( uiLum )
      {
        Int P2 = rcFilterParameter.pFlt[-3*rcFilterParameter.iOffset] ;
        Int Q2 = rcFilterParameter.pFlt[ 2*rcFilterParameter.iOffset] ;
        Int aq = (( abs( Q2 - Q0 ) < iBeta ) ? 1 : 0 );
        Int ap = (( abs( P2 - P0 ) < iBeta ) ? 1 : 0 );

        if( ap )
        {
          rcFilterParameter.pFlt[-2*rcFilterParameter.iOffset] = P1 + gClipMinMax((P2 + ((P0 + Q0 + 1)>>1) - (P1<<1)) >> 1, -C0, C0 );
        }

        if( aq  )
        {
          rcFilterParameter.pFlt[   rcFilterParameter.iOffset] = Q1 + gClipMinMax((Q2 + ((P0 + Q0 + 1)>>1) - (Q1<<1)) >> 1, -C0, C0 );
        }

        C0 += ap + aq -1;
      }

      C0++;
      Int iDiff      = gClipMinMax(((iDelta << 2) + (P1 - Q1) + 4) >> 3, -C0, C0 ) ;
      rcFilterParameter.pFlt[-rcFilterParameter.iOffset] = gClip( P0 + iDiff );
      rcFilterParameter.pFlt[       0] = gClip( Q0 - iDiff );
      return;
    }


    if( ! uiLum )
    {
      rcFilterParameter.pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
      rcFilterParameter.pFlt[  -rcFilterParameter.iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
    }
    else
    {
      Int P2 = rcFilterParameter.pFlt[-3*rcFilterParameter.iOffset] ;
      Int Q2 = rcFilterParameter.pFlt[ 2*rcFilterParameter.iOffset] ;
      Bool bEnable  = (iAbsDelta < ((iAlpha >> 2) + 2));
      Bool aq       = bEnable & ( abs( Q2 - Q0 ) < iBeta );
      Bool ap       = bEnable & ( abs( P2 - P0 ) < iBeta );
      Int PQ0 = P0 + Q0;

      if( aq )
      {
        rcFilterParameter.pFlt[         0] = (P1 + ((Q1 + PQ0) << 1) +  Q2 + 4) >> 3;
        rcFilterParameter.pFlt[   rcFilterParameter.iOffset] = (PQ0 +Q1 + Q2 + 2) >> 2;
        rcFilterParameter.pFlt[ 2*rcFilterParameter.iOffset] = (((rcFilterParameter.pFlt[ 3*rcFilterParameter.iOffset] + Q2) <<1) + Q2 + Q1 + PQ0 + 4) >> 3;
      }
      else
      {
        rcFilterParameter.pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
      }

      if( ap )
      {
        rcFilterParameter.pFlt[  -rcFilterParameter.iOffset] = (Q1 + ((P1 + PQ0) << 1) +  P2 + 4) >> 3;
        rcFilterParameter.pFlt[-2*rcFilterParameter.iOffset] = (PQ0 +P1 + P2 + 2) >> 2;
        rcFilterParameter.pFlt[-3*rcFilterParameter.iOffset] = (((rcFilterParameter.pFlt[-4*rcFilterParameter.iOffset] + P2) <<1) + P2 + P1 + PQ0 + 4) >> 3;
      }
      else
      {
        rcFilterParameter.pFlt[  -rcFilterParameter.iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
      }
    }
  }

};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_LOOPFILTER_H__1860BB4C_C677_487A_A81F_0BD39DA40284__INCLUDED_)
