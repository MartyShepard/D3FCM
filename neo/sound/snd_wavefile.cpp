/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "snd_local.h"

//-----------------------------------------------------------------------------
// Name: idWaveFile::idWaveFile()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.  
//       Then call Read() as needed.  Calling the destructor or Close() 
//       will close the file.  
//-----------------------------------------------------------------------------
idWaveFile::idWaveFile( void ) {
	memset( &mpwfx, 0, sizeof( waveformatextensible_t ) );
	mhmmio		= NULL;
	mdwSize		= 0;
	mseekBase	= 0;
	mbIsReadingFromMemory = false;
	mpbData		= NULL;
	ogg			= NULL;
	isOgg		= false;
}

//-----------------------------------------------------------------------------
// Name: idWaveFile::~idWaveFile()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
idWaveFile::~idWaveFile( void ) {
	Close();

	if ( mbIsReadingFromMemory && mpbData ) {
		Mem_Free( mpbData );
	}

	memset( &mpwfx, 0, sizeof( waveformatextensible_t ) );
}

//-----------------------------------------------------------------------------
// Name: idWaveFile::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
int idWaveFile::Open( const char* strFileName, waveformatex_t* pwfx ) {

	mbIsReadingFromMemory = false;

	mpbData     = NULL;
	mpbDataCur  = mpbData;

	if( strFileName == NULL ) {
		return -1;
	}

	idStr name = strFileName;

	if (idSoundSystemLocal::cmp_DefaultOGGLoading.GetInteger() == 1)
	{
		// note: used to only check for .wav when making a build
		name.SetFileExtension(".ogg");
		if (fileSystem->ReadFile(name, NULL, NULL) != -1) {
			return OpenOGG(name, pwfx);
		}

		memset(&mpwfx, 0, sizeof(waveformatextensible_t));

		mhmmio = fileSystem->OpenFileRead(strFileName);
		if (!mhmmio) {
			mdwSize = 0;
			return -1;
		}
		if (mhmmio->Length() <= 0) {
			mhmmio = NULL;
			return -1;
		}
		if (ReadMMIO() != 0) {
			// ReadMMIO will fail if its an not a wave file
			Close();
			return -1;
		}
	}
	else
	{
		/* Marty
		* Load Waves Files. No idea why Ogg files use as Standard. if you have
		* wave files this will only load as ogg with name.SetFileExtension(".ogg")
		* I changed this code to load both wave as first, ogg as second.
		*/
		#if defined DEBUG
			if (idStr::Cmp(strFileName, "sound/vo/intro/intro_test12.wav")==0)
				TRACE("Moment");
		#endif
		memset(&mpwfx, 0, sizeof(waveformatextensible_t)); /* ok ? as first */

		mhmmio = fileSystem->OpenFileRead(strFileName);

		if (!mhmmio || mhmmio->Length() <= 0 || ReadMMIO() != 0)
		{
			mdwSize = 0;
			mhmmio	= NULL;
			
			Close();

			/* Load as Ogg */
			name.SetFileExtension(".ogg");
			if (fileSystem->ReadFile(name, NULL, NULL) != -1)
			{
				#if defined DEBUG
					TRACE("idWaveFile::Open (!mhmmio) as OGG '%s'\r", strFileName);
				#endif		
				return OpenOGG(name, pwfx);
			}
		}		
	}

	mfileTime = mhmmio->Timestamp();

	if ( ResetFile() != 0 ) {
		Close();
		return -1;
	}

	#if defined DEBUG
		TRACE("idWaveFile::Open as WAVE '%s'\r", strFileName);
	#endif // defined DEBUG	

	// After the reset, the size of the wav file is mck.cksize so store it now
	mdwSize = mck.cksize / sizeof( short );
	mMemSize = mck.cksize;

	if ( mck.cksize != 0xffffffff ) {
		if ( pwfx ) {
			memcpy( pwfx, (waveformatex_t *)&mpwfx, sizeof(waveformatex_t));
		}
		return 0;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Name: idWaveFile::OpenFromMemory()
// Desc: copy data to idWaveFile member variable from memory
//-----------------------------------------------------------------------------
int idWaveFile::OpenFromMemory( short* pbData, int ulDataSize, waveformatextensible_t* pwfx ) {
	mpwfx       = *pwfx;
	mulDataSize = ulDataSize;
	mpbData     = pbData;
	mpbDataCur  = mpbData;
	mdwSize		= ulDataSize / sizeof( short );
	mMemSize	= ulDataSize;
	mbIsReadingFromMemory = true;

	return 0;
}

//-----------------------------------------------------------------------------
// Name: idWaveFile::ReadMMIO()
// Desc: Support function for reading from a multimedia I/O stream.
//       mhmmio must be valid before calling.  This function uses it to
//       update mckRiff, and mpwfx. 
//-----------------------------------------------------------------------------
int idWaveFile::ReadMMIO( void ) {
	mminfo_t		ckIn;           // chunk info. for general use.
	pcmwaveformat_t pcmWaveFormat;  // Temp PCM structure to load in.       

	memset( &mpwfx, 0, sizeof( waveformatextensible_t ) );

	mhmmio->Read( &mckRiff, 12 );
	assert( !isOgg );
	mckRiff.ckid = LittleLong( mckRiff.ckid );
	mckRiff.cksize = LittleLong( mckRiff.cksize );
	mckRiff.fccType = LittleLong( mckRiff.fccType );
	mckRiff.dwDataOffset = 12;

	// Check to make sure this is a valid wave file
	if( (mckRiff.ckid != fourcc_riff) || (mckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) ) {
		common->Printf("^3---- Sample File: ^8WAV ^1%s ^0( Has no Wave or Riff Header)\n", mhmmio->GetName());
		return -1;
	}

	// Search the input file for for the 'fmt ' chunk.
	ckIn.dwDataOffset = 12;
	do {
		if (8 != mhmmio->Read( &ckIn, 8 ) ) {
			return -1;
		}
		assert( !isOgg );
		ckIn.ckid = LittleLong( ckIn.ckid );
		ckIn.cksize = LittleLong( ckIn.cksize );
		ckIn.dwDataOffset += ckIn.cksize-8;
	} while (ckIn.ckid != mmioFOURCC('f', 'm', 't', ' '));

	// Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
	// if there are extra parameters at the end, we'll ignore them
	if( ckIn.cksize < sizeof(pcmwaveformat_t) ) {
		return -1;
	}

	// Read the 'fmt ' chunk into <pcmWaveFormat>.
	if( mhmmio->Read( &pcmWaveFormat, sizeof(pcmWaveFormat) ) != sizeof(pcmWaveFormat) ) {
		return -1;
	}
	assert( !isOgg );
	pcmWaveFormat.wf.wFormatTag = LittleShort( pcmWaveFormat.wf.wFormatTag );
	pcmWaveFormat.wf.nChannels = LittleShort( pcmWaveFormat.wf.nChannels );
	pcmWaveFormat.wf.nSamplesPerSec = LittleLong( pcmWaveFormat.wf.nSamplesPerSec );
	pcmWaveFormat.wf.nAvgBytesPerSec = LittleLong( pcmWaveFormat.wf.nAvgBytesPerSec );
	pcmWaveFormat.wf.nBlockAlign = LittleShort( pcmWaveFormat.wf.nBlockAlign );
	pcmWaveFormat.wBitsPerSample = LittleShort( pcmWaveFormat.wBitsPerSample );

	// Copy the bytes from the pcm structure to the waveformatex_t structure
	memcpy( &mpwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );

	/* Marty -- Show WAV Sample Loading*/
	if (idSoundSystemLocal::s_ShowSampleWAVLoading.GetInteger() == 1)
	{
		switch (pcmWaveFormat.wf.nSamplesPerSec)
		{
		case 11025:
		case 22050:
		case 44100:
			break;
		default:
			common->Printf("^3---- Sample File: ^8WAV ^1%s ^0( Channels:%s - ^1%dHZ^0 - Bits:%d )\n", mhmmio->GetName(), pcmWaveFormat.wf.nChannels ? "Mono" : "Stereo", pcmWaveFormat.wf.nSamplesPerSec, pcmWaveFormat.wBitsPerSample);
			common->Warning("Sample File: %s  Format Expected 11025HZ or 22050HZ or 41000HZ", mhmmio->GetName());
			return -1;
		}

		if (pcmWaveFormat.wBitsPerSample != 16) {
			common->Printf("^3---- Sample File: ^8WAV ^1%s ^0( Channels:%s - %dhz - Bits:^1%d^0 )\n", mhmmio->GetName(), pcmWaveFormat.wf.nChannels ? "Mono" : "Stereo", pcmWaveFormat.wf.nSamplesPerSec, pcmWaveFormat.wBitsPerSample);
			common->Warning("Sample File: %s  Format Expected 16it", mhmmio->GetName());
			return -1;
		}
	}

	// Allocate the waveformatex_t, but if its not pcm format, read the next
	// word, and thats how many extra bytes to allocate.
	if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_TAG_PCM) {
			mpwfx.Format.cbSize = 0;
	} else {
			common->Printf("^3-??- Sample File: ^8WAV ^1%s ^0( Channels:%s - %dhz - Bits:%d)\n", mhmmio->GetName(),	pcmWaveFormat.wf.wFormatTag,pcmWaveFormat.wf.nChannels ? "Mono" : "Stereo",	pcmWaveFormat.wf.nSamplesPerSec, pcmWaveFormat.wBitsPerSample);			
		
			return 1;	// we don't handle these (32 bit wavefiles, etc)
#if 0
		// Read in length of extra bytes.
		word cbExtraBytes = 0L;
		if( mhmmio->Read( (char*)&cbExtraBytes, sizeof(word) ) != sizeof(word) )
			return -1;

		mpwfx.Format.cbSize = cbExtraBytes;

		// Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
		if( mhmmio->Read( (char*)(((byte*)&(mpwfx.Format.cbSize))+sizeof(word)), cbExtraBytes ) != cbExtraBytes ) {
			memset( &mpwfx, 0, sizeof( waveformatextensible_t ) );
			return -1;
		}
#endif
	}

	if (idSoundSystemLocal::s_ShowSampleWAVLoading.GetInteger() == 1) /* Marty -- Show WAV Sample Loading*/
		common->Printf("^3Open Sample File: ^8WAV ^0%s\n", mhmmio->GetName());

	return 0;
}

//-----------------------------------------------------------------------------
// Name: idWaveFile::ResetFile()
// Desc: Resets the internal mck pointer so reading starts from the 
//       beginning of the file again 
//-----------------------------------------------------------------------------
int idWaveFile::ResetFile( void ) {
	if( mbIsReadingFromMemory ) {
		mpbDataCur = mpbData;
	} else  {
		if( mhmmio == NULL ) {
			return -1;
		}

		// Seek to the data
		if( -1 == mhmmio->Seek( mckRiff.dwDataOffset + sizeof(fourcc), FS_SEEK_SET ) ) {
			return -1;
		}

		// Search the input file for for the 'fmt ' chunk.
		mck.ckid = 0;
		do {
			byte ioin;
			if ( !mhmmio->Read( &ioin, 1 ) ) {
				return -1;
			}
			mck.ckid = (mck.ckid>>8) | (ioin<<24);
		} while (mck.ckid != mmioFOURCC('d', 'a', 't', 'a'));
		
		mhmmio->Read( &mck.cksize, 4 );
		assert( !isOgg );
		mck.cksize = LittleLong( mck.cksize );
		mseekBase = mhmmio->Tell();
	}
	
	return 0;
}

//-----------------------------------------------------------------------------
// Name: idWaveFile::Read()
// Desc: Reads section of data from a wave file into pBuffer and returns 
//       how much read in pdwSizeRead, reading not more than dwSizeToRead.
//       This uses mck to determine where to start reading from.  So 
//       subsequent calls will be continue where the last left off unless 
//       Reset() is called.
//-----------------------------------------------------------------------------
int idWaveFile::Read( byte* pBuffer, int dwSizeToRead, int *pdwSizeRead ) {

	if ( ogg != NULL ) {

		return ReadOGG( pBuffer, dwSizeToRead, pdwSizeRead );

	} else if ( mbIsReadingFromMemory ) {

		if( mpbDataCur == NULL ) {
			return -1;
		}
		if( (byte*)(mpbDataCur + dwSizeToRead) > (byte*)(mpbData + mulDataSize) ) {
			dwSizeToRead = mulDataSize - (int)(mpbDataCur - mpbData);
		}
 		SIMDProcessor->Memcpy( pBuffer, mpbDataCur, dwSizeToRead );
		mpbDataCur += dwSizeToRead;

		if ( pdwSizeRead != NULL ) {
			*pdwSizeRead = dwSizeToRead;
		}

		return dwSizeToRead;

	} else {

		if( mhmmio == NULL ) {
			return -1;
		}
		if( pBuffer == NULL ) {
			return -1;
		}
		
		dwSizeToRead = mhmmio->Read( pBuffer, dwSizeToRead );
		// this is hit by ogg code, which does it's own byte swapping internally
		if ( !isOgg ) {
			LittleRevBytes( pBuffer, 2, dwSizeToRead / 2 );
		}

		if ( pdwSizeRead != NULL ) {
			*pdwSizeRead = dwSizeToRead;
		}

		return dwSizeToRead;
	}
}

//-----------------------------------------------------------------------------
// Name: idWaveFile::Close()
// Desc: Closes the wave file 
//-----------------------------------------------------------------------------
int idWaveFile::Close( void ) {
	if ( ogg != NULL ) {
		return CloseOGG();
	}
	if( mhmmio != NULL ) {
		fileSystem->CloseFile( mhmmio );
		mhmmio = NULL;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: idWaveFile::Seek()
//-----------------------------------------------------------------------------
int idWaveFile::Seek( int offset ) {

	if ( ogg != NULL ) {

		common->FatalError( "idWaveFile::Seek: cannot seek on an OGG file\n" );

	} else if( mbIsReadingFromMemory ) {

		mpbDataCur = mpbData + offset;

	} else {
		if( mhmmio == NULL ) {
			return -1;
		}

		if ((int)(offset+mseekBase) == mhmmio->Tell()) {
			return 0;
		}
		mhmmio->Seek( offset + mseekBase, FS_SEEK_SET );
		return 0;
	}
	return -1;
}
