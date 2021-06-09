/******************************************************************************
*
* @file mx_test.h
*
*
* Ver   Who   Date		Changes
* 1.00  RY	  03/23/17	First release
* 2.00  RY	  02/27/18	Change function names.
******************************************************************************/
#ifndef MX_TEST_H_
#define MX_TEST_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**************Request*******************/
#define PAGE     0
#define SECTOR   1
#define BLOCK32K 2
#define BLOCK64K 3
#define FLASH_INIT_ADDR 0x100

#define  Error_inc(x)   x = x + 1; \
                        Mx_printf("\t @Error Occur!!!\r\n");

#define RandomData( mem_exp, length ) \
          for( i=0; i< length; i=i+1 ) \
              mem_exp[i] = i%256;

#define CompareData( mem, mem_exp, flash_addr, compare_len, offset ) \
     Mx_printf("\t\t **Start Dump data=> address %8X\r\n", flash_addr); \
     Mx_printf("\t\t            Length=> %X\r\n", compare_len); \
    for( i=0 ; i < compare_len ; i=i+1 ){  \
      if( mem[offset+i] != mem_exp[offset+i] ){ \
          Error_inc( ErrorCnt ); \
      } \
      Mx_printf("\t\t READ address=> %08X\r\n", (flash_addr+i)); \
      Mx_printf("\t\t         data=> %02X\r\n", *(mem+offset+i)); \
      Mx_printf("\t\t     expected=> %02X\r\n", *(mem_exp+offset+i)); \
    }

#define CompareValue( mem, value, flash_addr, compare_len, offset ) \
     Mx_printf("\t\t **Start Dump data=> address %8X\r\n", flash_addr); \
     Mx_printf("\t\t            Length=> %X\r\n", compare_len); \
    for( i=0 ; i < compare_len ; i=i+1 ){  \
      if( mem[offset+i] != value ){ \
          Error_inc( ErrorCnt ); \
      } \
	  Mx_printf("\t\t READ address=> %08x\r\n", (flash_addr+i)); \
      Mx_printf("\t\t         data=> %02X\r\n", *(mem+offset+i)); \
      Mx_printf("\t\t     expected=> %02X\r\n", value); \
    }

int MxSimpleTest(MxChip *Mxic);
int MxComplicatedTest(MxChip *Mxic);
u32 MxRequestAddress(u8 request_size);

#ifdef __cplusplus
}
#endif

#endif /* MX_TEST_H_ */
