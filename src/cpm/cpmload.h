//------------------------------------------------------------------------
// Copyright (c) 2019, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//------------------------------------------------------------------------
#ifndef CPMLOAD_H
#define CPMLOAD_H
//------------------------------------------------------------------------
#define CPM_RAM_SIZE      ( 65536 ) /* can be less than 64k but why? */
#define CPM_RAM_ALIGNMENT ( 65536 ) /* must always be 64k */
//------------------------------------------------------------------------
#define MEMSIZE  ( 62 ) /* give extra room for disk tables and buffers */
//------------------------------------------------------------------------
#define CCP      ( ( MEMSIZE - 7 ) * 1024 )
#define BDOS     ( CCP + 0x800 )
#define BIOS     ( CCP + 0x1600 )
//------------------------------------------------------------------------
void cpmLoadCcpBdos( void );
void loadCcpBdosBios( void );
void loadEntryAndStart( void );
void startCpm( void );
//------------------------------------------------------------------------
#endif // CPMLOAD_H
//------------------------------------------------------------------------
