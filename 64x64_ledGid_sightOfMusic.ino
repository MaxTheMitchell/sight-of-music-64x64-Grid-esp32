//#define PxMATRIX_double_buffer true
#include <PxMatrix.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
const char* ssid = "YourWifiSSID";
const char* password = "YourWifiPassword";

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#define matrix_width 64
#define matrix_height 64

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time=10; //30-70 is usually fine

PxMATRIX display(64,64,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);

void IRAM_ATTR display_updater(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void display_update_enable(bool is_enable){
  if (is_enable){
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 4000, true);
    timerAlarmEnable(timer);
  }else{
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
   delay(4000);
//  Serial.println("Test");
  // Define your display layout here, e.g. 1/8 step, and optional SPI pins begin(row_pattern, CLK, MOSI, MISO, SS)
  display.begin(32);
  display.clearDisplay();
  display_update_enable(true);
  delay(4000);
}

void loop() {
   for(int y=0;y<64;y++){
    parseLEDString(
      getReq(
        (String)"http://sight-of-music-stage.herokuapp.com/image/pixles/64/section?start="+(String)(y*64)+"&end="+(String)((y+1)*64)
        )
      ,y
      );
    delay(100);
   }
   delay(10000);
}

void parseLEDString(String ledString,int y){
  int rgb[3];
  int cIndex = 0;
  String tmpRGB = "";
  for(int x=0;x<64;x++){
    for(int colorIndex=0;colorIndex<3;colorIndex++){
      char c = ledString.charAt(cIndex);
      while((c > '/' && c < ':') || tmpRGB.length() <= 0){
        if((c > '/' && c < ':'))tmpRGB += c;
        c = ledString.charAt(cIndex++);
      }
//      Serial.println(tmpRGB);
      rgb[colorIndex] = tmpRGB.toInt();
      tmpRGB = "";
    }
    display.drawPixel(x,y,display.color565(rgb[0],rgb[2],rgb[1]));
    delay(10);
  }
}

String getReq(String url){
  WiFiClient client;
  HTTPClient http;
  if (http.begin(client,url)) {
    if(http.GET() > 0){
      String payload = http.getString();
      http.end();
      return payload;
    }
  }
  http.end();
  return getReq(url);  
}
