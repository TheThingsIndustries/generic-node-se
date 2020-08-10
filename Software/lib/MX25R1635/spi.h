/******************************************************************************
*
* @file spi.h
*
*
* Ver   Who   Date        Changes
* 1.00  AZ      02/23/17    First release
*
******************************************************************************/

#ifndef SPI_H_
#define SPI_H_

#include "mxic_hc.h"

int MxSpiFlashWrite(MxSpi *Spi, u32 Addr, u32 ByteCount, u8 *WrBuf, u8 WrCmd);
int MxSpiFlashRead(MxSpi *Spi, u32 Addr, u32 ByteCount, u8 *RdBuf, u8 RdCmd);

#endif /* SPI_H_ */
