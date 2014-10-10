/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).  

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/


/*****************************************************************************
 * name:		snd_dma.c
 *
 * desc:		main control for any streaming sound output device
 *
 * $Archive: /Wolfenstein MP/src/client/snd_dma.c $
 *
 *****************************************************************************/

#include "snd_local.h"
//#include "client.h"


snd_t snd;  // globals for sound

// Ridah, streaming sounds
// !! NOTE: the first streaming sound is always the music
int numStreamingSounds = 0;

// =======================================================================
// Internal sound data & structures
// =======================================================================

// only begin attenuating sound volumes when outside the FULLVOLUME range
#define     SOUND_FULLVOLUME    80

#define     SOUND_ATTENUATE     0.0008f
#define     SOUND_RANGE_DEFAULT 1250

dma_t dma;

int s_soundtime;                // sample PAIRS
int s_paintedtime;              // sample PAIRS

// MAX_SFX may be larger than MAX_SOUNDS because
// of custom player sounds
#define     MAX_SFX         4096
sfx_t s_knownSfx[MAX_SFX];


/*
================
S_HashSFXName

return a hash value for the sfx name
================
*/
static long S_HashSFXName( const char *name ) {
	int i;
	long hash;
	char letter;

	hash = 0;
	i = 0;
	while ( name[i] != '\0' ) {
		letter = tolower( name[i] );
		if ( letter == '.' ) {
			break;                          // don't include extension
		}
		if ( letter == '\\' ) {
			letter = '/';                   // damn path names
		}
		hash += (long)( letter ) * ( i + 119 );
		i++;
	}
	hash &= ( LOOP_HASH - 1 );
	return hash;
}

/*
==================
S_FindName

Will allocate a new sfx if it isn't found
==================
*/
static sfx_t *S_FindName( const char *name ) {
	int i;
	int hash;

	sfx_t   *sfx;

	if ( !name ) {
		//Com_Error (ERR_FATAL, "S_FindName: NULL\n");
		name = "*default*";
	}
	if ( !name[0] ) {
		//Com_Error (ERR_FATAL, "S_FindName: empty name\n");
		name = "*default*";
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Error( ERR_FATAL, "Sound name too long: %s", name );
	}

	hash = S_HashSFXName( name );

	sfx = snd.sfxHash[hash];
	// see if already loaded
	while ( sfx ) {
		if ( !Q_stricmp( sfx->soundName, name ) ) {
			return sfx;
		}
		sfx = sfx->next;
	}

	// find a free sfx
	for ( i = 0 ; i < snd.s_numSfx ; i++ ) {
		if ( !s_knownSfx[i].soundName[0] ) {
			break;
		}
	}

	if ( i == snd.s_numSfx ) {
		if ( snd.s_numSfx == MAX_SFX ) {
			Com_Error( ERR_FATAL, "S_FindName: out of sfx_t" );
		}
		snd.s_numSfx++;
	}

	sfx = &s_knownSfx[i];
	Com_Memset( sfx, 0, sizeof( *sfx ) );
	strcpy( sfx->soundName, name );

	sfx->next = snd.sfxHash[hash];
	snd.sfxHash[hash] = sfx;

	return sfx;
}


/*
==================
S_RegisterSound

Creates a default buzz sound if the file can't be loaded
==================
*/
sfxHandle_t S_RegisterSound( const char *name, qboolean compressed ) {
	sfx_t   *sfx;

	//compressed = qfalse; // Arnout: memory corruption with compressed sounds?

	if ( !snd.s_soundStarted ) {
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Printf( "Sound name exceeds MAX_QPATH\n" );
		return 0;
	}

	sfx = S_FindName( name );
	if ( sfx->soundData ) {
		if ( sfx->defaultSound ) {
			if ( com_developer->integer ) {
				Com_Printf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->soundName );
			}
			return 0;
		}
		return sfx - s_knownSfx;
	}

	sfx->inMemory = qfalse;
	sfx->soundCompressed = compressed;

//	if (!compressed) {
	S_memoryLoad( sfx );
//	}

	if ( sfx->defaultSound ) {
		if ( com_developer->integer ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->soundName );
		}
		return 0;
	}

	return sfx - s_knownSfx;
}

/*
=================
S_memoryLoad
=================
*/
void S_memoryLoad( sfx_t *sfx ) {
	sfx->inMemory = qfalse;
}

//=============================================================================



/*
==============================================================

continuous looping sounds are added each frame

==============================================================
*/


//=============================================================================


//=============================================================================


/*
===============================================================================

console functions

===============================================================================
*/

/*
===============================================================================

STREAMING SOUND

===============================================================================
*/

//----(SA)	end

// START	xkan, 9/23/2002
// returns how long the sound lasts in milliseconds
int S_GetSoundLength( sfxHandle_t sfxHandle ) {
	if ( sfxHandle < 0 || sfxHandle >= snd.s_numSfx ) {
		Com_DPrintf( S_COLOR_YELLOW "S_StartSound: handle %i out of range\n", sfxHandle );
		return -1;
	}
	return (int)( (float)s_knownSfx[ sfxHandle ].soundLength / dma.speed * 1000.0 );
}
// END		xkan, 9/23/2002

// ydnar: for looped sound synchronization
int S_GetCurrentSoundTime( void ) {
	return s_soundtime + dma.speed;
//	 return s_paintedtime;
}
