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



#if !defined(AFX_SCALINGMATRIX_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
#define AFX_SCALINGMATRIX_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_


#include "H264AVCCommonLib/HeaderSymbolReadIf.h"
#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"


H264AVC_NAMESPACE_BEGIN


#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API ScalingMatrix
{
public:
  template< UInt uiBufSize >
  class ScalingList : public StatBuf<UChar,uiBufSize>
  {
  public:
    ScalingList ();
    ErrVal        read      ( HeaderSymbolReadIf*   pcReadIf,
                              const UChar*          pucScan,
                              Bool&                 rbUseDefaultScalingMatrixFlag );
    ErrVal        write     ( HeaderSymbolWriteIf*  pcWriteIf,
                              const UChar*          pucScan,
                              const Bool            bUseDefaultScalingMatrixFlag  ) const;
  };

  template< UInt uiBufSize >
  class SeqScaling
  {
  public:
    SeqScaling();
    ErrVal        read      ( HeaderSymbolReadIf*   pcReadIf,
                              const UChar*          pucScan );
    ErrVal        write     ( HeaderSymbolWriteIf*  pcWriteIf,
                              const UChar*          pucScan )   const;
    const UChar*  getMatrix ()                                  const;
  
  private:
    Bool                    m_bScalingListPresentFlag;
    Bool                    m_bUseDefaultScalingMatrixFlag;
    ScalingList<uiBufSize>  m_aiScalingList;
  };

public:
  ScalingMatrix();

  ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf,
                        Bool                  bRead8x8 );
  ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf,
                        Bool                  bWrite8x8 ) const;
  const UChar*  get   ( UInt                  uiMatrix )  const;

private:
  StatBuf<SeqScaling<16>,6> m_acScalingMatrix4x4;
  StatBuf<SeqScaling<64>,2> m_acScalingMatrix8x8;
};


#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif





template<UInt uiBufSize>
ScalingMatrix::ScalingList<uiBufSize>::ScalingList()
{
  this->setAll(16); // leszek: C++ standard compliance
}


template<UInt uiBufSize>
ErrVal
ScalingMatrix::ScalingList<uiBufSize>::read( HeaderSymbolReadIf*  pcReadIf,
                                             const UChar*         pucScan,
                                             Bool&                rbUseDefaultScalingMatrixFlag )
{
  Int   iDeltaScale;
 	RNOK( pcReadIf->getSvlc( iDeltaScale,     "SCALING: delta_scale" ) );
  rbUseDefaultScalingMatrixFlag = ( iDeltaScale == -8 );
  ROTRS( rbUseDefaultScalingMatrixFlag, Err::m_nOK );
  
  this->get(0)  = ( ( 8 + iDeltaScale ) & 0xff );
  UInt  n       = 1;
  for(; n < this->size(); n++ ) // leszek
  {
   	RNOK( pcReadIf->getSvlc( iDeltaScale,   "SCALING: delta_scale" ) );
    this->get( pucScan[n] ) = ( ( this->get( pucScan[n-1] ) + iDeltaScale ) & 0xff );
    if( 0 == this->get( pucScan[n] ) )
    {
      break;
    }
  }
  for( ; n < this->size(); n++ ) // leszek
  {
    this->get( pucScan[n] ) = this->get( pucScan[n - 1] );
  }
  
  return Err::m_nOK;
}


template<UInt uiBufSize>
ErrVal
ScalingMatrix::ScalingList<uiBufSize>::write( HeaderSymbolWriteIf*  pcWriteIf,
                                              const UChar*          pucScan,
                                              const Bool            bUseDefaultScalingMatrixFlag ) const
{
  if( bUseDefaultScalingMatrixFlag )
  {
   	RNOK( pcWriteIf->writeSvlc( -8,         "SCALING: delta_scale" ) );
    return Err::m_nOK;
  }

  const UChar ucLast    = this->get( pucScan[this->size()-1] ); // leszek
  Int         iLastDiff = this->size() - 2; // leszek
  for( ; iLastDiff >= 0; iLastDiff-- ) 
  {
    if( ucLast != this->get( pucScan[iLastDiff] ) )
    {
      break;
    }
  }
  Int  iLast = 8;
  for( Int n = 0; n < iLastDiff+2; n++ )
  {
    AOT(1); // check modulo
    Int iDeltaScale = this->get( pucScan[n] ) - iLast;
    iDeltaScale     = ( ( iDeltaScale << 24 ) >> 24 );
   	RNOK( pcWriteIf->writeSvlc( iDeltaScale, "SCALING: delta_scale" ) );
    iLast           = this->get( pucScan[n] );
  }

  return Err::m_nOK;
}


template<UInt uiBufSize>
ScalingMatrix::SeqScaling<uiBufSize>::SeqScaling()
: m_bScalingListPresentFlag       ( false )
, m_bUseDefaultScalingMatrixFlag  ( true )
{
}


template<UInt uiBufSize>
const UChar*
ScalingMatrix::SeqScaling<uiBufSize>::getMatrix () const
{
  return ( m_bScalingListPresentFlag ? &m_aiScalingList.get(0) : 0 );
}


template<UInt uiBufSize>
ErrVal
ScalingMatrix::SeqScaling<uiBufSize>::read( HeaderSymbolReadIf* pcReadIf,
                                            const UChar*        pucScan )
{
  RNOK  ( pcReadIf->getFlag( m_bScalingListPresentFlag,     "SCALING: scaling_list_present_flag" ) );
  ROTRS ( ! m_bScalingListPresentFlag, Err::m_nOK );
  
  RNOK  ( m_aiScalingList.read( pcReadIf, pucScan, m_bUseDefaultScalingMatrixFlag ) );

  m_bScalingListPresentFlag = ! m_bUseDefaultScalingMatrixFlag;
  
  return Err::m_nOK;
}


template<UInt uiBufSize>
ErrVal
ScalingMatrix::SeqScaling<uiBufSize>::write( HeaderSymbolWriteIf* pcWriteIf,
                                             const UChar*         pucScan  ) const
{
  RNOK  ( pcWriteIf->writeFlag( m_bScalingListPresentFlag,  "SCALING: scaling_list_present_flag" ) );
  ROTRS ( ! m_bScalingListPresentFlag, Err::m_nOK );
  RNOK  ( m_aiScalingList.write( pcWriteIf, pucScan, m_bUseDefaultScalingMatrixFlag ) );

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END




#endif //!defined(AFX_SCALINGMATRIX_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
