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





#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/ParameterSetMng.h"


H264AVC_NAMESPACE_BEGIN

ParameterSetMng::ParameterSetMng() :
  m_uiActiveSPSId( MSYS_UINT_MAX ),
  m_uiActivePPSId( MSYS_UINT_MAX )
{
  m_cSPSBuf.clear();
  m_cPPSBuf.clear();
}

ErrVal ParameterSetMng::create( ParameterSetMng*& rpcParameterSetMng )
{
  rpcParameterSetMng = new ParameterSetMng;

  ROT( NULL == rpcParameterSetMng );

  return Err::m_nOK;
}


ErrVal ParameterSetMng::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal ParameterSetMng::uninit()
{
  for( UInt uiSPSId = 0; uiSPSId < m_cSPSBuf.size(); uiSPSId++)
  {
    RNOK( xDestroySPS( uiSPSId ) )
  }

  for( UInt uiPPSId = 0; uiPPSId < m_cPPSBuf.size(); uiPPSId++)
  {
    RNOK( xDestroyPPS( uiPPSId ) )
  }

  return Err::m_nOK;
}


ErrVal ParameterSetMng::get( SequenceParameterSet*& rpcSPS, UInt uiSPSId)
{
  RNOK( m_cSPSBuf.get( rpcSPS, uiSPSId) );

  ROT( NULL == rpcSPS);

  m_uiActiveSPSId = uiSPSId;
  return Err::m_nOK;
}



ErrVal ParameterSetMng::store( SequenceParameterSet* pcSPS )
{
  ROT( NULL == pcSPS );

  UInt uiSPSId = pcSPS->getSeqParameterSetId();

  ROF( m_cSPSBuf.isValidOffset(uiSPSId) )

  RNOK( xDestroySPS( uiSPSId ) );

  m_cSPSBuf.set( uiSPSId, pcSPS );

  return Err::m_nOK;
}


ErrVal ParameterSetMng::xDestroySPS( UInt uiSPSId )
{
  ROF( m_cSPSBuf.isValidOffset(uiSPSId) )

  RNOK( m_cSPSBuf.get( uiSPSId )->destroy() );

  m_cSPSBuf.set( uiSPSId, NULL );

  return Err::m_nOK;
}



ErrVal ParameterSetMng::get( PictureParameterSet*& rpcPPS, UInt uiPPSId )
{
  RNOK( m_cPPSBuf.get( rpcPPS, uiPPSId ) );

  ROT( NULL == rpcPPS );

  m_uiActivePPSId = uiPPSId;
  return Err::m_nOK;
}

ErrVal ParameterSetMng::store( PictureParameterSet* pcPPS )
{
  ROT( NULL == pcPPS );
  UInt uiPPSId = pcPPS->getPicParameterSetId();
  ROF( m_cPPSBuf.isValidOffset( uiPPSId ) )

  RNOK( xDestroyPPS( uiPPSId ) );

  m_cPPSBuf.set( uiPPSId, pcPPS );

  return Err::m_nOK;
}


ErrVal ParameterSetMng::xDestroyPPS(UInt uiPPSId)
{
  PictureParameterSet* pcPPS;

  RNOK( m_cPPSBuf.get( pcPPS, uiPPSId ) );

  ROTRS( NULL == pcPPS, Err::m_nOK );

  RNOK( pcPPS->destroy() );

  m_cPPSBuf.set( uiPPSId, NULL );
  return Err::m_nOK;
}


ErrVal ParameterSetMng::setParamterSetList( std::list<SequenceParameterSet*>& rcSPSList, std::list<PictureParameterSet*>& rcPPSList) const
{
  {
    // collect valid sps
    rcSPSList.clear();
    const UInt uiMaxIndex = m_cSPSBuf.size();
    for( UInt uiIndex = 0; uiIndex < uiMaxIndex; uiIndex++ )
    {
      if( NULL != m_cSPSBuf.get( uiIndex ) )
      {
        rcSPSList.push_back( m_cSPSBuf.get( uiIndex ) );
      }
    }
  }
  {
    // collect valid pps
    rcPPSList.clear();
    const UInt uiMaxIndex = m_cPPSBuf.size();
    for( UInt uiIndex = 0; uiIndex < uiMaxIndex; uiIndex++ )
    {
      if( NULL != m_cPPSBuf.get( uiIndex ) )
      {
        rcPPSList.push_back( m_cPPSBuf.get( uiIndex ) );
      }
    }
  }
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END

