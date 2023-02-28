#include "esphome.h"
#include "esphome/components/sensor/sensor.h"
#include "relais_switch.h"
#include "SoftwareSerial.h"
using namespace esphome;

SoftwareSerial hc12(D6, D5);

class ReceiverComponent : public PollingComponent
{
public:
  Switch *relaisSwitch = new RelaisSwitch();
  Sensor *sensor = new Sensor();

  void setup() override
  {
    ESP_LOGD("receiver_component", "Setup");
    hc12.begin(9600);

    relaisSwitch->add_on_state_callback([=](bool state) -> void {
      ESP_LOGD ("receiver_component", "switch callback %d", state);
      this->sendRelaisCommand();
    });
  }

  void update() override
  {
    // Publish sensor values
    // sensor->publish_state(42);
    // ESP_LOGD("receiver_component", "switchState=%d", relaisSwitch->state);
  }
  
  void sendRelaisCommand()
  {
    ESP_LOGD("receiver_component", "sendRelaisCommand...");
    hc12.print('<');
    hc12.print("relayleft");
    hc12.print("=");
    hc12.print(relaisSwitch->state);
    hc12.println('>');
  }
};