#ifndef APSERVER_H 
#define APSERVER_H

// #include "FS.h"

#include "LittleFS.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include "ESPAsyncWebServer.h"

// We are hosting a webserver on the NodeMCU that communicates with the client (the website you see in your browser) over http POST und GET requests.
// These requests are handled in the "server handles" section
AsyncWebServer server(80);        //start the webserver on port 80 (standard html port)
// File html;                        //file to read the html and push it to the client
// File patterns;                    //file to read one of the patterns stored on the sd

IPAddress apIP(192, 168, 2, 1);
// IPAddress netMsk(255, 255, 255, 0);

void handleRoot();
void handleNotFound();
void handlePlain();
void handleGetNames();
void handleLoad();
void handleSave();
void handleDelete();
void handleMode();
void handleChangeCanvas();

#endif