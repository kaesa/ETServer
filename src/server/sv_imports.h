#ifndef __SV_IMPORTS_H__
#define __SV_IMPORTS_H__

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

//Print types
#define PRT_MESSAGE             1
#define PRT_WARNING             2
#define PRT_ERROR               3
#define PRT_FATAL               4
#define PRT_EXIT                5


int SV_BotAllocateClient( int clientNum );
void SV_BotFreeClient( int clientNum );
__attribute__ ((format(printf, 2, 3))) void QDECL BotImport_Print(int type, char *fmt, ...);
void *BotImport_GetMemory( int size );
void BotImport_FreeMemory( void *ptr );
void BotImport_FreeZoneMemory( void );
void *BotImport_HunkAlloc( int size );
int SV_BotGetConsoleMessage( int client, char *buf, int size );

#endif
