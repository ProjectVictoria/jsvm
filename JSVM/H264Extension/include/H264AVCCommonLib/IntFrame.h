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




#if !defined(AFX_INTFRAME_H__98AFB9AC_5EE3_45A9_B09B_859511AC9090__INCLUDED_)
#define AFX_INTFRAME_H__98AFB9AC_5EE3_45A9_B09B_859511AC9090__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/IntYuvPicBuffer.h"


H264AVC_NAMESPACE_BEGIN


class QuarterPelFilter;
class MbDataCtrl;


class H264AVCCOMMONLIB_API IntFrame
{
public:
	IntFrame                ( YuvBufferCtrl&    rcYuvFullPelBufferCtrl,
                            YuvBufferCtrl&    rcYuvHalfPelBufferCtrl );
	virtual ~IntFrame       ();

  ErrVal  init            ( Bool              bHalfPel = false );
  ErrVal  initHalfPel     ();
  ErrVal  uninit          ();
  ErrVal  uninitHalfPel   ();

  ErrVal  load            ( PicBuffer*        pcPicBuffer );
  ErrVal  store           ( PicBuffer*        pcPicBuffer );
  ErrVal  extendFrame     ( QuarterPelFilter* pcQuarterPelFilter );

  
  ErrVal clip()
  {
    RNOK( getFullPelYuvBuffer()->clip() );
    return Err::m_nOK;
  }
  
  ErrVal prediction       ( IntFrame* pcMCPFrame, IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->prediction       ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame->getFullPelYuvBuffer() ) );
    return Err::m_nOK;
  }
  
  ErrVal update           ( IntFrame* pcMCPFrame, IntFrame* pcSrcFrame, UInt uiShift )
  {
    RNOK( getFullPelYuvBuffer()->update           ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame->getFullPelYuvBuffer(), uiShift ) );
    return Err::m_nOK;
  }
  
  ErrVal inverseUpdate    ( IntFrame* pcMCPFrame, IntFrame* pcSrcFrame, UInt uiShift )
  {
    RNOK( getFullPelYuvBuffer()->inverseUpdate    ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame->getFullPelYuvBuffer(), uiShift ) );
    return Err::m_nOK;
  }

  ErrVal adaptiveWeighting( UShort*   pusUpdateWeights )
  {
    RNOK( getFullPelYuvBuffer()->adaptiveWeighting( pusUpdateWeights ) );
    return Err::m_nOK;
  }
  
  ErrVal update           ( IntFrame* pcMCPFrame0, IntFrame* pcMCPFrame1, IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->update           ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame0->getFullPelYuvBuffer(), pcMCPFrame1->getFullPelYuvBuffer() ) );
    return Err::m_nOK;
  }
  
  ErrVal inverseUpdate    ( IntFrame* pcMCPFrame0, IntFrame* pcMCPFrame1, IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->inverseUpdate    ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame0->getFullPelYuvBuffer(), pcMCPFrame1->getFullPelYuvBuffer() ) );
    return Err::m_nOK;
  }

  ErrVal inversePrediction( IntFrame* pcMCPFrame, IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->inversePrediction( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame->getFullPelYuvBuffer() ) );
    return Err::m_nOK;
  }


  ErrVal  copyAll     ( IntFrame* pcSrcFrame )
  {
    m_uiBandType  = pcSrcFrame->m_uiBandType;
    m_uiRecLayer  = pcSrcFrame->m_uiRecLayer;
    m_iPOC        = pcSrcFrame->m_iPOC;
    RNOK( m_cFullPelYuvBuffer.copy( &pcSrcFrame->m_cFullPelYuvBuffer ) );
  
    return Err::m_nOK;
  }

  ErrVal  copy        ( IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->copy( pcSrcFrame->getFullPelYuvBuffer()) );
    return Err::m_nOK;
  }
  
  ErrVal  subtract    ( IntFrame* pcSrcFrame0, IntFrame* pcSrcFrame1 )
  {
    RNOK( getFullPelYuvBuffer()->subtract( pcSrcFrame0->getFullPelYuvBuffer(), pcSrcFrame1->getFullPelYuvBuffer() ) );
    return Err::m_nOK;
  }
  
  ErrVal  add         ( IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->add ( pcSrcFrame->getFullPelYuvBuffer()) );
    return Err::m_nOK;
  }
  
  ErrVal  setZero     ()
  {
    getFullPelYuvBuffer()->setZero();
    return Err::m_nOK;
  }

  ErrVal  setNonZeroFlags( UShort* pusNonZeroFlags, UInt uiStride )
  {
    return getFullPelYuvBuffer()->setNonZeroFlags( pusNonZeroFlags, uiStride );
  }

  ErrVal getSSD( Double& dSSDY, Double& dSSDU, Double& dSSDV, PicBuffer* pcOrgPicBuffer )
  {
    RNOK( m_cFullPelYuvBuffer.getSSD( dSSDY, dSSDU, dSSDV, pcOrgPicBuffer ) );
    return Err::m_nOK;
  }

  ErrVal dump( FILE* pFile, MbDataCtrl* pcMbDataCtrl )
  {
    if( m_uiBandType != 0 )
    {
      RNOK( getFullPelYuvBuffer()->dumpHPS( pFile, pcMbDataCtrl ) );
    }
    else
    {
      RNOK( getFullPelYuvBuffer()->dumpLPS( pFile ) );
    }
    return Err::m_nOK;
  }


  ErrVal loadFromFile8BitAndFillMargin( FILE* pFILE )
  {
    RNOK( getFullPelYuvBuffer()->loadFromFile8Bit( pFILE ) );
    RNOK( getFullPelYuvBuffer()->fillMargin      () );
    return Err::m_nOK;
  }


  ErrVal upsample     ( DownConvert& rcDownConvert, UInt uiStages, Bool bClip )
  {
    RNOK( getFullPelYuvBuffer()->upsample( rcDownConvert, uiStages, bClip ) );
    return Err::m_nOK;
  }

  ErrVal upsampleResidual( DownConvert& rcDownConvert, UInt uiStages, MbDataCtrl* pcMbDataCtrl, Bool bClip )
  {
    RNOK( getFullPelYuvBuffer()->upsampleResidual( rcDownConvert, uiStages, pcMbDataCtrl, bClip ) );
    return Err::m_nOK;
  }

  IntYuvPicBuffer*  getFullPelYuvBuffer     ()        { return &m_cFullPelYuvBuffer; }
  IntYuvPicBuffer*  getHalfPelYuvBuffer     ()        { return &m_cHalfPelYuvBuffer; }


  Void  setBandType   ( UInt ui ) { m_uiBandType = ui;    }
  UInt  getBandType   ()  const   { return m_uiBandType;  }

  UInt  getRecLayer   ()  const   { return m_uiRecLayer;  }
  Void  addRecLayer   ()          { m_uiRecLayer++;       }
  Void  initRecLayer  ()          { m_uiRecLayer = 0;     }
  Void  setRecLayer   ( UInt ui ) { m_uiRecLayer = ui;    }

  Int   getPOC()          const   { return m_iPOC; }
  Void  setPOC( Int iPoc)         { m_iPOC = iPoc; }

  Void  setValid  ()  { m_bValid = true; }
  Void  setUnvalid()  { m_bValid = false; }
  Bool  isHalfPel()   { return m_bHalfPel; }
  Bool  isValid()     { return m_bValid; }

  Bool  isExtended () { return m_bExtended; }
  Void  clearExtended() { m_bExtended = false; }


protected:
  IntYuvPicBuffer m_cFullPelYuvBuffer;
  IntYuvPicBuffer m_cHalfPelYuvBuffer;
  
  UInt            m_uiBandType;
  UInt            m_uiRecLayer;
  Int             m_iPOC;
  Bool            m_bValid;
  Bool            m_bHalfPel;
  Bool            m_bExtended;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_INTFRAME_H__98AFB9AC_5EE3_45A9_B09B_859511AC9090__INCLUDED_)
