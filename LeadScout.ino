#include <Pinoccio.h>

#define Debug 1
#define Log(string) if (Debug == 1) { Serial.print(string); }

#define LeadScoutAddress 1

class LeadScout;

static LeadScout *leadScout;
static mqttClient *mqtt;

static void MessageSendConfirmation(NWK_DataReq_t *req) {
  if (NWK_SUCCESS_STATUS == req->status)
    Serial.println("Message successfully sent");
  else {
    Serial.print("Error sending message: ");
    Serial.println(req->status, HEX);
  }
}

class LeadScout {
public:
  LeadScout(uint16_t address_, WIFI_PROFILE wifiProfile_, mqttClient *mqtt, char *channel_);
  void Setup();
  void StartListening();
  void Loop();
  void SendMessage(byte *message, unsigned int length);
private:
  void SetupPinoccio();
  void StartWireless();

  uint16_t address;

  NWK_DataReq_t nwkDataReq;

  mqttClient *mqtt;
  char *channel;

  WIFI_PROFILE wifiProfile;
};

LeadScout::LeadScout(uint16_t address_, WIFI_PROFILE wifiProfile_, mqttClient *mqtt, char *channel_) {
  address = address_;
  wifiProfile = wifiProfile_;
  channel = channel_;
}

void LeadScout::Loop() {
}

void LeadScout::Setup() {
  SetupPinoccio();
  StartWireless();
}

void LeadScout::SetupPinoccio() {
  Log("Initialize Pinoccio and the mesh network");
  Pinoccio.init();
  Pinoccio.initMesh(address, 0x4567, 0x1a);
}

void LeadScout::StartWireless() {
  Log("Start up the wireless");
  Wifi.begin(&wifiProfile);
}

void LeadScout::StartListening() {
  Log("LeadScout now listening");
  Log(channel);
  if (mqtt->connect("pinoccio")) {
    mqtt->subscribe(channel);
  }
}

void LeadScout::SendMessage(byte *message, unsigned int length) {
  Log("Sending message");
  nwkDataReq.dstAddr = 1;
  nwkDataReq.dstEndpoint = 1;
  nwkDataReq.srcEndpoint = 1;
  nwkDataReq.options = 0;
  nwkDataReq.data = message;
  nwkDataReq.size = length;
  nwkDataReq.confirm = &MessageSendConfirmation;
  NWK_DataReq(&nwkDataReq);
}

static void MqttCallback(char *topic, byte *payload, unsigned int length) {
  RgbLed.blinkCyan(100);
  leadScout->SendMessage(payload, length);
}

WIFI_PROFILE profile = {
                /* SSID */ "",
 /* WPA/WPA2 passphrase */ "",
          /* IP address */ "",
         /* subnet mask */ "",
          /* Gateway IP */ "" };
IPAddress mqttServerIP(192,168,0,1);

void setup() {
  PinoccioWifiClient wifiClient;

  Log("Creating mqtt client");
  mqtt = new mqttClient(mqttServerIP, 1883, MqttCallback, wifiClient);

  Log("Creating and setting up LeadScout");
  leadScout = new LeadScout(LeadScoutAddress, profile, mqtt, "bdotdub/test");
  leadScout->Setup();
}

void loop() {
  Pinoccio.loop();
  mqtt->loop();
}

