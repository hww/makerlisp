//----------------------------------------------------------------------------
// Copyright (c) 2019, Christopher D. Farrar
//----------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#ifndef SDPARAMS_H
#define SDPARAMS_H
//----------------------------------------------------------------------------
#include "cpmdiskio.h"
//----------------------------------------------------------------------------
#define SD_CARD_DISKS (4)
//----------------------------------------------------------------------------
// note:
//
// The sd card has 4 drive images on it.
//
// Each image has reserved system tracks on them even though
// only the first disk needs systems tracks. This is to be able
// to have one disk parameter block to describe all four images.
//
// also the makerlisp-cpm is actually in rom so no system tracks are
// actually needed but they are included to ensure that the images
// are compatible with the romwbw sd card/cf card images. This allows
// easy system disk transfer between the computers.
//
//----------------------------------------------------------------------------
// The sd card has 4 drive images on it.
// Each image conforms to the following specifications:
// physical disk images:
//----------------------------------------------------------------------------
//     16640 sectors per disk
//       512 bytes per sector
//       256 sectors per track
//        65 tracks per disk
//   8519680 bytes per disk
//         1 reserved track each disk
//   8388608 bytes available for directory and data
//       512 directory entries
//      2048 allocation blocks
//         4 allocation blocks reserved for directory
//      4096 bytes per allocation block
//----------------------------------------------------------------------------
// cp/m sees the above as:
// logical disk images:
//----------------------------------------------------------------------------
//     66560 sectors per disk
//       128 bytes per sector
//      1024 sectors per track
//        65 tracks per disk
//   8519680 bytes per disk
//         4 reserved tracks each disk
//   8388608 bytes available for directory and data
//       512 directory entries
//      2048 allocation blocks
//         4 allocation blocks reserved for directory
//      4096 bytes per allocation block
//----------------------------------------------------------------------------
#define SD_TRACKS_PER_DISK      (65)
#define SD_SECTORS_PER_TRACK    (1024)
#define SD_BLOCK_SHIFT_FACTOR   (5)
#define SD_BLOCK_MASK           (31)
#define SD_EXTENT_MASK          (1)
#define SD_DISK_BLOCKS          (2048)
#define SD_DIRECTORY_ENTRIES    (512)
#define SD_ALLOCATION_BITMAP_0  (0xf0)
#define SD_ALLOCATION_BITMAP_1  (0x00)
#define SD_CHECKSUM_VECTOR_SIZE (SD_DIRECTORY_ENTRIES/4)
#define SD_ALLCATION_VECTOR_SIZE (((SD_DISK_BLOCKS - 1)/8)+1)
#define SD_TRACK_OFFSET         (1)       // system tracks for each disk
//----------------------------------------------------------------------------
// sector number translation (deblocking) 4*128 vs. 1*512
//----------------------------------------------------------------------------
#define SD_PHYSICAL_SECTOR_SIZE (512)
#define SD_LOGICAL_PER_PHYSICAL (SD_PHYSICAL_SECTOR_SIZE/LOGICAL_SECTOR_SIZE)
#define SD_SECTOR_MASK          (3)       // L_P_P - 1 for masking off sector bits
#define SD_SECTOR_SHIFT         (2)       // for / L_P_P
//----------------------------------------------------------------------------
#endif // SDPARAMS_H
//----------------------------------------------------------------------------
