#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include "index.h"
#include <ESP32Servo.h>
#include <soc/gpio_struct.h>



// camera pins
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  9
#define SIOD_GPIO_NUM  12
#define SIOC_GPIO_NUM  13

#define Y9_GPIO_NUM    8
#define Y8_GPIO_NUM    18
#define Y7_GPIO_NUM    17
#define Y6_GPIO_NUM    15
#define Y5_GPIO_NUM    6
#define Y4_GPIO_NUM    4
#define Y3_GPIO_NUM    5
#define Y2_GPIO_NUM    7
#define VSYNC_GPIO_NUM 10
#define HREF_GPIO_NUM  11
#define PCLK_GPIO_NUM  16


//  other pins
#define LED_PIN 42
#define NUM_LEDS 9
#define SERVO_CTR 47
#define SERVO_PWR 48
#define BUZZER 14
#define KEEP_ALIVE 41

//  timers
#define SERVO_PWR_TIME 2000
#define KEEP_ALIVE_TIME 500
#define KEEP_ALIVE_DELAY 10000
#define CONNECTION_CHECK_DELAY 1000
unsigned long servo_pwr_timer = 0, keep_alive_timer = 0, keep_alive_delay_timer = KEEP_ALIVE_DELAY, connection_check_timer = 0;


// LEDs constants
const uint32_t color_err  = Adafruit_NeoPixel::Color(50, 0, 0);
const uint32_t color_ok   = Adafruit_NeoPixel::Color(0, 50, 0);
const uint32_t color_conn = Adafruit_NeoPixel::Color(0, 0, 50);


// access point constants
const char* ssid = "Guest36893";
const char* password = "6pSA#831&cpHdt";
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255, 255, 255, 0);


//  stream constants
const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                        "Access-Control-Allow-Origin: *\r\n" \
                        "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";           // marks end of each image frame
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";       // marks start of image data
const int hdrLen = strlen(HEADER);         // length of the stored text, used when sending to web page
const int bdrLen = strlen(BOUNDARY);
const int cntLen = strlen(CTNTTYPE);



//  globals
WebServer server(80);
Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);
Servo servo;
bool stream = false;
WiFiClient stream_client;




void critical_error(const String msg){
  Serial.println(msg);
  leds.setPixelColor(0, color_err);
  leds.show();
  while(1)
    delay(1000);
}



void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_VGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    critical_error("");
  }

  sensor_t* s = esp_camera_sensor_get();
  s->set_awb_gain(s, 1);

}



void toggle_stream(){
  stream_client = server.client();
  stream = true;
  stream_client.write(HEADER, hdrLen);
  stream_client.write(BOUNDARY, bdrLen);
  handle_stream();
}



void handle_stream() {
  if(!stream_client.connected()){
    stream = false;
    stream_client.stop();
  }
  char buf[32];
  camera_fb_t* fb = NULL;
  fb = esp_camera_fb_get();                   // capture live image as jpg
  stream_client.write(CTNTTYPE, cntLen);             // send content type html (i.e. jpg image)
  sprintf( buf, "%d\r\n\r\n", fb->len);       // format the image's size as html and put in to 'buf'
  stream_client.write(buf, strlen(buf));             // send result (image size)
  stream_client.write((char *)fb->buf, fb->len);     // send the image data
  stream_client.write(BOUNDARY, bdrLen);             // send html boundary      see https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Type
  esp_camera_fb_return(fb);                  // return camera frame buffer
}



void handle_index() {
  if(server.hasArg("light")){
    int value = server.arg("light").toInt();
    Serial.println(value);
    leds.fill(Adafruit_NeoPixel::Color(0, 0, 0, value), 1, 8);
    leds.show();
  }

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}



void handle_close(){
  digitalWrite(KEEP_ALIVE, LOW);
  digitalWrite(SERVO_PWR, HIGH);
  servo_pwr_timer = SERVO_PWR_TIME;
  delay(100);
  servo.write(145);
  server.send(200, "text/plain", "Pułapka pomyślnie zamknięta");
}



void handle_reset(){
  digitalWrite(KEEP_ALIVE, LOW);
  digitalWrite(SERVO_PWR, HIGH);
  servo_pwr_timer = SERVO_PWR_TIME;
  delay(100);
  servo.write(45);
  server.send(200, "text/plain", "Pułapka pomyślnie zresetowana");
}


void check_connection(){
  if(WiFi.AP.stationCount())
    leds.setPixelColor(0, color_conn);
  else
    leds.setPixelColor(0, color_ok);
  leds.show();
}

unsigned long subtract_time(unsigned long time1, unsigned long time2){
  if(time2 > time1)
    return 0;
  return time1 - time2;
}

void handle_timers(unsigned long time){
  //  checking connection
  connection_check_timer = subtract_time(connection_check_timer, time);
  if(!connection_check_timer){
    check_connection();
    connection_check_timer = CONNECTION_CHECK_DELAY;
    Serial.println("CONNECTION CHECK");
  }
  //  servo power
  servo_pwr_timer = subtract_time(servo_pwr_timer, time);
  if(!servo_pwr_timer){
    digitalWrite(SERVO_PWR, LOW);
    Serial.println("SERVO POWER OFF");
  }
  //  keep alive
  static bool flag = false;
  keep_alive_timer = subtract_time(keep_alive_timer, time);
  keep_alive_delay_timer = subtract_time(keep_alive_delay_timer, time);
  if(!keep_alive_delay_timer){
    if(!servo_pwr_timer){
      digitalWrite(KEEP_ALIVE, HIGH);
      Serial.println("KEEP ALIVE ON");
      flag = true;
    }

    keep_alive_timer = KEEP_ALIVE_TIME;
    keep_alive_delay_timer = KEEP_ALIVE_DELAY + KEEP_ALIVE_TIME;
  }
  if(!keep_alive_timer & flag){
    flag = false;
    digitalWrite(KEEP_ALIVE, LOW);
    Serial.println("KEEP ALIVE OFF");
  }
}



void setup() {
  Serial.begin(115200);
  delay(100);


  //  LEDs setup
  leds.begin();
  leds.clear();
  leds.show();


  startCamera();

  //  servo setup
  pinMode(SERVO_PWR, OUTPUT);
  digitalWrite(SERVO_PWR, LOW);
  servo.attach(SERVO_CTR);
  GPIO.func_out_sel_cfg[SERVO_CTR].inv_sel = 1;

  //  keep alive setup
  pinMode(KEEP_ALIVE, OUTPUT);
  digitalWrite(KEEP_ALIVE, LOW);


  WiFi.softAP(ssid, password, 1, 0, 1);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  server.on("/", handle_index);
  server.on("/stream", toggle_stream);
  server.on("/close", HTTP_GET, handle_close);
  server.on("/reset", HTTP_GET, handle_reset);

  server.begin();
  leds.setPixelColor(0, color_ok);
  leds.show();
}



void loop() {
  static unsigned long time_curr = millis(), time_prev = millis();
  time_prev = time_curr;
  time_curr = millis();
  handle_timers(time_curr - time_prev);
  server.handleClient();
  handle_stream();
  delay(5);
}
