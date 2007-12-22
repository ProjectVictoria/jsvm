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




#if !defined(AFX_INTYUVPICBUFFER_H__5AB262CF_4876_47A2_97A8_5500F7416A8C__INCLUDED_)
#define AFX_INTYUVPICBUFFER_H__5AB262CF_4876_47A2_97A8_5500F7416A8C__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/MbTransformCoeffs.h"
#include "DownConvert.h"



H264AVC_NAMESPACE_BEGIN



class YuvMbBuffer;
class MbDataCtrl;



class H264AVCCOMMONLIB_API YuvPicBuffer
{
public:
	YuvPicBuffer         ( YuvBufferCtrl& rcYuvBufferCtrl, PicType ePicType );
	virtual ~YuvPicBuffer();

	const Int     getLStride    ()                const { return m_rcBufferParam.getStride()   ; }
  const Int     getCStride    ()                const { return m_rcBufferParam.getStride()>>1; }
  
  XPel*         getLumBlk     ()                      { return m_pPelCurrY; }
  XPel*         getCbBlk      ()                      { return m_pPelCurrU; }
  XPel*         getCrBlk      ()                      { return m_pPelCurrV; }

  Void          set4x4Block   ( LumaIdx cIdx )
  {
    m_pPelCurrY = m_pucYuvBuffer + m_rcBufferParam.getYBlk( cIdx );
    m_pPelCurrU = m_pucYuvBuffer + m_rcBufferParam.getUBlk( cIdx );
    m_pPelCurrV = m_pucYuvBuffer + m_rcBufferParam.getVBlk( cIdx );
  }

  // Hanke@RWTH
  Bool          isCurr4x4BlkNotZero ( LumaIdx cIdx );
  Bool          isLeft4x4BlkNotZero ( LumaIdx cIdx );
  Bool          isAbove4x4BlkNotZero( LumaIdx cIdx );
 
  XPel*         getYBlk       ( LumaIdx cIdx )        { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getYBlk( cIdx ); }
  XPel*         getUBlk       ( LumaIdx cIdx )        { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getUBlk( cIdx ); }
  XPel*         getVBlk       ( LumaIdx cIdx )        { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getVBlk( cIdx ); }
  
  XPel*         getMbLumAddr  ()                const { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbLum(); }
  XPel*         getMbCbAddr   ()                const { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbCb (); }
  XPel*         getMbCrAddr   ()                const { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbCr (); }

  XPel*         getMbLumAddr  ( UInt uiX, UInt uiY, Bool bMbAff ) const { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcYuvBufferCtrl.getMbLum( m_ePicType, uiY, uiX, bMbAff ); }
  XPel*         getMbCbAddr   ( UInt uiX, UInt uiY, Bool bMbAff ) const { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcYuvBufferCtrl.getMbCb ( m_ePicType, uiY, uiX, bMbAff ); }
  XPel*         getMbCrAddr   ( UInt uiX, UInt uiY, Bool bMbAff ) const { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcYuvBufferCtrl.getMbCr ( m_ePicType, uiY, uiX, bMbAff ); }

  const Int     getLWidth     ()                const { return m_rcBufferParam.getWidth ();    }
  const Int     getLHeight    ()                const { return m_rcBufferParam.getHeight();    }
  const Int     getCWidth     ()                const { return m_rcBufferParam.getWidth ()>>1; }
  const Int     getCHeight    ()                const { return m_rcBufferParam.getHeight()>>1; }
  
  const Int     getLXMargin   ()                const { return m_rcYuvBufferCtrl.getXMargin(); }
  const Int     getLYMargin   ()                const { return m_rcYuvBufferCtrl.getYMargin(); }
  const Int     getCXMargin   ()                const { return m_rcYuvBufferCtrl.getXMargin()>>1; }
  const Int     getCYMargin   ()                const { return m_rcYuvBufferCtrl.getYMargin()>>1; }

  Bool          isValid       ()                      { return NULL != m_pucYuvBuffer; }
  XPel*         getLumOrigin  ()                const { return m_pucYuvBuffer + m_rcYuvBufferCtrl.getLumOrigin( m_ePicType ); }
  XPel*         getCbOrigin   ()                const { return m_pucYuvBuffer + m_rcYuvBufferCtrl.getCbOrigin ( m_ePicType ); }
  XPel*         getCrOrigin   ()                const { return m_pucYuvBuffer + m_rcYuvBufferCtrl.getCrOrigin ( m_ePicType ); }
  
  ErrVal        loadFromPicBuffer       ( PicBuffer*        pcPicBuffer );
  ErrVal        storeToPicBuffer        ( PicBuffer*        pcPicBuffer );

  ErrVal        loadBuffer              ( YuvMbBuffer*   pcYuvMbBuffer );
  ErrVal        loadBuffer_MbAff        ( YuvMbBuffer *pcYuvMbBuffer   ,  UInt uiMask) ;//TMM_INTERLACE

  ErrVal        fillMargin              ();

  ErrVal        prediction              ( YuvPicBuffer*  pcSrcYuvPicBuffer, YuvPicBuffer*  pcMCPYuvPicBuffer );
  ErrVal        update                  ( YuvPicBuffer*  pcSrcYuvPicBuffer, YuvPicBuffer*  pcMCPYuvPicBuffer, UInt uiShift );
  ErrVal        inversePrediction       ( YuvPicBuffer*  pcSrcYuvPicBuffer, YuvPicBuffer*  pcMCPYuvPicBuffer );
  ErrVal        inverseUpdate           ( YuvPicBuffer*  pcSrcYuvPicBuffer, YuvPicBuffer*  pcMCPYuvPicBuffer, UInt uiShift );

  ErrVal        update                  ( YuvPicBuffer*  pcSrcYuvPicBuffer, YuvPicBuffer*  pcMCPYuvPicBuffer0, YuvPicBuffer*  pcMCPYuvPicBuffer1 );
  ErrVal        inverseUpdate           ( YuvPicBuffer*  pcSrcYuvPicBuffer, YuvPicBuffer*  pcMCPYuvPicBuffer0, YuvPicBuffer*  pcMCPYuvPicBuffer1 );

  ErrVal        copy                    ( YuvPicBuffer*  pcSrcYuvPicBuffer );
  //JVT-U106 Behaviour at slice boundaries{
  ErrVal        copyMask                    ( YuvPicBuffer*  pcSrcYuvPicBuffer,Int**ppiMaskL,Int**ppiMaskC );
  ErrVal        copyPortion                  ( YuvPicBuffer*  pcSrcYuvPicBuffer);
  //JVT-U106 Behaviour at slice boundaries}
  ErrVal        copyMSB8BitsMB          ( YuvPicBuffer*  pcSrcYuvPicBuffer );
  ErrVal        setZeroMB               ();

  ErrVal        subtract                ( YuvPicBuffer*  pcSrcYuvPicBuffer0, YuvPicBuffer* pcSrcYuvPicBuffer1 );
  ErrVal        add                     ( YuvPicBuffer*  pcSrcYuvPicBuffer );

  ErrVal        addWeighted             ( YuvPicBuffer*  pcSrcYuvPicBuffer, Double dWeight );

  ErrVal        dumpLPS                 ( FILE* pFile );
  ErrVal        dumpHPS                 ( FILE* pFile, MbDataCtrl* pcMbDataCtrl );

  ErrVal        init                    ( XPel*&            rpucYuvBuffer );
  ErrVal        uninit                  ();
  Void          setZero                 ();
  ErrVal        clip                    ();

  ErrVal        loadFromFile8Bit        ( FILE* pFile );

  ErrVal        getSSD                  ( Double& dSSDY, Double& dSSDU, Double& dSSDV, PicBuffer* pcOrgPicBuffer );

    // TMM_ESS {
    ErrVal        upsampleResidual        ( DownConvert& rcDownConvert, ResizeParameters *pcParameters, MbDataCtrl* pcMbDataCtrl, Bool bClip );
    ErrVal        upsample                ( DownConvert& rcDownConvert, ResizeParameters *pcParameters, Bool bClip );
    // TMM_ESS }
  ErrVal        setNonZeroFlags         ( UShort* pusNonZeroFlags, UInt uiStride );

  ErrVal        clear();
  ErrVal        clearCurrMb();


  YuvBufferCtrl& getBufferCtrl()  { return m_rcYuvBufferCtrl; }
  XPel*          getBuffer    ()  { return m_pucYuvBuffer; }

  ErrVal loadBufferAndFillMargin( YuvPicBuffer *pcSrcYuvPicBuffer );
  ErrVal loadBuffer             ( YuvPicBuffer *pcSrcYuvPicBuffer );

	//--
	// JVT-R057 LA-RDO{
	YuvBufferCtrl& getYuvBufferCtrl(){ return m_rcYuvBufferCtrl;}
	// JVT-R057 LA-RDO} 
	//JVT-X046 {
	void   setMBZero( UInt uiMBY, UInt uiMBX );
  ErrVal predictionSlices(YuvPicBuffer*  pcSrcYuvPicBuffer, YuvPicBuffer*  pcMCPYuvPicBuffer, UInt uiMbY, UInt uiMbX );
  ErrVal inversepredictionSlices(YuvPicBuffer*  pcSrcYuvPicBuffer, YuvPicBuffer*  pcMCPYuvPicBuffer, UInt uiMbY, UInt uiMbX );
  ErrVal copyMb(YuvPicBuffer* pcSrcYuvPicBuffer,UInt uiMbY, UInt uiMbX);
	ErrVal copyMB( YuvPicBuffer* pcSrcYuvPicBuffer, UInt uiMbAddress)
	{
		Int iSrcStride = pcSrcYuvPicBuffer->getLStride();
		Int iDesStride = getLStride();
		UInt uiWidth     = pcSrcYuvPicBuffer->getLWidth ()/16;
		UInt uiXPos,uiYPos;
		uiXPos = uiMbAddress % uiWidth;
		uiYPos = (uiMbAddress)/uiWidth;
		pcSrcYuvPicBuffer->getYuvBufferCtrl().initMb(uiYPos,uiXPos,false);
		getYuvBufferCtrl().initMb(uiYPos,uiXPos,false);
		XPel* pSrc = pcSrcYuvPicBuffer->getMbLumAddr();
		XPel* pDes = getMbLumAddr();
		
		UInt y,x;
		for ( y = 0; y < 16; y++ )
		{
			for ( x = 0; x < 16; x++ )
				pDes[x]=pSrc[x];
			pSrc += iSrcStride;
			pDes += iDesStride;
		}
		iSrcStride >>= 1;
		iDesStride >>= 1;

		pSrc = pcSrcYuvPicBuffer->getMbCbAddr();
		pDes = getMbCbAddr();

		for ( y = 0; y < 8; y++ )
		{
			for ( x = 0; x < 8; x++ )
				pDes[x] = pSrc[x];
			pSrc += iSrcStride;
			pDes += iDesStride;
		}

		pSrc = pcSrcYuvPicBuffer->getMbCrAddr();
		pDes = getMbCrAddr();
		for ( y = 0; y < 8; y++ )
		{

			for ( x = 0; x < 8; x++)
				pDes[x] = pSrc[x];
			pSrc += iSrcStride;
			pDes += iDesStride;
		}
		return Err::m_nOK;
	}
	ErrVal copySlice( YuvPicBuffer* pcSrcYuvPicBuffer,UInt uiFirstMB,UInt uiLastMB)
	{
		for (UInt uiMbAddress=uiFirstMB;uiMbAddress<uiLastMB;uiMbAddress++)
		{
			RNOK(copyMB(pcSrcYuvPicBuffer,uiMbAddress));
		}
		return Err::m_nOK;
	}
  //JVT-X046 }
protected:
  Void xCopyFillPlaneMargin( XPel *pucSrc, XPel *pucDest, Int iHeight, Int iWidth, Int iStride, Int iXMargin, Int iYMargin );
  Void xCopyPlane          ( XPel *pucSrc, XPel *pucDest, Int iHeight, Int iWidth, Int iStride );
  Void xFillPlaneMargin     ( XPel *pucDest, Int iHeight, Int iWidth, Int iStride, Int iXMargin, Int iYMargin );

protected:
  const YuvBufferCtrl::YuvBufferParameter&  m_rcBufferParam;
  YuvBufferCtrl&                            m_rcYuvBufferCtrl;

  XPel*           m_pPelCurrY;
  XPel*           m_pPelCurrU;
  XPel*           m_pPelCurrV;
  
  XPel*           m_pucYuvBuffer;
  XPel*           m_pucOwnYuvBuffer;

  const PicType   m_ePicType;
private:
  YuvPicBuffer();
};



H264AVC_NAMESPACE_END



#endif // !defined(AFX_INTYUVPICBUFFER_H__5AB262CF_4876_47A2_97A8_5500F7416A8C__INCLUDED_)
