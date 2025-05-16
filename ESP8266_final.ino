#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "esp8266";       // Replace with your Wi-Fi SSID
const char* password = "82666esp";  // Replace with your Wi-Fi password

const char* serverUrl = "http://192.168.18.12/sphygdb/save_data.php";  // Replace with your local IP and file path

WiFiClient client;  // Create a WiFiClient object

void setup() {
    Serial.begin(9600);   // Serial Monitor for debugging

    Serial.println("🚀 Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print("...");
    }

    Serial.println("\n✅ Connected to Wi-Fi!");
    Serial.print("📡 IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.println("🔄 Waiting for data from ESP32...");
}

void loop() {
    if (Serial.available()) { // Check if ESP32 sent data
        String receivedData = Serial.readStringUntil('\n');  // Read the incoming data

        // Split the received data (e.g., "95 57 82") into individual values
        int sys, dia, bpm;
        sscanf(receivedData.c_str(), "%d %d %d", &sys, &dia, &bpm);

        // Print the values with appropriate labels
        Serial.println("Systolic: " + String(sys));
        Serial.println("Diastolic: " + String(dia));
        Serial.println("BPM: " + String(bpm));

        // Form the data string to send
        String postData = "systolic=" + String(sys) + "&diastolic=" + String(dia) + "&bpm=" + String(bpm);
        
        // Debugging: Print the data to be sent
        Serial.println("🔄 Sending data to server: " + postData);

        // Send data to the server
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            http.begin(client, serverUrl);  // Use WiFiClient object to specify the URL
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");  // Set the content type

            // Send POST request
            int httpResponseCode = http.POST(postData);

            // Check if the POST request was successful
            if (httpResponseCode > 0) {
                Serial.println("✅ Data sent successfully");
                String response = http.getString();
                Serial.println("Response: " + response);
            } else {
                Serial.println("❌ Error sending data: " + String(httpResponseCode));
            }

            // End the HTTP request
            http.end();
        } else {
            Serial.println("❌ WiFi not connected");
        }
    }

    delay(1000); // Small delay for stability
}
