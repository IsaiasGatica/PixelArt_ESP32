
// #include <FastLED.h>
#include <FS.h> 
#include "LittleFS.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>

// // Display settings
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another
//MatrixPanel_I2S_DMA dma_display;
MatrixPanel_I2S_DMA *dma_display = nullptr;

#define AP_NOT_WIFI //Si quiero que sea AP o que se conecte a alguna red
                    // Como AP queda en 192.168.2.1
                    // 
#ifdef AP_NOT_WIFI
    #define STASSID "PixelArtToy"         //SSID of your WIFI
    #define STAPSK "123456789"      //password of your WIFI
#else
// WiFi settings
    #define STASSID "ReplicaGuGa"         //SSID of your WIFI
    #define STAPSK "Ilegalisimo"      //password of your WIFI
#endif
#define NUM_LEDS 4096                //16*16 matrix = 256 leds / 64*64=4096
const char* ssid = STASSID;
const char* password = STAPSK;

// // We are hosting a webserver on the NodeMCU that communicates with the client (the website you see in your browser) over http POST und GET requests.
// // These requests are handled in the "server handles" section
// WiFiServer server(80);        //start the webserver on port 80 (standard html port)
// File html;                          //file to read the html and push it to the client
// File patterns;                      //file to read one of the patterns stored on the sd

//CRGB leds[NUM_LEDS];                //initalise the array of the leds that we can later assign colorvalues
extern const uint8_t gamma8[];      //this is a forward declaration for the gamma correction (used to make the colors look more natural)
long pattern[NUM_LEDS];             //array of the current displayed led pattern
bool live = true;                   //determines if NodeMCU is in live draw mode or shows the saved patterns in a loop
long currentMillis = 0;             //we all know these variables.. blink without delay...
long prevMillis = millis();
int interval = 7000;
int loopIndex = 0;                  //index of the loop for displaying the saved images
long large_canvas_ghost[NUM_LEDS];  //trampita pa grabar canvas que no entra en el POST
volatile bool SAVE_LARGE_FILE;
volatile int file_to_save_to;
//Just in case constants
uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);

