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




#if !defined(AFX_CREATERH264AVCENCODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_)
#define AFX_CREATERH264AVCENCODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



H264AVC_NAMESPACE_BEGIN



class H264AVCEncoder;
class MbData;
class Frame;
class FrameMng;
class BitWriteBuffer;
class Transform;
class YuvBufferCtrl;
class QuarterPelFilter;
class ParameterSetMng;
class LoopFilter;
class SampleWeighting;
class PocCalculator;

class BitCounter;
class SliceEncoder;
class UvlcWriter;
class MbCoder;
class MbEncoder;
class IntraPredictionSearch;
class CodingParameter;
class CabacWriter;
class NalUnitEncoder;
class Distortion;
class MotionEstimation;
class MotionEstimationQuarterPel;
class RateDistortion;
class RateDistortionRateConstraint;
class History;
class MCTFEncoder;
class XDistortion;
class ControlMngH264AVCEncoder;
class ReconstructionBypass;
class RQFGSEncoder;



class H264AVCENCODERLIB_API CreaterH264AVCEncoder 
{
protected:
	CreaterH264AVCEncoder();
	virtual ~CreaterH264AVCEncoder();

public:
  static ErrVal create  ( CreaterH264AVCEncoder*& rpcCreaterH264AVCEncoder );
  ErrVal        destroy ();

  ErrVal init               ( CodingParameter*    pcCodingParameter);
  ErrVal uninit             ();
  ErrVal writeParameterSets ( ExtBinDataAccessor* pcExtBinDataAccessor,
                              Bool&               rbMoreSets );
  
  ErrVal process(  ExtBinDataAccessorList&  rcExtBinDataAccessorList, 
                   PicBuffer*               apcOriginalPicBuffer    [MAX_LAYERS],
                   PicBuffer*               apcReconstructPicBuffer [MAX_LAYERS],
                   PicBufferList*           apcPicBufferOutputList,
                   PicBufferList*           apcPicBufferUnusedList );

  ErrVal finish (  ExtBinDataAccessorList&  rcExtBinDataAccessorList, 
                   PicBufferList*           apcPicBufferOutputList,
                   PicBufferList*           apcPicBufferUnusedList,
                   UInt&                    ruiNumCodedFrames,
                   Double&                  rdHighestLayerOutputRate );


  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  ErrVal writeQualityLevelInfosSEI(ExtBinDataAccessor* pcExtBinDataAccessor,
                                   UInt* uiaQualityLevel, 
                                   UInt *uiaDelta, 
                                   UInt uiNumLevels, 
                                   UInt uiLayer);
  //}}Quality level estimation and modified truncation- JVTO044 and m12007

  Bool getScalableSeiMessage ( Void );
	Void SetVeryFirstCall ( Void );

protected:
  ErrVal xCreateEncoder();

protected:
  H264AVCEncoder*           m_pcH264AVCEncoder;
  RQFGSEncoder*             m_pcRQFGSEncoder;
  FrameMng*                 m_pcFrameMng;

  MCTFEncoder*              m_apcMCTFEncoder          [MAX_LAYERS];
  SliceEncoder*             m_pcSliceEncoder;
  ControlMngH264AVCEncoder* m_pcControlMng;
  BitWriteBuffer*           m_pcBitWriteBuffer;
  BitCounter*               m_pcBitCounter;
  NalUnitEncoder*           m_pcNalUnitEncoder;

  UvlcWriter*               m_pcUvlcWriter;
  UvlcWriter*               m_pcUvlcTester;
  MbCoder*                  m_pcMbCoder;
  LoopFilter*               m_pcLoopFilter;
  MbEncoder*                m_pcMbEncoder;
  Transform*                m_pcTransform;
  IntraPredictionSearch*    m_pcIntraPrediction;
  YuvBufferCtrl*            m_apcYuvFullPelBufferCtrl [MAX_LAYERS];
  YuvBufferCtrl*            m_apcYuvHalfPelBufferCtrl [MAX_LAYERS];
  QuarterPelFilter*         m_pcQuarterPelFilter;
  CodingParameter*          m_pcCodingParameter;
  ParameterSetMng*          m_pcParameterSetMng;
  PocCalculator*            m_apcPocCalculator        [MAX_LAYERS];
  SampleWeighting*          m_pcSampleWeighting;
  CabacWriter*              m_pcCabacWriter;
  XDistortion*              m_pcXDistortion;
  MotionEstimation*         m_pcMotionEstimation;
  RateDistortion*           m_pcRateDistortion;
  History*                  m_pcHistory;
  ReconstructionBypass*     m_pcReconstructionBypass;

  Bool                      m_bTraceEnable;

};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_CREATERH264AVCENCODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_)
