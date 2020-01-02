#include "w25qxx.h"
#include "variants.h"

/*************************** W25Qxx SPI Ä£Êœµ×²ãÒÆÖ²µÄœÓ¿Ú ***************************/
//#define W25Qxx_SPI     _SPI3
//#define W25Qxx_SPEED   0

//Æ¬Ñ¡
void W25Qxx_SPI_CS_Set(u8 level)
{
   #if defined(MKS_32_V1_4)
   if (level==0)
    GPIO_ResetBits(FLASH_nCS_GPIO_Port,FLASH_nCS_Pin);
   else
   GPIO_SetBits(FLASH_nCS_GPIO_Port,FLASH_nCS_Pin);
   #else
      SPI_CS_Set(W25Qxx_SPI,level);
  #endif
}

//¶ÁÐŽº¯Êý
uint8_t W25Qxx_SPI_Read_Write_Byte(uint8_t data)
{
  return SPI_Read_Write(W25Qxx_SPI,data);
}

//³õÊŒ»¯
void W25Qxx_Init(void)
{
  SPI_Config(W25Qxx_SPI);
  SPI_Protocol_Init(W25Qxx_SPI, W25Qxx_SPEED);
  W25Qxx_SPI_CS_Set(1);
}
/*************************************************************************************/


// ÐŽÊ¹ÄÜ
void W25Qxx_WriteEnable(void)
{
  W25Qxx_SPI_CS_Set(0);
  W25Qxx_SPI_Read_Write_Byte(CMD_WRITE_ENABLE);
  W25Qxx_SPI_CS_Set(1);
}
//µÈŽýW25Qxx¿ÕÏÐ
void W25Qxx_WaitForWriteEnd(void)
{
  uint8_t flashstatus = 0;

  W25Qxx_SPI_CS_Set(0);
  W25Qxx_SPI_Read_Write_Byte(CMD_READ_REGISTER1);
  do
  {
    flashstatus = W25Qxx_SPI_Read_Write_Byte(W25QXX_DUMMY_BYTE);
  }
  while ((flashstatus & 0x01) == SET); 
  W25Qxx_SPI_CS_Set(1);
}

//°ŽÒ³ÐŽ
void W25Qxx_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  W25Qxx_WriteEnable();
  W25Qxx_SPI_CS_Set(0);
	
  W25Qxx_SPI_Read_Write_Byte(CMD_PAGE_PROGRAM);
  W25Qxx_SPI_Read_Write_Byte((WriteAddr & 0xFF0000) >> 16);
  W25Qxx_SPI_Read_Write_Byte((WriteAddr & 0xFF00) >> 8);
  W25Qxx_SPI_Read_Write_Byte(WriteAddr & 0xFF);
	
  while (NumByteToWrite--)
  {
    W25Qxx_SPI_Read_Write_Byte(*pBuffer);
    pBuffer++;
  }
	
  W25Qxx_SPI_CS_Set(1);
  W25Qxx_WaitForWriteEnd();
}

//ÏòFLASHÖÐ·¢ËÍbufferµÄÊýŸÝ
void W25Qxx_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % W25QXX_SPI_PAGESIZE;//Žú±íŽÓÒ»Ò³ÖÐÄÄžöµØÖ·¿ªÊŒÐŽÊýŸÝ
  count = W25QXX_SPI_PAGESIZE - Addr;//Žú±ížÃÒ³»¹¿ÉÒÔÐŽ¶àÉÙÊýŸÝ
  NumOfPage =  NumByteToWrite / W25QXX_SPI_PAGESIZE;//Žú±íÒªŽæÈëµÄÊýŸÝ³€¶È¿ÉÒÔÐŽ¶àÉÙÒ³
  NumOfSingle = NumByteToWrite % W25QXX_SPI_PAGESIZE;//Žú±í³ýÁËÕûÒ³ÊýŸÝÍâ£¬»¹¿ÉÒÔÐŽµÄÊýŸÝ³€¶È

  if (Addr == 0) /*!< WriteAddr is sFLASH_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
    {
      W25Qxx_WritePage(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      while(NumOfPage--)
      {
        W25Qxx_WritePage(pBuffer, WriteAddr, W25QXX_SPI_PAGESIZE);
        WriteAddr +=  W25QXX_SPI_PAGESIZE;
        pBuffer += W25QXX_SPI_PAGESIZE;
      }
      W25Qxx_WritePage(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /*!< WriteAddr is not sFLASH_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
    {
      if (NumOfSingle > count) /*!< (NumByteToWrite + WriteAddr) > sFLASH_PAGESIZE */
      {
        temp = NumOfSingle - count;

        W25Qxx_WritePage(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        W25Qxx_WritePage(pBuffer, WriteAddr, temp);
      }
      else
      {
        W25Qxx_WritePage(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / W25QXX_SPI_PAGESIZE;
      NumOfSingle = NumByteToWrite % W25QXX_SPI_PAGESIZE;

      W25Qxx_WritePage(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        W25Qxx_WritePage(pBuffer, WriteAddr, W25QXX_SPI_PAGESIZE);
        WriteAddr +=  W25QXX_SPI_PAGESIZE;
        pBuffer += W25QXX_SPI_PAGESIZE;
      }

      if (NumOfSingle != 0)
      {
        W25Qxx_WritePage(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/*ŽÓFLASHÖÐ¶ÁÊýŸÝ*/
void W25Qxx_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
  W25Qxx_SPI_CS_Set(0);

  W25Qxx_SPI_Read_Write_Byte(CMD_READ_DATA);

  W25Qxx_SPI_Read_Write_Byte((ReadAddr & 0xFF0000) >> 16);
  W25Qxx_SPI_Read_Write_Byte((ReadAddr& 0xFF00) >> 8);
  W25Qxx_SPI_Read_Write_Byte(ReadAddr & 0xFF);

  while (NumByteToRead--)
  {
    *pBuffer = W25Qxx_SPI_Read_Write_Byte(W25QXX_DUMMY_BYTE);
    pBuffer++;
  }
	
  W25Qxx_SPI_CS_Set(1);
}

//¶ÁID
uint32_t W25Qxx_ReadID(void)
{
  uint32_t Temp = 0;

  W25Qxx_SPI_CS_Set(0);
  W25Qxx_SPI_Read_Write_Byte(CMD_READ_ID);

  Temp = W25Qxx_SPI_Read_Write_Byte(W25QXX_DUMMY_BYTE);
  Temp = (Temp<<8) | W25Qxx_SPI_Read_Write_Byte(W25QXX_DUMMY_BYTE);
  Temp = (Temp<<8) | W25Qxx_SPI_Read_Write_Byte(W25QXX_DUMMY_BYTE);

  W25Qxx_SPI_CS_Set(1);

  return Temp;
}

//ÉÈÇø²Á³ý
void W25Qxx_EraseSector(uint32_t SectorAddr)
{
  W25Qxx_WriteEnable();

  W25Qxx_SPI_CS_Set(0);
  W25Qxx_SPI_Read_Write_Byte(CMD_SECTOR_ERASE);
  W25Qxx_SPI_Read_Write_Byte((SectorAddr & 0xFF0000) >> 16);
  W25Qxx_SPI_Read_Write_Byte((SectorAddr & 0xFF00) >> 8);
  W25Qxx_SPI_Read_Write_Byte(SectorAddr & 0xFF);
  W25Qxx_SPI_CS_Set(1);

  W25Qxx_WaitForWriteEnd();
}

//¿é²Á³ý
void W25Qxx_EraseBlock(uint32_t BlockAddr)
{
  W25Qxx_WriteEnable();
	
  W25Qxx_SPI_CS_Set(0);
  W25Qxx_SPI_Read_Write_Byte(CMD_BLOCK_ERASE);
  W25Qxx_SPI_Read_Write_Byte((BlockAddr & 0xFF0000) >> 16);
  W25Qxx_SPI_Read_Write_Byte((BlockAddr & 0xFF00) >> 8);
  W25Qxx_SPI_Read_Write_Byte(BlockAddr & 0xFF);
  W25Qxx_SPI_CS_Set(1);

  W25Qxx_WaitForWriteEnd();
}

//È«Æ¬²Á³ý
void W25Qxx_EraseBulk(void)
{
  W25Qxx_WriteEnable();

  W25Qxx_SPI_CS_Set(0);
  W25Qxx_SPI_Read_Write_Byte(CMD_FLASH__BE);
  W25Qxx_SPI_CS_Set(1);

  W25Qxx_WaitForWriteEnd();
}


