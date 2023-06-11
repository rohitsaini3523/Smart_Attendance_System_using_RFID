#include <SPI.h>
#include <MFRC522.h>
#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include <time.h>
#include <SD.h>

constexpr uint8_t RST_PIN = D3;             // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;              // Configurable, see typical pin layout above
unsigned long ch_no = 2087297;              // Replace with ThingSpeak Channel number
const char *write_api = "S1TPYDW6T8Z0D2DE"; // Replace with ThingSpeak write API
MFRC522 rfid(SS_PIN, RST_PIN);              // Instance of the class
MFRC522::MIFARE_Key key;
char ssid[] = "INTERNET";
char pass[] = "12345678";
String tag;
WiFiClient client;
void setup()
{
    Serial.begin(9600);
    SPI.begin();     // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    ThingSpeak.begin(client);
}

// void clear_thingspeak_data()
// {
//     // Connect to Wi-Fi
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         delay(1000);
//         Serial.println("Connecting to WiFi...");
//     }

//     // Delete all data from ThingSpeak channel
//     ThingSpeak.begin(client);
//     int success = ThingSpeak.deleteAllChannelData(channelID, writeAPIKey);

//     if (success)
//     {
//         Serial.println("ThingSpeak data cleared successfully");
//     }
//     else
//     {
//         Serial.println("Failed to clear ThingSpeak data");
//     }
// }

void loop()
{
    //read the valid rfuid list

    //get the time from the internet
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    String contents = import_time_table();
    //compare current time with the content in the csv file
    if(contents.indexOf(String(now)) != -1)
    {
        clear_thingspeak_data();
        Serial.println("Access Granted!");
    }
    if (!rfid.PICC_IsNewCardPresent())
        return;
    if (rfid.PICC_ReadCardSerial())
    {
        for (byte i = 0; i < 4; i++)
        {
            tag += rfid.uid.uidByte[i];
        }
        Serial.println(tag);
        senddata(tag);
        if (tag == "211914726")
        {
            Serial.println("Access Granted!");
            digitalWrite(D8, HIGH);
            delay(100);
            digitalWrite(D8, LOW);
            delay(100);
            digitalWrite(D8, HIGH);
            delay(100);
        }
        else
        {
            Serial.println("Access Denied!");
            digitalWrite(D8, HIGH);
            delay(2000);
        }
        digitalWrite(D8, LOW);
        tag = "";
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }
}

String import_time_table()
{
    // Open the CSV file
    File file = SD.open("timetable.csv", FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open timetable.csv");
        return;
    }
    String contents = file.readString();
    file.close();
    return contents;
}

void senddata(String input)
{
    ThingSpeak.setField(1, input);
    ThingSpeak.writeFields(ch_no, write_api);
}