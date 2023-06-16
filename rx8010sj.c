#include <intrins.h>
#include "rx8010sj.h"
#include "T5LDriver.h"

#define SDA_OUT (P1MDOUT |= (1 << 3))
#define SDA_IN (P1MDOUT &= ~(1 << 3))

#define SDA_HIGH (P1 |= (1 << 3))
#define SDA_LOW (P1 &= ~(1 << 3))
#define SCL_HIGH (P1 |= (1 << 2))
#define SCL_LOW (P1 &= ~(1 << 2))
#define SDA_MASK 0x08
#define READ_SDA ((P1 & SDA_MASK) >> 3)

sbit RTC_SDA = P1 ^ 3;
sbit RTC_SCL = P1 ^ 2;
sbit mIRQ1 = P1 ^ 1;
sbit mIRQ2 = P1 ^ 0;

/***************IIC***************************/
void delayus(unsigned char t)
{
    char i;
    while (t)
    {
        for (i = 0; i < 50; i++)
        {
            i = i;
        }
        t--;
    }
}
void Delayus()
{
    u8 i;
    for (i = 0; i < 80; i++)
        ;
} // 50
u8 BCD(u8 dat)
{
    return ((dat / 10) << 4) | (dat % 10);
}

static void i2c_init()
{
    P1MDOUT |= 0x0C; // 2,3输出
    P1 |= 0x08;      // SDA 拉高
}

static void i2c_start()
{
    SDA_OUT;
    RTC_SDA = 1;
    RTC_SCL = 1;
    delayus(15);
    RTC_SDA = 0;
    delayus(15);
    RTC_SCL = 0;
    delayus(15);
}

static void i2c_stop()
{
    SDA_OUT;
    RTC_SDA = 0;
    RTC_SCL = 1;
    delayus(15);
    RTC_SDA = 1;
    delayus(15);
    SDA_IN;
}

static void i2c_Mack(void)
{
    SDA_OUT;
    RTC_SDA = 0;
    delayus(5);
    RTC_SCL = 1;
    delayus(5);
    RTC_SCL = 0;
    delayus(5);
}

static void i2c_Mnak(void)
{
    SDA_OUT;
    RTC_SDA = 1;
    delayus(5);
    RTC_SCL = 1;
    delayus(5);
    RTC_SCL = 0;
    delayus(5);
}

static void cack(void)
{
    unsigned char i;
    SDA_IN;
    RTC_SDA = 1;
    delayus(5);
    RTC_SCL = 1;
    delayus(5);
    for (i = 0; i < 50; i++)
    {
        if (!RTC_SDA)
        {
            break;
        }
        delayus(5);
    }
    RTC_SCL = 0;
    delayus(5);
    SDA_OUT;
}

/**
 * @description: 一次完整的写
 * @return {*}
 */
static void i2cWriteByte(u8 dat)
{
    char i;
    SDA_OUT;
    for (i = 0; i < 8; i++)
    {
        if (dat & 0x80)
        {
            RTC_SDA = 1;
        }
        else
        {
            RTC_SDA = 0;
        }
        dat = (dat << 1);
        delayus(5);
        RTC_SCL = 1;
        delayus(5);
        RTC_SCL = 0;
        delayus(5);
    }
    cack();
}

static u8 i2cReadByte()
{
    char i;
    unsigned char dat;
    SDA_IN;
    for (i = 0; i < 8; i++)
    {
        delayus(5);
        RTC_SCL = 1;
        delayus(5);
        dat = (dat << 1);
        if (RTC_SDA)
        {
            dat = dat | 0x01;
        }
        else
        {
            dat = dat & 0xFE;
        }
        // dat=(dat<<1);
        RTC_SCL = 0;
        delayus(5);
    }
    return (dat);
}
/*****************RX8010****************************************/

void RX8010_config()
{
    u8 loss;
    i2c_start(); // 检测是否掉电
    i2cWriteByte(RX8010_WRITE);
    i2cWriteByte(RX8010_ADDR_FLAG_REG);
    i2c_stop();
    i2c_start();
    i2cWriteByte(RX8010_READ);
    loss = i2cReadByte();
    i2c_Mack();
    i2cReadByte();
    i2c_Mnak();
    i2c_stop();
    if ((loss & 0x02) != 0) // 数据丢失
    {
        i2c_start(); // 17=D8
        i2cWriteByte(RX8010_WRITE);
        i2cWriteByte(RX8010_ADDR_RSV17);
        i2cWriteByte(0xD8);
        i2c_stop();
        i2c_start(); // 30-32
        i2cWriteByte(RX8010_WRITE);
        i2cWriteByte(RX8010_ADDR_RSV1);
        i2cWriteByte(0x00);
        i2cWriteByte(0x08);
        i2cWriteByte(0x00); // IRQ select
        i2c_stop();
        i2c_start(); // 1D-1F=48 00 40 10
        i2cWriteByte(RX8010_WRITE);
        i2cWriteByte(RX8010_ADDR_EXT_REG);
        i2cWriteByte(0x48); // set 1d.4(TE) to 0,set FSEL1,0 bit optionally
        i2cWriteByte(0x00); // clear VLF bit to 0. Its value changes from "0" to "1" when data loss occurs
        i2cWriteByte(0x00); // set TEST bit to 0,set AIE, TIE, UIE bit to “0 " to prevent unprepared interruption output
        i2c_stop();
        i2c_start(); // 10-16=RTC设置值 BCD格式
        i2cWriteByte(RX8010_WRITE);
        i2cWriteByte(RX8010_ADDR_SECOND);
        i2cWriteByte(0x00); // second
        i2cWriteByte(0x00); // minute
        i2cWriteByte(0x00); // hour
        i2cWriteByte(0x01); // week
        i2cWriteByte(0x17); // day
        i2cWriteByte(0x06); // month
        i2cWriteByte(0x23); // year
        i2c_stop();
        // setting the alarm function
        // setting the timer funciton
        // setting the update function

        i2cWriteByte(RX8010_WRITE);
        i2cWriteByte(RX8010_ADDR_CTRL_REG);
        i2cWriteByte(0x00); // set STOP to 0 finish initial
    }
}

void getTime()
{
    u8 i, N, M;
    u8 Rtcdata[] = {0, 0, 0, 0, 0, 0, 0, 0};
    u16 hour, min, sec;
    i2c_start();
    i2cWriteByte(RX8010_WRITE);
    i2cWriteByte(RX8010_ADDR_SECOND);
    i2c_stop();
    i2c_start();
    i2cWriteByte(RX8010_READ);
    for (i = 6; i > 0; i--)
    {
        Rtcdata[i] = i2cReadByte();
        i2c_Mack();
    }
    Rtcdata[0] = i2cReadByte();
    i2c_Mnak();
    i2c_stop();
    for (i = 0; i < 3; i++) // 年月日BCD转10进制
    {
        N = Rtcdata[i] >> 4;
        M = Rtcdata[i] & 0x0F;
        Rtcdata[i] = N * 10 + M;
    }
    for (i = 4; i < 7; i++) // 时分秒转BCD转10进制
    {
        N = Rtcdata[i] >> 4;
        M = Rtcdata[i] & 0x0f;
        Rtcdata[i] = N * 10 + M;
    }
    hour = Rtcdata[4];
    min = Rtcdata[5];
    sec = Rtcdata[6];
    // Rtcdata[3] = RTC_Get_Week(Rtcdata[0], Rtcdata[1], Rtcdata[2]); // 周
    T5L_WriteDgusBytes(0x1025, (u8 *)&hour, 2);
    T5L_WriteDgusBytes(0x1026, (u8 *)&min, 2);
    T5L_WriteDgusBytes(0x1027, (u8 *)&sec, 2);
}
/**
 * @description:  周的每一位对应一周的一天，周日开始起算
 * @param {RTC} *rtc
 * @return {*}
 */
void setTime(RTC *rtc)
{
    u8 sec, min, hour, week, day, month, year;
    if (rtc->sec > 59)
        rtc->sec = 59;
    if (rtc->min > 59)
        rtc->min = 59;
    if (rtc->hour > 24)
        rtc->hour = 24;
    if (rtc->week > 7) // 周日为第一天
        rtc->week = 7;
    if (rtc->day > 31)
        rtc->day = 31;
    if (rtc->month > 12)
        rtc->month = 12;
    if (rtc->year > 99)
        rtc->year = 99;

    sec = BCD(rtc->sec);
    min = BCD(rtc->min);
    hour = BCD(rtc->hour); // 24小时制
    day = BCD(rtc->day);
    week = 1 << (rtc->week - 1);
    month = BCD(rtc->month);
    year = BCD(rtc->year);

    i2c_start();
    i2cWriteByte(RX8010_WRITE);
    i2cWriteByte(RX8010_ADDR_CTRL_REG);
    i2cWriteByte(0x40);                     //set STOP to 1
    i2c_stop();
    // i2c_start(); // 1D-1F=48 00 40 10
    // i2cWriteByte(RX8010_READ);
    // set = i2cReadByte();
    // i2c_Mack();
    // i2cReadByte();
    // i2c_Mnak();
    // i2c_stop();
    // set |= (1 << 6);
    // i2c_start();
    // i2cWriteByte(RX8010_WRITE);
    // i2cWriteByte(RX8010_ADDR_CTRL_REG);
    // i2cWriteByte(set);
    // i2c_stop();
    // write time
    i2cWriteByte(0x48); // set 1d.4(TE) to 0,set FSEL1,0 bit optionally
    i2cWriteByte(0x00); // clear VLF bit to 0. Its value changes from "0" to "1" when data loss occurs
    i2cWriteByte(0x40); // set TEST bit to 0,set AIE, TIE, UIE bit to “0 " to prevent unprepared interruption output
    i2c_stop();
    i2c_start();
    i2cWriteByte(RX8010_WRITE);
    i2cWriteByte(RX8010_ADDR_SECOND);
    i2cWriteByte(sec);
    i2cWriteByte(min);
    i2cWriteByte(hour);
    i2cWriteByte(week); // 周只与天相关联，改变年月时需自行更改对应的周
    i2cWriteByte(day);
    i2cWriteByte(month);
    i2cWriteByte(year);
    i2c_stop();
    i2c_start(); // set STOP to 0
    i2cWriteByte(RX8010_WRITE);
    i2cWriteByte(RX8010_ADDR_CTRL_REG);
    i2cWriteByte(0x00);
    i2c_stop();
}

void RTC_SetClock()
{
    i2c_start(); // 17=D8
    i2cWriteByte(RX8010_WRITE);
    i2cWriteByte(RX8010_ADDR_RSV17);
    i2cWriteByte(0xD8);
    i2c_stop();
    i2c_start(); // 30-32
    i2cWriteByte(RX8010_WRITE);
    i2cWriteByte(RX8010_ADDR_RSV1);
    i2cWriteByte(0x00);
    i2cWriteByte(0x08);
    i2cWriteByte(0x00); // IRQ select
    i2c_stop();
    i2c_start(); // 30-32
    i2cWriteByte(RX8010_WRITE);
    i2cWriteByte(RX8010_ADDR_EXT_REG);
    i2cWriteByte(0x04);
    i2cWriteByte(0x00);
    i2cWriteByte(0x40); // IRQ select
    i2c_stop();
    i2c_start(); // present time
    i2cWriteByte(RX8010_WRITE);
    i2cWriteByte(RX8010_ADDR_SECOND);
    i2cWriteByte(0x04);
    i2cWriteByte(0x00);
    i2cWriteByte(0x40); // IRQ select
    i2c_stop();
}

void setAlarm(u8 index)
{
    // TODO
}

void RX8010_init()
{
    i2c_init();
    RX8010_config();
    delayus(255);
}