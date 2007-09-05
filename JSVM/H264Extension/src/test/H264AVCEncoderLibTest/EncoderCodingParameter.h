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



#if !defined(AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_)
#define AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
 
 
#include <string> 
#include "CodingParameter.h"

#define ROTREPORT(x,t) {if(x) {::printf("\n%s\n",t); assert(0); return Err::m_nInvalidParameter;} }

class EncoderConfigLineStr : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineStr( Char* pcTag, std::string* pcPar, Char* pcDefault ) : EncoderConfigLineBase( pcTag, 1 ), m_pcPar( pcPar )
  {
    *m_pcPar = pcDefault;
  };
  Void setVar( std::string& pvValue )
  {
    *m_pcPar = pvValue;
  };
protected:
  std::string* m_pcPar;
};

class EncoderConfigLineDbl : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineDbl( Char* pcTag, Double* pdPar, Double pdDefault ) :  EncoderConfigLineBase( pcTag, 2 ), m_pdPar( pdPar ) 
  {
    *m_pdPar = pdDefault;
  };
  Void setVar( std::string& pvValue )
  {
    *m_pdPar = atof( pvValue.c_str() );
  };
protected:
  Double* m_pdPar;
};

class EncoderConfigLineInt : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineInt( Char* pcTag, Int* piPar, Int piDefault ) : EncoderConfigLineBase( pcTag, 3 ), m_piPar( piPar )
  {
    *m_piPar = piDefault;
  };
  Void setVar( std::string& pvValue)
  {
    *m_piPar = atoi( pvValue.c_str() );
  };
protected:
  Int* m_piPar;
};

class EncoderConfigLineUInt : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineUInt( Char* pcTag, UInt* puiPar, UInt puiDefault ) : EncoderConfigLineBase( pcTag, 4 ), m_puiPar( puiPar )
  {
    *m_puiPar = puiDefault;
  };
  Void setVar( std::string& pvValue)
  {
    *m_puiPar = atoi( pvValue.c_str() );
  };
protected:
  UInt* m_puiPar;
};

class EncoderConfigLineChar : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineChar( Char* pcTag, Char* pcPar, Char pcDefault ) : EncoderConfigLineBase( pcTag, 5 ), m_pcPar( pcPar )
  {
    *m_pcPar = pcDefault;
  };
  Void setVar( std::string& pvValue )
  {
    *m_pcPar = (Char)atoi( pvValue.c_str() );
  };
protected:
  Char* m_pcPar;
};



class EncoderCodingParameter : 
public h264::CodingParameter 
{
protected: 
  EncoderCodingParameter          (){}
  virtual ~EncoderCodingParameter (){}

public:
  static ErrVal create    ( EncoderCodingParameter*& rpcEncoderCodingParameter );
  ErrVal        destroy   ();
  ErrVal        init      ( Int     argc,
                            Char**  argv,
                            std::string&               rcBitstreamFile );

  Void          printHelp ();

protected:
  Bool    equals( const Char* str1, const Char* str2, UInt nLetter ) { return 0 == ::strncmp( str1, str2, nLetter); }

  ErrVal  xReadFromFile      ( std::string&            rcFilename,
                               std::string&            rcBitstreamFile  );
  ErrVal  xReadLayerFromFile ( std::string&            rcFilename,
                               h264::LayerParameters&  rcLayer );
  ErrVal  xReadLine          ( FILE*                   hFile,
                               std::string*            pacTag );
  ErrVal xReadSliceGroupCfg(h264::LayerParameters&  rcLayer );
  ErrVal xReadROICfg(h264::LayerParameters&  rcLayer );
};




ErrVal EncoderCodingParameter::create( EncoderCodingParameter*& rpcEncoderCodingParameter )
{
  rpcEncoderCodingParameter = new EncoderCodingParameter;
  
  ROT( NULL == rpcEncoderCodingParameter );

  return Err::m_nOK;
}


ErrVal EncoderCodingParameter::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal EncoderCodingParameter::init( Int     argc,
                                     Char**  argv,
                                     std::string& rcBitstreamFile  )
{
  Char* pcCom;

  rcBitstreamFile = "";

  ROTS( argc < 2 )

  for( Int n = 1; n < argc; n++ )
  {
    pcCom = argv[n++];

    if( equals( pcCom, "-rcdo-bs", 8 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiValue = atoi( argv[n] );
      CodingParameter::setRCDOBlockSizes( uiValue );
      continue;
    }
    if( equals( pcCom, "-rcdo-mc-y", 10 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiValue = atoi( argv[n] );
      CodingParameter::setRCDOMotionCompensationY( uiValue );
      continue;
    }
    if( equals( pcCom, "-rcdo-mc-c", 10 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiValue = atoi( argv[n] );
      CodingParameter::setRCDOMotionCompensationC( uiValue );
      continue;
    }
    if( equals( pcCom, "-rcdo-mc", 8 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiValue = atoi( argv[n] );
      CodingParameter::setRCDOMotionCompensationY( uiValue );
      CodingParameter::setRCDOMotionCompensationC( uiValue );
      continue;
    }
    if( equals( pcCom, "-rcdo-db", 8 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiValue = atoi( argv[n] );
      CodingParameter::setRCDODeblocking( uiValue );
      continue;
    }
    if( equals( pcCom, "-rcdo", 5 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiValue = atoi( argv[n] );
      CodingParameter::setRCDOBlockSizes         ( uiValue );
      CodingParameter::setRCDOMotionCompensationY( uiValue );
      CodingParameter::setRCDOMotionCompensationC( uiValue );
      CodingParameter::setRCDODeblocking         ( uiValue );
      continue;
    }
    if( equals( pcCom, "-4tap-mc-y", 10 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiValue = atoi( argv[n] );
      CodingParameter::set4TapMotionCompensationY( uiValue );
	  if(uiValue != 0)
		  CodingParameter::setRCDOMotionCompensationY( 0 );
      continue;
    }
    if( equals( pcCom, "-kpm", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      UInt  uiMode = atoi( argv[n] );
      CodingParameter::setEncodeKeyPictures( uiMode );
      continue;
    }
    if( equals( pcCom, "-mgsctrl", 5 ) )
    {
      ROTS( NULL == argv[n] );
      UInt uiMode = atoi( argv[n] );
      CodingParameter::setMGSKeyPictureControl( uiMode );
      continue;
    }
    if( equals( pcCom, "-mgsmotr", 5 ) )
    {
      ROTS( NULL == argv[n] );
      UInt uiMode = atoi( argv[n] );
      CodingParameter::setMGSKeyPictureMotRef( uiMode );
      continue;
    }
    if( equals( pcCom, "-eqpc", 5 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt  uiLayer = atoi( argv[n  ] );
      UInt  uiValue = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setExplicitQPCascading( uiValue );
      n += 1;
      continue;
    }
    if( equals( pcCom, "-dqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiLevel = atoi( argv[n+1] );
      Double  dValue  = atof( argv[n+2] );
      CodingParameter::getLayerParameters( uiLayer ).setDeltaQPTLevel( uiLevel, dValue );
      n += 2;
      continue;
    }
    if( equals( pcCom, "-aeqpc", 6 ) )
    {
      ROTS( NULL == argv[n  ] );
      UInt  uiValue = atoi( argv[n] );
      for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setExplicitQPCascading( uiValue );
      }
      continue;
    }
    if( equals( pcCom, "-adqp", 5 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLevel = atoi( argv[n  ] );
      Double  dValue  = atof( argv[n+1] );
      for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setDeltaQPTLevel( uiLevel, dValue );
      }
      n += 1;
      continue;
    }
    if( equals( pcCom, "-xdqp", 5 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      Double  dDQP0   = atof( argv[n  ] );
      Double  dDDQP1  = atof( argv[n+1] );
      Double  dDDQPN  = atof( argv[n+2] );
      for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setExplicitQPCascading( 1 );
        for( UInt uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
        {
          Double  dDQP = dDQP0;
          if( uiLevel > 0 )
          {
            dDQP += dDDQP1 + (Double)( uiLevel - 1 ) * dDDQPN;
          }
          CodingParameter::getLayerParameters( uiLayer ).setDeltaQPTLevel( uiLevel, dDQP );
        }
      }                                          
      n += 2;
      continue;
    }
    if( equals( pcCom, "-bf", 3 ) )
    {
      ROTS( NULL == argv[n] );
      rcBitstreamFile = argv[n];
      continue;
    }
    if( equals( pcCom, "-numl", 5 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiNumLayers = atoi( argv[n] );
      CodingParameter::setNumberOfLayers( uiNumLayers );
      continue;
    }
    if( equals( pcCom, "-ecmf", 5 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer                = atoi( argv[n  ] );
      Bool    bEntropyCodingModFlag  = (atoi( argv[n+1] ) == 1) ;
      CodingParameter::getLayerParameters( uiLayer ).setEntropyCodingModeFlag( bEntropyCodingModFlag );
      n += 1;
      continue;
    }
    if( equals( pcCom, "-rqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      Double  dResQp  = atof( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setBaseQpResidual( dResQp );
      printf("\n********** layer %1d - rqp = %f **********\n\n",uiLayer,dResQp);
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-mqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiStage = atoi( argv[n+1] );
      Double  dMotQp  = atof( argv[n+2] );
      CodingParameter::getLayerParameters( uiLayer ).setQpModeDecision( uiStage, dMotQp );
      n += 2;
      continue;      
    }
    if( equals( pcCom, "-lqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      Double  dQp     = atof( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setBaseQpResidual( dQp );
      for( UInt uiStage = 0; uiStage < MAX_DSTAGES; uiStage++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setQpModeDecision( uiStage, dQp );
      }
      CodingParameter::getLayerParameters( uiLayer ).setQpModeDecisionLP( dQp );
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-meqplp", 7 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      Double  dQp     = atof( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setQpModeDecisionLP( dQp );
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-ilpred", 7 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiBLRes = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setInterLayerPredictionMode( uiBLRes );
      n += 1;
      continue;
    }
    if( equals( pcCom, "-mfile", 6 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiMode  = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setMotionInfoMode( uiMode );
      CodingParameter::getLayerParameters( uiLayer ).setMotionInfoFilename( argv[n+2] );
      n += 2;
      continue;
    }
    if( equals( pcCom, "-gop", 4 ) )
    {
      ROTS( NULL == argv[n] ); 
      UInt uiGOPSize = atoi( argv[n] );
      CodingParameter::setGOPSize( uiGOPSize );
      continue;
    }
		if( equals( pcCom, "-iper", 5 ) )
    {
      ROTS( NULL == argv[n] ); 
      Int iIntraPeriod = atoi( argv[n] );
      CodingParameter::setIntraPeriod( iIntraPeriod );
      continue;
    }
    if( equals( pcCom, "-blid", 5 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiBlId  = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setBaseLayerId( uiBlId );
      n += 1;
      continue;
    }
    if( equals( pcCom, "-frms", 5 ) )
    {
      ROTS( NULL == argv[n] ); 
      UInt uiFrms = atoi( argv[n] );
      CodingParameter::setTotalFrames( uiFrms );
      continue;
    }
    if( equals( pcCom, "-bcip", 5 ) )
    {
      n--;
      ROTS( NULL == argv[n] );
      CodingParameter::getLayerParameters(0).setContrainedIntraForLP();
      continue;
    }
    if( equals( pcCom, "-pf", 3) )
    {
      ROTS( NULL == argv[n] );
      std::string cFilename = argv[n];
      RNOKS( xReadFromFile( cFilename, rcBitstreamFile ) );  
      continue;
    }
	
	//S051{
	if( equals( pcCom, "-encsip", 7 ) )
    {
		ROTS( NULL == argv[n  ] );
		ROTS( NULL == argv[n+1] );
		
		UInt    uiLayer = atoi( argv[n  ] );
		CodingParameter::getLayerParameters( uiLayer ).setEncSIP(true);
		CodingParameter::getLayerParameters( uiLayer ).setInSIPFileName(argv[n+1]);
		n += 1;
		continue;
    }
	if( equals( pcCom, "-anasip", 7 ) )
    {
		ROTS( NULL == argv[n  ] );
		ROTS( NULL == argv[n+1] );
		ROTS( NULL == argv[n+2] );
		
		UInt    uiLayer = atoi( argv[n  ] );
		UInt	uiMode = atoi( argv[n+1] );
		
		if(uiMode!=0)
			CodingParameter::getLayerParameters( uiLayer ).setAnaSIP(2);
		else
			CodingParameter::getLayerParameters( uiLayer ).setAnaSIP(1);
		
		CodingParameter::getLayerParameters( uiLayer ).setOutSIPFileName(argv[n+2]);
		n += 2;
		continue;
    }
	//S051}

    //JVT-W052 bug_fixed
    if( equals( pcCom, "-icsei", 6 ) )
    {
      ROTS( NULL == argv[n] );
      UInt uiIntegrityCheckSei = atoi( argv[n] );
      CodingParameter::setIntegrityCheckSEIEnable( uiIntegrityCheckSei );
      continue;
    }
    //JVT-W052 bug_fixed

    // JVT-U116 LMI {
    if( equals( pcCom, "-tlidx", 6 ) )
    {
      ROTS( NULL == argv[n] );
      UInt uiTl0PicIdxPresentFlag = atoi( argv[n] );
      CodingParameter::setTl0PicIdxPresentFlag( uiTl0PicIdxPresentFlag );
      continue;
    }
    // JVT-U116 LMI }

    // JVT-U085 LMI {
    if( equals( pcCom, "-tlnest", 7 ) )
    {
      ROTS( NULL == argv[n] );
      UInt uiTlNestFlag = atoi( argv[n] );
      CodingParameter::setTlevelNestingFlag( uiTlNestFlag );
      continue;
    }
    // JVT-U085 LMI }
	
    if( equals( pcCom, "-org", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      ROF(    uiLayer < MAX_LAYERS );
      CodingParameter::getLayerParameters( uiLayer ).setInputFilename( argv[n+1] );
      n += 1;
      continue;      
    }
		//JVT-W049 {
    if( equals( pcCom, "-plr", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      ROF(    uiLayer < MAX_LAYERS );
			UInt    uiPLR = atoi( argv[n+1  ] );
      CodingParameter::getLayerParameters( uiLayer ).setPLR( uiPLR );
      n += 1;
      continue;      
    }
		if( equals( pcCom, "-redu", 5 ) )
		{
			ROTS( NULL == argv[n  ] );
			ROTS( NULL == argv[n+1] );
			UInt    uiLayer = atoi( argv[n  ] );
			ROF(    uiLayer < MAX_LAYERS );
			Bool    bReduFlag = (atoi( argv[n+1  ] ) > 0 ? true:false);
			CodingParameter::getLayerParameters( uiLayer ).setUseRedundantSliceFlag( bReduFlag );
			n += 1;
			continue;      
		}

		if( equals( pcCom, "-kpredu", 7 ) )
		{
			ROTS( NULL == argv[n  ] );
			ROTS( NULL == argv[n+1] );
			UInt    uiLayer = atoi( argv[n  ] );
			ROF(    uiLayer < MAX_LAYERS );
			Bool    bReduKeyFlag = (atoi( argv[n+1  ] ) > 0 ? true:false);
			CodingParameter::getLayerParameters( uiLayer ).setUseRedundantKeySliceFlag( bReduKeyFlag );
			n += 1;
			continue;      
    }
    //JVT-W049 }
    if( equals( pcCom, "-rec", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      ROF(    uiLayer < MAX_LAYERS );
      CodingParameter::getLayerParameters( uiLayer ).setOutputFilename( argv[n+1] );
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-ec", 3 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer  = atoi( argv[n  ] );
      ROF(    uiLayer < MAX_LAYERS );
      UInt    uiECmode = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setEntropyCodingModeFlag( uiECmode != 0 );
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-vlc", 4 ) )
    {
      n--;
      ROTS( NULL == argv[n] );
      for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setEntropyCodingModeFlag( false );
      }
      continue;
    }
    if( equals( pcCom, "-cabac", 6 ) )
    {
      n--;
      ROTS( NULL == argv[n] );
      for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setEntropyCodingModeFlag( true );
      }
      continue;
    }
    //JVT-U106 Behaviour at slice boundaries{
    if( equals( pcCom, "-ciu", 3 ) )
    {
	    ROTS( NULL == argv[n] );
	    UInt flag = atoi( argv[n] );
	    CodingParameter::setCIUFlag( flag );
	    continue;
    }
    //JVT-U106 Behaviour at slice boundaries}

    if( equals( pcCom, "-h", 2) )
    {
      printHelp();
      return Err::m_nOK;
    }

    return Err::m_nERR;
  }


  RNOKS( check() );
  
  return Err::m_nOK;
}


Void EncoderCodingParameter::printHelp()
{
  printf("\n supported options:\n\n");
  printf("  -pf     Parameter File Name\n\n");

  printf("  -bf     BitStreamFile\n");
  printf("  -frms   Number of total frames\n");
	printf("  -gop    GOPSize - GOP size (2,4,8,16,32,64, default: 1)\n");
	printf("  -iper   Intra period (default: -1) : must be a power of 2 of GOP size (or -1)\n");
  printf("  -numl   Number Of Layers\n");
  printf("  -cabac  CABAC for all layers as entropy coding mode\n");
  printf("  -vlc    VLC for all layers as entropy coding mode\n");
	printf("  -ecmf   (Layer) (entropy_coding_mod_flag)\n");
  printf("  -org    (Layer) (original file)\n");
  printf("  -rec    (Layer) (reconstructed file)\n");
  printf("  -ec     (Layer) (entropy coding mode)\n");
  printf("  -rqp    (Layer) (ResidualQP)\n");
  printf("  -mqp    (Layer) (Stage) (MotionQP)\n");
  printf("  -lqp    (Layer) (ResidualAndMotionQP)\n");
  printf("  -meqplp (Layer) (MotionQPLowpass)\n");
  printf("  -ilpred (Layer) (InterLayerPredictionMode)\n");
	printf("  -blid   (Layer) (BaseLayerId)\n");
  printf("  -mfile  (Layer) (Mode) (MotionInfoFile)\n");
  printf("  -bcip   Constrained intra prediction for base layer (needed for single-loop) in scripts\n");
  //S051{
  printf("  -anasip (Layer) (SIP Analysis Mode)[0: persists all inter-predictions, 1: forbids all inter-prediction.] (File for storing bits information)\n");
  printf("  -encsip (Layer) (File with stored SIP information)\n");
  //S051}
  //JVT-W052 bug_fixed
  printf("  -icsei   (IntegrityCheckSEIEnableFlag)[0: IntegrityCheckSEI is not applied, 1: IntegrityCheckSEI is applied.]\n");
  //JVT-W052 bug_fixed
   //JVT-U085 LMI
  printf("  -tlnest (TlevelNestingFlag)[0: temporal level nesting constraint is not applied, 1: the nesting constraint is applied.]\n");
  //JVT-U116 JVT-V088 LMI
  printf("  -tlidx (Tl0PicIdxPresentFlag)[0: tl0_pic_idx is not present, 1: tl0_pic_idx is present.]\n");
  //JVT-U106 Behaviour at slice boundaries{
  printf("  -ciu    (Constrained intra upsampling)[0: no, 1: yes]\n");
  //JVT-U106 Behaviour at slice boundaries}
   
  printf("  -rcdo-bs   (value)  RDCO block size restriction     (0:off,1:ELonly,2:on)\n" );
  printf("  -rcdo-mc-y (value)  RDCO motion compensation luma   (0:off,1:ELonly,2:on)\n" );
  printf("  -rcdo-mc-c (value)  RDCO motion compensation chroma (0:off,1:ELonly,2:on)\n" );
  printf("  -rcdo-mc   (value)  RDCO motion compensation        (0:off,1:ELonly,2:on)\n" );
  printf("  -rcdo-db   (value)  RDCO deblocking                 (0:off,1:ELonly,2:on)\n" );
  printf("  -rcdo      (value)  RDCO (all components)           (0:off,1:ELonly,2:on)\n" );

  printf("  -4tap-mc-y (value)  4-tap motion compensation luma   (0:off,1:ELonly,2:on)\n" );  // V090

  printf("  -kpm       (mode) [0:only for FGS(default), 1:FGS&MGS, 2:always]\n");
  printf("  -mgsctrl   (mode) [0:normal encoding(default), 1:EL ME, 2:EL ME+MC]\n");
  printf("  -mgsmotr   (mode) [0:no MGS mot ref, 1:normal mot ref (default)\n");
  
  printf("  -eqpc   (layer) (value)         sets explicit QP cascading mode for given layer [0: no, 1: yes]\n");
  printf("  -dqp    (layer) (level) (value) sets delta QP for given layer and temporal level (in explicit mode)\n");
  printf("  -aeqpc  (value)                 sets explicit QP cascading mode for all layers  [0: no, 1: yes]\n");
  printf("  -adqp   (level) (value)         sets delta QP for all layers and given temporal level (in explicit mode)\n");
  printf("  -xdqp   (DQP0) (DDQP1) (DDQPN)  sets delta QP for all layers (in explicit mode)\n");

  printf("  -h       Print Option List \n");
  printf("\n");
}


ErrVal EncoderCodingParameter::xReadLine( FILE* hFile, std::string* pacTag )
  {
  ROF( pacTag );

    Int  n;
    UInt uiTagNum = 0;
  Bool          bComment  = false;
  std::string*  pcTag     = &pacTag[0];

    for( n = 0; n < 4; n++ )
    {
      pacTag[n] = "";
    }

  for( n = 0; ; n++ )
    {
      Char cChar = (Char) fgetc( hFile );
    ROTRS( cChar == '\n' || feof( hFile ), Err::m_nOK );  // end of line
    if   ( cChar == '#' )
      {
      bComment = true;
      }
    if( ! bComment )
      {
      if ( cChar == '\t' || cChar == ' ' ) // white space
        {
          ROTR( uiTagNum == 3, Err::m_nERR );
        if( ! pcTag->empty() )
          {
            uiTagNum++;
          pcTag = &pacTag[uiTagNum]; 
          }
  }
      else
  {
        *pcTag += cChar;
  }
}
  }

 }

ErrVal EncoderCodingParameter::xReadFromFile( std::string& rcFilename, std::string& rcBitstreamFile )
{
  std::string acLayerConfigName[MAX_LAYERS];
  std::string acTags[4];
  UInt        uiLayerCnt   = 0;
  UInt        uiParLnCount = 0;

  FILE *f = fopen( rcFilename.c_str(), "r");
  if( NULL == f )
  { 
    printf( "failed to open %s parameter file\n", rcFilename.c_str() );
    return Err::m_nERR;
  } 

  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr ("OutputFile",              &rcBitstreamFile,                                      "test.264");
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("FrameRate",               &m_dMaximumFrameRate,                                  60.0      );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("MaxDelay",                &m_dMaximumDelay,                                      1200.0    );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("FramesToBeEncoded",       &m_uiTotalFrames,                                      1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("GOPSize",                 &m_uiGOPSize,                                          1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("IntraPeriod",             &m_uiIntraPeriod,                                      MSYS_UINT_MAX );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("NumberReferenceFrames",   &m_uiNumRefFrames,                                     1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("BaseLayerMode",           &m_uiBaseLayerMode,                                    3 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("NumLayers",               &m_uiNumberOfLayers,                                   1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SearchRange",             &(m_cMotionVectorSearchParams.m_uiSearchRange),        96);
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("BiPredIter",              &(m_cMotionVectorSearchParams.m_uiNumMaxIter),         4 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("IterSearchRange",         &(m_cMotionVectorSearchParams.m_uiIterSearchRange),    8 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("LoopFilterDisable",       &(m_cLoopFilterParams.m_uiFilterIdc),                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("LoopFilterAlphaC0Offset", &(m_cLoopFilterParams.m_iAlphaOffset),                 0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("LoopFilterBetaOffset",    &(m_cLoopFilterParams.m_iBetaOffset),                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SearchMode",              &(m_cMotionVectorSearchParams.m_uiSearchMode),         0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SearchFuncFullPel",       &(m_cMotionVectorSearchParams.m_uiFullPelDFunc),       0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SearchFuncSubPel",        &(m_cMotionVectorSearchParams.m_uiSubPelDFunc),        0 );

//TMM_WP
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("WeightedPrediction",         &m_uiIPMode,                                     0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("WeightedBiprediction",       &m_uiBMode,                                      0 );  
//TMM_WP
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt("NonRequiredEnable",			&m_bNonRequiredEnable,							 0 );  //NonRequired JVT-Q066
  std::string cInputFile, cReconFile;
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("AVCMode",                 &m_uiAVCmode,                                          0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr ("InputFile",               &cInputFile,                                           "in.yuv");
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr ("ReconFile",               &cReconFile,                                           "rec.yuv");
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceWidth",             &m_uiFrameWidth,                                       0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceHeight",            &m_uiFrameHeight,                                      0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SymbolMode",              &m_uiSymbolMode,                                       1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("FRExt",                   &m_ui8x8Mode,                                          1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("BasisQP",                 &m_dBasisQp,                                          26 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("DPBSize",                 &m_uiDPBSize,                                           1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("NumRefFrames",            &m_uiNumDPBRefFrames,                                  1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("Log2MaxFrameNum",         &m_uiLog2MaxFrameNum,                                  4 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("Log2MaxPocLsb",           &m_uiLog2MaxPocLsb,                                    4 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr ("SequenceFormatString",    &m_cSequenceFormatString,                              "A0*n{P0}" );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer0Quant",        &m_adDeltaQpLayer[0],                                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer1Quant",        &m_adDeltaQpLayer[1],                                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer2Quant",        &m_adDeltaQpLayer[2],                                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer3Quant",        &m_adDeltaQpLayer[3],                                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer4Quant",        &m_adDeltaQpLayer[4],                                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer5Quant",        &m_adDeltaQpLayer[5],                                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MaxRefIdxActiveBL0",      &m_uiMaxRefIdxActiveBL0,                               1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MaxRefIdxActiveBL1",      &m_uiMaxRefIdxActiveBL1,                               1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MaxRefIdxActiveP",        &m_uiMaxRefIdxActiveP,                                 1 );

  //JVT-R057 LA-RDO{
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("LARDO",                   &m_uiLARDOEnable,                                      0 ); 
  //JVT-R057 LA-RDO}

  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("RPEncCheck",							 &m_uiEssRPChkEnable,																					0  );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MVDiffThreshold",				 &m_uiMVThres,																					20 );

	//JVT-T073 {
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("NestingSEI",              &m_uiNestingSEIEnable,                                 0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SceneInfo",               &m_uiSceneInfoEnable,                                  0 );
  //JVT-T073 }

	//JVT-W052
	m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("IntegrityCheckSEI",          &m_uiIntegrityCheckSEIEnable,                          false ); // Disabled due to buggy behaviour. mwi 070803, bug_fixed
	//JVT-W052

//JVT-S036 lsj start  //bug-fix suffix{{
//PreAndSuffixUnitEnable shall always be on in SVC contexts (i.e. when there are FGS/CGS/spatial enhancement layers)
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("PreAndSuffixUnitEnable",  &m_uiPreAndSuffixUnitEnable,                           1 ); //prefix unit
	m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MMCOBaseEnable",					 &m_uiMMCOBaseEnable,                                   1 ); 
//JVT-S036 lsj end //bug-fix suffix}}
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("CgsSnrRefinement",        &m_uiCGSSNRRefinementFlag,                             0 );  //JVT-T054
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("TLNestingFlag",           &m_uiTlevelNestingFlag,                                0 );  //JVT-U085
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("TLPicIdxEnable",          &m_uiTl0PicIdxPresentFlag,                             0 );  //JVT-U116

  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("RCDOBlockSizes",          &m_uiRCDOBlockSizes,                                   0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("RCDOMotionCompensationY", &m_uiRCDOMotionCompensationY,                          0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("RCDOMotionCompensationC", &m_uiRCDOMotionCompensationC,                          0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("RCDODeblocking",          &m_uiRCDODeblocking,                                   0 );

  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("4TapMotionCompensationY", &m_ui4TapMotionCompensationY,                          0 ); // V090

  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("EncodeKeyPictures",       &m_uiEncodeKeyPictures,                                0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSControl",              &m_uiMGSKeyPictureControl,                             0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSKeyPicMotRef",         &m_uiMGSKeyPictureMotionRefinement,                    1 );

// JVT-V068 HRD {
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("EnableNalHRD",            &m_uiNalHRD,                                           0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("EnableVclHRD",            &m_uiVclHRD,                                           0 );
// JVT-V068 HRD }
//JVT-W049 {
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("EnableRedundantKeyPic",   &m_uiRedundantKeyPic,                                  0 );
//JVT-W049 }

  m_pEncoderLines[uiParLnCount] = NULL;

  while (!feof(f))
  {
    RNOK( xReadLine( f, acTags ) );
    if ( acTags[0].empty() )
    {
      continue;
    }
    for (UInt ui=0; m_pEncoderLines[ui] != NULL; ui++)
    {
      if( acTags[0] == m_pEncoderLines[ui]->getTag() )
      {
        m_pEncoderLines[ui]->setVar( acTags[1] );
        break;
      }
    }
    if( acTags[0] == "LayerCfg" )
    {
      acLayerConfigName[uiLayerCnt++] = acTags[1];
      continue;
    }
  }

  uiParLnCount = 0;
  while (m_pEncoderLines[uiParLnCount] != NULL)
  {
    delete m_pEncoderLines[uiParLnCount];
    m_pEncoderLines[uiParLnCount] = NULL;
    uiParLnCount++;
  }

  if( m_uiAVCmode )
  {
    m_uiNumberOfLayers = 0;
    getLayerParameters(0).setInputFilename  ( (Char*)cInputFile.c_str() );
    getLayerParameters(0).setOutputFilename ( (Char*)cReconFile.c_str() );
    getLayerParameters(0).setFrameWidth     ( m_uiFrameWidth );
    getLayerParameters(0).setFrameHeight    ( m_uiFrameHeight );
    fclose( f );
    return Err::m_nOK;
  }

  if ( uiLayerCnt != m_uiNumberOfLayers )
  {
    fprintf(stderr, "Could not locate all layer config files: check config file syntax\n");
    AF();
  }

  fclose( f );
//JVT-T054{
  UInt uiPrevLayer        = 0;
  Double  dPrevTemp       = 0.0;
  UInt uiPrevWidth        = 0;
  UInt uiPrevHeight       = 0;
  UInt uiLastLayer        = 0;
//JVT-T054}
  Bool bInterlaced        = false; 
  UInt ui                 = 0;

  for( ui = 0; ui < m_uiNumberOfLayers; ui++ )
  {
    getLayerParameters(ui).setLayerId(ui);
    RNOK( xReadLayerFromFile( acLayerConfigName[ui], getLayerParameters(ui) ) );
		if ( getLayerParameters(ui).m_uiMbAff || getLayerParameters(ui).m_uiPaff )
    {
      bInterlaced = true;			
    }
    if( m_uiCGSSNRRefinementFlag )
    {
      if(ui == 0)
      {
        uiPrevLayer  = ui;
        dPrevTemp    = getLayerParameters(ui).getOutputFrameRate();
        uiPrevWidth  = getLayerParameters(ui).getFrameWidth();
        uiPrevHeight = getLayerParameters(ui).getFrameHeight();
        getLayerParameters(ui).setLayerCGSSNR(ui);
        getLayerParameters(ui).setQualityLevelCGSSNR(0);
        uiLastLayer = uiPrevLayer;
        getLayerParameters(ui).setBaseLayerCGSSNR( MSYS_UINT_MAX );
        getLayerParameters(ui).setBaseQualityLevelCGSSNR( 0 );
      }
      else
      {
        if( dPrevTemp == getLayerParameters(ui).getOutputFrameRate() &&
            uiPrevWidth == getLayerParameters(ui).getFrameWidth()    &&
            uiPrevHeight == getLayerParameters(ui).getFrameHeight() )
        {
          // layer can be considered as a CGS refinement
          UInt uiLayerTemp = getLayerParameters(ui-1).getLayerCGSSNR();
          getLayerParameters(ui).setLayerCGSSNR(uiLayerTemp);
          UInt uiQualityLevelTemp = getLayerParameters(ui-1).getQualityLevelCGSSNR();
          getLayerParameters(ui).setQualityLevelCGSSNR( uiQualityLevelTemp + getLayerParameters(ui-1).getNumberOfQualityLevelsCGSSNR() );
        }
        else
        {
          //layer is not a refinement from previous CGS layer
          uiLastLayer++;
          uiPrevLayer = uiLastLayer;
          dPrevTemp    = getLayerParameters(ui).getOutputFrameRate();
          uiPrevWidth = getLayerParameters(ui).getFrameWidth();
          uiPrevHeight = getLayerParameters(ui).getFrameHeight();
          getLayerParameters(ui).setLayerCGSSNR(uiLastLayer);
          getLayerParameters(ui).setQualityLevelCGSSNR(0);
        }
        getLayerParameters(ui).setBaseLayerCGSSNR( getLayerParameters(ui-1).getLayerCGSSNR() );
        getLayerParameters(ui).setBaseQualityLevelCGSSNR( getLayerParameters(ui-1).getQualityLevelCGSSNR() + getLayerParameters(ui-1).getNumberOfQualityLevelsCGSSNR() - 1 );
      }
    }
    else
    {
      getLayerParameters(ui).setLayerCGSSNR(ui);
      getLayerParameters(ui).setQualityLevelCGSSNR(0);
    }

//JVT-T054}
// TMM_ESS {
    ResizeParameters * curr;
    curr = getResizeParameters(ui);

// JVT-Q065 EIDR{
    if(ui > 0 && getLayerParameters(ui-1).getIDRPeriod() == getLayerParameters(ui).getIDRPeriod())
    {
	    getLayerParameters(ui).setBLSkipEnable(true);
    }
// JVT-Q065 EIDR}

    // HS: set base layer id
    UInt uiBaseLayerId = getLayerParameters(ui).getBaseLayerId();
    if( ui && uiBaseLayerId == MSYS_UINT_MAX )
    {
      uiBaseLayerId = ui - 1; // default value
    }
    getLayerParameters(ui).setBaseLayerId(uiBaseLayerId);
    // HS: set base layer id
//DS_FIX_FT_09_2007
  //uiBaseLayerId is no more discardable
    if(uiBaseLayerId != MSYS_UINT_MAX)
    {
      getLayerParameters(uiBaseLayerId).setNonDiscardable();
      getLayerParameters(uiBaseLayerId).setQLDiscardable(getLayerParameters(ui).getBaseQualityLevel()+1);
    }
//~DS_FIX_FT_09_2007

    if (ui>0)
    {
      ResizeParameters * prev = getResizeParameters(uiBaseLayerId); // HS: use "real" base layer
      //curr->m_iInWidth  = prev->m_iOutWidth;
      //curr->m_iInHeight = prev->m_iOutHeight;
      curr->m_iInWidth = prev->m_iGlobWidth; //TMM
      curr->m_iInHeight = prev->m_iGlobHeight; //TMM

      bool is_crop_aligned = (curr->m_iPosX%16 == 0) && (curr->m_iPosY%16 == 0);
      if      ((curr->m_iInWidth == curr->m_iOutWidth) && (curr->m_iInHeight == curr->m_iOutHeight) &&
               is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
        curr->m_iSpatialScalabilityType = SST_RATIO_1;
      else if ((curr->m_iInWidth*2 == curr->m_iOutWidth) && (curr->m_iInHeight*2 == curr->m_iOutHeight) &&
               is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
        curr->m_iSpatialScalabilityType = SST_RATIO_2;
      else 
      {
        curr->m_iSpatialScalabilityType = SST_RATIO_X;
        if ( curr->m_iExtendedSpatialScalability == ESS_NONE )
          curr->m_iExtendedSpatialScalability = ESS_SEQ;
      }
     }
    else
    {
      curr->m_iSpatialScalabilityType = SST_RATIO_1;
      curr->m_iExtendedSpatialScalability = ESS_NONE;
    }
// TMM_ESS }
  }

  
//TMM_INTERLACE{
  for( ui = 0; ui < m_uiNumberOfLayers; ui++ )
    getResizeParameters(ui)->m_bInterlaced = bInterlaced;
//TMM_INTERLACE}

  return Err::m_nOK;
}



ErrVal EncoderCodingParameter::xReadLayerFromFile ( std::string&            rcFilename,
                                                    h264::LayerParameters&  rcLayer )
{
  std::string acTags[4];
  std::string cInputFilename, cOutputFilename, cMotionFilename, cESSFilename;

  //S051{
  std::string cEncSIPFilename;
  //S051}
  
  UInt        uiParLnCount = 0;
  
  FILE *f = fopen( rcFilename.c_str(), "r");
  if( NULL == f )
  { 
    printf( "failed to open %s layer config file\n", rcFilename.c_str() );
    return Err::m_nERR;
  } 

  //--ICU/ETRI FMO Implementation
  UInt bSliceGroupChangeDirection_flag=0;

  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceWidth",    &(rcLayer.m_uiFrameWidth),               176       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceHeight",   &(rcLayer.m_uiFrameHeight),              352       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("FrameRateIn",    &(rcLayer.m_dInputFrameRate),            30        );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("FrameRateOut",   &(rcLayer.m_dOutputFrameRate),           30        );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("InputFile",      &cInputFilename,                         "test.yuv");
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("ReconFile",      &cOutputFilename,                        "rec.yuv" );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MbAff",          &(rcLayer.m_uiMbAff),                    0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("Paff",           &(rcLayer.m_uiPaff),                     0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SymbolMode",     &(rcLayer.m_uiEntropyCodingModeFlag),    1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FRExt",          &(rcLayer.m_uiAdaptiveTransform),        0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MaxDeltaQP",     &(rcLayer.m_uiMaxAbsDeltaQP),            1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("QP",             &(rcLayer.m_dBaseQpResidual),            32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQPLP",         &(rcLayer.m_dQpModeDecisionLP),          -1.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP0",          &(rcLayer.m_adQpModeDecision[0]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP1",          &(rcLayer.m_adQpModeDecision[1]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP2",          &(rcLayer.m_adQpModeDecision[2]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP3",          &(rcLayer.m_adQpModeDecision[3]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP4",          &(rcLayer.m_adQpModeDecision[4]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP5",          &(rcLayer.m_adQpModeDecision[5]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("InterLayerPred", &(rcLayer.m_uiInterLayerPredictionMode), 0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("BaseQuality",    &(rcLayer.m_uiBaseQualityLevel),         15        );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MotionInfoMode", &(rcLayer.m_uiMotionInfoMode),           0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("MotionInfoFile", &cMotionFilename,                        "test.mot");
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("LowComplexityMbMode",         &(rcLayer.m_uiLowComplexMbEnable), 0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("UseESS",         &(rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability), 0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("ESSPicParamFile",&cESSFilename,                                              "ess.dat" );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSCropWidth",   &(rcLayer.m_ResizeParameter.m_iOutWidth),                   0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSCropHeight",  &(rcLayer.m_ResizeParameter.m_iOutHeight),                  0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSOriginX",     &(rcLayer.m_ResizeParameter.m_iPosX),                       0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSOriginY",     &(rcLayer.m_ResizeParameter.m_iPosY),                       0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSChromaPhaseX",&(rcLayer.m_ResizeParameter.m_iChromaPhaseX),              -1         );  // SSUN, Nov2005
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSChromaPhaseY",&(rcLayer.m_ResizeParameter.m_iChromaPhaseY),               0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSBaseChromaPhaseX",&(rcLayer.m_ResizeParameter.m_iBaseChromaPhaseX),      -1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSBaseChromaPhaseY",&(rcLayer.m_ResizeParameter.m_iBaseChromaPhaseY),       0         );  // SSUN, Nov2005
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("ForceReOrdering",&(rcLayer.m_uiForceReorderingCommands),  0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("BaseLayerId",    &(rcLayer.m_uiBaseLayerId),              MSYS_UINT_MAX );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SliceMode",      &(rcLayer.m_uiSliceMode),                             0       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SliceArgument",  &(rcLayer.m_uiSliceArgument),                        50       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("NumSlicGrpMns1", &(rcLayer.m_uiNumSliceGroupsMinus1),                  0       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SlcGrpMapType",  &(rcLayer.m_uiSliceGroupMapType),                     2       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SlcGrpChgDrFlag",&(bSliceGroupChangeDirection_flag),         0       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SlcGrpChgRtMus1",&(rcLayer.m_uiSliceGroupChangeRateMinus1),           85       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("SlcGrpCfgFileNm",&rcLayer.m_cSliceGroupConfigFileName,             "sgcfg.cfg" );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("NumROI", &(rcLayer.m_uiNumROI),                  0       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("ROICfgFileNm",&rcLayer.m_cROIConfigFileName,             "roicfg.cfg" );
// JVT-Q065 EIDR{
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("IDRPeriod",	  &(rcLayer.m_iIDRPeriod),								0		);
// JVT-Q065 EIDR}
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt ("PLR",	          &(rcLayer.m_uiPLR),								0		); //JVT-R057 LA-RDO
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("UseRedundantSlc",&(rcLayer.m_uiUseRedundantSlice), 0   );  //JVT-Q054 Red. Picture
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("UseRedundantKeySlc",&(rcLayer.m_uiUseRedundantKeySlice), 0   );  //JVT-W049
  //S051{
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr( "EncSIPFile", &cEncSIPFilename, ""); 
  //S051}

  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVectorMode", &(rcLayer.m_uiMGSVectorMode), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector0", &(rcLayer.m_uiMGSVect[0]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector1", &(rcLayer.m_uiMGSVect[1]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector2", &(rcLayer.m_uiMGSVect[2]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector3", &(rcLayer.m_uiMGSVect[3]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector4", &(rcLayer.m_uiMGSVect[4]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector5", &(rcLayer.m_uiMGSVect[5]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector6", &(rcLayer.m_uiMGSVect[6]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector7", &(rcLayer.m_uiMGSVect[7]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector8", &(rcLayer.m_uiMGSVect[8]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector9", &(rcLayer.m_uiMGSVect[9]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector10", &(rcLayer.m_uiMGSVect[10]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector11", &(rcLayer.m_uiMGSVect[11]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector12", &(rcLayer.m_uiMGSVect[12]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector13", &(rcLayer.m_uiMGSVect[13]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector14", &(rcLayer.m_uiMGSVect[14]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MGSVector15", &(rcLayer.m_uiMGSVect[15]), 0 );

  // JVT-V035
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt ("AvcRewriteFlag",          &(rcLayer.m_bAVCRewriteFlag),                            0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt ("AvcAdaptiveRewriteFlag",  &(rcLayer.m_bAVCAdaptiveRewriteFlag),                    0 );

  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("ExplicitQPCascading", &(rcLayer.m_uiExplicitQPCascading), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("DQP4TLevel0",         &(rcLayer.m_adDeltaQPTLevel[0]),    0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("DQP4TLevel1",         &(rcLayer.m_adDeltaQPTLevel[1]),    0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("DQP4TLevel2",         &(rcLayer.m_adDeltaQPTLevel[2]),    0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("DQP4TLevel3",         &(rcLayer.m_adDeltaQPTLevel[3]),    0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("DQP4TLevel4",         &(rcLayer.m_adDeltaQPTLevel[4]),    0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("DQP4TLevel5",         &(rcLayer.m_adDeltaQPTLevel[5]),    0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("DQP4TLevel6",         &(rcLayer.m_adDeltaQPTLevel[6]),    0 );

  m_pLayerLines[uiParLnCount] = NULL;

  while (!feof(f))
  {
    RNOK( xReadLine( f, acTags ) );
    if ( acTags[0].empty() )
    {
      continue;
    }
    for (UInt ui=0; m_pLayerLines[ui] != NULL; ui++)
    {
      if( acTags[0] == m_pLayerLines[ui]->getTag() )
      {
        m_pLayerLines[ui]->setVar( acTags[1] );
        break;
      }
    }
  }

  //S051{
  if(cEncSIPFilename.length())
  {
    rcLayer.setEncSIP(true);
    rcLayer.setInSIPFileName( (char*) cEncSIPFilename.c_str());
  }  
  //S051}

  rcLayer.setInputFilename     ( (Char*)cInputFilename.c_str() );
  rcLayer.setOutputFilename    ( (Char*)cOutputFilename.c_str() );
  rcLayer.setMotionInfoFilename( (Char*)cMotionFilename.c_str() );

  uiParLnCount = 0;
  while (m_pLayerLines[uiParLnCount] != NULL)
  {
    delete m_pLayerLines[uiParLnCount];
    m_pLayerLines[uiParLnCount] = NULL;
    uiParLnCount++;
  }

// TMM_ESS {
  // default values
  rcLayer.m_ResizeParameter.m_iInWidth    = rcLayer.m_uiFrameWidth;
  rcLayer.m_ResizeParameter.m_iInHeight   = rcLayer.m_uiFrameHeight;
  rcLayer.m_ResizeParameter.m_iGlobWidth  = rcLayer.m_uiFrameWidth;
  rcLayer.m_ResizeParameter.m_iGlobHeight = rcLayer.m_uiFrameHeight;
  rcLayer.m_ResizeParameter.m_bCrop       = false;
  if(rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability)  
  {
//TMM {
   rcLayer.m_ResizeParameter.m_bCrop = true;        
    /*if (rcLayer.m_ResizeParameter.m_iInWidth != rcLayer.m_ResizeParameter.m_iOutWidth || 
			  rcLayer.m_ResizeParameter.m_iInHeight != rcLayer.m_ResizeParameter.m_iOutHeight || 
			  rcLayer.m_ResizeParameter.m_iPosX != 0 || 
			  rcLayer.m_ResizeParameter.m_iPosY != 0 ) */
		   
//TMM }     
    if(rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability==2)
    {
      rcLayer.m_ResizeParameter.m_pParamFile = fopen( cESSFilename.c_str(), "r");
      if( NULL == rcLayer.m_ResizeParameter.m_pParamFile )
      { 
        printf( "failed to open resize parameter file %s\n", cESSFilename.c_str() );
        return Err::m_nERR;
      }
      rcLayer.m_ResizeParameter.m_iSpatialScalabilityType = SST_RATIO_X;
    }
  } else {
    // default values
    rcLayer.m_ResizeParameter.m_iOutWidth   = rcLayer.m_uiFrameWidth;
    rcLayer.m_ResizeParameter.m_iOutHeight  = rcLayer.m_uiFrameHeight;
    rcLayer.m_ResizeParameter.m_iPosX       = 0;
    rcLayer.m_ResizeParameter.m_iPosY       = 0;
    rcLayer.m_ResizeParameter.m_iBaseChromaPhaseX = rcLayer.m_ResizeParameter.m_iChromaPhaseX;  // SSUN, Nov2005
    rcLayer.m_ResizeParameter.m_iBaseChromaPhaseY = rcLayer.m_ResizeParameter.m_iChromaPhaseY;
  }
// TMM_ESS }

  //--ICU/ETRI FMO Implementation : FMO stuff start
  rcLayer.m_bSliceGroupChangeDirection_flag = ( bSliceGroupChangeDirection_flag != 0 );
  RNOK( xReadSliceGroupCfg( rcLayer)); //Slice group configuration file
  //--ICU/ETRI FMO Implementation : FMO stuff end

  // ROI Config ICU/ETRI
  RNOK( xReadROICfg( rcLayer)); 

  ::fclose(f);

  return Err::m_nOK;
}

ErrVal EncoderCodingParameter::xReadSliceGroupCfg( h264::LayerParameters&  rcLayer )
{
	UInt mapunit_height;
	UInt mb_height;
	UInt i;
	UInt mb_width;
 	FILE* sgfile=NULL;

	if( (rcLayer.getNumSliceGroupsMinus1()!=0)&&
		((rcLayer.getSliceGroupMapType() == 0) || (rcLayer.getSliceGroupMapType() == 2) || (rcLayer.getSliceGroupMapType() == 6)) )
	{ 
    if ( ! rcLayer.getSliceGroupConfigFileName().empty() &&
         ( sgfile = fopen( rcLayer.getSliceGroupConfigFileName().c_str(), "r" ) ) == NULL )
		{
      printf("Error open file %s", rcLayer.getSliceGroupConfigFileName().c_str() );
		}
		else
		{
			if (rcLayer.getSliceGroupMapType() == 0) 
			{
				for(i=0;i<=rcLayer.getNumSliceGroupsMinus1();i++)
				{
					fscanf(sgfile,"%d",(rcLayer.getArrayRunLengthMinus1()+i));
					fscanf(sgfile,"%*[^\n]");

				}
			}
			else if (rcLayer.getSliceGroupMapType() == 2)
			{
				// every two lines contain 'top_left' and 'bottom_right' value
				for(i=0;i<rcLayer.getNumSliceGroupsMinus1();i++)
				{
					fscanf(sgfile,"%d",(rcLayer.getArrayTopLeft()+i));
					fscanf(sgfile,"%*[^\n]");
					fscanf(sgfile,"%d",(rcLayer.getArrayBottomRight()+i));
					fscanf(sgfile,"%*[^\n]");
				}

			}
			else if (rcLayer.getSliceGroupMapType()== 6)
			{
				//--ICU/ETRI
				//TODO : currently map type 6 is partially supported 
				// Assume that only frame mode(no interlaced mode) is available
				// Assume that Frame cropping is not avaliable

				Int tmp;

				/*
				frame_mb_only = !(input->getPicInterlace() || input->getMbInterlace());
				mb_width= (input->get_img_width()+img->get_auto_crop_right())/16;
				mb_height= (input->get_img_height()+img->get_auto_crop_bottom())/16;
				mapunit_height=mb_height/(2-frame_mb_only);
				*/

				
				mb_width= (rcLayer.getFrameWidth())/16;
				mb_height= (rcLayer.getFrameHeight())/16;
				mapunit_height=mb_height;


				// each line contains slice_group_id for one Macroblock
				for (i=0;i<mapunit_height*mb_width;i++)
				{
					fscanf(sgfile,"%d", &tmp);
					//input->set_slice_group_id_ith( i, (unsigned) tmp);
					rcLayer.setSliceGroupId(i,(UInt)tmp);
					assert(*(rcLayer.getArraySliceGroupId()+i) <= rcLayer.getNumSliceGroupsMinus1() );
					fscanf(sgfile,"%*[^\n]");
				}

			}
			fclose(sgfile);

		}
	}
	return Err::m_nOK;

}


// ROI Config Read ICU/ETRI
ErrVal EncoderCodingParameter::xReadROICfg( h264::LayerParameters&  rcLayer )
{
	UInt i;
 	FILE* roifile=NULL;

	if ( (0 < rcLayer.getNumROI()) )
	{
		if ( ! rcLayer.getROIConfigFileName().empty() &&
         ( roifile = fopen( rcLayer.getROIConfigFileName().c_str(), "r" ) ) == NULL )
		{
			printf("Error open file %s", rcLayer.getROIConfigFileName().c_str() );
		}

		else
		{
			// every two lines contain 'top_left' and 'bottom_right' value
			for(i=0;i<rcLayer.getNumROI(); i++)
			{
				fscanf(roifile, "%d",(rcLayer.getROIID()+i));
				fscanf(roifile, "%*[^\n]");
				fscanf(roifile, "%d",(rcLayer.getSGID()+i));
				fscanf(roifile, "%*[^\n]");
				fscanf(roifile, "%d",(rcLayer.getSLID()+i));
				fscanf(roifile, "%*[^\n]");
			}

			fclose(roifile);
		}

	}
	
	
	return Err::m_nOK;
}

#endif // !defined(AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_)
