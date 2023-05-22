// 17/05/2023 Me descargo y sirvo font-awesome icons y algunos archivos que necesita
// para que funcione offline como AP
// Consumo todo blanco 1.228A
// Consumo todo negro: 0.142A
// 06/05/2023 Casi lindo. Con 16x16 y 32x32 (se acomoda solito el canvas)
// El Save funciona siempre que no borremos nada. Lo voy a hacer desaparecer
// El puto botón del tamaño del lienzo está feo
// Arreglar los containers para celular y casi que es un lindo beta
// Estudiar el WLED con su JSON polenta y su resizeador de imágenes!
// Para canvas de 64x64, si el get/post no puede alojar todo; puedo hacer
// que el micro se acuerde solito los pixeles, y cuando dan clic a save
// lo guarde desde "su caché". Workaround pa guardar,: no se podrá cargar en el web server
// pero algo es algo
// 05/05/2023 Quiero creer que algo he aprendido este tiempo. Como está de origen el código
// dudo que funcione mucho más alla de 16x16 (quizá 32x32)
// Creo que merece más esfuerzo desasnarme del JSON de WLED que puede recbir un JSON grandote
// en partes; y quizá despues reconvertir el c+odigo del pixel ar, para que entregue lo mismo
// Para dejarlo "semi funcional" sacar el tamaño del canvas del file size
// (o ponerlo en el JSON), iterar escritura y lectura hasta ese tamaño
// y a la raja.:insisto en que no creo que llegue a mas de 32x32 sin "partir"
// el Json....
// Despues de save a file; reload index! así veo los iles recien creados!
// NOP! no funciona así...pide getNames cuando reload el navegador y crea el canvas...
// El programa se acomoda solito el tamaño del canvas usado (en la pantalla LED)
// Hay que hacer que la web también pueda acomodarse ;)....
// 30/04/2023 Parto de ejemplos varios (MatrixPanelDMA + Arduino WebServer PixelArt)
// Y trato de hacer una primera aproximación. Web server embebid que permita dibujar
// en la matriz y refrescarla con métodos de paneles 64x64 en lugar
// de los de FASTLED de tira de leds tipo el 2812...
// primero crudazo, después iré puliendo (ponele)
// Cuidado que al traer las librerías DMS I2S me rompe el pin map y deja de andar todo!

#include <main.h>
#include "../include/FileHandling.cpp"
#include "../include/APServer.cpp"
#include "../include/MatrixFunctions.cpp"



void setup() 
{
  Serial.begin(115200);
  Serial.println("Hello World!");


  init_spi_sd();

  // Initialize FS
  if(!LittleFS.begin(true)){
    Serial.println("An Error has occurred while mounting FS");
    return;
  }

  // Module configuration
  HUB75_I2S_CFG mxconfig
  (
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN    // Chain length
  );

  mxconfig.gpio.e = 17;
  mxconfig.driver = HUB75_I2S_CFG::SHIFTREG;
  mxconfig.latch_blanking = 4;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M; //8,10,15 y 20

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90); //0-255
  dma_display->clearScreen();


  #ifdef AP_NOT_WIFI
    IPAddress LocalIP = apIP;   // IPAddress;
    WiFi.mode(WIFI_MODE_APSTA); // FUNDAMENTAL!-o no tanto
    WiFi.softAP(ssid, password);
    delay(2000); // delay de workaround para que no se cuelgue con multiples clientes
    WiFi.softAPConfig(LocalIP, IPAddress(0, 0, 0, 0), IPAddress(255, 255, 255, 0)); // IPAddress(192,168,1,1);
    LocalIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(LocalIP);
  #else
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println(WiFi.localIP());
  #endif 
  handleRoot();
  handleNotFound();
  handlePlain();
  handleMode();
  handleGetNames();
  handleLoad();
  handleSave();
  handleDelete();
  handleChangeCanvas();

  server.begin();
  // dma_display->fillScreen(myGREEN);
  // dma_display->fillRect(0, 0, dma_display->width(), dma_display->height(), dma_display->color444(0, 15, 0));
  // delay(500);
  // dma_display->setTextSize(1);     // size 1 == 8 pixels high
  // dma_display->setTextWrap(true); // Don't wrap at end of line - will do ourselves
  // dma_display->setCursor(5, 0);    // start at top left, with 8 pixel of spacing
  // dma_display->setTextColor(dma_display->color444(15,15,15));
  // dma_display->println("PUTO EL   QUE LEE!");


}


//just to test!
void prepare_read_and_draw(int Index)
{
  File loadPat = SD.open("/" + String(Index) + ".txt", FILE_READ);
  size_t estimated_size = loadPat.size();

  int bucle_read_file2 = 255;
  canvas_size = 16;       //default, check further to see if not

  if(estimated_size > 2304) //if 32x32 canvas: file size will be greater than..
  {
    bucle_read_file2 = 1023;    
    canvas_size = 32; 
  }
  readFile(Index, pattern);
  showPattern(estimated_size);

}

// Funcion para evitar cuelgues de WDT en async tcp cuando guardan CANVAS de 64x64
void save_large()
{
  
  if(SAVE_LARGE_FILE)
  {
    Serial.println("Large save fired!");
    long millis_enter = millis();
    SAVE_LARGE_FILE = false;
    File newFile = SD.open("/" + String(file_to_save_to) + ".txt", FILE_WRITE);
    for (int j= 0 ; j <= 4095; j++)
    {
      newFile.print("0x" );    //manteniendo el formato del ejemplo de partida
      newFile.println( large_canvas_ghost[j] , 16 );
      Serial.println(j);
    }       
    newFile.close(); 
    long millis_leave = millis();
    Serial.println("Task Save Large took:" + String (millis_leave-millis_enter) + "ms");  
  }
 
}


uint8_t wheelval = 0;
size_t size_of_file = 0;
void loop() 
{
  currentMillis = millis();

  save_large();

 // cuando loopee debo saber el file size para hacer el canvas del tamaño adecuado
  if (currentMillis - prevMillis >= interval && live == false) 
  {  //if the mode is looping through the saved pictures and its time for a new one
    if (!(SD.exists("/" + String(loopIndex) + ".txt"))) 
    {      //check if file with given index exists,otherwise begin from beginning
      loopIndex = 0;
    }

    prepare_read_and_draw(loopIndex);

    prevMillis = millis(); 
    loopIndex++;
  }
  
}






