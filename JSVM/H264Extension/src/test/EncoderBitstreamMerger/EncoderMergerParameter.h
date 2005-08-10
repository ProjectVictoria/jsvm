/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************
This software module was originally developed by

CAMMAS Nathalie (France T�l�com)

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

To the extent that France T�l�com owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, France T�l�com will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

France T�l�com retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005.

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The France T�l�com hereby donate this source code to the ITU, with the following
understanding:
    1. France T�l�com retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. France T�l�com retain full patent rights (if any exist) in the technical
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
#ifndef _CODER_EXTRACTOR_PARAMETER_H_
#define _CODER_EXTRACTOR_PARAMETER_H_

#include "Typedefs.h"
#include "string.h" 
#include "H264AVCEncoderLib.h"
#include "H264AVCCommonLib.h"
#include "CodingParameter.h"

typedef struct
{
  UInt    uiNumberOfLayers;
  Char    pBitstreamFile[256];
  Int     nResult; 
  UInt    nFrames;
}EncoderIoParameter;

class EncoderMergerParameter:
	public h264::CodingParameter
   
{
public:

    Int m_uiNumOfLayer;
	std::string  m_acInFile[MAX_LAYERS];
	std::string m_cOutFile;
    UInt m_uiGOPSize[MAX_LAYERS];
	Bool m_bDS;
	Bool m_bNivQ;

	std::string    m_cDistoFilename[MAX_LAYERS];
    std::string	 m_cFGSRateFilename[MAX_LAYERS];

	EncoderMergerParameter();
	~EncoderMergerParameter();

	static ErrVal create( EncoderMergerParameter*& rpcEncoderMergerParameter );
	ErrVal destroy();
	void init(Int argc, char **argv );
	const std::string&    getInFile           (Int i)            const { return m_acInFile[i];         }
    const std::string&    getOutFile          ()            const { return m_cOutFile;        }

	std::string& getFGSRateFilename(UInt uiLayer) { return m_cFGSRateFilename[uiLayer];}
    std::string& getDistoFilename(UInt uiLayer) { return m_cDistoFilename[uiLayer];}

	Bool getInsertDS() { return m_bDS;}
	Bool getInsertQL() { return m_bNivQ;}

};
#endif

