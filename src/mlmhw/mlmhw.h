/* MakerLisp Machine low-level hardware support */
#ifndef _MLMHW_H_
#define _MLMHW_H_

#include <defines.h>

/* Initialize MLM devices */
extern void init_mlmhw();

/* SD Card access */
extern int init_sdcard();
extern int read_sdcard(BYTE *buf, UINT32 n, UINT blks);
extern int write_sdcard(BYTE *buf, UINT32 n);

/* Get time from RTC */
extern UINT8 *get_time();

/* Is expansion board attached ? */
extern int mlmeb();

/* Read and write registers */
extern int _readreg(int regaddr);
extern void _writereg(int regaddr, int d);

/* SPI mode and rate */
extern int _mode_spi(int d);

/* SPI data read/write */
extern char _xchg_spi(char d);

#endif /* _MLMHW_H_ */
