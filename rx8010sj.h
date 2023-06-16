/*
 * @Author: LanRu
 * @Date: 2023-05-27 11:16:44
 * @LastEditors: LanRu
 * @LastEditTime: 2023-06-16 10:53:30
 * @Description: 
 */
#ifndef __RX8010SJ_H
#define __RX8010SJ_H
#include "T5LDriver.h"
#define RX8010_READ   0x65
#define RX8010_WRITE  0x64

// register address , for RX-8010SJ
#define RX8010_ADDR_SECOND      ( 0x10 )
#define RX8010_ADDR_MINUTE      ( 0x11 )
#define RX8010_ADDR_HOUR        ( 0x12 )
#define RX8010_ADDR_WEEK        ( 0x13 )
#define RX8010_ADDR_DATE        ( 0x14 )
#define RX8010_ADDR_MONTH       ( 0x15 )
#define RX8010_ADDR_YEAR        ( 0x16 )
#define RX8010_ADDR_RSV17       ( 0x17 )
#define RX8010_ADDR_ALM_MINUTE  ( 0x18 )
#define RX8010_ADDR_ALM_HOUR    ( 0x19 )
#define RX8010_ADDR_ALM_WEEK    ( 0x1A )
#define RX8010_ADDR_ALM_DATE    ( 0x1A )
#define RX8010_ADDR_TMR_CNT0    ( 0x1B )
#define RX8010_ADDR_TMR_CNT1    ( 0x1C )
#define RX8010_ADDR_EXT_REG     ( 0x1D )
#define RX8010_ADDR_FLAG_REG    ( 0x1E )
#define RX8010_ADDR_CTRL_REG    ( 0x1F )

#define RX8010_ADDR_RSV1        (0x30)
#define RX8010_ADDR_RSV2        (0x31)
#define RX8010_ADDR_RSV3        (0x32)


typedef struct rx8010sj
{
    char sec;
    char min;
    char hour;
    char day;
    char week;
    char month;
    char year;
}RTC;

void RX8010_init();

// void testRX8010();

void getTime();

void setTime(RTC *);
#endif