#ifndef APSERVER_C 
#define APSERVER_C

#include "../include/APServer.h"
#include "../include/MatrixFunctions.h"
#include "../include/FileHandling.h"

/*------------------------------------------------------------
                      server handles
  -----------------------------------------------------------*/
void handleRoot() 
{                                               //handle normal opening of the website
  // html = SD.open("index.html");
  // server.streamFile(html, "text/html");                           //stream the contents of the .html file to the client to be displayed
  // html.close();                                                 //close the file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false);
  });
  // server.on("/a10ccb6ec5.js", HTTP_GET, [](AsyncWebServerRequest *request)
  //         { request->send(LittleFS, "/a10ccb6ec5.js", "text/javascript"); });
  server.on("/fontawesome-all.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/fontawesome-all.css", "text/css" );
  });


  server.on("/webfonts/fa-solid-900.woff2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/webfonts/fa-solid-900.woff2", String(), false);
  });
  server.on("/webfonts/fa-solid-900.woff", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/webfonts/fa-solid-900.woff", String(), false);
  });
    server.on("/webfonts/fa-solid-900.ttf", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/webfonts/fa-solid-900.ttf", String(), false);
  });  

}

void handleNotFound() 
{
  String message = "File Not Found\n\n";
  server.onNotFound( [](AsyncWebServerRequest *request){
    // request->send(404, "text/plain", message);
    request->send(404);
  });
}

void handlePlain() 
{                                               //handles the post requests for clearing the canvas and drawing a pixel
  server.on("/live", HTTP_POST, [](AsyncWebServerRequest *request){
    //Serial.println("POST method!");
    //request->send(LittleFS, "/index.html", String(), false);
    if(request->hasArg("cl"))
    {
      Serial.println("Clear Matrix");
        clearMatrix();
    }
    else
    {
      //Serial.println("Draw pixel:" + String (request->arg("i").toInt()) );
      //i for the index, c for the color
      draw(request->arg("i").toInt(), strtoul(request->arg("c").c_str(), NULL, 16));
      if(canvas_size == 64)
      {
        //Serial.println("Save to ghost");
        large_canvas_ghost[request->arg("i").toInt()] = strtoul(request->arg("c").c_str(), NULL, 16);
      }
    }
    request->send(200);
  });  
}

void handleGetNames() 
{                                                //handles the request for getting the names of the stored patterns
//  String names = getNames();

 server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Get names from SD card (hopefully)");
      request->send(200, "text/plain", getNames());
        });

  // server.send(200, "text/plain", names);
}

// #define VERBOSE_HANDLE_LOAD
void handleLoad() 
{                                               //handles loading of a pattern
  server.on("/load", HTTP_POST, [](AsyncWebServerRequest *request){
    bool SKIP_JSON = false;
    Serial.println("Load File on pattern!");

    if(request->hasArg("id"))
    {
      #ifdef VERBOSE_HANDLE_LOAD
      Serial.println("ARG ID:" + request->arg("id"));
      #endif
      String jsonarray = "[\"";                                       //response to the request will be a JSON array of the color values
      String id_requested = request->arg("id");
      File loadPattern = SD.open("/" + id_requested + ".txt", FILE_READ);
      size_t estimated_size_canvas = loadPattern.size();

      Serial.println("Of size:"+String(estimated_size_canvas));

      int bucle_read_file = 255;
      canvas_size = 16;       //default, check further to see if not

      if(estimated_size_canvas > 2304) //if 32x32 canvas: file size will be greater than..
      {
        bucle_read_file = 1023;    
        canvas_size = 32; 
      }
      //check file size of 64x64 fie and make some answer for web server
      if(estimated_size_canvas > 20000)
      {
        bucle_read_file = 4096;    
        canvas_size = 64;   
        SKIP_JSON = true; 
        Serial.println("Send empty JSON");
      }


      if (loadPattern) 
      {
        #ifdef VERBOSE_HANDLE_LOAD
        Serial.println("Pattern:"); 
        #endif
        int i = 0;                                                    //index how many values have been written to the array, used to exclude the final newline in the .txt file
        while (loadPattern.available() && !SKIP_JSON)
        {
          char inChar = loadPattern.read();                           //read a new char
          byte inCharAscii = inChar;                                  //save the ascii of the char
          if (inCharAscii == 10) 
          {                                    //ASCII 10 = newline
            if (i < bucle_read_file)            //el pattern (cantdad de bytes) dependerá del tamaño del lienzo
            {
              jsonarray += "\",\"";
              i++;
            }
            else                                //end of file!
            {
              break;  // Check if i works, at least for 16x16 size! GuGa
            }
          }
          else if (inCharAscii == 13) 
          {                               //ASCII 13 = character return
            #ifdef VERBOSE_HANDLE_LOAD
            Serial.println("handleLOAD char 13");
            #endif
          }
          else 
          {
            jsonarray += inChar;                                      //schreiben des chars in das tempArray
          }
        }
        jsonarray += "\"]";                                           //close the JSON array
        loadPattern.close();                                          //close the file      
        #ifdef VERBOSE_HANDLE_LOAD
        Serial.println("Json Array"+ jsonarray);
        #endif
        request->send(200, "text/plain", jsonarray);                     //send the response
        readFile(request->arg("id").toInt(), pattern);                   //show the loaded pattern
        // Get size of canvas automatically by file size (16x16 or 32x32)
        // no creo que dé para más en un solo POST!
        showPattern(estimated_size_canvas);      
      } 
      else 
      {                                                        
        Serial.println("Cant open names file"); 
        request->send(404, "text/plain", "error");
      }
    }
    else
    {
      #ifdef VERBOSE_HANDLE_LOAD
      Serial.println("NO arg ID");
      #endif
      request->send(200, "text/plain","No id to read");
    }
  });  

}

// #define VERBOSE_HANDLE_SAVE
void handleSave() 
{
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request)
  {
      long millis_enter = millis();
      Serial.println("Save to SD card");

      #ifdef VERBOSE_HANDLE_SAVE
      Serial.println("pt" + String(request->arg("pt")));
      #endif
      File names = SD.open("/names.txt", FILE_APPEND);        //tenia FILE_WRITE          
      if (names) 
      {
        names.println(request->arg("name"));                            //add the newly added pattern to the names file
        names.close();
        #ifdef VERBOSE_HANDLE_SAVE
        Serial.println("name:" + String(request->arg("name")));
        #endif
      } 
      else 
      {
        #ifdef VERBOSE_HANDLE_SAVE
        Serial.println("Can't open name.txt");
        #endif
      }
      names.close();

      int i = 0; 
      while (SD.exists("/" + String(i) + ".txt")) 
      {                         //iterate over the existing files till we have a free spot
        i++;
      }    
// Si el canvas es de 64x64, el POST no me manda la matriz "me la acuerdo y la vuelco desde el ghost"
// contesto antes de volcar, porque me parece que al tardar tanto, me resetea el micro porque async_rcp se chifla!
      File newFile = SD.open("/" + String(i) + ".txt", FILE_WRITE);         //create a new file for the new pattern
      if(canvas_size != 64)
      {
        // File newFile = SD.open("/" + String(i) + ".txt", FILE_WRITE);         //create a new file for the new pattern
        //File newFile = SD.open("/" + String(i) + ".txt", FILE_WRITE);         //create a new file for the new pattern
        // Serial.println("Ghost array! Haciendo trampa");
        // for (int j= 0 ; j <= 4095; j++)
        // {

        //   newFile.print("0x");    //manteniendo el formato del ejemplo de partida
        //   newFile.println( large_canvas_ghost[j] , 16 );
        //   // Serial.println( large_canvas_ghost[j] , 16);
        // }
      // }
      // else 
      // {
        // en los canvas 16x16 y 32x32, grabo lo que el POST me manda
        newFile.print(request->arg("pt"));    
        newFile.close();                          //pt for pattern, argument has the right format so we can just write it to SD
        request->redirect("/");
      }
      else
      {
        request->send(200, "text/plain", "done");
        SAVE_LARGE_FILE = true;     //en el main guardaré. Ya sé que todo existe y está ahí!
        file_to_save_to = i;        // me guardo cosillas
        //Serial.println("Ghost array! Haciendo trampa");
        // Pero escribir byte a byte tarda mucho y a veces se cuelga! usar otro método o un buffer!
        // for (int j= 0 ; j <= 4095; j++)
        // {
        //   // newFile.println("0x" + String(large_canvas_ghost[j]).toInt() ,16);    //manteniendo el formato del ejemplo de partida
        //   //newFile.println( large_canvas_ghost[j] , 16 );
        //   // newFile.flush();
        //   //delayMicroseconds(1);
        //   // Serial.println(j);
          
        // }       
        // newFile.close(); 
      }
      newFile.close();
      long millis_leave = millis();
      Serial.println("Task handleSave took:" + String (millis_leave-millis_enter) + "ms");  
      
      // #ifdef VERBOSE_HANDLE_SAVE
      // Serial.println("pt" + String(request->arg("pt")));
      // #endif
      // File names = SD.open("/names.txt", FILE_APPEND);        //tenia FILE_WRITE          
      // if (names) {
      //   names.println(request->arg("name"));                            //add the newly added pattern to the names file
      //   names.close();
      //   #ifdef VERBOSE_HANDLE_SAVE
      //   Serial.println("name:" + String(request->arg("name")));
      //   #endif
      // } 
      // else 
      // {
      //   #ifdef VERBOSE_HANDLE_SAVE
      //   Serial.println("Can't open name.txt");
      //   #endif
      // }
      // names.close();
      // request->redirect("/"); 
      // request->send(200, "text/plain", "done");

  });
}

// Borra desde el archivo escogido PARA ABAJO!
// Y luego cuando grabo otro queda como el OJETE!
// Y si pongo el nombre de archivo en el nombre de archivo?
void handleDelete()
 {                                             //response to the client
  server.on("/delete", HTTP_POST, [](AsyncWebServerRequest *request){
    long pos = findPos(request->arg("name").c_str());                 //find the position of the name to delete in the name file
    File names = SD.open("/names.txt", FILE_WRITE);
    Serial.println("Position:"+ String(pos) );
    names.seek(pos);    
    Serial.println("Position1:"+ String(names.position()) );  //add by guga 
                                               //open the file with the cursor at the pos
    names.write('^');                                               //^ is the character for deleted names
    for (int i = 0; i < request->arg("name").length(); i++) {
      names.write(' ');                                             //there is no easy way to really delete something from a file so we just ust spaces
    }
    names.close();
    request->redirect("/"); 
  });
}

void handleMode() 
{                                               //displaying saved patterns in a loop or draw live 
  server.on("/mode", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("MODE selection");
      //request->send(LittleFS, "/index.html", String(), false);
      if(request->hasArg("mode"))
      {
        if(request->arg("mode") == "true")
        {
          live = true;
          Serial.println("MODE TRUE");
        }
        else
        {
          live = false;
          Serial.println("MODE FALSE");
        }

      }
      request->send(200);
  });   
  // server.send(200);
  // if (server.arg("mode") == "true") {                             //toggle the variable
  //   live = true;
  // }
  // else {
  //   live = false;
  // }
}
// Limpiar el canvas en el ESP!
void handleChangeCanvas()
{
  server.on("/changecanvas", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("Change Canvas size");
      //request->send(LittleFS, "/index.html", String(), false);
      if(request->hasArg("canvas"))
      {
        if(request->arg("canvas") == "16")
        {
          Serial.println("Canvas 16x16");
          canvas_size = 16;
        }
        else if(request->arg("canvas") == "32")
        {
          Serial.println("Canvas 32x32");
          canvas_size = 32;
        }
        else if(request->arg("canvas") == "64")
        {
          Serial.println("Canvas 64x64");
          canvas_size = 64;
        }

        clearMatrix();
      }
      request->send(200);
  });     
}

#endif