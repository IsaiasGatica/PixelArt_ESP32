// Primitivas básicas de uso del panel.
// 

#ifndef MATRIXFUNC_C 
#define MATRIXFUNC_C

#include <MatrixFunctions.h>

void clearMatrix() 
{   
    dma_display->clearScreen();
}

// #define VERBOSE_DRAW
void draw(int index, long color) 
{                                //draw one pixel on the matrix Starting at [0,0]
    uint16_t colorformat = 0;
    #ifdef VERBOSE_DRAW
        Serial.println();
        Serial.println("Index:" + String(index));
    #endif
    int row = 0;     //row           
    int column = 0;  //column
    // Canvas size debe coincidir en la Web Page y en la aplicacion para que no se vaya todo
    // a la mierda
    row = index / canvas_size;   
    column = ( index - (row * canvas_size) ) ;
   
    #ifdef VERBOSE_DRAW
    Serial.println("To Row: " + String(row) + "Column: " + String(column));
    Serial.println("To color: "+ String(color) );
    #endif
    //Estos tres juntos funcionan igual que el de abajo solito; pero creo que el resultado es el mismo
    colorformat = dma_display->color565(pgm_read_byte(&gamma8[(color >> 16) & 255]), pgm_read_byte(&gamma8[(color >> 8) & 255]) , pgm_read_byte(&gamma8[(color) & 255]));
    #ifdef VERBOSE_DRAW
    Serial.println("To color formatted:" + String(colorformat) );
    #endif
    //Acomodando el tamaño de los pixwles al lienzo elegido
    if(canvas_size == 32)
    {
        // Serial.println("Canvas 32");    //pixeles al doble
        row = row * 2;
        column = column * 2;
        dma_display->drawPixel(column,row,colorformat);
        dma_display->drawPixel(column,row+1,colorformat);
        dma_display->drawPixel(column+1,row,colorformat);
        dma_display->drawPixel(column+1,row+1,colorformat);
    }
    else if(canvas_size == 16)
    {
        // Serial.println("Canvas 16");    //pixel 4 veces más grandes
        row = row * 4;
        column = column * 4;
        dma_display->drawPixel(column,row,colorformat);
        dma_display->drawPixel(column,row+1,colorformat);
        dma_display->drawPixel(column,row+2,colorformat);
        dma_display->drawPixel(column,row+3,colorformat);

        dma_display->drawPixel(column+1,row,colorformat);
        dma_display->drawPixel(column+1,row+1,colorformat);
        dma_display->drawPixel(column+1,row+2,colorformat);
        dma_display->drawPixel(column+1,row+3,colorformat);

        dma_display->drawPixel(column+2,row,colorformat);
        dma_display->drawPixel(column+2,row+1,colorformat);
        dma_display->drawPixel(column+2,row+2,colorformat);
        dma_display->drawPixel(column+2,row+3,colorformat);

        dma_display->drawPixel(column+3,row,colorformat);
        dma_display->drawPixel(column+3,row+1,colorformat);
        dma_display->drawPixel(column+3,row+2,colorformat);
        dma_display->drawPixel(column+3,row+3,colorformat);        
   
    }
    else if(canvas_size == 64)
    {
        #ifdef VERBOSE_DRAW
        Serial.println("Canvas 64");
        #endif
        dma_display->drawPixel(column,row,colorformat);
    }

}


void showPattern(size_t sizeofcanvas) 
{                                              //display pattern currently stored in pattern[256]
    long millis_enter = millis();
    int cycle_to = 256;             //default 16x16 canvas
    // canvas_size = 16;

    Serial.println("Pattern Size of file:" + String (sizeofcanvas));
    // 2304 bytes for 16x16
    // 9216 bytes for 32x32
    // 20486 bytes for 64x64
    if(sizeofcanvas >= 2304) //if 32x32 canvas: file size will be greater than..
    {
        cycle_to = 1024;
        // canvas_size = 32;   //last minuto to make loadpattern on loop works
    }
    if(sizeofcanvas >= 20000)
    {
         cycle_to = 4096;
        //  canvas_size = 64;
    }

    for (int i = 0; i < cycle_to; i++) 
    {
        draw(i,pattern[i]);
    }
    long millis_leave = millis();
    Serial.println("Task showPattern took:" + String (millis_leave-millis_enter) + "ms");  
}

#endif