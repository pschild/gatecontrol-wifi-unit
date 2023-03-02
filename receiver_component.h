#include "esphome.h"
#include "relais_switch.h"
#include "SoftwareSerial.h"
using namespace esphome;

SoftwareSerial hc12(D6, D5);

class ReceiverComponent : public PollingComponent
{
private: 
  // Variables and functions for Receiving
  static const byte numChars = 64;
  char receivedChars[numChars];
  char tempChars[numChars];
  boolean newData = false;

public:
  Switch *relaisSwitch1 = new RelaisSwitch();
  Switch *relaisSwitch2 = new RelaisSwitch();
  BinarySensor *sensorGate1 = new BinarySensor();
  BinarySensor *sensorGate2 = new BinarySensor();
  Sensor *heartbeatSensor = new Sensor();

  void setup() override
  {
    ESP_LOGD("receiver_component", "Setup");
    hc12.begin(9600);
    randomSeed(analogRead(0));

    relaisSwitch1->add_on_state_callback([=](bool state) -> void {
      ESP_LOGD("receiver_component", "switch 1 callback %d", state);
      char command[] = "setrelay1";
      this->sendRelaisCommand(command, relaisSwitch1->state);
    });

    relaisSwitch2->add_on_state_callback([=](bool state) -> void {
      ESP_LOGD("receiver_component", "switch 2 callback %d", state);
      char command[] = "setrelay2";
      this->sendRelaisCommand(command, relaisSwitch2->state);
    });
  }

  void update() override
  {
    // Receiving
    this->recvWithStartEndMarkers();
    if (newData == true)
    {
      strcpy(tempChars, receivedChars);
      this->parseData();
      newData = false;
    }
  }
  
  void sendRelaisCommand(char command[], int newState)
  {
    ESP_LOGD("receiver_component", "sendRelaisCommand...");
    hc12.print('<');
    hc12.print("command");
    hc12.print("=");
    hc12.print(command);
    hc12.print(",");
    hc12.print("state");
    hc12.print("=");
    hc12.print(newState);
    hc12.println('>');
  }

  void recvWithStartEndMarkers()
  {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (hc12.available() > 0 && newData == false)
    {
      rc = hc12.read();

      if (recvInProgress == true)
      {
        if (rc != endMarker)
        {
          receivedChars[ndx] = rc;
          ndx++;
          if (ndx >= numChars)
          {
            ndx = numChars - 1;
          }
        }
        else
        {
          receivedChars[ndx] = '\0'; // terminate the string
          recvInProgress = false;
          ndx = 0;
          newData = true;
        }
      }

      else if (rc == startMarker)
      {
        recvInProgress = true;
      }
    }
  }

  void parseData()
  {
    ESP_LOGD("receiver_component", "Parsing %s", tempChars);

    char *strtokIndx;
    char command[numChars] = {0};

    strtokIndx = strtok(tempChars, "=");
    strtokIndx = strtok(NULL, ",");
    strcpy(command, strtokIndx);

    if (strcmp(command, "publishstates") == 0)
    {
      strtokIndx = strtok(NULL, "=");
      strtokIndx = strtok(NULL, ",");
      int relay1State = atoi(strtokIndx);
      relaisSwitch1->publish_state(relay1State);

      strtokIndx = strtok(NULL, "=");
      strtokIndx = strtok(NULL, ",");
      int relay2State = atoi(strtokIndx);
      relaisSwitch2->publish_state(relay2State);

      strtokIndx = strtok(NULL, "=");
      strtokIndx = strtok(NULL, ",");
      int gate1State = atoi(strtokIndx);
      sensorGate1->publish_state(gate1State);

      strtokIndx = strtok(NULL, "=");
      strtokIndx = strtok(NULL, ",");
      int gate2State = atoi(strtokIndx);
      sensorGate2->publish_state(gate2State);

      heartbeatSensor->publish_state(millis());
    }
  }
};