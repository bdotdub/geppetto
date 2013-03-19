#include "config.h"
#include <Pinoccio.h>

#define RECIPIENT_ADDRESS 1

static SYS_Timer_t periodicTimer;
static NWK_DataReq_t messageData;
static bool messageDataBusy = false;
static uint8_t appPingCounter = 0;

static void messageSentConfirmation(NWK_DataReq_t *req) {
  messageDataBusy = false;
  (void)req;
}

static void sendMessage(void) {
  if (messageDataBusy) {
    return;
  }

  messageData.dstAddr = RECIPIENT_ADDRESS;
  messageData.dstEndpoint = APP_ENDPOINT;
  messageData.srcEndpoint = APP_ENDPOINT;
  messageData.options = NWK_OPT_ENABLE_SECURITY;
  messageData.data = &appPingCounter;
  messageData.size = sizeof(appPingCounter);
  messageData.confirm = messageSentConfirmation;
  NWK_DataReq(&messageData);

  appPingCounter++;
  messageDataBusy = true;

  RgbLed.blinkCyan(100);
}

static bool isSender() {
  return APP_ADDR == 0;
}

static void periodicTimerHandler(SYS_Timer_t *timer) {
  if (isSender()) {
    sendMessage();
    (void)timer;
  } else {
    Serial.println("Waiting....");
  }
}

static bool processMessage(NWK_DataInd_t *ind) {
  RgbLed.blinkCyan(100);

  Serial.print("lqi: ");
  Serial.print(ind->lqi, DEC);

  Serial.print("  ");

  Serial.print("rssi: ");
  Serial.print(ind->rssi, DEC);
  Serial.print("  ");

  Serial.print("data: ");
  for (int i=0; i<1; i++) {
    Serial.print(ind->data[i], DEC);
  }
  Serial.println("");
  return true;
}

void setupMeshNetwork() {
  NWK_SetAddr(APP_ADDR);
  NWK_SetPanId(APP_PANID);
  PHY_SetChannel(APP_CHANNEL);
  PHY_SetRxState(true);
  NWK_OpenEndpoint(APP_ENDPOINT, processMessage);
}

void startPeriodicTimer() {
  periodicTimer.interval = 50000;
  periodicTimer.mode = SYS_TIMER_PERIODIC_MODE;
  periodicTimer.handler = periodicTimerHandler;
  SYS_TimerStart(&periodicTimer);
}

void setup() {
  Pinoccio.init();

  appPingCounter = 0;

  setupMeshNetwork();
  startPeriodicTimer();
}

void loop() {
  Pinoccio.loop();
}
