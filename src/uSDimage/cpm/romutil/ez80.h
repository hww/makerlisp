/* eZ80 register I/O address definitions */
#define FLASH_KEY 0xF5
#define FLASH_DATA 0xF6
#define FLASH_FDIV 0xF9
#define FLASH_PROT 0xFA
#define FLASH_PAGE 0xFC
#define FLASH_ROW 0xFD
#define FLASH_COL 0xFE
#define FLASH_PGCTL 0xFF

/* Hop over to eZ80 ADL mode to do this I/O */
#define readreg(a) bios(7, a, 0)
#define writereg(a, d) bios(6, a, d)
