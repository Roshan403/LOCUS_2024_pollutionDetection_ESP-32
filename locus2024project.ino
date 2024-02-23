            #include <DHT.h>
            #include <DHT_U.h>
            #include <WiFi.h>
            #include<math.h>
            #include <cstdlib>
            #include<string.h>


            using namespace std;
            // pin_2 to wake it up
              RTC_DATA_ATTR int count=0;
              RTC_DATA_ATTR int pin_1=12;
              RTC_DATA_ATTR int pin_2=13;
              RTC_DATA_ATTR bool Flag= false;
              RTC_DATA_ATTR uint8_t touchValue_1;
              RTC_DATA_ATTR uint8_t touchValue_2;
              RTC_DATA_ATTR int touchThreshold_1=30;
              RTC_DATA_ATTR int touchThreshold_2=20;
              RTC_DATA_ATTR void esp_sleep_and_wake();
              #define GREEN 18
              #define RED 19
              #define YELLOW 21


              const String apiKey = "H9OVDFEQM7U3V39H";     //  Enter your Write API key from ThingSpeak
              
              const char *ssid =  "Bibhav's Galaxy M32";     // replace with your wifi ssid and wpa2 key
              const char *pass =  "jvke4348";
              const char* server = "api.thingspeak.com";
              String error;
              float h,t,pm;
              void checkAndReconnectTOWifi();
              void sensorDataFetching(float h,float t,float pm);
              void print_values_and_led(float humidity,float temperature,float pm);

              
              unsigned long previousMillis = 0;
              unsigned long interval = 60000;
              unsigned long currentMillis=0;

            // DHT portions
            #define DHTPIN 5       // what pin we're connected to
            #define DHTTYPE DHT22   // DHT 22  (AM2302)
            DHT dht(DHTPIN, DHTTYPE);

            WiFiClient client;
          
              class writeToThingSpeak{
                private:
                float h,t,pm;
                public:
                writeToThingSpeak():h(0.0),t(0.0),pm(0.0){}
                writeToThingSpeak(float humidity,float temperature,float p_m_index):h(humidity),t(temperature),pm(p_m_index){}
                  void sendData()
                  {
                    if (client.connect(server,80))   //   "184.106.153.149" or api.thingspeak.com
                                      {     
                                            String postStr = apiKey;
                                            postStr +="&field1=";
                                            postStr += String(t);
                                            postStr +="&field2=";
                                            postStr += String(h);
                                            postStr +="&field3=";
                                            postStr += String(pm);
                                            postStr += "\r\n\r\n";
                
                                            client.print("POST /update HTTP/1.1\n");
                                            client.print("Host: api.thingspeak.com\n");
                                            client.print("Connection: close\n");
                                            client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                                            client.print("Content-Type: application/x-www-form-urlencoded\n");
                                            client.print("Content-Length: ");
                                            client.print(postStr.length());
                                            client.print("\n\n");
                                            client.print(postStr);
                
                                            Serial.print("Temperature: ");
                                            Serial.println(t);
                                            Serial.print("Humidity: ");
                                            Serial.println(h);
                                            Serial.print("P.M 2.5: ");
                                            Serial.println(pm);
                                            Serial.println("Data Sending To Thingspeak......");
                                            delay(1000);
                                          }
                  }
                    void stopClient()
                            {
                              client.stop();
                            }
                };


              class Exception{
                private:
                String error;
                public:
                  Exception(String err)
                {
                  error=err;
                }
                void what()
                {
                  Serial.println(error);
                }
              };


              void setup() 
              {

              Serial.begin(115200);
              delay(10);
              Serial.println("Connecting to ");
              Serial.println(ssid);
              WiFi.begin(ssid, pass);
              while (WiFi.status() != WL_CONNECTED) 
              {
                  delay(500);
                  Serial.print(".");
              }
              Serial.println("");
              Serial.println("WiFi connected");
              pinMode(GREEN, OUTPUT);
              pinMode(RED, OUTPUT);
              pinMode(YELLOW, OUTPUT);
              dht.begin();
              //Setup sleep wakeup on Touch Pad 13 (GPI13) 
              touchSleepWakeUpEnable(pin_2,touchThreshold_2);
              }
              
              void loop() 
              {
                  touchValue_1=touchRead(pin_1);
                  touchValue_2=touchRead(pin_2);
                  Serial.print("Sleep touch=");
                  Serial.println(touchValue_1);
                  Serial.print("Waking up touch=");
                  Serial.println(touchValue_2);
                  delay(100);
                  esp_sleep_and_wake();
                  currentMillis = millis();
                // if WiFi is down, try reconnecting
                  checkAndReconnectTOWifi();
            // Getting data from the sensor
          sensorDataFetching();
          print_values_and_led(h,t,pm);

  
    try {
      if ((sizeof(h) == sizeof(float)) && (sizeof(t) == sizeof(float)) && (sizeof(pm) == sizeof(float)) &&
        !isnan(h) && !isnan(t) && !isnan(pm)) {
            class writeToThingSpeak wtts(h, t, pm);
            wtts.sendData();
            wtts.stopClient();
            Serial.println("Waiting...");
            // thingspeak needs minimum 15 sec delay between updates
            delay(1000 * 15);
            Serial.println("Successfully Data Sent!!!");
        } else {
            error="One or more values are NaN or not of float type. Restarting loop...";
            throw Exception(error);// Exception class forms a nameless temporary object and work as an argument to the catch.
        }
    } catch (Exception &e) {
        e.what();
        Serial.print("Humidity:");
        Serial.println(h);
        Serial.print("Temperature:");
        Serial.println(t);
        Serial.print("PM:");
        Serial.println(pm);
        delay(1000);
        return;
    }
              }


              void checkAndReconnectTOWifi()
              {
                if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
                Serial.print(millis());
                Serial.println("Reconnecting to WiFi...");
                WiFi.disconnect();
                WiFi.reconnect();
                previousMillis = currentMillis;
              }
              return;
              }

              RTC_DATA_ATTR void esp_sleep_and_wake()
              {
                if(touchValue_1<touchThreshold_1 && Flag==false)
                {
                  Serial.println("Taking into deep Sleep");
                  esp_deep_sleep_start();
                  Flag=true;
                }
                if(touchValue_2<touchThreshold_2 && Flag==true)
                {
                  Serial.println("Waking up from deep Sleep");
                  esp_sleep_enable_touchpad_wakeup();
                  Flag=false;
                }  
                return;
              }



      void print_values_and_led(float humidity,float temperature,float pm){

        if(pm > 1.0 && pm < 50.0) {
          digitalWrite(GREEN, HIGH);
          digitalWrite(YELLOW, LOW);
          digitalWrite(RED, LOW);
          Serial.print("Air quality: ");
          Serial.print(pm);
          Serial.print("Humidity: ");
          Serial.print(humidity);
          Serial.print(" %, Temp: ");
          Serial.print(temperature);
          Serial.println(" Celsius");
          delay(2000);
        }
        else if(pm > 50.0 && pm < 100.0){
          digitalWrite(YELLOW, HIGH);
          digitalWrite(GREEN, LOW);
          digitalWrite(RED, LOW);
          Serial.print("Air quality: ");
          Serial.print(pm);
          Serial.print("Humidity: ");
          Serial.print(humidity);
          Serial.print(" %, Temp: ");
          Serial.print(temperature);
          Serial.println(" Celsius");
          delay(2000);
        }
      else if(pm > 100.0){
          digitalWrite(RED, HIGH);
          digitalWrite(GREEN, LOW);
          digitalWrite(YELLOW, LOW);
          Serial.print("Air quality: ");
          Serial.print(pm);
          Serial.print("Humidity: ");
          Serial.print(humidity);
          Serial.print(" %, Temp: ");
          Serial.print(temperature);
          Serial.println(" Celsius");
          delay(2000);
      }
      else{
          digitalWrite(RED, LOW);
          digitalWrite(GREEN, LOW);
          digitalWrite(YELLOW, LOW);
          delay(2000);
      }
      }

// dummy formula to calculate pm index due to sudden issue in pm detecting device at the end in which pm dependent variable depends up on temperature and humudity.
              void sensorDataFetching()
                {
                  h = (float)dht.readHumidity();
                          t =(float)dht.readTemperature();
                          pm;
                          if(t>25)
                          {
                          pm =((3*t)+(h));
                          }
                          else{
                            pm=((t+h)-9);
                          }
                          return;
                }

