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




#if !defined(AFX_FGSSUBBANDENCODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_)
#define AFX_FGSSUBBANDENCODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MbSymbolWriteIf.h"
#include "ControlMngH264AVCEncoder.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/Transform.h"

#include "H264AVCCommonLib/FGSCoder.h"


H264AVC_NAMESPACE_BEGIN


class YuvBufferCtrl;
class Transform;
class MbDataCtrl;
class MbEncoder;



#define TWSOT(x)  { if( x) { xThrowWriteStop(); } }
#define TWSOF(x)  { if(!x) { xThrowWriteStop(); } }


class RQFGSEncoder  
  : public FGSCoder
{
private:
  enum
  {
    RQ_QP_DELTA = 6
  };
  class WriteStop
  {
  };

  __inline Void xThrowWriteStop()
  {
    throw WriteStop();
  }

protected:
	RQFGSEncoder         ();
	virtual ~RQFGSEncoder();

public:
  static ErrVal     create                ( RQFGSEncoder*&              rpcFGSSubbandEncoder );
  ErrVal            destroy               ();

  ErrVal            init                  ( YuvBufferCtrl**             apcYuvFullPelBufferCtrl,
                                            Transform*                  pcTransform,
                                            ControlMngH264AVCEncoder*   pcControlMng,
                                            MbEncoder*                  pcMbEncoder );
  ErrVal            uninit                ();
  ErrVal            initSPS               ( const SequenceParameterSet& rcSPS );


  ErrVal            initPicture           ( SliceHeader*                pcSliceHeader,
                                            MbDataCtrl*                 pcCurrMbDataCtrl,
                                            IntFrame*                   pcOrgResidual,
                                            Double                      dNumFGSLayers,
                                            Double                      dLambda,
                                            Int                         iMaxQpDelta,
                                            Bool&                       rbFinished,
                                            Bool                        bTruncate );
  ErrVal            encodeNextLayer       ( Bool&                       rbFinished,
                                            Bool&                       rbCorrupted,
                                            UInt                        uiMaxBits,
                                            FILE*                       pFile );
  ErrVal            reconstruct           ( IntFrame*                   pcRecResidual );
  ErrVal            finishPicture         ();
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  ErrVal            setNextLayerForRD     (IntFrame*                   pOrgResidual, 
                                            Double&                     rdScalFactor, 
                                            FILE*&                      rpDistoFile, 
                                            FILE*&                      rpRateFile);
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
  
private:
  ErrVal            xVLCParseLuma         ( UInt   uiBlockYIndex,
                                            UInt   uiBlockXIndex,
                                            UInt*  pauiNumCoefHist,
                                            UInt*  pauiHighMagHist);
  ErrVal            xVLCParseChromaAC     ( UInt   uiPlane,
                                            UInt   uiBlockYIndex,
                                            UInt   uiBlockXIndex,
                                            UInt*  pauiNumCoefHist);

  ErrVal            xSetSymbolsChroma     ( TCoeff*                     piCoeff,
                                            UInt                        uiMbX,
                                            UInt                        uiMbY,
                                            const QpParameter&          cQP,
                                            UInt&                       uiCoeffCostDC,
                                            UInt&                       uiCoeffCostAC,
                                            Bool&                       bSigDC,
                                            Bool&                       bSigAC,
                                            ChromaIdx                   cIdx );
  ErrVal            xSetSymbols4x4        ( TCoeff*                     piCoeff,
                                            UInt                        uiMbX,
                                            UInt                        uiMbY,
                                            const QpParameter&          cQP,
                                            UInt&                       uiCoeffCost,
                                            UInt&                       ruiCbp,
                                            LumaIdx                     cIdx,
                                            UInt                        uiStart );
  ErrVal            xSetSymbols8x8        ( TCoeff*                     piCoeff,
                                            TCoeff*                     piCoeffBase,
                                            const QpParameter&          cQP,
                                            UInt&                       uiCoeffCost,
                                            UInt&                       ruiCbp,
                                            LumaIdx                     cIdx );


  ErrVal            xRequantizeMacroblock ( MbDataAccess&               rcMbDataAccess,
                                            MbDataAccess&               rcMbDataAccessBL,
                                            IntYuvMbBuffer&             rcBLRecBuffer );

  ErrVal            xScaleBaseLayerCoeffs ();
  ErrVal            xResidualTransform    ();



  ErrVal            xEncodingFGS                  ( Bool&               rbFinished,
                                                    Bool&               rbCorrupted,
                                                    UInt                uiMaxBits,
                                                    FILE*               pFile );

  
  ErrVal            xEncodeNewCoefficientLuma     ( UInt                uiBlockYIndex,
                                                    UInt                uiBlockXIndex,
                                                    UInt&               uiComplete,
                                                    Int&                riLastQp );
  ErrVal            xEncodeNewCoefficientChromaDC ( UInt                uiPlane,
                                                    UInt                uiMbYIdx,
                                                    UInt                uiMbXIdx,
                                                    UInt&               uiComplete,
                                                    Int&                riLastQP );
  ErrVal            xEncodeNewCoefficientChromaAC ( UInt                uiPlane,
                                                    UInt                uiB8YIdx,
                                                    UInt                uiB8XIdx,
                                                    UInt&               uiComplete,
                                                    Int&                riLastQP );
  
  ErrVal            xEncodeCoefficientLumaRef     ( UInt                uiBlockYIndex,
                                                    UInt                uiBlockXIndex,
                                                    UInt                uiScanIndex );
  ErrVal            xEncodeCoefficientChromaDCRef ( UInt                uiPlane,
                                                    UInt                uiMbYIdx,
                                                    UInt                uiMbXIdx,
                                                    UInt                uiDCIdx );
  ErrVal            xEncodeCoefficientChromaACRef ( UInt                uiPlane,
                                                    UInt                uiB8YIdx,
                                                    UInt                uiB8XIdx,
                                                    UInt                uiScanIdx );

  UInt              xGetSigCtxLuma                ( UInt                uiScanIdx,
                                                    UInt                uiBlockY,
                                                    UInt                uiBlockX );
  UInt              xGetSigCtxChromaDC            ( UInt                uiPlane,
                                                    UInt                uiScanIdx,
                                                    UInt                uiMbY,
                                                    UInt                uiMbX );
  UInt              xGetSigCtxChromaAC            ( UInt                uiPlane,
                                                    UInt                uiScanIdx,
                                                    UInt                uiB8Y,
                                                    UInt                uiB8X );

  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  UInt              xCalculateNormalizedMSD       ();
  ErrVal            xReconstructBetweenPasses     (IntFrame*            pcRecResidual);
  ErrVal			CheckRD                       (UInt                 uiWrittenBits);
  //}}Quality level estimation and modified truncation- JVTO044 and m12007

private:
  MbSymbolWriteIf*  m_pcSymbolWriter;
  ControlMngH264AVCEncoder* m_pcControlMng;
  MbEncoder*        m_pcMbEncoder;

  Int               m_iRemainingTCoeff;
  Double            m_dLambda;
  Int               m_iMaxQpDelta;
  SliceHeader*      m_pcSliceHeader;
  SliceType         m_eSliceType;
  UInt              m_uiFirstMbInSlice;
  UInt              m_uiNumMbsInSlice;

  IntFrame*         m_pcOrgResidual;

  
  Bool              m_bTraceEnable;
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  IntFrame*         m_pcOrgResidualFT;
  Double            m_dScalFactorFT;
  FILE *            m_pDistoFile;
  FILE *            m_pRateFile;
  UInt				m_uiNumBitsForRDPoint;
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_FGSSUBBANDENCODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_)
