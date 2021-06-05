#include <Grove_LCD_RGB_Backlight.h>
#include <HttpClient.h>

#define BUZZER D2
#define BUTTON D7
#define SOUND_SENSOR A0
#define QUIET_HOURS 1
#define NORMAL_HOURS 0
#define THRESHOLD 2500
#define QUIET_THRESHOLD 1500
#define BAR_COUNT 10    // the number of bars in the bar graph


HttpClient http;
// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers with NULL
};

http_request_t request;
http_response_t response;

rgb_lcd lcd;

bool buzzer_on = false;
int device_mode;

void setup() {
    Serial.begin(9600);
    
    // Configure pins
    pinMode(BUZZER, OUTPUT);
    pinMode(SOUND_SENSOR, INPUT);
    pinMode(BUTTON, INPUT);
    
    device_mode = NORMAL_HOURS;
    lcd.begin(16,2);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Mode: Normal");
    lcd.setCursor(0,1);
    lcd.print("Lvl:");
    delay(500);
}

void loop() {
    if (digitalRead(BUTTON) == HIGH) {
        if (device_mode==NORMAL_HOURS) {
            device_mode = QUIET_HOURS;
            lcd.setCursor(0,0);
            lcd.print("Mode: Quiet ");
        } else {
            device_mode = NORMAL_HOURS;
            lcd.setCursor(0,0);
            lcd.print("Mode: Normal");
        }
    }
    
    long sum = 0;
    for(int i=0; i<32; i++)
    {
        sum += analogRead(SOUND_SENSOR);
    }
 
    sum >>= 5;  // Gets the average input reading
    
    // map the result to a range from 0 to the number of LEDs:
    int barLevel = map(sum, 0, 4095, 0, BAR_COUNT);
    
    char buf[BAR_COUNT+2];
    for (int bar = 0; bar < BAR_COUNT; bar++) {
        if (bar < barLevel) {
            buf[bar] = char(255);   
        } else {
            buf[bar] = ' ';
        }
    }
    buf[BAR_COUNT] = char(37);
    buf[BAR_COUNT+1] = 0;
    lcd.setCursor(4,1);
    lcd.print(buf);
    
    String exceededThreshold;
    if (device_mode == NORMAL_HOURS) {
        exceededThreshold = sum > THRESHOLD ? "Y" : "N";
        if (sum > THRESHOLD && !buzzer_on) {
            buzzer_on = true;
            digitalWrite(BUZZER, HIGH);
        } else if (sum <= THRESHOLD && buzzer_on) {
            buzzer_on = false;
            digitalWrite(BUZZER, LOW);
    }
    } else {
        exceededThreshold = sum > QUIET_THRESHOLD ? "Y" : "N";
        if (sum > QUIET_THRESHOLD && !buzzer_on) {
            buzzer_on = true;
            digitalWrite(BUZZER, HIGH);
        } else if (sum <= QUIET_THRESHOLD && buzzer_on) {
            buzzer_on = false;
            digitalWrite(BUZZER, LOW);
        }
    }
    
    request.hostname = "52.91.173.107";
    request.port = 5000;
    request.path = "/sendData?noiseLevel="+String(sum)+"&exceededThreshold="+exceededThreshold;
    
    // Get request
    http.get(request, response, headers);
    Serial.print("Application>\tResponse status: ");
    Serial.println(response.status);
    
    Serial.print("Application>\tHTTP Response Body: ");
    Serial.println(response.body);
 
    Serial.println(sum);
    delay(1000);
}