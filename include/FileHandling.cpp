#ifndef FILE_HAND_C
#define FILE_HAND_C

#include "../include/FileHandling.h"


void init_spi_sd()
{
    sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);   
    // begin(int8_t sck=-1, int8_t miso=-1, int8_t mosi=-1, int8_t ss=-1);
    if( !SD.begin(SD_CS, sd_spi, 20000000)  )   // El valor por defecto era una garcha!
    {
        Serial.println("Card Mount Failed");
    }
    else
    {
        uint8_t cardType = SD.cardType();

        if(cardType == CARD_NONE)
        {
            Serial.println("No SD card attached");
            return;
        }
        else
        {
            Serial.print("SD Card Type: ");
            if(cardType == CARD_MMC){
                Serial.println("MMC");
            } else if(cardType == CARD_SD){
                Serial.println("SDSC");
            } else if(cardType == CARD_SDHC){
                Serial.println("SDHC");
            } else {
                Serial.println("UNKNOWN");
            }
        }
    }
    
}
/*------------------------------------------------------------
                      file handling
  -----------------------------------------------------------*/

void parsePattern(long (&colorArray)[4096]) 
{
  char buf[9];                                      //array zum Speichern eines Hexwertes (temp)
  int i = 0;                                        //Index für das buf array
  int j = 0;                                        //index für das patternArray
  while (patterns.available()) 
  {
    char inChar = patterns.read();                  //ein neuen char einlesen
    byte inCharAscii = inChar;                      //asciiwert des chars speichern
    if (inCharAscii == 10) {                        //ASCII 10 = newline
      buf[i] = '\0';                                //Cstring terminieren
      colorArray[j] = strtoul(buf + 2, NULL, 16);   //cstring in unsigned long umwandeln
      j++;
      i = 0;
    }
    else if (inCharAscii == 13) {                   //ASCII 13 = character return
    }
    else {
      buf[i] = inChar;                              //add the inChar into a buffer
      i++;
    }
  }
}

// Devuelvo size of file para mis métodos
size_t readFile(int index, long (&colorArray)[4096]) 
{ 
  patterns = SD.open("/"+ (String)index + ".txt");  //open the file with the given index
  size_t sizeoffile = patterns.size();

  if (patterns) 
  {                                   //opening succesful
    Serial.println("Parse pattern");
    parsePattern(colorArray);                       //read the file and write to pattern array
  }
  else 
  {
    Serial.println("Error opening file");
    return(0);
  }

  patterns.close();
  return (sizeoffile);
}

String getNames() 
{                                 //get the names of the patterns
  Serial.println("GetNames method!");

  String jsonarray = "[\"";                         //names will be send in a JSON array
  File names = SD.open("/names.txt");
  if (names) {
    while (names.available()) {
      char inChar = names.read();                   //read a new char
      byte inCharAscii = inChar;                    //save the ASCII of the char
      if (inCharAscii == 10) {                      //ASCII 10 = newline
        jsonarray += "\",\"";
      }
      else if (inCharAscii == 13) {                 //ASCII 13 = character return
      }
      else {
        jsonarray += inChar;                        //write the char into the array
      }
    }
    jsonarray += "\"]";
    names.close();
    return jsonarray;
  } else {
    Serial.println("Cant open names file");
    return jsonarray;
  }
}

// 
long findPos(const char searchString[20]) 
{         //finds the position of a string in a file
  Serial.println("Find pos routine");
  File names = SD.open("/names.txt");
  bool found = true;
  // long pos = 0;
  long pos = -1;
  int i = 0;                                        //Index für das buf array

  while (names.available()) 
  {
    Serial.println("Name available");
    char inChar = names.read();                     
    byte inCharAscii = inChar; 
    if (inCharAscii == 10) 
    {                        //ASCII 10 = newline
    Serial.println("New line");
    //pos = names.position();
      if (found == true) 
      {                          //if the characters of one line match the search string
        Serial.println("string found");
        return pos;
      }
      i = 0;
      found = true;
      pos = names.position();                       //save the position of the beginning of the line
    }
    else if (inCharAscii == 13) 
    {                   //ASCII 13 = carriage return
    Serial.println("carr ret");
    }
    else 
    {
      Serial.println("non special char");
      if (found == true) 
      {
        if (inChar == searchString[i]) 
        {            
          Serial.println("True:");
          found = true;
          Serial.println(String(inChar));
        }
        else 
        {
          Serial.println("False:");
          found = false;
          Serial.println(String(inChar));
        }
        i++;
      }
    }
  }
  return pos;
}

#endif