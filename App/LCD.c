#include "include.h"
#include <stdarg.h>
#include <stdio.h>

void LCD_printf(uint16 x,uint16 y,const char* format,...)
{
    static char buffer[32];
    va_list ag;
    va_start(ag,format);
    vsprintf(buffer,format,ag);
    va_end(ag);
    
    Site_t site = {x,y};
    LCD_str(site,(uint8*)buffer,FCOLOUR,BCOLOUR);
}