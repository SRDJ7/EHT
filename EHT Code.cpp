#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ThingSpeak.h"  //official ThingSpeak library
#include <ESP8266WiFi.h>  // ESP8266 WiFi MODULE LIBRARY
#include "MAX30100_PulseOximeter.h"
#include <DallasTemperature.h>
#include <DHT.h> // DHT library
#include <OneWire.h>

#define DHTPIN 14 //pin where the dht11 is connected          

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SECRET_SSID "MySSID"    // replace MySSID with your WiFi network name
#define SECRET_PASS "MyPassword"  // replace MyPassword with your WiFi password

#define SECRET_CH_ID   1222177    // replace 0000000 with your ThingSpeak channel number
#define SECRET_WRITE_APIKEY "LI5TY0JPA8E95RG2"   // replace XYZ with your channel write API Key

#define LOGO_HEIGHT   16                   //Heart Logo Height
#define LOGO_WIDTH    16                  //Heart Logo Width
#define REPORTING_PERIOD_MS 3500
PulseOximeter pox;

DHT dht(DHTPIN, DHT11); // Initialize our values

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int i = 0;
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;
long int entry = 0;
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
String myStatus = "";

// GPIO where the DS18B20 is connected to
const int oneWireBus = 13; 

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
uint32_t tsLastReport = 0;
 
 //This is an bitmap array for the heart logo/animation
const unsigned char bitmap [] PROGMEM=
{
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x0f, 0xe0, 0x7f, 0x00, 0x3f, 0xf9, 0xff, 0xc0,
0x7f, 0xf9, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xf7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0x7f, 0xdb, 0xff, 0xe0,
0x7f, 0x9b, 0xff, 0xe0, 0x00, 0x3b, 0xc0, 0x00, 0x3f, 0xf9, 0x9f, 0xc0, 0x3f, 0xfd, 0xbf, 0xc0,
0x1f, 0xfd, 0xbf, 0x80, 0x0f, 0xfd, 0x7f, 0x00, 0x07, 0xfe, 0x7e, 0x00, 0x03, 0xfe, 0xfc, 0x00,
0x01, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00,
0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void onBeatDetected()
{
  Serial.println("Beat!");
  display.drawBitmap( 50, 20, bitmap, 28, 28, 1);
  display.display();
}


void tspk(float heartrate, float spo2,float tempf,float atemp,float hum, int i){

//Code to get temperature readings from DS18B20 Probe (Sensor)
  sensors.requestTemperatures(); 
//Reading the values of temperature and humidity from DHT sensor 
  //float hum = dht.readHumidity();  
  //float atemp = dht.readTemperature();

  if (isnan(hum) || isnan(atemp))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
//isnan -> is not a number, basically elements such as the solution of 0/0.
//Here h and t variables will carry the values of humidity and temperature respectively.
//If values of humidity or temperature is not real then “Failed to read from DHT sensor!” is present at output. 

  
  // The following code is for updating in Thingspeak
ThingSpeak.setField(1,heartrate);
ThingSpeak.setField(2, tempf);
ThingSpeak.setField(3, spo2);
ThingSpeak.setField(4, atemp);
ThingSpeak.setField(5, hum);
  Serial.print("Heart Rate: ");
  Serial.print(heartrate);

  Serial.print(" SPO2:");
  Serial.print(spo2);
  Serial.print(" Body Temperature ");
  Serial.println(tempf);
  Serial.println("Ambient Temp:"+String((atemp * 9.0) / 5.0 + 32.0)+"F Humidity:"+String(hum));
  Serial.println("%. Send to Thingspeak.");

  // figure out the status message

  myStatus = String("Heart Rate, Approximate Body Temperature, SpO2, Ambient Temperature and Humidity Levels are being displayed in 20 second intervals  ");
  // set the status which will be visible on the ThingSpeak website interface.
  ThingSpeak.setStatus(myStatus);


  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
    entry += 1;
    Serial.print("Entry No:");
    Serial.println(entry);


  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

delay(2000);
    
  }

void setup() {
  Serial.begin(115200);
// Start the DS18B20 sensor
  sensors.begin();
  dht.begin();
  
WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak


//setup() function is called when a sketch starts. Use it to initialize variables, pin modes etc. It will only run once after each powerup or reset of the Arduino.
//serial.begin(speed) sets the data rate in bits per second (baud) for serial data transmission like communicating with serial monitor.
//delay(xx) replace xx by a number, it pauses the program for the amount of time in milliseconds.
//WiFi.mode(WIFI_STA)  devices that connect to Wi-Fi networks are called stations (STA). This mode is used to get the ESP module connected to a WiFI network established by an access point.


  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

 // initialize with the I2C addr 0x3C
 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  

  // Clear the buffer.
  display.clearDisplay();
  
display.setTextColor(WHITE);
display.setCursor(0,0);
display.setTextSize(2);
display.println("");
display.println("E-Health \n  Tracker!");
display.println("");
display.display();
display.startscrollright(0x00, 0x07);
delay(2000);
display.stopscroll();
delay(1000);
display.startscrollleft(0x00, 0x07);
delay(2000);
display.stopscroll();
delay(1000);    
display.startscrolldiagright(0x00, 0x07);
delay(2000);
display.startscrolldiagleft(0x00, 0x07);
delay(2000);
display.stopscroll();
// Initialzing sensors message

display.clearDisplay();
display.setTextSize(1);
display.setTextColor(1);
display.setCursor(0, 0);
 
  display.println("Initializing Sensors...");
  delay(2000);
   display.display();
  Serial.print("Initializing Sensors...");
 
  if (!pox.begin()) {
  Serial.println("FAILED(P)");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);
  display.println("FAILED(P)");
  display.display();
  for(;;);
  } else {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);
  display.println("SUCCESS (No WiFi)");
  display.display();
  Serial.println("SUCCESS");
  }
  pox.setOnBeatDetectedCallback(onBeatDetected); 

}

void loop() {
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }
// loop() is a function in which the code inside the curly bracket runs over and over as long as the maker board is on.
//WiFi.Status() returns the connection status,  WL_CONNECTED is assigned when connected to a Wi-Fi network.
//Serial.print prints data to the serial porta as human readable ASCII text.
//WIFI.begin(SSID,PASS) initializes the WiFi library’s network settings and provides the current status.
//[Here ESP module connects to wifi network.] 
  
 
// Clear the buffer.
  display.clearDisplay();
  
  float temperatureF = 0;
  //float temperatureC = sensors.getTempCByIndex(0);
   temperatureF = sensors.getTempFByIndex(0)+1.5; 
   //Reading the values of temperature and humidity from DHT sensor 
  float hum = dht.readHumidity();  
  float atemp = dht.readTemperature();
  atemp = (atemp * 9.0) / 5.0 + 32.0;

// The following code is to get the sensor readings as BPM and SPO2, displayed on the OLED screen
pox.update();
int heartrate=int(pox.getHeartRate());
float  spo2=pox.getSpO2();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,0);
  display.print("Heart BPM | A.T");
  
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 16);
  display.println(String(heartrate)+"        |"+String(atemp)+"F");

  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 30);
  display.println("SpO2% | B.T");
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,45);
 display.print(String(spo2)+"%");
 display.print(" |"+String(temperatureF)+"F");
  display.display();
  pox.shutdown();
tspk(heartrate,spo2,temperatureF,atemp,hum,i);
pox.resume();
tsLastReport = millis();
}

i=i+1;
//Serial.println("i="+String(i));
}
  