#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

//Define the pin for the factory reset button. 
//This should be a pin that can trigger an interrupt (like GPIO3).
#define RESET_BUTTON_PIN 3 // GPIO3

// We use a volatile variable to indicate that the factory reset button was pressed. 
//This is set in the ISR and checked in the main loop.
volatile bool factoryResetRequested = false;

// This is our Interrupt Service Routine (ISR)
// It must be as fast as possible!
void IRAM_ATTR onFactoryReset() 
{
    factoryResetRequested = true;
}

void setup() 
{
    // Set up the reset button with an interrupt
    pinMode(RESET_BUTTON_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN), onFactoryReset, RISING);

    // Start serial communication
    Serial.begin(115200);
    Serial.println("--- Starting ESP32 ---");

    // Initialize WiFiManager
    // Local intialization. There is no need to keep it in memory after setup() has finished.
    WiFiManager wm;

    // Set a timeout so the ESP doesn't hang forever if nobody configures the WiFi
    wm.setConfigPortalTimeout(300); // Auto close portal after 5 minutes

    // Print debug info to the Serial monitor (helpful for troubleshooting)
    wm.setDebugOutput(true); 
    // -------------------------

    Serial.println("Starting WiFiManager...");
    Serial.println("If no valid WiFi is saved, it will create an Access Point.");

    // autoConnect() performs the magic.
    // It tries to connect to the last saved WiFi. 
    //    - WiFiManager stores the SSID/password as key/value pairs via NVS and reads them back on boot.
    // If it fails or no WiFi is saved, it sets up an Access Point.
    // AP Name: "ESP32_Config_AP", AP Password: "pizza4tree"
    bool res;
    res = wm.autoConnect("ESP32_Config_AP", "pizza4tree");

    // Check the result
    if(!res) 
    {
        Serial.println("Failed to connect or hit timeout.");
        Serial.println("Rebooting to try again...");
        delay(3000);
        ESP.restart();
    } 
    else 
    {
        // If you get here, you have successfully connected to the local router!
        Serial.println("");
        Serial.println("SUCCESS! Connected to WiFi.");
        Serial.print("Assigned IP Address: ");
        Serial.println(WiFi.localIP());
    }
}

void loop() 
{
    // Check if the factory reset button was pressed
    if (factoryResetRequested) 
    {
        // do the heavy lifting outside of interrupt context
        WiFiManager wm;
        wm.resetSettings();        // clears NVS
        delay(100);                // give NVS a moment to commit
        ESP.restart();             // now reboot
    }

    // Your main project code goes here.
    
    // For demonstration, we just print a heartbeat every 5 seconds
    if (WiFi.status() == WL_CONNECTED) 
    {
        Serial.println("Running normally... WiFi is connected.");
    } 
    else  
    {
        Serial.println("WiFi is disconnected! Attempting to reconnect...");
        WiFi.reconnect(); 
    }
    
    delay(5000);
}