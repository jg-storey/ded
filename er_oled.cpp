/***************************************************
//Web: http://www.buydisplay.com
EastRising Technology Co.,LTD
****************************************************/

//May 2022. Contains modifications by James Storey to imitate F-16 Data Entry Display.

#include <SPI.h>
#include "er_oled.h"


void command(uint8_t cmd)
{
  digitalWrite(OLED_DC, LOW);
  digitalWrite(OLED_CS, LOW);
  SPI.transfer(cmd);
  digitalWrite(OLED_CS, HIGH);
}

void data(uint8_t dat)
{
  digitalWrite(OLED_DC, HIGH);
  digitalWrite(OLED_CS, LOW );
  SPI.transfer(dat);
  digitalWrite(OLED_CS, HIGH);
}


void er_oled_begin()
{
  pinMode(OLED_RST, OUTPUT);
  pinMode(OLED_DC, OUTPUT);
  pinMode(OLED_CS, OUTPUT);
  SPI.begin();
    
  SPI.setClockDivider(SPI_CLOCK_DIV128);
    
  digitalWrite(OLED_CS, LOW);
  digitalWrite(OLED_RST, HIGH);
  delay(10);
  digitalWrite(OLED_RST, LOW);
  delay(10);
  digitalWrite(OLED_RST, HIGH);
    
  
  command(0xFD); /*SET COMMAND LOCK*/ 
  data(0x12); /* UNLOCK */ 
  command(0xAE); /*DISPLAY OFF*/ 
  command(0xB3);/*DISPLAYDIVIDE CLOCKRADIO/OSCILLATAR FREQUANCY*/ 
  data(0x91); command(0xCA); /*multiplex ratio*/ 
  data(0x3F); /*duty = 1/64*/ 
  command(0xA2); /*set offset*/ 
  data(0x00);
  command(0xA1); /*start line*/ 
  data(0x00); 
  command(0xA0); /*set remap*/
  data(0x14); 
  data(0x11);
  	
  command(0xAB); /*funtion selection*/ 
  data(0x01); /* selection external vdd */ 
  command(0xB4); /* */ 
  data(0xA0);
  data(0xfd); 
  command(0xC1); /*set contrast current */ 
  data(0x80); 
  command(0xC7); /*master contrast current control*/ 
  data(0x0f); 
  	
  command(0xB1); /*SET PHASE LENGTH*/
  data(0xE2); 
  command(0xD1); /**/
  data(0x82); 
  data(0x20); 
  command(0xBB); /*SET PRE-CHANGE VOLTAGE*/ 
  data(0x1F);
  command(0xB6); /*SET SECOND PRE-CHARGE PERIOD*/
  data(0x08); 
  command(0xBE); /* SET VCOMH */ 
  data(0x07); 
  command(0xA6); /*normal display*/ 
  command(0xAF); /*display ON*/  
}

void er_oled_SetWindow(uint8_t Xstart, uint8_t Ystart, uint8_t Xend, uint8_t Yend)
{ 
  command(0x15);
  data(Xstart+0x1c);
  data(Xend+0x1c);
  command(0x75);
  data(Ystart);
  data(Yend);
  command(0x5c);//write ram command
}

void er_oled_clear()
{int i,row;
  command(0x15);
  data(0x00); //col start
  data(0x77); //col end 
  command(0x75);
  data(0x00); //row start
  data(0x7f);  //row end 
  command(0x5c); 
  for (row = 0; row < 128; row++) {              
        for(i = 0; i< 240; i++ ) {
          data(0x00);// write data       
        }        
  }
}

// Modified by James Storey for characters with 11 rows of pixels (instead of 16 rows).
void er_oled_char(uint8_t x, uint8_t y, const char  *acsii, uint8_t mode)
{ uint8_t i,str;uint16_t OffSet;
  x=x/4;
  
  OffSet = (*acsii - 32)*11;
  
  er_oled_SetWindow(x, y, x+1, y+11);
  for (i=0;i<11;i++)
  {     str =pgm_read_byte(&AsciiLib[OffSet + i]);  
        if(mode) str=~str;
         Data_processing (str);             
  }
}

void er_oled_string(uint8_t x, uint8_t y, const char *pString,  uint8_t Mode)
{     
  while(1)
  {
        if (*pString == 0)
        {
            return;
        }
            er_oled_char(x, y, pString,Mode);
            x += 8;
            pString += 1;              
  }   
}

//Created by James Storey. Similar to er_oled_string but changes asterisk color and only prints first 24 characters
//because sometimes the DCS bios string contains extra characters.
void er_oled_dedstring(uint8_t x, uint8_t y, const char *pString,  uint8_t Mode)
{     
  while(1)
  {
        if (*pString == 0)
        {
            return;
        }
            if(*pString==42){
              //character is an asterisk so make it dark on light background.
              er_oled_char(x,y,pString,1);
            }else{
              er_oled_char(x,y,pString,Mode);
            }
            x += 8;
            if(x>24*8){
              return;
            }
            pString += 1;              
  }   
}

void Data_processing(uint8_t temp)  //turns 1byte B/W data to 4 bye gray data  with 8 Pixel
{uint8_t temp1,temp2;

  if(temp&0x80)temp1=0xf0;
  else temp1=0x00;
  if(temp&0x40)temp2=0x0f;
  else temp2=0x00;
  temp1=temp1|temp2;
  data(temp1); //Pixel1,Pixel2
  if(temp&0x20)temp1=0xf0;
  else temp1=0x00;
  if(temp&0x10)temp2=0x0f;
  else temp2=0x00;
  temp1=temp1|temp2;
  data(temp1);  //Pixel3,Pixel4                
  if(temp&0x08)temp1=0xf0;
  else temp1=0x00;
  if(temp&0x04)temp2=0x0f;
  else temp2=0x00;
  temp1=temp1|temp2;
  data(temp1);  //Pixel5,Pixel6               
  if(temp&0x02)temp1=0xf0;
  else temp1=0x00;
  if(temp&0x01)temp2=0x0f;
  else temp2=0x00;
  temp1=temp1|temp2;
  data(temp1);  //Pixel7,Pixel8               
}
/*
void er_oled_bitmap_mono(const uint8_t * pBuf)
{ uint8_t row,col,dat; 
  er_oled_SetWindow(0, 0, 255/4, 63); 
  for (row = 0; row < 64; row++) {              
        for(col = 0;col<256/8; col++ ) {
        dat=(pgm_read_byte(pBuf));
        *pBuf++;  
        Data_processing(dat);               
        }
  }    	
}

void er_oled_bitmap_gray(const uint8_t * pBuf)
{   uint8_t row,col; 
  er_oled_SetWindow(0, 0, 255/4, 63);
  for (row = 0; row < 64; row++) {              
        for(col = 0;col<128; col++ ) {
        data(pgm_read_byte(pBuf));
        * pBuf++;
        }
  }    	
}
*/
