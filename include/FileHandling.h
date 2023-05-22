#ifndef FILE_HAND_H 
#define FILE_HAND_H

#include <SPI.h>
#include <SD.h>


// Software SPI pins to use for SD card reading
#define SD_CS 22
#define SD_MOSI 21
#define SD_SCK 19
#define SD_MISO 2



SPIClass sd_spi = SPIClass(3);

File html;                          //file to read the html and push it to the client
File patterns;                      //file to read one of the patterns stored on the sd

void init_spi_sd();
void parsePattern(long (&colorArray)[4096]);
size_t readFile(int index, long (&colorArray)[4096]);
String getNames();
long findPos(const char searchString[20]);

#endif