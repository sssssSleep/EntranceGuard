#if USE_RC522_NFC
#ifndef __RC522_H
#define __RC522_H
#include "main.h"
#include <string.h>
#include <stdio.h>
#include "eeprom.h"
#include "acousto_optic.h"
#include "OLED.h"

/*******************************
 *连线说明：
 *1--SDA  <----->PA4
 *2--SCK  <----->PA5
 *3--MOSI <----->PA7
 *4--MISO <----->PA6
 *5--悬空
 *6--GND <----->GND
 *7--RST <----->PB0
 *8--VCC <----->VCC
 ************************************/
// #define u8 unsigned char
// #define u16 unsigned int
// #define u32 unsigned long int

// MF522命令代码
#define PCD_IDLE 0x00       // 取消当前命令
#define PCD_AUTHENT 0x0E    // 验证密钥
#define PCD_RECEIVE 0x08    // 接收数据
#define PCD_TRANSMIT 0x04   // 发送数据
#define PCD_TRANSCEIVE 0x0C // 发送并接收数据
#define PCD_RESETPHASE 0x0F // 复位
#define PCD_CALCCRC 0x03    // CRC计算

// Mifare_One卡片命令代码
#define PICC_REQIDL 0x26    // 寻天线区内未进入休眠状态
#define PICC_REQALL 0x52    // 寻天线区内全部卡
#define PICC_ANTICOLL1 0x93 // 防冲撞
#define PICC_ANTICOLL2 0x95 // 防冲撞
#define PICC_AUTHENT1A 0x60 // 验证A密钥
#define PICC_AUTHENT1B 0x61 // 验证B密钥
#define PICC_READ 0x30      // 读块
#define PICC_WRITE 0xA0     // 写块
#define PICC_DECREMENT 0xC0 // 扣款
#define PICC_INCREMENT 0xC1 // 充值
#define PICC_RESTORE 0xC2   // 调块数据到缓冲区
#define PICC_TRANSFER 0xB0  // 保存缓冲区中数据
#define PICC_HALT 0x50      // 休眠

#define DEF_FIFO_LENGTH 64 // FIFO size=64byte
#define MAXRLEN 18

// MF522寄存器定义

// PAGE 0
#define RFU00 0x00
#define CommandReg 0x01
#define ComIEnReg 0x02
#define DivlEnReg 0x03
#define ComIrqReg 0x04
#define DivIrqReg 0x05
#define ErrorReg 0x06
#define Status1Reg 0x07
#define Status2Reg 0x08
#define FIFODataReg 0x09
#define FIFOLevelReg 0x0A
#define WaterLevelReg 0x0B
#define ControlReg 0x0C
#define BitFramingReg 0x0D
#define CollReg 0x0E
#define RFU0F 0x0F
// PAGE 1
#define RFU10 0x10
#define ModeReg 0x11
#define TxModeReg 0x12
#define RxModeReg 0x13
#define TxControlReg 0x14
#define TxAutoReg 0x15
#define TxSelReg 0x16
#define RxSelReg 0x17
#define RxThresholdReg 0x18
#define DemodReg 0x19
#define RFU1A 0x1A
#define RFU1B 0x1B
#define MifareReg 0x1C
#define RFU1D 0x1D
#define RFU1E 0x1E
#define SerialSpeedReg 0x1F
// PAGE 2
#define RFU20 0x20
#define CRCResultRegM 0x21
#define CRCResultRegL 0x22
#define RFU23 0x23
#define ModWidthReg 0x24
#define RFU25 0x25
#define RFCfgReg 0x26
#define GsNReg 0x27
#define CWGsCfgReg 0x28
#define ModGsCfgReg 0x29
#define TModeReg 0x2A
#define TPrescalerReg 0x2B
#define TReloadRegH 0x2C
#define TReloadRegL 0x2D
#define TCounterValueRegH 0x2E
#define TCounterValueRegL 0x2F
// PAGE 3
#define RFU30 0x30
#define TestSel1Reg 0x31
#define TestSel2Reg 0x32
#define TestPinEnReg 0x33
#define TestPinValueReg 0x34
#define TestBusReg 0x35
#define AutoTestReg 0x36
#define VersionReg 0x37
#define AnalogTestReg 0x38
#define TestDAC1Reg 0x39
#define TestDAC2Reg 0x3A
#define TestADCReg 0x3B
#define RFU3C 0x3C
#define RFU3D 0x3D
#define RFU3E 0x3E
#define RFU3F 0x3F

// 和RC522通讯时返回的M1卡状态
#define MI_OK 0x26
#define MI_NOTAGERR 0xcc
#define MI_ERR 0xbb

// 和MF522通讯时返回的错误代码
#define SHAQU1 0X01
#define KUAI4 0X04
#define KUAI7 0X07
#define REGCARD 0xa1
#define CONSUME 0xa2
#define READCARD 0xa3
#define ADDMONEY 0xa4

#define SPI_RC522_ReadByte() SPI_RC522_SendByte(0)

/***********************RC522 函数宏定义**********************/
#define RC522_CS_Enable() LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_3)
#define RC522_CS_Disable() LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_3)

#define RC522_Reset_Enable() LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4)
#define RC522_Reset_Disable() LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4)

#define RC522_SCK_0() LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5)
#define RC522_SCK_1() LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5)

#define RC522_MOSI_0() LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7)
#define RC522_MOSI_1() LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7)

#define RC522_MISO_GET() LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_6)

typedef struct
{
  bool Match_Card;
  bool Add_Card;
  bool Delete_Card;
} nfc_event_t;

typedef enum 
{
    Add_Card_Error = 0,
    Add_Card_OK,
    Card_Exist,
    Not_Found,
    Over_Time,
}addcard_state;

extern uint8_t uid_buff[4];
extern nfc_event_t NFC_Event;

u8 SPI_RC522_SendByte(u8 byte);
u8 ReadRawRC(u8 ucAddress);
void WriteRawRC(u8 ucAddress, u8 ucValue);
void SPI1_Init(void);
void RC522_Handel(void);
void RC522_Init(void);                      // 初始化
void PcdReset(void);                        // 复位
void M500PcdConfigISOType(u8 type);         // 工作方式
char PcdRequest(u8 req_code, u8 *pTagType); // 寻卡
char PcdAnticoll(u8 *pSnr);                 // 防冲撞

void PcdAntennaOn(void);  // 开启天线
void PcdAntennaOff(void); // 关闭天线
void SetBitMask(u8 ucReg, u8 ucMask);
void ClearBitMask(u8 ucReg, u8 ucMask);
char PcdSelect(u8 *pSnr);                                         // 选择卡片
char PcdAuthState(u8 ucAuth_mode, u8 ucAddr, u8 *pKey, u8 *pSnr); // 验证密码
char PcdWrite(u8 ucAddr, u8 *pData);
char PcdRead(u8 ucAddr, u8 *pData);
char PcdHalt(void);                                           // 命令卡片进入休眠状态
void CalulateCRC(u8 *pIndata, u8 ucLen, u8 *pOutData);

addcard_state Add_Card();
uint8_t NFC_Search_Card_Once(uint8_t *ucUID);
void Match_Card();
#endif
#endif