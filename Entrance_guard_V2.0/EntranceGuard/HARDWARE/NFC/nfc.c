#include "nfc.h"
#include "main.h"
// M1卡分为16个扇区，每个扇区由四个块（块0、块1、块2、块3）组成
// 将16个扇区的64个块按绝对地址编号为：0~63
// 第0个扇区的块0（即绝对地址0块），用于存放厂商代码，已经固化不可更改
// 每个扇区的块0、块1、块2为数据块，可用于存放数据
// 每个扇区的块3为控制块（绝对地址为:块3、块7、块11.....）包括密码A，存取控制、密码B等

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

nfc_uid_t nfc_uid;
uint8_t uid_buff[4];
nfc_event_t NFC_Event = {0, 0, 0};

/**
 * @brief I2C的短暂延时
 * @param None
 * @retval None
 */
static void delay2(unsigned int n)
{
    for (uint32_t i = n; i > 0; i--)
    {
        __NOP();
    }
}
#define RC522_DELAY() msDelay(1)
#define delay_us(x) delay2(x * 20)
/*全局变量*/
unsigned char CT[2];    // 卡类型
unsigned char SN[4];    // 卡号
unsigned char RFID[16]; // 存放RFID
unsigned char lxl_bit = 0;
unsigned char card1_bit = 0;
unsigned char card2_bit = 0;
unsigned char card3_bit = 0;
unsigned char card4_bit = 0;
unsigned char total = 0;
unsigned char lxl[4] = {196, 58, 104, 217};
unsigned char card_1[4] = {83, 106, 11, 1};
unsigned char card_2[4] = {208, 121, 31, 57};
unsigned char card_3[4] = {176, 177, 143, 165};
unsigned char card_4[4] = {5, 158, 10, 136};
u8 KEY[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
u8 AUDIO_OPEN[6] = {0xAA, 0x07, 0x02, 0x00, 0x09, 0xBC};
unsigned char RFID1[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x07, 0x80, 0x29, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
/*函数声明*/
unsigned char status;
unsigned char s = 0x08;

/* 函数名：RC522_Init
 * 描述  ：初始化RC522配置
 * 输入  ：无
 * 返回  : 无
 * 调用  ：外部调用              */
void RC522_Init(void)
{
    SPI1_Init();
    RC522_Reset_Disable();     // 将RST置高，启动内部复位阶段；
    PcdReset();                // 复位RC522
    PcdAntennaOff();           // 关闭天线
    RC522_DELAY();             // delay 1ms
    PcdAntennaOn();            // 打开天线
    M500PcdConfigISOType('A'); // 设置工作方式
}

/* 函数名：SPI1_Init
 * 描述  ：初始化SPI1的配置
 * 输入  ：无
 * 返回  : 无
 * 调用  ：外部调用              */
void SPI1_Init(void)
{
    /* USER CODE BEGIN SPI1_Init 0 */
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    /**/
    /* GPIO Ports Clock Enable */
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
    /*Configure GPIO pins : PA0 PA1 PA2 PA15 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /*Configure GPIO pin Output Level */
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7);
    /* USER CODE END SPI1_Init 0 */

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    /**SPI1 GPIO Configuration
    PA5   ------> SPI1_SCK
    PA6   ------> SPI1_MISO
    PA7   ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN SPI1_Init 1 */

    /* USER CODE END SPI1_Init 1 */
    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly = 7;
    LL_SPI_Init(SPI1, &SPI_InitStruct);
    LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_Enable(SPI1);

    /* USER CODE BEGIN SPI1_Init 2 */

    //   /* USER CODE END SPI1_Init 2 */
    //     SPI_InitTypeDef SPI_InitStructure;
    //     GPIO_InitTypeDef GPIO_InitStructure;
    //     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); // PORTB时钟使能

    //     // CS
    //     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    //     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
    //     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
    //     GPIO_Init(GPIOA, &GPIO_InitStructure);            // 根据设定参数初始化PF0、PF1

    //     // SCK
    //     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    //     GPIO_Init(GPIOA, &GPIO_InitStructure);

    //     // MISO
    //     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    //     GPIO_Init(GPIOA, &GPIO_InitStructure);

    //     // MOSI
    //     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    //     GPIO_Init(GPIOA, &GPIO_InitStructure);

    //     // RST
    //     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    //     GPIO_Init(GPIOB, &GPIO_InitStructure);

    //     // 置高CS口
    //     RC522_CS_Disable();

    //     // 其他SPI1配置
    //     SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  // 全双工；
    //     SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                       // 主机模式；
    //     SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                   // 传输数据为8位；
    //     SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                          // 时钟极性CPOL为空闲时低电平；
    //     SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                        // 时钟采样点为时钟奇数沿（上升沿）；
    //     SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                           // NSS引脚由软件改变；
    //     SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; // 预分频系数64；
    //     SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                  // MSB先行模式；
    //     SPI_InitStructure.SPI_CRCPolynomial = 7;                            // CRC校验；

    //     // 初始化SPI1
    //     SPI_Init(SPI1, &SPI_InitStructure);

    //     // 使能SPI1
    //     SPI_Cmd(SPI1, ENABLE);
}

/* 函数名：PcdRese
 * 描述  ：复位RC522
 * 输入  ：无
 * 返回  : 无
 * 调用  ：外部调用              */
void PcdReset(void)
{
    RC522_Reset_Disable();
    delay_us(1);
    RC522_Reset_Enable();
    delay_us(1);
    RC522_Reset_Disable();
    delay_us(1);
    WriteRawRC(CommandReg, 0x0f);

    while (ReadRawRC(CommandReg) & 0x10)
        ;

    delay_us(1);
    WriteRawRC(ModeReg, 0x3D);       // 定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL, 30);     // 16位定时器低位
    WriteRawRC(TReloadRegH, 0);      // 16位定时器高位
    WriteRawRC(TModeReg, 0x8D);      // 定义内部定时器的设置
    WriteRawRC(TPrescalerReg, 0x3E); // 设置定时器分频系数
    WriteRawRC(TxAutoReg, 0x40);     // 调制发送信号为100%ASK
}

/* 函数名：SPI_RC522_SendByte
 * 描述  ：向RC522发送1 Byte 数据
 * 输入  ：byte，要发送的数据
 * 返回  : RC522返回的数据
 * 调用  ：内部调用                 */
u8 SPI_RC522_SendByte(u8 byte)
{
    // while (LL_SPI_IsActiveFlag_TXE(SPI1) == 1)
    //     ;
    // LL_SPI_TransmitData8(SPI1, byte);
    // while (LL_SPI_IsActiveFlag_TXE(SPI1) == 1)
    //     ;
    // return LL_SPI_ReceiveData8(SPI1);

    int state = 0;
    uint32_t timeout_cnt;
    static const uint32_t timeout_cnt_num = 10000;

    // Wait until TXE flag is set to send data
    timeout_cnt = 0;
    while (!LL_SPI_IsActiveFlag_TXE(SPI1))
    {
        timeout_cnt++;
        if (timeout_cnt > timeout_cnt_num)
        {
            state = -1;
            break;
        }
    }

    // Transmit data in 16 Bit mode
    LL_SPI_TransmitData16(SPI1, byte);

    // Check BSY flag
    timeout_cnt = 0;
    while (LL_SPI_IsActiveFlag_BSY(SPI1))
    {
        timeout_cnt++;
        if (timeout_cnt > timeout_cnt_num)
        {
            state = -1;
            break;
        }
    }

    // Check RXNE flag
    timeout_cnt = 0;
    while (!LL_SPI_IsActiveFlag_RXNE(SPI1))
    {
        timeout_cnt++;
        if (timeout_cnt > timeout_cnt_num)
        {
            state = -1;
            break;
        }
    }

    // Read 16-Bits in the data register

    return LL_SPI_ReceiveData8(SPI1);
}

/* 函数名：ReadRawRC
 * 描述  ：读RC522寄存器
 * 输入  ：ucAddress，寄存器地址
 * 返回  : 寄存器的当前值
 * 调用  ：内部调用                 */
u8 ReadRawRC(u8 ucAddress)
{
    u8 ucAddr, ucReturn;
    ucAddr = ((ucAddress << 1) & 0x7E) | 0x80;

    RC522_CS_Enable();
    SPI_RC522_SendByte(ucAddr);
    ucReturn = SPI_RC522_ReadByte();
    RC522_CS_Disable();
    return ucReturn;
}

/* 函数名：WriteRawRC
 * 描述  ：写RC522寄存器
 * 输入  ：ucAddress，寄存器地址  、 ucValue，写入寄存器的值
 * 返回  : 无
 * 调用  ：内部调用   */
void WriteRawRC(u8 ucAddress, u8 ucValue)
{
    u8 ucAddr;
    ucAddr = (ucAddress << 1) & 0x7E;

    RC522_CS_Enable();
    SPI_RC522_SendByte(ucAddr);
    SPI_RC522_SendByte(ucValue);
    RC522_CS_Disable();
}

/* 函数名：M500PcdConfigISOType
 * 描述  ：设置RC522的工作方式
 * 输入  ：ucType，工作方式
 * 返回  : 无
 * 调用  ：外部调用        */
void M500PcdConfigISOType(u8 ucType)
{
    if (ucType == 'A') // ISO14443_A
    {
        ClearBitMask(Status2Reg, 0x08);

        WriteRawRC(ModeReg, 0x3D);   // 3F
        WriteRawRC(RxSelReg, 0x86);  // 84
        WriteRawRC(RFCfgReg, 0x7F);  // 4F
        WriteRawRC(TReloadRegL, 30); // tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
        WriteRawRC(TReloadRegH, 0);
        WriteRawRC(TModeReg, 0x8D);
        WriteRawRC(TPrescalerReg, 0x3E);
        delay_us(2);

        PcdAntennaOn(); // 开天线
    }
}

/*
 * 函数名：SetBitMask
 * 描述  ：对RC522寄存器置位
 * 输入  ：ucReg，寄存器地址
 *         ucMask，置位值
 * 返回  : 无
 * 调用  ：内部调用
 */
void SetBitMask(u8 ucReg, u8 ucMask)
{
    u8 ucTemp;

    ucTemp = ReadRawRC(ucReg);
    WriteRawRC(ucReg, ucTemp | ucMask); // set bit mask
}

/* 函数名：ClearBitMask
 * 描述  ：对RC522寄存器清位
 * 输入  ：ucReg，寄存器地址
 *         ucMask，清位值
 * 返回  : 无
 * 调用  ：内部调用           */
void ClearBitMask(u8 ucReg, u8 ucMask)
{
    u8 ucTemp;
    ucTemp = ReadRawRC(ucReg);

    WriteRawRC(ucReg, ucTemp & (~ucMask)); // clear bit mask
}

/* 函数名：PcdAntennaOn
 * 描述  ：开启天线
 * 输入  ：无
 * 返回  : 无
 * 调用  ：内部调用            */
void PcdAntennaOn(void)
{
    u8 uc;
    uc = ReadRawRC(TxControlReg);

    if (!(uc & 0x03))
        SetBitMask(TxControlReg, 0x03);
}

/* 函数名：PcdAntennaOff
 * 描述  ：开启天线
 * 输入  ：无
 * 返回  : 无
 * 调用  ：内部调用             */
void PcdAntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);
}

/* 函数名：PcdComMF522
 * 描述  ：通过RC522和ISO14443卡通讯
 * 输入  ：ucCommand，RC522命令字
 *         pInData，通过RC522发送到卡片的数据
 *         ucInLenByte，发送数据的字节长度
 *         pOutData，接收到的卡片返回数据
 *         pOutLenBit，返回数据的位长度
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：内部调用              */
char PcdComMF522(u8 ucCommand, u8 *pInData, u8 ucInLenByte, u8 *pOutData, u32 *pOutLenBit)
{
    char cStatus = MI_ERR;
    u8 ucIrqEn = 0x00;
    u8 ucWaitFor = 0x00;
    u8 ucLastBits;
    u8 ucN;
    u32 ul;

    switch (ucCommand)
    {
    case PCD_AUTHENT:     // Mifare认证
        ucIrqEn = 0x12;   // 允许错误中断请求ErrIEn  允许空闲中断IdleIEn
        ucWaitFor = 0x10; // 认证寻卡等待时候 查询空闲中断标志位
        break;

    case PCD_TRANSCEIVE:  // 接收发送 发送接收
        ucIrqEn = 0x77;   // 允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        ucWaitFor = 0x30; // 寻卡等待时候 查询接收中断标志位与 空闲中断标志位
        break;

    default:
        break;
    }

    WriteRawRC(ComIEnReg, ucIrqEn | 0x80); // IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反
    ClearBitMask(ComIrqReg, 0x80);         // Set1该位清零时，CommIRqReg的屏蔽位清零
    WriteRawRC(CommandReg, PCD_IDLE);      // 写空闲命令
    SetBitMask(FIFOLevelReg, 0x80);        // 置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除

    for (ul = 0; ul < ucInLenByte; ul++)
        WriteRawRC(FIFODataReg, pInData[ul]); // 写数据进FIFOdata

    WriteRawRC(CommandReg, ucCommand); // 写命令

    if (ucCommand == PCD_TRANSCEIVE)
        SetBitMask(BitFramingReg, 0x80); // StartSend置位启动数据发送 该位与收发命令使用时才有效

    ul = 1000; // 根据时钟频率调整，操作M1卡最大等待时间25ms

    do // 认证 与寻卡等待时间
    {
        ucN = ReadRawRC(ComIrqReg); // 查询事件中断
        ul--;
    } while ((ul != 0) && (!(ucN & 0x01)) && (!(ucN & ucWaitFor))); // 退出条件i=0,定时器中断，与写空闲命令

    ClearBitMask(BitFramingReg, 0x80); // 清理允许StartSend位

    if (ul != 0)
    {
        if (!((ReadRawRC(ErrorReg) & 0x1B))) // 读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
        {
            cStatus = MI_OK;

            if (ucN & ucIrqEn & 0x01) // 是否发生定时器中断
                cStatus = MI_NOTAGERR;

            if (ucCommand == PCD_TRANSCEIVE)
            {
                ucN = ReadRawRC(FIFOLevelReg); // 读FIFO中保存的字节数

                ucLastBits = ReadRawRC(ControlReg) & 0x07; // 最后接收到得字节的有效位数

                if (ucLastBits)
                    *pOutLenBit = (ucN - 1) * 8 + ucLastBits; // N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
                else
                    *pOutLenBit = ucN * 8; // 最后接收到的字节整个字节有效

                if (ucN == 0)
                    ucN = 1;

                if (ucN > MAXRLEN)
                    ucN = MAXRLEN;

                for (ul = 0; ul < ucN; ul++)
                    pOutData[ul] = ReadRawRC(FIFODataReg);
            }
        }
        else
            cStatus = MI_ERR;
    }

    SetBitMask(ControlReg, 0x80); // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE);

    return cStatus;
}

/* 函数名：PcdRequest
 * 描述  ：寻卡
 * 输入  ：ucReq_code，寻卡方式
 *                     = 0x52，寻感应区内所有符合14443A标准的卡
 *                     = 0x26，寻未进入休眠状态的卡
 *         pTagType，卡片类型代码
 *                   = 0x4400，Mifare_UltraLight
 *                   = 0x0400，Mifare_One(S50)
 *                   = 0x0200，Mifare_One(S70)
 *                   = 0x0800，Mifare_Pro(X))
 *                   = 0x4403，Mifare_DESFire
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用            */
char PcdRequest(u8 ucReq_code, u8 *pTagType)
{
    char cStatus;
    u8 ucComMF522Buf[MAXRLEN];
    u32 ulLen;

    ClearBitMask(Status2Reg, 0x08);  // 清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况
    WriteRawRC(BitFramingReg, 0x07); //	发送的最后一个字节的 七位
    SetBitMask(TxControlReg, 0x03);  // TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号

    ucComMF522Buf[0] = ucReq_code; // 存入 卡片命令字

    cStatus = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &ulLen); // 寻卡

    if ((cStatus == MI_OK) && (ulLen == 0x10)) // 寻卡成功返回卡类型
    {
        *pTagType = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }

    else
        cStatus = MI_ERR;

    return cStatus;
}

/* 函数名：PcdAnticoll
 * 描述  ：防冲撞
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用           */
char PcdAnticoll(u8 *pSnr)
{
    char cStatus;
    u8 uc, ucSnr_check = 0;
    u8 ucComMF522Buf[MAXRLEN];
    u32 ulLen;

    ClearBitMask(Status2Reg, 0x08);  // 清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
    WriteRawRC(BitFramingReg, 0x00); // 清理寄存器 停止收发
    ClearBitMask(CollReg, 0x80);     // 清ValuesAfterColl所有接收的位在冲突后被清除

    ucComMF522Buf[0] = 0x93; // 卡片防冲突命令
    ucComMF522Buf[1] = 0x20;

    cStatus = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &ulLen); // 与卡片通信

    if (cStatus == MI_OK) // 通信成功
    {
        for (uc = 0; uc < 4; uc++)
        {
            *(pSnr + uc) = ucComMF522Buf[uc]; // 读出UID
            ucSnr_check ^= ucComMF522Buf[uc];
        }

        if (ucSnr_check != ucComMF522Buf[uc])
            cStatus = MI_ERR;
    }

    SetBitMask(CollReg, 0x80);

    return cStatus;
}

/* 函数名：PcdSelect
 * 描述  ：选定卡片
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用         */
char PcdSelect(u8 *pSnr)
{
    char ucN;
    u8 uc;
    u8 ucComMF522Buf[MAXRLEN];
    u32 ulLen;

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;

    for (uc = 0; uc < 4; uc++)
    {
        ucComMF522Buf[uc + 2] = *(pSnr + uc);
        ucComMF522Buf[6] ^= *(pSnr + uc);
    }

    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);
    ClearBitMask(Status2Reg, 0x08);
    ucN = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &ulLen);

    if ((ucN == MI_OK) && (ulLen == 0x18))
        ucN = MI_OK;
    else
        ucN = MI_ERR;

    return ucN;
}

/* 函数名：CalulateCRC
 * 描述  ：用RC522计算CRC16
 * 输入  ：pIndata，计算CRC16的数组
 *         ucLen，计算CRC16的数组字节长度
 *         pOutData，存放计算结果存放的首地址
 * 返回  : 无
 * 调用  ：内部调用              */
void CalulateCRC(u8 *pIndata, u8 ucLen, u8 *pOutData)
{
    u8 uc, ucN;

    ClearBitMask(DivIrqReg, 0x04);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);

    for (uc = 0; uc < ucLen; uc++)
        WriteRawRC(FIFODataReg, *(pIndata + uc));

    WriteRawRC(CommandReg, PCD_CALCCRC);
    uc = 0xFF;

    do
    {
        ucN = ReadRawRC(DivIrqReg);
        uc--;
    } while ((uc != 0) && !(ucN & 0x04));

    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/* 函数名：PcdAuthState
 * 描述  ：验证卡片密码
 * 输入  ：ucAuth_mode，密码验证模式
 *                     = 0x60，验证A密钥
 *                     = 0x61，验证B密钥
 *         u8 ucAddr，块地址
 *         pKey，密码
 *         pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用          */
char PcdAuthState(u8 ucAuth_mode, u8 ucAddr, u8 *pKey, u8 *pSnr)
{
    char cStatus;
    u8 uc, ucComMF522Buf[MAXRLEN];
    u32 ulLen;

    ucComMF522Buf[0] = ucAuth_mode;
    ucComMF522Buf[1] = ucAddr;

    for (uc = 0; uc < 6; uc++)
        ucComMF522Buf[uc + 2] = *(pKey + uc);

    for (uc = 0; uc < 6; uc++)
        ucComMF522Buf[uc + 8] = *(pSnr + uc);

    cStatus = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &ulLen);

    if ((cStatus != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        cStatus = MI_ERR;
    }

    return cStatus;
}

/* 函数名：PcdWrite
 * 描述  ：写数据到M1卡一块
 * 输入  ：u8 ucAddr，块地址
 *         pData，写入的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用           */
char PcdWrite(u8 ucAddr, u8 *pData)
{
    char cStatus;
    u8 uc, ucComMF522Buf[MAXRLEN];
    u32 ulLen;

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = ucAddr;

    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    cStatus = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &ulLen);

    if ((cStatus != MI_OK) || (ulLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        cStatus = MI_ERR;

    if (cStatus == MI_OK)
    {
        memcpy(ucComMF522Buf, pData, 16);
        for (uc = 0; uc < 16; uc++)
            ucComMF522Buf[uc] = *(pData + uc);

        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        cStatus = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &ulLen);

        if ((cStatus != MI_OK) || (ulLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
            cStatus = MI_ERR;
    }

    return cStatus;
}

/* 函数名：PcdRead
 * 描述  ：读取M1卡一块数据
 * 输入  ：u8 ucAddr，块地址
 *         pData，读出的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用             */
char PcdRead(u8 ucAddr, u8 *pData)
{
    char cStatus;
    u8 uc, ucComMF522Buf[MAXRLEN];
    u32 ulLen;

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = ucAddr;

    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    cStatus = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &ulLen);

    if ((cStatus == MI_OK) && (ulLen == 0x90))
    {
        for (uc = 0; uc < 16; uc++)
            *(pData + uc) = ucComMF522Buf[uc];
    }

    else
        cStatus = MI_ERR;

    return cStatus;
}

/* 函数名：PcdHalt
 * 描述  ：命令卡片进入休眠状态
 * 输入  ：无
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用        */
char PcdHalt(void)
{
    u8 ucComMF522Buf[MAXRLEN];
    u32 ulLen;

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;

    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
    PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &ulLen);

    return MI_OK;
}

addcard_state Add_Card()
{
    PcdAntennaOn(); // 开启天线
    if (NFC_Search_Card_Once(uid_buff) == MI_OK)
    {

        if (nfc_uid.len == 0)
        {
            Buzzer_One(100);
            eeprom_add_card(uid_buff, (u8 *)"***");
            Buzzer_One(100);
            return Add_Card_OK;
        }

        for (uint8_t i = 0; i < nfc_uid.len; i++)
        {
            if (strncmp((const char *)uid_buff, (const char *)nfc_uid.uid[i], 4) == 0)
            {
                Buzzer(50, 5);
                // break;
                return Card_Exist;
            }
            else if (i == nfc_uid.len - 1)
            {
                Buzzer_One(100);
                eeprom_add_card(uid_buff, (u8 *)"***");
                Buzzer_One(100);
                return Add_Card_OK;
            }
        }
        return Not_Found;
        memset(uid_buff, 0x00, 4);
        // Buzzer(500, 2);
    }
    PcdAntennaOff(); // 关闭天线
    return Add_Card_Error;
}

uint8_t NFC_Search_Card_Once(uint8_t *ucUID)
{
    uint8_t ucArray_ID[4];  /*先后存放IC卡的类型和UID(IC卡序列号)*/
    uint8_t ucStatusReturn; /*返回状态*/
    /*寻卡*/
    if ((ucStatusReturn = PcdRequest(PICC_REQALL, ucArray_ID)) != MI_OK)
    {
        ucStatusReturn = PcdRequest(PICC_REQALL, ucArray_ID); // PICC_REQALL   PICC_REQIDL
    }
    if (ucStatusReturn == MI_OK)
    {

        /*防冲撞（当有多张卡进入读写器操作范围时，防冲突机制会从其中选择一张进行操作）*/
        if (PcdAnticoll(ucArray_ID) == MI_OK)
        {
            PcdSelect(ucArray_ID);
            memcpy(ucUID, ucArray_ID, 4);
            // sprintf(cStr, "The Card ID is: %02X%02X%02X%02X", ucArray_ID[0], ucArray_ID[1], ucArray_ID[2], ucArray_ID[3]);
            // printf("%s\r\n", cStr); // 打印卡片ID
            PcdHalt();
            // Buzzer(500, 2);
        }
    }
    return ucStatusReturn;
}

void Match_Card()
{
    PcdAntennaOn(); // 开启天线
    if (NFC_Search_Card_Once(uid_buff) == MI_OK)
    {
        if (nfc_uid.len == 0)
        {
            RED_ON;
            Buzzer(200, 3);
            RED_OFF;
            return;
        }
        for (uint8_t i = 0; i < nfc_uid.len; i++)
        {
            if (strncmp((const char *)uid_buff, (const char *)nfc_uid.uid[i], 4) == 0)
            {
                Buzzer(200, 1);
                LL_TIM_CC_EnableChannel(TIM21, LL_TIM_CHANNEL_CH1);
                LL_TIM_EnableCounter(TIM21);
                LL_TIM_OC_SetCompareCH1(TIM21, 2500 - 1);
                GREEN_ON;
                // LL_TIM_OC_SetCompareCH1(TIM21, 0);
                OLED_ShowStr(32 + 0, 6, (unsigned char *)"open_door", SIZE_0806, NORMAL);
                msSleep(2000);
                OLED_ShowStr(32 + 0, 6, (unsigned char *)"         ", SIZE_0806, NORMAL);
                LL_TIM_OC_SetCompareCH1(TIM21, 500 - 1);
                msSleep(2000);
                // LL_TIM_OC_SetCompareCH1(TIM21, 0);
                LL_TIM_CC_DisableChannel(TIM21, LL_TIM_CHANNEL_CH1);
                LL_TIM_DisableCounter(TIM21);
                GREEN_OFF;
                Buzzer(200, 1);
                break;
            }
            else if (i == nfc_uid.len - 1)
            {
                RED_ON;
                Buzzer(200, 3);
                RED_OFF;
            }
        }
    }
    memset(uid_buff, 0x00, 4);
    PcdAntennaOff(); // 关闭天线
    NFC_Event.Match_Card = OFF;
}