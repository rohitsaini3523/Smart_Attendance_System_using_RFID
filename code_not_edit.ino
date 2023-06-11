#include <SPI.h>
#include <MFRC522.h>
#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include <time.h>

constexpr uint8_t RST_PIN = D0; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D8;  // Configurable, see typical pin layout above

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

char ssid[] = "INTERNET";
char pass[] = "12345678";

WiFiClient client;

const unsigned int ch_no = 2087297;         // Replace with ThingSpeak Channel number
const char *write_api = "S1TPYDW6T8Z0D2DE"; // Replace with ThingSpeak write API

const int MAX_UIDS = 60;
String nuidPICC[MAX_UIDS];
int nuidCount = 0;

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

void loop()
{
    if (!rfid.PICC_IsNewCardPresent())
        return;

    if (rfid.PICC_ReadCardSerial())
    {
        String tag = "";

        for (byte i = 0; i < 4; i++)
        {
            tag += rfid.uid.uidByte[i];
        }

        Serial.println(tag);

        bool isNewTag = true;

        for (int i = 0; i < nuidCount; i++)
        {
            if (nuidPICC[i] == tag)
            {
                isNewTag = false;
                break;
            }
        }

        digitalWrite(D2, HIGH);
        delay(100);
        digitalWrite(D2, LOW);

        // if (tag == "211914726") {
        //   Serial.println("Access Granted!");
        //   digitalWrite(D8, HIGH);
        //   delay(100);
        //   digitalWrite(D8, LOW);
        //   delay(100);
        //   digitalWrite(D8, HIGH);
        //   delay(100);
        // } else {
        //   Serial.println("Access Denied!");
        //   digitalWrite(D8, HIGH);
        //   delay(2000);
        // }

        digitalWrite(D8, LOW);
        if (isNewTag)
        {
            if (nuidCount < MAX_UIDS)
            {
                nuidPICC[nuidCount] = tag;
                senddata(tag);
                nuidCount++;
            }
            else
            {
                Serial.println("Maximum number of UIDs stored, cannot add more.");
            }
        }
        else
        {
            Serial.println("Tag already stored.");
        }
        tag = "";

        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }
}

void senddata(String input)
{
    ThingSpeak.setField(1, input);
    ThingSpeak.writeFields(ch_no, write_api);
}
