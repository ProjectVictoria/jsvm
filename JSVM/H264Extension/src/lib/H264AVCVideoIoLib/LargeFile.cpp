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




#include "H264AVCVideoIoLib.h"

#include "LargeFile.h"

#include <errno.h>

#if defined( MSYS_WIN32 )
# include <io.h>
# include <sys/stat.h>
# include <fcntl.h>
#else
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
# include <sys/ioctl.h>
# include <sys/types.h>
# include <fcntl.h>
# include <dlfcn.h>
# include <cerrno>
# include <unistd.h>
#endif


LargeFile::LargeFile() :
	m_iFileHandle( -1 )
{
}


LargeFile::~LargeFile()
{
	if( -1 != m_iFileHandle )
	{
		::close( m_iFileHandle );
	}
}


ErrVal LargeFile::open( const std::string& rcFilename, enum OpenMode eOpenMode, int iPermMode )
{
	ROT( rcFilename.empty() );
	ROF( -1 == m_iFileHandle );

	int iOpenMode;

#if defined( MSYS_WIN32 )

	if( eOpenMode == OM_READONLY )
	{
		iOpenMode = _O_RDONLY;
	}
	else if( eOpenMode == OM_WRITEONLY )
	{
		iOpenMode = _O_CREAT | _O_TRUNC | _O_WRONLY;
	}
	else if( eOpenMode == OM_APPEND )
	{
    //append mode does not imply write access and the create flag
    //simplifies the program's logic (in most cases).
		iOpenMode = _O_APPEND | _O_CREAT | _O_WRONLY;
	}
	else if( eOpenMode == OM_READWRITE )
	{
		iOpenMode = _O_CREAT | _O_RDWR;
	}
	else
	{
		AOT( 1 );
		return Err::m_nERR;
	}

	iOpenMode |= _O_SEQUENTIAL | _O_BINARY;

	m_iFileHandle = ::open( rcFilename.c_str(), iOpenMode, iPermMode );
	
#elif defined( MSYS_UNIX_LARGEFILE )

	if( eOpenMode == OM_READONLY )
	{
		iOpenMode = O_RDONLY;
	}
	else if( eOpenMode == OM_WRITEONLY )
	{
		iOpenMode = O_CREAT | O_TRUNC | O_WRONLY;
	}
	else if( eOpenMode == OM_APPEND )
	{
		iOpenMode = O_APPEND | O_CREAT | O_WRONLY;
	}
	else if( eOpenMode == OM_READWRITE )
	{
		iOpenMode = O_CREAT | O_RDWR;
	}
	else
	{
		AOT( 1 );
		return Err::m_nERR;
	}

	m_iFileHandle = open64( rcFilename.c_str(), iOpenMode, iPermMode );

#endif

  // check if file is really open
  ROTS( -1 == m_iFileHandle );

  // and return
	return Err::m_nOK;
}

ErrVal LargeFile::close()
{
	int iRetv;

	ROTS( -1 == m_iFileHandle );
	
	iRetv = ::close( m_iFileHandle );

	m_iFileHandle = -1;
	
	return ( iRetv == 0 ) ? Err::m_nOK : Err::m_nERR;
}


ErrVal LargeFile::seek( Int64 iOffset, int iOrigin )
{ 
	Int64 iNewOffset;
	ROT( -1 == m_iFileHandle );

#if defined( MSYS_WIN32 )
	iNewOffset = _lseeki64( m_iFileHandle, iOffset, iOrigin );
#elif defined( MSYS_UNIX_LARGEFILE )
	iNewOffset = lseek64( m_iFileHandle, iOffset, iOrigin );
#endif
	
	return ( iNewOffset == -1 ) ? Err::m_nERR : Err::m_nOK;
}


Int64 LargeFile::tell()
{
	ROTR( -1 == m_iFileHandle, -1 );
  Int64 iOffset;
#if defined( MSYS_WIN32 )
	iOffset = _telli64( m_iFileHandle );
#elif defined( MSYS_UNIX_LARGEFILE )
	iOffset = lseek64( m_iFileHandle, 0, SEEK_CUR );
#endif
  ROT( iOffset == -1 )

  // and return
	return iOffset;
}


ErrVal LargeFile::read( Void *pvBuffer, UInt32 uiCount, UInt32& ruiBytesRead )
{
	int iRetv;

	ROT( -1 == m_iFileHandle );
	ROT( 0 == uiCount );
	
	ruiBytesRead = 0;

	iRetv = ::read( m_iFileHandle, pvBuffer, uiCount );
	if( iRetv != (Int)uiCount )
	{
		//need to handle partial reads before hitting EOF
		
		//If the function tries to read at end of file, it returns 0. 
		//If the handle is invalid, or the file is not open for reading, 
		//or the file is locked, the function returns -1 and sets errno to EBADF.
		if( iRetv > 0 )
		{
			//partial reads are acceptable and return the standard success code. Anything
			//else must be implemented by the caller.
			ruiBytesRead = iRetv;
			return Err::m_nOK;
		}
		else if( iRetv == -1 )
		{
			return errno;
		}
		else if( iRetv == 0)
		{
			return Err::m_nEndOfFile;
		}
		else
		{
			AOF( ! "fix me, unexpected return code" );
			return Err::m_nERR;
		}
	}
	else
	{
		ruiBytesRead = uiCount;
	}

	ROF( iRetv == (Int)uiCount );

	return Err::m_nOK;
}


ErrVal LargeFile::write( const Void *pvBuffer, UInt32 uiCount )
{	
	int iRetv;

	ROT( -1 == m_iFileHandle );
	ROT( 0 == uiCount );

	iRetv = ::write( m_iFileHandle, pvBuffer, uiCount );
	ROF( iRetv == (Int)uiCount );

	return Err::m_nOK;
}


