#include "rfid.h"
//#include "init.h"
//#include "gpio.h"
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

/*
    File: MFRC522.h
    MFRC522 - Library for communicating with cheap MFRC522 based NFC Reader/Writers
    Created by Eelco Rouw - Mainly based on code from Grant Gibson (www.grantgibson.co.uk) and Dr.Leong ( WWW.B2CQSHOP.COM )
    Ported to C by Stephen S. M.
    Released into the public domain
*/


#include "hw_memap.h"
#include "hw_types.h"
#include "gpio.h"
#include "pin_map.h"
#include "sysctl.h"
#include "ssi.h"
#include "rfid.h"


/*
 * Function: WriteReg
 * Description: write a byte data into one register of MR RC522
 * Input parameter: addr--register address；val--the value that need to write in
 * Return: Null
 */
uint8_t SPI_transfer(uint8_t data)
{
    uint32_t rxtxData;
    rxtxData = data;
    SSIDataPut(SSI_BASE, (uint8_t) rxtxData);
    while(SSIBusy(SSI_BASE));
    SSIDataGet(SSI_BASE, &rxtxData);
    return (uint8_t) rxtxData;
}


void MFRC552_WriteReg(unsigned char addr, unsigned char val)
{
    GPIOPinWrite(CHIP_SELECT_BASE, CHIP_SELECT_PIN, 0);
    SPI_transfer((addr<<1)&0x7E);
    SPI_transfer(val);
    GPIOPinWrite(CHIP_SELECT_BASE, CHIP_SELECT_PIN, CHIP_SELECT_PIN);
}

/*
 * Function: ReadReg
 * Description: read a byte data into one register of MR RC522
 * Input parameter: addr--register address
 * Return: return the read value
 */
unsigned char MFRC552_ReadReg(unsigned char addr)
{
    unsigned char val;
    GPIOPinWrite(CHIP_SELECT_BASE, CHIP_SELECT_PIN, 0);

    SPI_transfer(((addr<<1)&0x7E) | 0x80);
    val = SPI_transfer(0x00);

    GPIOPinWrite(CHIP_SELECT_BASE, CHIP_SELECT_PIN, CHIP_SELECT_PIN);

    return val;
}


/*
 * Function: SetBitMask
 * Description: set RC522 register bit
 * Input parameter: reg--register address;mask--value
 * Return: null
 */
void MFRC552_SetBitMask(unsigned char reg, unsigned char mask)
{
    unsigned char tmp;
    tmp = MFRC552_ReadReg(reg);
    MFRC552_WriteReg(reg, tmp | mask);  // set bit mask
}


/*
 * Function: ClearBitMask
 * Description: clear RC522 register bit
 * Input parameter: reg--register address;mask--value
 * Return: null
 */
void MFRC552_ClearBitMask(unsigned char reg, unsigned char mask)
{
    unsigned char tmp;
    tmp = MFRC552_ReadReg(reg);
    MFRC552_WriteReg(reg, tmp & (~mask));  // clear bit mask
}


/*
 * Function: AntennaOn
 * Description: Turn on antenna, every time turn on or shut down antenna need at least 1ms delay
 * Input parameter: null
 * Return: null
 */
void MFRC552_AntennaOn(void)
{
    unsigned char temp;

    temp = MFRC552_ReadReg(TxControlReg);
    if (!(temp & 0x03))
    {
        MFRC552_SetBitMask(TxControlReg, 0x03);
    }
}


/*
 * Function: AntennaOff
 * Description: Turn off antenna, every time turn on or shut down antenna need at least 1ms delay
 * Input parameter: null
 * Return: null
 */
void MFRC552_AntennaOff(void)
{
    MFRC552_ClearBitMask(TxControlReg, 0x03);
}


/*
 * Function: Reset
 * Description:  reset RC522
 * Input parameter: null
 * Return: null
 */
void MFRC552_Reset(void)
{
    MFRC552_WriteReg(CommandReg, PCD_RESETPHASE);
}


/*
 * Function: Init
 * Description: initilize RC522
 * Input parameter: null
 * Return: null
 */
void MFRC552_Init(void)
{
    GPIOPinWrite(NRSTPD_BASE, NRSTPD_PIN, NRSTPD_PIN);

    MFRC552_Reset();

    //Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    MFRC552_WriteReg(TModeReg, 0x8D);        //Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    MFRC552_WriteReg(TPrescalerReg, 0x3E);    //TModeReg[3..0] + TPrescalerReg
    MFRC552_WriteReg(TReloadRegL, 30);
    MFRC552_WriteReg(TReloadRegH, 0);

    MFRC552_WriteReg(TxAutoReg, 0x40);        //100%ASK
    MFRC552_WriteReg(ModeReg, 0x3D);

    //ClearBitMask(Status2Reg, 0x08);        //MFCrypto1On=0
    //WriteReg(RxSelReg, 0x86);        //RxWait = RxSelReg[5..0]
    //WriteReg(RFCfgReg, 0x7F);           //RxGain = 48dB

    MFRC552_AntennaOn();
}


/*
 * Function: Request
 * Description: Searching card, read card type
 * Input parameter: reqMode--search methods:
 *             TagType--return card types
 *                 0x4400 = Mifare_UltraLight
 *                0x0400 = Mifare_One(S50)
 *                0x0200 = Mifare_One(S70)
 *                0x0800 = Mifare_Pro(X)
 *                0x4403 = Mifare_DESFire
 * return: return MI_OK if successed
 */
unsigned char MFRC552_Request(unsigned char reqMode, unsigned char *TagType)
{
    unsigned char status;
    unsigned int backBits;

    MFRC552_WriteReg(BitFramingReg, 0x07);        //TxLastBists = BitFramingReg[2..0]    ???

    TagType[0] = reqMode;
    status = MFRC552_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

    if ((status != MI_OK) || (backBits != 0x10))
    {
        status = MI_ERR;
    }

    return status;
}


/*
 * Function: ToCard
 * Description: communicate between RC522 and ISO14443
 * Input parameter: command--MF522 command bits
 *             sendData--send data to card via rc522
 *             sendLen--send data length
 *             backData--the return data from card
 *             backLen--the length of return data
 * return: return MI_OK if successed
 */
unsigned char MFRC552_ToCard(unsigned char command, unsigned char *sendData, unsigned char sendLen, unsigned char *backData, unsigned int *backLen)
{
    unsigned char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitIRq = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;

    switch (command)
    {
        case PCD_AUTHENT:
        {
            irqEn = 0x12;
            waitIRq = 0x10;
            break;
        }
        case PCD_TRANSCEIVE:
        {
            irqEn = 0x77;
            waitIRq = 0x30;
            break;
        }
        default:
            break;
    }

    MFRC552_WriteReg(CommIEnReg, irqEn|0x80);
    MFRC552_ClearBitMask(CommIrqReg, 0x80);
    MFRC552_SetBitMask(FIFOLevelReg, 0x80);

    MFRC552_WriteReg(CommandReg, PCD_IDLE);

    for (i=0; i < sendLen; i++)
    {
        MFRC552_WriteReg(FIFODataReg, sendData[i]);
    }

    MFRC552_WriteReg(CommandReg, command);
    if (command == PCD_TRANSCEIVE)
    {
        MFRC552_SetBitMask(BitFramingReg, 0x80);        //StartSend=1,transmission of data starts
    }

    i = 10000;
    do
    {
        n = MFRC552_ReadReg(CommIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitIRq));

    MFRC552_ClearBitMask(BitFramingReg, 0x80);            //StartSend=0

    if (i != 0)
    {
        if(!(MFRC552_ReadReg(ErrorReg) & 0x1B))    //BufferOvfl Collerr CRCErr ProtecolErr
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;            //??
            }

            if (command == PCD_TRANSCEIVE)
            {
                n = MFRC552_ReadReg(FIFOLevelReg);
                lastBits = MFRC552_ReadReg(ControlReg) & 0x07;
                if (lastBits)
                {
                    *backLen = (n-1)*8 + lastBits;
                }
                else
                {
                    *backLen = n*8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAX_LEN)
                {
                    n = MAX_LEN;
                }

                for (i=0; i<n; i++)
                {
                    backData[i] = MFRC552_ReadReg(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

//SetBitMask(ControlReg,0x80);           //timer stops
//WriteReg(CommandReg, PCD_IDLE);

    return status;
}


/*
 * Function: MFRC522_Anticoll
 * Description: Prevent conflict, read the card serial number
 * Input parameter: serNum--return the 4 bytes card serial number, the 5th byte is recheck byte
 * return: return MI_OK if successed
 */
unsigned char MFRC552_Anticoll(unsigned char *serNum)
{
    unsigned char status;
    unsigned char i;
    unsigned char serNumCheck=0;
    unsigned int unLen;


//ClearBitMask(Status2Reg, 0x08);        //TempSensclear
//ClearBitMask(CollReg,0x80);            //ValuesAfterColl
    MFRC552_WriteReg(BitFramingReg, 0x00);        //TxLastBists = BitFramingReg[2..0]
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC552_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
    {
        for (i=0; i<4; i++)
        {
            serNumCheck ^= serNum[i];
        }
        if (serNumCheck != serNum[i])
        {
            status = MI_ERR;
        }
    }

    //SetBitMask(CollReg, 0x80);        //ValuesAfterColl=1

    return status;
}


/*
 * Function: CalulateCRC
 * Description: Use MF522 to calculate CRC
 * Input parameter: pIndata--the CRC data need to be read，len--data length，pOutData-- the caculated result of CRC
 * return: Null
 */
void MFRC552_CalulateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
    unsigned char i, n;

    MFRC552_ClearBitMask(DivIrqReg, 0x04);            //CRCIrq = 0
    MFRC552_SetBitMask(FIFOLevelReg, 0x80);
    //WriteReg(CommandReg, PCD_IDLE);

    for (i=0; i<len; i++)
    {
        MFRC552_WriteReg(FIFODataReg, *(pIndata+i));
    }
    MFRC552_WriteReg(CommandReg, PCD_CALCCRC);

    i = 0xFF;
    do
    {
        n = MFRC552_ReadReg(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));            //CRCIrq = 1

    pOutData[0] = MFRC552_ReadReg(CRCResultRegL);
    pOutData[1] = MFRC552_ReadReg(CRCResultRegM);
}


/*
 * Function: SelectTag
 * Description: Select card, read card storage volume
 * Input parameter: serNum--Send card serial number
 * return: return the card storage volume
 */
unsigned char MFRC552_SelectTag(unsigned char *serNum)
{
    unsigned char i;
    unsigned char status;
    unsigned char size;
    unsigned int recvBits;
    unsigned char buffer[9];

    //ClearBitMask(Status2Reg, 0x08);            //MFCrypto1On=0

    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    for (i=0; i<5; i++)
    {
        buffer[i+2] = *(serNum+i);
    }
    MFRC552_CalulateCRC(buffer, 7, &buffer[7]);        //??
    status = MFRC552_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

    if ((status == MI_OK) && (recvBits == 0x18))
    {
        size = buffer[0];
    }
    else
    {
        size = 0;
    }

    return size;
}


/*
 * Function: Auth
 * Description: verify card password
 * Input parameters: authMode--password verify mode
                 0x60 = verify A password key
                 0x61 = verify B password key
             BlockAddr--Block address
             Sectorkey--Block password
             serNum--Card serial number ，4 bytes
 * return: return MI_OK if successed
 */
unsigned char MFRC552_Auth(unsigned char authMode, unsigned char BlockAddr, unsigned char *Sectorkey, unsigned char *serNum)
{
    unsigned char status;
    unsigned int recvBits;
    unsigned char i;
    unsigned char buff[12];

    buff[0] = authMode;
    buff[1] = BlockAddr;
    for (i=0; i<6; i++)
    {
        buff[i+2] = *(Sectorkey+i);
    }
    for (i=0; i<4; i++)
    {
        buff[i+8] = *(serNum+i);
    }
    status = MFRC552_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

    if ((status != MI_OK) || (!(MFRC552_ReadReg(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}


/*
 * Function: ReadBlock
 * Description: Read data
 * Input parameters: blockAddr--block address;recvData--the block data which are read
 * return: return MI_OK if successed
 */
unsigned char MFRC552_ReadBlock(unsigned char blockAddr, unsigned char *recvData)
{
    unsigned char status;
    unsigned int unLen;

    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    MFRC552_CalulateCRC(recvData,2, &recvData[2]);
    status = MFRC552_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

    if ((status != MI_OK) || (unLen != 0x90))
    {
        status = MI_ERR;
    }

    return status;
}


/*
 * Function: WriteBlock
 * Description: write block data
 * Input parameters: blockAddr--block address;writeData--Write 16 bytes data into block
 * return: return MI_OK if successed
 */
unsigned char MFRC552_WriteBlock(unsigned char blockAddr, unsigned char *writeData)
{
    unsigned char status;
    unsigned int recvBits;
    unsigned char i;
    unsigned char buff[18];

    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    MFRC552_CalulateCRC(buff, 2, &buff[2]);
    status = MFRC552_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        for (i=0; i<16; i++)
        {
            buff[i] = *(writeData+i);
        }
        MFRC552_CalulateCRC(buff, 16, &buff[16]);
        status = MFRC552_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

        if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}


/*
 * Function: Halt
 * Description: Command the cards into sleep mode
 * Input parameters: null
 * return: null
 */
void MFRC552_Halt(void)
{
    unsigned char status;
    unsigned int unLen;
    unsigned char buff[4];

    buff[0] = PICC_HALT;
    buff[1] = 0;
    MFRC552_CalulateCRC(buff, 2, &buff[2]);

    status = MFRC552_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}
