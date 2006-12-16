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




#if !defined(AFX_FGSSUBBANDDECODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_)
#define AFX_FGSSUBBANDDECODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MbSymbolReadIf.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/Transform.h"

#include "H264AVCCommonLib/FGSCoder.h"

H264AVC_NAMESPACE_BEGIN


class YuvBufferCtrl;
class Transform;
class MbDataCtrl;
class CabacReader;
class UvlcReader;
class MbParser;
class MbDecoder;



class RQFGSDecoder
  : public FGSCoder
{
private:
  class ReadStop
  {
  };

protected:
  RQFGSDecoder         ();
  virtual ~RQFGSDecoder();

public:
  static ErrVal     create                ( RQFGSDecoder*&              rpcFGSSubbandDecoder );
  ErrVal            destroy               ();

  ErrVal            init                  ( YuvBufferCtrl**             apcYuvFullPelBufferCtrl,
                                            Transform*                  pcTransform,
                                            MbParser*                   pcMbParser,
                                            MbDecoder*                  pcMbDecoder,
                                            UvlcReader*                 pcUvlcReader,
                                            CabacReader*                pcCabacReader );
  ErrVal            uninit                ();

  ErrVal            initPicture           ( SliceHeader*                pcSliceHeader,
                                            MbDataCtrl*                 pcCurrMbDataCtrl );
  ErrVal            decodeNextLayer       ( SliceHeader*                pcSliceHeader );
  ErrVal            reconstruct           ( IntFrame*                   pcRecResidual );
  ErrVal            finishPicture         ();

  Bool              isInitialized         ()    { return m_bPicInit; }
  Bool              isFinished            ()    { return m_bPicFinished; }
  Bool              changed               ()    { return m_bPicChanged; }
  SliceHeader*      getSliceHeader        ()    { return m_pcCurrSliceHeader; }

  Bool        isFirstFGS      ()    { return m_bFirstFGS; }
  Void        SetIsFirstFGS    (Bool b) { m_bFirstFGS = b; }

  // ICU/ETRI FGS_MOT_USE
  Bool        isUseFGSMotion      (UInt uiLayer)    { return m_bFGSMotionUse[uiLayer]; }
  Void        SetUseFGSMotion      (UInt uiLayer, Bool b) { m_bFGSMotionUse[uiLayer] = b; }

private:
  ErrVal            xDecodeLumaCbpVlc     ( UInt                        uiCurrMbIdxX,
                                            UInt                        uiCurrMbIdxY );
  ErrVal            xDecodeChromaCbpVlc   ( UInt                        uiCurrMbIdxX,
                                            UInt                        uiCurrMbIdxY );
  ErrVal            xScaleBaseLayerCoeffs ();
  ErrVal            xDecodingFGS                  ( SliceHeader*        pcSliceHeader  );

  ErrVal            xDecodeMotionData             ( UInt                uiMbYIdx,
                                                    UInt                uiMbXIdx );
  ErrVal			      xDecodingFGSBlock             ( SliceHeader*    pcSliceHeader 	);
  ErrVal            xResidualBlock                ( MbDataAccess&   rcMbDataAccess,
                                                    MbDataAccess&   rcMbDataAccessBase,
                                                    ResidualMode    eResidualMode,
                                                    UInt            uiStride,
                                                    UInt            uiBlkIdx,
                                                    UInt&           uiBcbp,
                                                    Bool            bDecodeBcbpInside,
                                                    Int*            piMaxPos, 
                                                    RefCtx*         pcRefCtx, 
                                                    UInt&           ruiNumFrags,
                                                    UInt&           ruiCoeffsDecoded );

  ErrVal            xResidualBlock                ( MbDataAccess&   rcMbDataAccess,
                                                    MbDataAccess&   rcMbDataAccessBase,
                                                    LumaIdx         cIdx, 
                                                    ResidualMode    eResidualMode,
                                                    UInt            uiStride,
                                                    Int*            piMaxPos, 
                                                    UInt&           ruiNumFrags,
                                                    MbFGSCoefMap &  rcMbFGSCoefMap, 
                                                    UInt&           ruiCoeffsDecoded );

  ErrVal            xResidualBlock                ( MbDataAccess&   rcMbDataAccess,
                                                    MbDataAccess&   rcMbDataAccessBase,
                                                    ChromaIdx       cIdx, 
                                                    ResidualMode    eResidualMode,
                                                    Int*            piMaxPos, 
                                                    UInt&           ruiNumFrags,
                                                    MbFGSCoefMap &  rcMbFGSCoefMap, 
                                                    UInt&           ruiCoeffsDecoded );

  ErrVal            xDecodeNewCoefficientLumaMb   ( MbDataAccess*       pcMbDataAccessBL,
                                                    MbDataAccess*       pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    UInt                uiMbYIdx,
                                                    UInt                uiMbXIdx,
                                                    Int&                riLastQp,
                                                    Int                 iLumaScanIdx,
                                                    UInt                uiMaxPosLuma );

  ErrVal            xDecodeNewCoefficientLuma     ( MbDataAccess*       pcMbDataAccessBL,
                                                    MbDataAccess*       pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    const S4x4Idx      &rcIdx );
  ErrVal            xDecodeNewCoefficientChromaDC ( MbDataAccess*       pcMbDataAccessBL,
                                                    MbDataAccess*       pcMbDataAccessEL,
                                                    MbFGSCoefMap&       rcMbFGSCoefMap,
                                                    const CPlaneIdx    &rcCPlaneIdx,
                                                    Int&                riLastQP,
                                                    UInt                uiChromaScanIndex );
  ErrVal            xDecodeNewCoefficientChromaAC ( MbDataAccess*       pcMbDataAccessBL,
                                                    MbDataAccess*       pcMbDataAccessEL,
                                                    MbFGSCoefMap&       rcMbFGSCoefMap,
                                                    const CIdx         &rcCIdx,
                                                    Int&                riLastQP,
                                                    UInt                uiChromaScanIndex );
  ErrVal            xDecodeCoefficientLumaRef     ( MbDataAccess*       pcMbDataAccessBL,
                                                    MbDataAccess*       pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    const S4x4Idx      &rcIdx,
                                                    UInt                uiScanIndex );
  ErrVal            xDecodeCoefficientChromaDCRef ( MbDataAccess       *pcMbDataAccessBL,
                                                    MbDataAccess       *pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    const CPlaneIdx    &rcCPlaneIdx,
                                                    UInt                uiDCIdx );
  ErrVal            xDecodeCoefficientChromaACRef ( MbDataAccess* pcMbDataAccessBL,
                                                    MbDataAccess* pcMbDataAccessEL,
                                                    MbFGSCoefMap  &rcMbFGSCoefMap,
                                                    const CIdx    &rcCIdx,
                                                    UInt                uiScanIdx );
  ErrVal            xInitializeMacroblockQPs      ();


  ErrVal            xDecodeMbHeader               ( MbDataAccess*       pcMbDataAccessBL,
                                                    MbDataAccess*       pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    Int&                riLastQp );

private:
  MbSymbolReadIf*   m_pcSymbolReader;
  UvlcReader*       m_pcUvlcReader;
  CabacReader*      m_pcCabacReader;
  MbParser*         m_pcMbParser;
  MbDecoder*        m_pcMbDecoder;

  Bool              m_bPicChanged;
  Bool              m_bPicFinished;
  SliceHeader*      m_pcCurrSliceHeader;

  UInt              m_auiScanPosVectLuma    [16];
  UInt              m_auiScanPosVectChromaDC[ 4];
  UInt              m_uiLastMbNum;
  Bool        m_bFirstFGS;

  // ICU/ETRI FGS_MOT_USE
  Bool        m_bFGSMotionUse[8];
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_FGSSUBBANDDECODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_)
