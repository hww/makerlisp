/* Firmware update routine */
#include <stdint.h>
#include <sys/attribs.h>
#include <xc.h>

#include "fwupd.h"

/* Flash control */
#define FLASH_UNLOCK_KEY 0xAA996655
#define WRITE_DWORD_CODE (_NVMCON_WREN_MASK | 2)
#define ERASE_PAGE_CODE  (_NVMCON_WREN_MASK | 4)
#define PHYS_ADDR_MASK 0x1fffffff
#define PAGESIZE 2048

/* ESC command parameters */
static int escpn;
static uint8_t escparm[8];

/* Polled UART routines */
__ramfunc__ static uint8_t UART_Read()
{
    while (!(U1STA & _U1STA_URXDA_MASK));

    return U1RXREG;
}
__ramfunc__ static void UART_Write(uint8_t c)
{
    while (U1STA & _U1STA_UTXBF_MASK);

    U1TXREG = c;
}

/* Get ESC command parameters */
__ramfunc__ static uint8_t get1p(uint8_t i)
{
    int c, n, r;

    c = n = r = 0;
    while (isdigit((c = UART_Read())))
    {
        r = r*10;
        r = r + (c - '0');
        n = 1;
    }

    escparm[i] = r;
    escpn += n;

    return c;
}
__ramfunc__ static uint8_t getescp()
{
    int c, i;

    i = 0;
    escpn = 0;
    while (i < sizeof(escparm))
    {
        c = get1p(i);
        if (!isdigit(c) && !(c == ';'))
        {
            break;
        }
        ++i;
    }

    return c;
}

/* Flash programming routines */
__ramfunc__ static void FLASH_NVMUnlock(uint32_t operation)
{
    unsigned int status;

    // Flash operation to perform
    NVMCON = operation;

    // Write Keys
    NVMKEY = FLASH_UNLOCK_KEY;
    NVMKEY = ~FLASH_UNLOCK_KEY;

    // Start the operation using the Set Register
    NVMCONSET = _NVMCON_WR_MASK;

    // Wait for operation to complete
    while (NVMCON & _NVMCON_WR_MASK);

    // Disable NVM write enable
    NVMCONCLR = _NVMCON_WREN_MASK;
 }
__ramfunc__ static void FLASH_ErasePage(uint32_t address)
{
    // Set NVMADDR to the Start Address of page to erase
    NVMADDR = address & PHYS_ADDR_MASK;
    
    // Unlock and Erase Page
    FLASH_NVMUnlock(ERASE_PAGE_CODE);
}
__ramfunc__ static void FLASH_WriteDoubleWord(uint32_t address, uint32_t Data0, uint32_t Data1  )
{
    // Load data into NVMDATA register
    NVMDATA0 = Data0;
    NVMDATA1 = Data1;
    
    // Load address to program into NVMADDR register
    NVMADDR = address & PHYS_ADDR_MASK;

    // Unlock and Write Word
    FLASH_NVMUnlock(WRITE_DWORD_CODE);
}
__ramfunc__ static void FLASH_UnprotectBoot()
{
    // Write Keys
    NVMKEY = FLASH_UNLOCK_KEY;
    NVMKEY = ~FLASH_UNLOCK_KEY;

    // Un-protect boot pages
    NVMBWP = 0;
 }

/* Program s section, reading 8 bytes at a time from the CPU */
__ramfunc__ static void progsect(uint32_t address)
{
    uint32_t a, d0, d1, page;
    
    a = address;
    page = -1;
    while (1) {
        
        /* Ask for data */
        UART_Write('?');

        /* Get ESC command and parameters */
        UART_Read();
        UART_Read();
        getescp();

        /* Less than 8 bytes sent means we're done */
        if (escpn < 8) {
            break;
        }

        /* Otherwise, program 8 bytes */
  
        /* New page ? Erase it */
        if (!((a/PAGESIZE) == page)) {
            page = a/PAGESIZE;
            FLASH_ErasePage(a);
        }

        /* Program a double-word */
        d0 = *(uint32_t *)&escparm[0];
        d1 = *(uint32_t *)&escparm[4];
        FLASH_WriteDoubleWord(a, d0, d1);
        
        a += 8;
    }
}

/* Firmware update - program double-words sent by CPU, then hang */
__longramfunc__ void fwupd()
{
    /* Disable interrupts */
    __builtin_disable_interrupts();

    /* Un-protect boot pages */
    FLASH_UnprotectBoot();
    
    /* Program APP section, then BOOT */
    progsect(0x9d000000);
    progsect(0xbfc00000);

    /* Acknowledge completion */
    UART_Write('!');

    /* Done, hang */
    while (1);
}
