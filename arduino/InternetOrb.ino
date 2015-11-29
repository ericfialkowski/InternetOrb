#include <SPI.h>
#include <Ethernet.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

struct HOST_STRUCT {
  char address[15];
  char host_line[21];
};

// for the neopixel
#define NEO_PIXEL_PIN  9
const int TOTALPIXELS = 12;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(TOTALPIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };


HOST_STRUCT servers[] = {
  {"www.google.com", "Host: www.google.com"},
  {"www.yahoo.com", "Host: www.yahoo.com"},
  {"www.bing.com", "Host: www.bing.com"}
};
const int total_servers = sizeof(servers) / sizeof(HOST_STRUCT);
int server_ndx = 0;

// static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 9);

EthernetClient client;

unsigned long lastConnectionTime = 0;              // last time you connected to a server, in milliseconds
const unsigned long postingInterval = 30L * 1000L; // delay between updates, in milliseconds
int current_successes = 0;

void setup()
{
  Serial.begin(57600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pixels.begin(); // This initializes the NeoPixel library.
  blank();
  delay(250);
  OK();
  delay(250);
  BAD();
  delay(250);
  SOME();
  delay(250);
  blank();

  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void blank()
{
  setPixels(0, 0, 0);
}

void setPixels(int r, int g, int b)
{
  for (int i = 0; i < TOTALPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void BAD()
{
  setPixels(255, 0, 0);
}

void OK()
{
  setPixels(0, 255, 0);
}

void SOME()
{
  setPixels(255, 255, 0);
}


// this method makes a HTTP connection to the server:
boolean httpRequest(HOST_STRUCT *server)
{
  boolean rtn = false;
  client.stop();

  // if there's a successful connection:
  if (client.connect(server->address, 80))
  {
    client.println("HEAD / HTTP/1.1");
    client.println(server->host_line);
    client.println("Connection: close");
    client.println();
    rtn = true;
  }
  else
  {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }

  return rtn;
}

void loop()
{
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }

  // if 60 seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval)
  {

    if (httpRequest(&servers[server_ndx]))
    {
      current_successes = min(total_servers, current_successes + 1);
    }
    else
    {
      current_successes = max(0, current_successes - 1);
    }
    server_ndx = (++server_ndx) % 3;
    lastConnectionTime = millis();
  }

  if (current_successes == total_servers)
  {
    OK();
  }
  else if (current_successes > 0)
  {
    SOME();
  }
  else
  {
    BAD();
  }
}

