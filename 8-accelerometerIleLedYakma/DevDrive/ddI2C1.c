#include "stm32f411xe.h"
#include "ddI2C1.h"

// brief  : I2C1_Setup Setup
// Arg1   : None 
// return : None
void I2C1_Setup(void)
{
  // RCC
  I2C1_CLOCK_ON();
  PORTB_CLOCK_ON(); 

  // GPIO
  // SCL,SDA  
  GPIOB->MODER   |= (0x02 << (6*2));    //PB.6 AF --> I2C_SCL
  GPIOB->MODER   |= (0x02 << (9*2));    //PB.9 AF --> I2C_SDA
  GPIOB->OTYPER  |= (0x01 << (6*1));    //PB.6 OT --> Open_Drain
  GPIOB->OTYPER  |= (0x01 << (9*1));    //PB.9 OT --> Open_Drain
  GPIOB->AFR[0]  |= (0x04 << (6*4));    //PB.6 AF04
  GPIOB->AFR[1]  |= (0x04 << (1*4));    //PB.9 AF04
  GPIOB->OSPEEDR |= (0x02 << (6*2));    //PB.6 Fast Speed
  GPIOB->OSPEEDR |= (0x02 << (9*2));    //PB.9 Fast Speed

  // I2C
  I2C1->CR1 &= ~(I2C_CR1_PE);           //RESET PE Bit to Configure 
  I2C1->CR2 = 16;                       //TODO Hesap algoritmas� (pclk/1000000)
  I2C1->TRISE = (16+1);                 //TODO Hesap algoritmas�
  I2C1->CCR = (16000000 / (2 * 100000));//TODO
  I2C1->CR1 |= (I2C_CR1_PE);            //SET PE Bit to ENABL I2C 
}


// brief  : I2C_Read_Single_Register
// Arg1   : uint8_t slave_address : Slave I2C ID 
// Arg2   : uint8_t reg_address   : Slave Register Address 
// return : uint8_t : Received Data
uint8_t I2C1_Read_Single_Register(uint8_t slave_address, uint8_t reg_address)
{
  uint8_t rxData = 0;
  //
  while((I2C1->SR2 & I2C_SR2_BUSY) != 0);       //I2C Busy?
  I2C1->CR1 &= ~(I2C_CR1_POS);                  //POS bitini resetle (g�venlikama�l�)
  I2C1->CR1 |= I2C_CR1_START;                   //START olu�tur
  while((I2C1->SR1 & I2C_SR1_SB) == 0);         //I2C Start olu�tu??
  I2C1->DR = (slave_address & (~(0x01)));       //Slave Adresini Yazma biti iste�i ile g�nder
  while((I2C1->SR1 & I2C_SR1_ADDR) == 0);       //I2C Address g�nderildi??
  rxData = I2C1->SR1;                           //ADDR bitini temizlemek i�in
  rxData = I2C1->SR2;                           //ADDR bitini temizlemek i�in
  rxData = 0;                                   //g�venlikama�l�
  while((I2C1->SR1 & I2C_SR1_TXE) == 0);        //I2C TXE Bo� mu??
  I2C1->DR = reg_address;                       //Okunacak Slave Register Adresini g�nder
  while((I2C1->SR1 & I2C_SR1_TXE) == 0);        //I2C TXE Bo� mu??
  //
  I2C1->CR1 |= I2C_CR1_START;                   //RE-START olu�tur (okuma faz�na ge�ilmesi i�in)
  while((I2C1->SR1 & I2C_SR1_SB) == 0);         //I2C Start olu�tu??
  I2C1->DR = (slave_address | (0x01));          //Slave Adresini Okuma biti iste�i ile g�nder
  while((I2C1->SR1 & I2C_SR1_ADDR) == 0);       //I2C Address g�nderildi??
  rxData = I2C1->SR1;                           //ADDR bitini temizlemek i�in
  rxData = I2C1->SR2;                           //ADDR bitini temizlemek i�in
  rxData = 0;                                   //g�venlikama�l�
  I2C1->CR1 |= I2C_CR1_STOP;                    //STOP olu�tur (tek byte okuma)
  while((I2C1->SR1 & I2C_SR1_RXNE) == 0);       //I2C Data Geldi mi??
  rxData = I2C1->DR;                            //Get RX         
  return rxData;                       
}


// brief  : I2C1_Write_Single_Register
// Arg1   : uint8_t slave_address : Slave I2C ID 
// Arg2   : uint8_t reg_address   : Slave Register Address 
// Arg3   : uint8_t data   : Slave Register Data to Write
// return : Error Status : 0 No Error
uint8_t I2C1_Write_Single_Register(uint8_t slave_address, uint8_t reg_address, uint8_t data)
{
  uint8_t rxData = 0;
  //
  while((I2C1->SR2 & I2C_SR2_BUSY) != 0);       //I2C Busy?
  I2C1->CR1 &= ~(I2C_CR1_POS);                  //POS bitini resetle (g�venlikama�l�)
  I2C1->CR1 |= I2C_CR1_START;                   //START olu�tur
  while((I2C1->SR1 & I2C_SR1_SB) == 0);         //I2C Start olu�tu??
  I2C1->DR = (slave_address & (~(0x01)));       //Slave Adresini Yazma biti iste�i ile g�nder
  while((I2C1->SR1 & I2C_SR1_ADDR) == 0);       //I2C Address g�nderildi??
  rxData = I2C1->SR1;                           //ADDR bitini temizlemek i�in
  rxData = I2C1->SR2;                           //ADDR bitini temizlemek i�in
  rxData = 0;                                   //g�venlikama�l�
  while((I2C1->SR1 & I2C_SR1_TXE) == 0);        //I2C TXE Bo� mu??
  I2C1->DR = reg_address;                       //Yaz�lacak Slave Register Adresini g�nder
  while((I2C1->SR1 & I2C_SR1_TXE) == 0);        //I2C TXE Bo� mu??
  I2C1->DR = data;                              //Yaz�lacak Slave Register Datas�n� g�nder  
  while((I2C1->SR1 & I2C_SR1_BTF) == 0);        //I2C BTF Setleninceye kadar Bekle
  I2C1->CR1 |= I2C_CR1_STOP;                    //STOP olu�tur
  return 0;                       
}



