#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define LED_PIN 0
#define NUM_LEDS 11
#define BAUD_RATE 115200

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char *ssid = "TP-Link_B36D";
const char *password = "73039440";
const char *moonraker_url = "http://192.168.0.5:4409/printer/objects/query?display_status";

int spinnerIndex = 0;
unsigned long lastAnimTime = 0;
unsigned long wifiStartTime = 0;
const unsigned long wifiTimeout = 150000; // 15 seconds

void showSpinner(uint32_t colour = strip.Color(0, 0, 150))
{
  strip.clear();
  strip.setPixelColor(spinnerIndex, colour);
  strip.show();

  spinnerIndex = (spinnerIndex + 1) % NUM_LEDS;
  delay(100);
}

void showError(uint32_t colour = strip.Color(255, 0, 0))
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    strip.setPixelColor(i, colour);
  }
  strip.show();
}

void test()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(moonraker_url);

    int httpCode = http.GET();
    String payload = http.getString();

    int progressIndex = payload.indexOf("\"progress\":");

    if (progressIndex != -1)
    {
      float progress = payload.substring(progressIndex + 11).toFloat();
      String progressString = String(progress * 100);

      Serial.println("Progress: " + progressString + "%");

      int lit_leds = (int)(progress * NUM_LEDS);
      for (int i = 0; i < NUM_LEDS; i++)
      {
        if (i < lit_leds)
          strip.setPixelColor(i, strip.Color(0, 150, 0)); // Green
        else
          strip.setPixelColor(i, 0); // Off
          strip.show();
      }
    }
  }

  delay(5000); // Check every 5s
}

void setup()
{
  Serial.begin(BAUD_RATE); // Connect printer's USB serial to ESP32 RX
  strip.begin();
  strip.show();
  Serial.println("ESP32 Progress Monitor Started");

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  wifiStartTime = millis();

  // Spinner while waiting for Wi-Fi
  while (WiFi.status() != WL_CONNECTED)
  {
    showSpinner();
    Serial.print(".");

    // Timeout after 15s
    if (millis() - wifiStartTime > wifiTimeout)
    {
      Serial.println("\nWiFi Connection Timeout!");
      showError(); // Red
      while (true)
        ; // Stop everything
    }
  }

  Serial.println("\nWiFi connected!");
}

void usbserial()
{
  if (Serial.available())
  {
    String line = Serial.readStringUntil('\n');
    line.trim();

    // M73 P50
    if (line.startsWith("M73"))
    {
      int pIndex = line.indexOf('P');
      if (pIndex != -1)
      {
        int percent = line.substring(pIndex + 1).toInt();
        Serial.print("Progress: ");
        Serial.println(percent);

        int leds = map(percent, 0, 100, 0, NUM_LEDS);
        for (int i = 0; i < NUM_LEDS; i++)
        {
          if (i < leds)
            strip.setPixelColor(i, strip.Color(0, 150, 0)); // Green
          else
            strip.setPixelColor(i, 0);
        }
        strip.show();
      }
    }
  }
}

void wifiserial()
{
  //   if (WiFi.status() == WL_CONNECTED)
  //   {
  //     HTTPClient http;
  //     http.begin(moonraker_url);
  //     int httpCode = http.GET();

  //     if (httpCode == 200)
  //     {
  //       String payload = http.getString();

  //       int progressIndex = payload.indexOf("\"progress\":");
  //       int stateIndex = payload.indexOf("\"state\":\"");

  //       if (progressIndex != -1 && stateIndex != -1)
  //       {
  //         float progress = payload.substring(progressIndex + 11).toFloat();
  //         String state = payload.substring(stateIndex + 9, payload.indexOf("\"", stateIndex + 9));

  //         Serial.printf("State: %s, Progress: %.2f\n", state.c_str(), progress);

  //         if (state == "printing")
  //         {
  //           int lit_leds = (int)(progress * NUM_LEDS);
  //           for (int i = 0; i < NUM_LEDS; i++)
  //           {
  //             if (i < lit_leds)
  //               strip.setPixelColor(i, strip.Color(0, 150, 0)); // Green
  //             else
  //               strip.setPixelColor(i, 0); // Off
  //           }
  //         }
  //         else if (state == "complete")
  //         {
  //           // Blue flash for done
  //           for (int i = 0; i < NUM_LEDS; i++)
  //           {
  //             strip.setPixelColor(i, strip.Color(0, 0, 200));
  //           }
  //         }
  //         else
  //         {
  //           // Idle state
  //           for (int i = 0; i < NUM_LEDS; i++)
  //           {
  //             strip.setPixelColor(i, strip.Color(0, 0, 30)); // Dim blue
  //           }
  //         }
  //         strip.show();
  //       }
  //     }
  //     else
  //     {
  //       Serial.println("Moonraker API error");
  //       // Flash yellow
  //       for (int i = 0; i < NUM_LEDS; i++)
  //         strip.setPixelColor(i, strip.Color(200, 200, 0));
  //       strip.show();
  //       delay(300);
  //       strip.clear();
  //       strip.show();
  //       delay(300);
  //     }

  //     http.end();
  //   }
  //   else
  //   {
  //     Serial.println("Lost Wi-Fi");
  //     showError(); // Red
  //     while (true)
  //       ; // Halt
  //   }

  //   delay(5000); // Check every 5s
}

void loop()
{
  // usbserial();

  // wifiserial();

  test();
}
