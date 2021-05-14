/*
 * pixels in y axis from 1 dto 16 are yellow
 */
#include "DEV_Config.h"
#include "Debug.h"
#include "GUI_paint.h"
#include "ImageData.h"
#include "OLED_Driver.h"
#include <Adafruit_MLX90614.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>

#define WATER_SENSOR 3 // water temp sensor pin
#define SWITCH 2       // switch pin
#define DEBOUNCING_TIME 100000 // in us

UBYTE *BlackImage;
UWORD Imagesize = ((OLED_0in96_WIDTH%8==0)? (OLED_0in96_WIDTH/8): (OLED_0in96_WIDTH/8+1)) * OLED_0in96_HEIGHT;

bool volatile measureTemp = false;
unsigned long volatile last_micros = 0;
bool int_guard = true;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
OneWire oneWire(WATER_SENSOR);
DallasTemperature sensors(&oneWire);

void measure_object_temperature() {
  if(int_guard) {
    return;
  }
  if((micros()-last_micros) >= DEBOUNCING_TIME){
    measureTemp = !measureTemp;
    Serial.println("interrupt");
  }
  last_micros = micros();
}

void setup() {
  // put your setup code here, to run once:
  System_Init(); //
  OLED_0in96_Init();
  Driver_Delay_ms(500);
  OLED_0in96_clear();
  // start sensors
  sensors.begin();
  mlx.begin();
  pinMode(SWITCH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SWITCH), measure_object_temperature, CHANGE);
  
  
  if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) { 
      Serial.print("Failed to apply for black memory...\r\n");
      return -1;
  }
  
  Paint_NewImage(BlackImage, OLED_0in96_WIDTH, OLED_0in96_HEIGHT, 90, BLACK); 
  
  //1.Select Image
  Paint_SelectImage(BlackImage);
  Paint_Clear(BLACK);
  Driver_Delay_ms(500);
  
  // Narysowanie GUI
  Paint_DrawString_EN(0,2,"Temp. meter v1.0",&Font12, WHITE, WHITE);
  Paint_DrawString_EN(0,18,"Otoczenie ",&Font12, WHITE, WHITE);
  Paint_DrawString_EN(0,31,"Obiekt ",&Font12, WHITE, WHITE);
  Paint_DrawString_EN(1,44,"DS18B20",&Font12, WHITE, WHITE);
  OLED_0in96_display(BlackImage);

  if(!digitalRead(SWITCH)){
    // if switch iss pressed during boot - change initial value of measureTemp flag
    measureTemp = !measureTemp;
  }
  
  int_guard = false; // enable interrupts - some attachInterrupt bug??
}

void loop() {

  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  if(temp != DEVICE_DISCONNECTED_C){
    //wydrukuj wynik na ekran
    const char num[8];
    dtostrf(temp, -6, 2, num);
    Paint_ClearWindows(70, 44, 128, 62, BLACK);
    Paint_DrawNum(70, 44, num , &Font12, 2, WHITE, WHITE);
    OLED_0in96_display(BlackImage);
  }
  else {
    // print that its not connected "---"
    Paint_ClearWindows(70, 44, 128, 62, BLACK);
    Paint_DrawString_EN(70,44,"---",&Font12, WHITE, WHITE);
    OLED_0in96_display(BlackImage);
  }

  // mlx90614 sensor readings:
  // ambient
  temp = mlx.readAmbientTempC();
  const char num[8];
  dtostrf(temp, -6, 2, num);
  Paint_ClearWindows(70, 18, 128, 30, BLACK);
  Paint_DrawNum(70, 18, num , &Font12, 2, WHITE, WHITE);
  OLED_0in96_display(BlackImage);

  // object temperature if wanted
  if(measureTemp){
    temp = mlx.readObjectTempC();
    dtostrf(temp, -6, 2, num);
    Paint_ClearWindows(70, 31, 128, 43, BLACK);
    Paint_DrawNum(70, 31, num , &Font12, 2, WHITE, WHITE);
    OLED_0in96_display(BlackImage);
  }
  else{
    // print that its not measured "---"
    Paint_ClearWindows(70, 31, 128, 43, BLACK);
    Paint_DrawString_EN(70,31,"---",&Font12, WHITE, WHITE);
    OLED_0in96_display(BlackImage);  
  }

  delay(250);
}
