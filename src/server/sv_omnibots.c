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

// sv_bot.c

#include "server.h"
#include "sv_imports.h"
//#include "../game/botlib.h"
//#include "../botai/botai.h"

// omnibot compatibility
/*
==================
SV_BotAllocateClient
==================
*/
int SV_BotAllocateClient( int clientNum ) {
	int i;
	client_t    *cl;

	// Arnout: added possibility to request a clientnum
	if ( clientNum > 0 ) {
		if ( clientNum >= sv_maxclients->integer ) {
			return -1;
		}

		cl = &svs.clients[clientNum];
		if ( cl->state != CS_FREE ) {
			return -1;
		} else {
			i = clientNum;
		}
	} else {
		// find a client slot
		for ( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ ) {
			// Wolfenstein, never use the first slot, otherwise if a bot connects before the first client on a listen server, game won't start
			if ( i < 1 ) {
				continue;
			}
			// done.
			if ( cl->state == CS_FREE ) {
				break;
			}
		}
	}

	if ( i == sv_maxclients->integer ) {
		return -1;
	}

	cl->gentity = SV_GentityNum( i );
	cl->gentity->s.number = i;
	cl->state = CS_ACTIVE;
	cl->lastPacketTime = svs.time;
	cl->netchan.remoteAddress.type = NA_BOT;
	cl->rate = 16384;

	return i;
}

/*
==================
SV_BotFreeClient
==================
*/
void SV_BotFreeClient( int clientNum ) {
	client_t    *cl;

	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_BotFreeClient: bad clientNum: %i", clientNum );
	}
	cl = &svs.clients[clientNum];
	cl->state = CS_FREE;
	cl->name[0] = 0;
	if ( cl->gentity ) {
		cl->gentity->r.svFlags &= ~SVF_BOT;
	}
}

/*
==================
BotImport_Print
==================
*/
void QDECL BotImport_Print( int type, char *fmt, ... ) {
	char str[2048];
	va_list ap;

	va_start( ap, fmt );
	Q_vsnprintf( str, sizeof( str ), fmt, ap );
	va_end( ap );

	switch ( type ) {
	case PRT_MESSAGE: {
		Com_Printf( "%s", str );
		break;
	}
	case PRT_WARNING: {
		Com_Printf( S_COLOR_YELLOW "Warning: %s", str );
		break;
	}
	case PRT_ERROR: {
		Com_Printf( S_COLOR_RED "Error: %s", str );
		break;
	}
	case PRT_FATAL: {
		Com_Printf( S_COLOR_RED "Fatal: %s", str );
		break;
	}
	case PRT_EXIT: {
		Com_Error( ERR_DROP, S_COLOR_RED "Exit: %s", str );
		break;
	}
	default: {
		Com_Printf( "unknown print type\n" );
		break;
	}
	}
}

/*
==================
BotImport_FreeZoneMemory
==================
*/
void BotImport_FreeZoneMemory( void ) {
	Z_FreeTags( TAG_BOTLIB );
}


//
//  * * * BOT AI CODE IS BELOW THIS POINT * * *
//

/*
==================
SV_BotGetConsoleMessage
==================
*/
int SV_BotGetConsoleMessage( int client, char *buf, int size ) {
	client_t    *cl;
	int index;

	cl = &svs.clients[client];
	cl->lastPacketTime = svs.time;

	if ( cl->reliableAcknowledge == cl->reliableSequence ) {
		return qfalse;
	}

	cl->reliableAcknowledge++;
	index = cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 );

	if ( !cl->reliableCommands[index][0] ) {
		return qfalse;
	}

	//Q_strncpyz( buf, cl->reliableCommands[index], size );
	return qtrue;
}

