#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 5          //pin where the dht22 is connected
DHT dht(DHTPIN, DHT11);
#define ss 10
#define rst 9
#define dio0 2

LiquidCrystal_I2C lcd(0x27, 16, 2);
// Degree symbol:
byte Degree[] = {
  B00111,
  B00101,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

String outgoing;
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 50;          // interval between sends

String temp;     //DS18B20 temperature

void setup()
{
  Serial.begin(115200);
  dht.begin();
    // Start the LCD and turn on the backlight:
  lcd.init();
  lcd.backlight();
  lcd.clear();
  // Create a custom character:
  lcd.createChar(0, Degree);

  while (!Serial);
  Serial.println("LoRa Two Way Communication");
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    delay(100);
    while (1);
  }
}

void loop()
{
  if (millis() - lastSendTime > interval) {
    //--------------- Start Sending DHT22 Data --------------------
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();


    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.println("");

    String message = String(temperature) + "/" + String(humidity);
  sendMessage(message);
    // Serial.println("Sending " + message);
    lastSendTime = millis();            // timestamp the message
    interval = random(50) + 100;    // 2-3 seconds
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}


//--------------- Start receiving DS18B20 Temperature Data --------------------
void onReceive(int packetSize) {
  if (packetSize == 0) return;

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length



  // received a packet
  Serial.print("Received packet:  ");
  String LoRaData = LoRa.readString();
  Serial.print(LoRaData);
  // read packet
  while (LoRa.available()) {
    Serial.print((char)LoRa.read());
  }
  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());


  temp = LoRaData.substring(0, LoRaData.length());

  Serial.print(F("DS18B20 Temperature: "));
  Serial.print(temp);
  Serial.println(F(" °C"));
  Serial.println();
  
    // Print the temperature on the LCD;
  lcd.setCursor(0,0);
  lcd.print("Temperature:");
  lcd.setCursor(0,1);
  lcd.print(temp);
  lcd.write(0); // print the custom character
  lcd.print("C");
  delay(1000);
}