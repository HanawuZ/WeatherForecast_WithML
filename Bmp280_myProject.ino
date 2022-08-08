/* #########################  WiFi Initialization  #########################  */
#include <WiFi.h>                                       		/* Library for WiFi   */
#include <HTTPClient.h>                                 		/* Library for HTTP GET, POST and PUT requests to a web server. */
#define WebHooksKey         "YOUR_WEBHOOKS_KEY"    				/* Define your WebHooks Key 		*/
#define WebHooksEventName1  "YOUR_WEBHOOKS_EVENT_NAME"         	/* Define your WebHooks Event name 	*/
#define WebHooksEventName2  "YOUR_WEBHOOKS_EVENT_NAME"
const char* ssid = "YOUR_WIFI_NAME";                            /* Your WiFi name */
const char* password = "WIFI_PASSWORD";            				/* WiFi password  */

/* #########################  BMP-280 Initialization  ######################### */
#include <Wire.h>
#include <Adafruit_BMP280.h>            /* Includes Library for BMP-280         */
Adafruit_BMP280 bmp;                    /* Initialize BMP-280 module object     */

/* #########################  DHT-11 Initialization  #########################  */
#include "DHTesp.h"                     /* Includes library for DHT-11          */
#define DHT_pin 19                      /* Declare Pin for DHT-11 at GPIO 19    */
DHTesp dht;                             /* Initialize DHT-11 module object      */

/* #########################  TM1639 Initialization  #########################  */
#include <TM1638plus.h>                 /* Includes Library to control TM1638 7-segments modules  */
#define Brd_STB 15                      /* strobe = GPIO connected to strobe line of module */
#define Brd_CLK 4                       /* clock = GPIO connected to clock line of module   */
#define Brd_DIO 5                       /* data = GPIO connected to data line of module     */
#define high_freq true
TM1638plus tm(Brd_STB, Brd_CLK , Brd_DIO, high_freq);     /* Initialize TM1638 7-segments modules object  */

/* #########################  MAX7219 7-segments Initialization  #########################  */
#define DIN 13                          /* This pin is connected to DataIn(DIN) */
#define CS 12                           /* this pin is connected to LOAD(CS)    */ 
#define CLK 14                          /* this pin is connected to CLK         */
#include "LedController.hpp"            /* Includes Library for controlling MAX7219 7-segments      */
LedController lc;                       /* Initialize objecgt for controlling MAX7219 7-segments    */

#define delaytime 250                               /* Defines delay time, its value is 250 milliseconds  */   
TaskHandle_t SendSensorValuesTask = NULL;           /* Initialize Task variable for sending data value    */
int BMP280_status = 0;                              /* Variable for identifying BMP-280 status            */

/* ############################################################################################################################# */

void sayDisplayValue(int DisplayStatus , int dataValueStatus){
/*  ===========================================================================================
 *  Function to say display values or stop display values   
 *  o DisplayStatus indicates that are you want to say "Display values" or "Stop display values"
 *    - if DisplayStatus = 1  : Let the MAX7219 show "Display values" 
 *                              depends on a paramenter value of dataValueStatus.
 *    - if DisplayStatus = 0  : Let the MAX7219 show "Stop Display values" 
 *                              also depends on the paramenter dataValueStatus.
 *  
 *  o dataValueStatus indicates that what value will be display on TM1638
 *  +-------------------------------------------------------------+
 *  |  - dataValueStatus = 1 : Display DHT-11 temperature value   |
 *  |  - dataValueStatus = 2 : Display DHT-11 Humid value         |
 *  |  - dataValueStatus = 3 : Display BMP-280 Pressure value     |
 *  |  - dataValueStatus = 4 : Display BMP-280 Altitude value     |
 *  |  - dataValueStatus = 5 : Display BMP-280 temperature value  |
 *  +-------------------------------------------------------------+
 *  =========================================================================================== */
    String Text;

    /* =========================  If wants MAX7219 show "Display values"  ========================= */
    if (DisplayStatus) {
        
        /* +++++++  Display DHT-11 Temp or BMP280 Temp  +++++++ */
        if (dataValueStatus == 1  || dataValueStatus == 5) { 
            Display("Display t");
            if (dataValueStatus == 1) Text = "DHT11-T";     /* Display DHT-11 Temperature */
            else  Text = "BP280-T";                         /* Display BMP-280 Temperature */
        }
    
        /* Display DHT-11 Humidity */
        else if (dataValueStatus == 2){ 
            Display("Display H");
            Text = "DHT11-H";
        }
    
        /* Display BMP-280 Pressure */
        else if (dataValueStatus == 3){ 
            Display("Display Pressure"); 
            Text = "BP280-P";
        }
    
        /* Display BMP-280 Altitude */
        else if (dataValueStatus == 4){ 
            Display("Display Altitude");
            Text = "BP280-Al";
        }
    }

    /* =========================  If wants MAX7219 display "Stop display values"  ========================= */
    else {
        if (dataValueStatus == 1 || dataValueStatus == 5) {  
            Display("Stop display Temp");
        }
        else if (dataValueStatus == 2){   
            Display("Stop display Humid"); 
        }
        else if (dataValueStatus == 3){   
            Display("Stop display Pressure");  
        }
        else if (dataValueStatus == 4){   
            Display("Stop display Altitude");  
        }
    }
    delay(delaytime);

    /* ===================  Let's MAX7219 7-segments always shows "display data value" ===================  */
    if (DisplayStatus){
        for (int i = 0 ; i < Text.length() ; i++) lc.setChar(0,7-i,Text[i],false);
    }
}

void Display(String cmd){
    lc.clearMatrix();
    String Text[8];     
    String temp;              /* String variable for temporarily storing a String.  */
    int TextIndex = 0;        /* Int variable for indicating index of String array  */

    /* ++++++++++ For loop for splitting sentence seperated by space(' ') ++++++++++ */
    for (int i = 0 ; i < cmd.length() ; i++){
        delay(10);

        /* ================ if character is not space(' ') ================ */
        if (cmd[i] != ' ') temp.concat(String(cmd[i]));         /* Start concatration this character with String temp. */

        /* ================ if character is space or i value is a last index of letter ================ */
        if (cmd[i] == ' ' || i == cmd.length()-1) {
            Text[TextIndex] = temp;                             /* Assign string temp to String array of Text */
            temp = "";                                          /* Clear string temp  */
            TextIndex++;                                        /* Increases index of String array  */
        }
    }

    /* ++++++++++++++ Let's MAX7219 7-segments shows Text ++++++++++++++  */
    for (int i = 0 ; i < sizeof(Text)/sizeof(Text[0]) ; i++){
        int strLength = Text[i].length();
        for (int j = 0 ; j < strLength ; j++) {
            lc.setChar(0,7-j,Text[i][j],false);
        }
        delay(delaytime);
        lc.clearMatrix();
        delay(delaytime);
    }
    
    /* ++++++++++++++  Let's MAX7219 7-segments always shows "Connects" ++++++++++++++ */
    if (cmd == "Connects Internet") {
        String DisplayText = "Connects";
        for (int i = 0 ; i < DisplayText.length() ; i++) lc.setChar(0,7-i,DisplayText[i],false);
    }
}

void showLoadingBar(){
    String LoadingText = "Loading";
    for (int i = 0 ; i < LoadingText.length() ; i++ ) lc.setChar(0,7-i,LoadingText[i],false);
    for (int i = 0 ; i < 8 ; i++) {
        tm.displayASCII(i,'=');   
        delay(delaytime*2);
    }
    lc.clearMatrix();  tm.reset();
}

void sendSensorValues(void* param){
    while (1){
        /* ++++++++++++++++++ Sending data value read from DHT-11 & BMP-280 to google spreadsheet ++++++++++++++++++ */
        float humidity = dht.getHumidity(); 
        float dht_temp = dht.getTemperature();
        float bmp280_temp = bmp.readTemperature();
        float pressure = bmp.readPressure();
        float altitude = bmp.readAltitude(1013.25);
        
        String serverName1 = "http://maker.ifttt.com/trigger/" + String(WebHooksEventName1) + "/with/key/" + String(WebHooksKey); 
        String httpRequestData1 =  "value1=" + String(dht_temp) + "&value2=" + String(humidity);

        String serverName2 = "http://maker.ifttt.com/trigger/" + String(WebHooksEventName2) + "/with/key/" + String(WebHooksKey); 
        String httpRequestData2 =  "value1=" + String(bmp280_temp) + "&value2=" + String(pressure) + "&value3=" + String(altitude);

        Serial.println(F("Start sending datas\n"));
        if (WiFi.status() == WL_CONNECTED) { 
            HTTPClient http1 , http2; 
            http1.begin(serverName1); 
            http2.begin(serverName2); 
            http1.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
            http2.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
            int httpResponseCode1 = http1.POST(httpRequestData1); 
            int httpResponseCode2 = http2.POST(httpRequestData2); 
            Serial.print(F("HTTP Response code: ")); 
            Serial.printf("%d\n" , httpResponseCode1); 
            http1.end(); http2.end(); 
            Serial.println(F("============================================"));
            if (httpResponseCode1 == 200 && httpResponseCode2 == 200) {
                Serial.println(F(" --> Successfully sent"));
                Serial.println(F("+++++++ Datas send +++++++")); 
                Serial.printf("DHT-11 Temperature : %.2f C\n", dht_temp);
                Serial.printf("DHT-11 Humidity : %.2f %%\n", humidity);
                Serial.printf("BMP-280 Temperature : %.2f C\n", bmp280_temp);
                Serial.printf("BMP-280 Pressure : %.2f Pa\n", pressure);
                Serial.printf("BMP-280 Altitude : %.2f M\n", altitude);
                Display("Success to send datas");
            }
            else {
                Serial.println(F(" --> Failed!")); 
                Display("Failed to send datas");
            }
            Serial.println(F("============================================\n"));
        } 
        else { Serial.println(F("WiFi Disconnected")); }
        Display("Send data");              
        String WaitTime;
        lc.setChar(0,7,'I',false);  lc.setChar(0,6,'n',false); 
        for (int i = 60; i >= 0; i--) { 
            WaitTime = String(i);
            if (i >= 10) {
                lc.setChar(0,1,WaitTime[0],false);
                lc.setChar(0,0,WaitTime[1],false);
            }
            else {
                lc.setChar(0,1,'0',false);
                lc.setChar(0,0,WaitTime[0],false);
            }
            delay(1000); 
        }
        lc.clearMatrix();
        delay(50);
        Display("Sending datas to Sheet");
        delay(50);
    }
} 

void WifiSetup(){
    delay(50);
    String DisplayText = "Connects";
    int ConnectTime = 0;                              /* WiFi connection time (in millisecond)  */
    int tm_index = 0;                                 /* Index of TM1638's 7-segments, initial value is 0.  */
    Serial.printf("Connecting to %s ", ssid);         /* Serial monitor shows "Connecting to [Wifi name]".  */
    WiFi.begin(ssid, password);                       /* Start WiFi connection.  */
    for (int i = 0 ; i < DisplayText.length() ; i++) lc.setChar(0,7-i,DisplayText[i],false);

    /* ++++++++++++++++++ While loop for waiting device completely connect to WiFi ++++++++++++++++++ 
      o This procedure is about WiFi Connection. When the connection is done, while loop will break.  */
    while (WiFi.status() != WL_CONNECTED) {
        tm.displayASCII(tm_index , '.');              /* Displays dot('.') on TM1638      */
        delay(500);                                   /* Delay for 5 second               */
        Serial.print(".");                            /* Print dot('.') on Serial monitor */
        ConnectTime+=500;                             /* Increses ConnectTime by 500 (milliseconds)   */
        tm_index++;                                   /* Increases index of TM1638's 7-segments.      */

        /* ============== If Connection time is too long(35 second) ============== */
        if (ConnectTime == 35000) {
            tm.reset();                   
            tm_index = 0;                             /* Reset index of TM1638's 7-segments */
            Display("Failed to Connect Internet");    
            ConnectTime = 0;                          /* Reset Connection time  */
            delay(50);
            Serial.printf("\nConnecting to %s ", ssid);   
            for (int i = 0 ; i < DisplayText.length() ; i++) lc.setChar(0,7-i,DisplayText[i],false);
        }

        /* Reset index of TM1638's 7-segments if its index is more than 7. */
        if (tm_index > 7) {
            tm.reset();
            tm_index = 0;
        }
    }
    tm.reset();
    Serial.println(F(" CONNECTED"));
    Display("Success to Connect Internet");
}

void setupBMP280(){
    unsigned status;
    status = bmp.begin(0x76);
    if (!status) {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                         "try a different address!"));
        Serial.print(F("SensorID was: 0x")); Serial.println(bmp.sensorID(),16);
        Serial.print(F("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n"));
        Serial.print(F("   ID of 0x56-0x58 represents a BMP 280,\n"));
        Serial.print(F("        ID of 0x60 represents a BME 280.\n"));
        Serial.print(F("        ID of 0x61 represents a BME 680.\n"));
    }
    else {
        BMP280_status = 1;
        /* Default settings from datasheet. */
        Serial.println(F("BMP280 test"));
        bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                        Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                        Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                        Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                        Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
    }
}

void showLoadBMP280(){
    Display("Start to load BP280");     
    while(1){
        showLoadingBar();             /* Display Loading bar  */

        /* If BMP-280 sensor is found, break from while loop.  */
        if (BMP280_status) { 
            Display("Found Valid BP280 sensor");
            BMP280_status = 0;
            break; 
        }

        /* If BMP280 sensor is not found, let device try searching for BMP-280 again until it was found.  */
        else { 
            Display("Could not find valid BP280 sensor");
            setupBMP280(); 
        }
    }
}

void showAskForConnectWiFi(){
    Display("Do you try to connect Internet");            /* Device will ask user "Do you try to connect Internet".   */
    String YesOrNoText = "Yes   No";                      /* String used for display "Yes" and "No" on MAX7219.       */
    String BtnText = "Btn1Btn8";                          /* String used for display "Btn1" and "Btn8" on TM1638.     */
    for (int i = 0 ; i < 8 ; i++) {
        lc.setChar(0,7-i,YesOrNoText[i],false);           /* display "Yes" and "No" on MAX7219.     */
        tm.displayASCII(i,BtnText[i]);                    /* display "Btn1" and "Btn8" on TM1638.   */
    }
    while (1){
        int Btn = tm.readButtons();

        /* If button byte value is 0x01(pressing button 1), Let device connects WiFi.  */
        if (Btn == 0x01) {
            tm.reset();
            Display("Start to Connects Internet");        
            WifiSetup();                                  
            break;
        }

        /* If button byte value is 0x80(pressing button 8), Let device ignore WiFi connection.  */
        else if (Btn == 0x80) {
            tm.reset();
            Display("Do not Connects Internet");
            break;
        }
        delay(50);
    }
}

void setup() {
    Serial.begin(115200);                   
    setupBMP280();                              /* Initializes BMP280 I2C Scanner.    */
    dht.setup(DHT_pin, DHTesp::DHT11);          /* Set up DHT-11 Module.              */
    tm.displayBegin();                          /* Initializes TM1638.                */
    tm.brightness(0x04);                        /* Set TM1638 brightness.             */
    lc = LedController(DIN, CLK, CS, 1);        /* Initializes MAX7219 7-segments.    */
    lc.activateAllSegments();                   /* Activates MAX7219 7-segments       */
    lc.setIntensity(8);                         /* Set MAX7219 brightness   */
    lc.clearMatrix();                           /* Clear MAX7219 display    */
    showLoadBMP280();                           /* Display BMP280 Status.   */      
    showAskForConnectWiFi();                    /* Let's device asks user for WiFi connection  */
    Display("This Device Is ready");            /* Say " This device is ready" when everything is ready.  */
}

void loop(){
    int Btn = tm.readButtons();                 /* Read byte value from TM1638's buttons    */
    int dot_index;                              /* Int variable for storing index of dot('.') in String   */
    int DisplayStatus = 0 , dataValueStatus = 0;    
    String dataValue;                         /* String for storing data value read from DHT-11 or BMP-280    */
    String DeviceText = "Device  " , IsIdle = "Is Idle ";

    /* If device is not doing procedure, let device display "Device is idle" */
    for (int i = 0 ; i < 8 ; i++){
        lc.setChar(0,7-i,DeviceText[i],false);
        tm.displayASCII(i,IsIdle[i]);    
    }

    /* If press button, clear 7-segments */
    if (Btn != 0x00) {
        lc.clearMatrix();             
        tm.reset(); 
    }
    
    /* ++++++++++++++ If press button 1-5 ++++++++++++++ */
    if (Btn == 0x01 || Btn == 0x02 || Btn == 0x04 || Btn == 0x08 || Btn == 0x10){
        DisplayStatus = 1;   
        if (Btn == 0x01) {      dataValueStatus = 1;  }
        else if (Btn == 0x02) { dataValueStatus = 2;  }
        else if (Btn == 0x04) { dataValueStatus = 3;  }
        else if (Btn == 0x08) { dataValueStatus = 4;  }
        else if (Btn == 0x10) { dataValueStatus = 5;  }

        /* Calls sayDisplayValue function to let MAX7219 show what data value will be displayed */
        sayDisplayValue(DisplayStatus ,dataValueStatus);    
        delay(50);
    }

    /* ++++++++++++++ Button for WiFi Connection ++++++++++++++ */
    else if (Btn == 0x20) {

        /* If WiFi is not connected, let device connect WiFi.   */
        if (WiFi.status() != WL_CONNECTED) {
            Display("Connects Internet");
            WifiSetup();
        }

        /* If WiFi is already connected, device will display "Internet is already Connect".   */
        else {
            Display("Internet is already Connect");
        }
    }

    /* ++++++++++++++ Button for sending data values to google sheet ++++++++++++++*/
    else if (Btn == 0x80) {
        Display("Sending datas to Sheet");              /* Device says "Sending datas to Sheet".  */

        /* Create Task for Sending Sensor values to google spreadsheet */
        xTaskCreatePinnedToCore(sendSensorValues, 
                                "SendSensorValuesTask", 
                                4096 , NULL , 
                                tskIDLE_PRIORITY , 
                                &SendSensorValuesTask, 0);
        vTaskResume(SendSensorValuesTask);              /* Let task start running its statements. */
        while(1){
            Btn = tm.readButtons();
            
            /* If press button 8 again, device will stop sending datas by deleting the task. */
            if (Btn == 0x80) {
                vTaskDelete(SendSensorValuesTask);      /* Delete Task  */
                Display("Stop sending datas");          /* Device says "Stop sending datas" and then exit while loop  */
                break;                                 
            }
        }
    }

    /* ++++++++++++++ While loop for display data value on TM1638's 7-segments ++++++++++++++ 
      o if dataValueStatus != 0, device will start displaying data value read from DHT-11 or BMP-280
        +-------------------------------------------------------------+
        |  - dataValueStatus = 1 : Display DHT-11 temperature value   |
        |  - dataValueStatus = 2 : Display DHT-11 Humid value         |
        |  - dataValueStatus = 3 : Display BMP-280 Pressure value     |
        |  - dataValueStatus = 4 : Display BMP-280 Altitude value     |
        |  - dataValueStatus = 5 : Display BMP-280 temperature value  |
        +-------------------------------------------------------------+
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    while (dataValueStatus != 0){
        Btn = tm.readButtons();
        if (dataValueStatus == 1 || dataValueStatus == 5) {
            if (dataValueStatus == 1) {
                dataValue = String(dht.getTemperature());
                //Serial.printf("DHT11-Temp: %s C\n",dataValue);
            }
            else {
                dataValue = String(bmp.readTemperature());
                //Serial.printf("BMP-280 Temp: %s C\n",dataValue);
            }
            tm.displayASCII(dataValue.length(), 'C');
        }

        else if (dataValueStatus == 2){
            dataValue = String(dht.getHumidity());   
            //Serial.printf("Humidity: %s\n",dataValue);
            tm.displayASCII(dataValue.length(), 'H');
        }
            
        else if (dataValueStatus == 3){
            dataValue = String(bmp.readPressure());     
            //Serial.printf("Pressure: %s Pa\n",dataValue);
        }

        else if (dataValueStatus == 4){
            dataValue = String(bmp.readAltitude(1013.25));   
            //Serial.printf("Altitude: %s m\n",dataValue);
        }

        /* ============ For loop for find index of dot('.') character in String of data value ============ */
        for (int i = 0 ; i < dataValue.length() ; i++) {
            if (dataValue[i] == '.') {  
                dot_index = i; 
                break;
            }
        }

        /* ============ For loop for display data value on TM1638's 7-segments ============ */
        for (int i = 0 ; i < dataValue.length() ; i++) {
            if (dataValue[i+1] == '.')  tm.displayASCIIwDot(i, dataValue[i]);
            else if (i < dot_index)     tm.displayASCII(i, dataValue[i]);
            else if (i > dot_index)     tm.displayASCII(i-1, dataValue[i]); 
        }

        /* ============ If press button 1-5 again, device will stop displaying data value ============ */
        if ((Btn == 0x01 && dataValueStatus == 1) || (Btn == 0x02 && dataValueStatus == 2) || 
            (Btn == 0x04 && dataValueStatus == 3) || (Btn == 0x08 && dataValueStatus == 4) || (Btn == 0x10 && dataValueStatus == 5) ) {
            lc.clearMatrix();
            tm.reset();
            DisplayStatus = 0;
            sayDisplayValue(DisplayStatus ,dataValueStatus);
            break;
        }
        delay(50);
    }
    delay(50);
}