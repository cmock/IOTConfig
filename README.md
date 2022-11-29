# IOTConfig

Manage configuration values for IOT projects. Variables can be
accessed via MQTT and are stored in ESP32 preferences. ESP32/Arduino
only.

## Motivation

I find myself building various small gadgets for home automation that
integrate via MQTT. But they must be able to continue to work even in
the absence of a network.

So the wheel I've been reinventing with every project roughly looks
like this:

* Configuration and state variables of various types (int,
  float, String)
* Published on change via MQTT, so I can monitor what's going on
* Settable via MQTT (and potentially on the gadget itself) to
  configure the gadget
* Variables are persistently stored on the gadget via [Preferences](https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/preferences.html)

## Dependencies

* [PubSubClient](https://github.com/knolleary/pubsubclient)

## Synopsis

```c++
#include <PubSubClient.h>
#include <IOTConfig.h>

PubSubClient mqttClient(...);

IOTConfig config; // The IOTConfig holder object

int myInt; // an IOTConfig managed variable

void setup() {
  // init the holder; the bool decides whether the published values
  // are retained.
  config.begin(mqttClient, "myPrefsNamespace", "base/topic", true);

  // myInt will be read from Preferences (first bool),
  // writeable via MQTT at base/topic/config/myInt (second bool),
  // and defaults to 42.
  config.addVar(&myInt, "myInt", true, true, 42);
}

void loop() {
  myInt++; // use the variable as normal

  // when variables have changed, publish them to MQTT (base/topic/myInt)
  // and store in Preferences.
  // Updating variables from MQTT is happening in a callback, no action needed.
  config.update();

  delay(1000);
}

```

## Supported variable types

* int
* float
* String

Because this is all I need at the moment. Cast as needed or open an
issue.

## Performance impact

In `update()`, all configured variables are compared to their storage
in Preferences to check whether they need to be written, so it may be
wise to only call it occasionally. Note that this also publishes the
changed state to MQTT, so for quick acknowledgement of MQTT-configured
changes, don't do it too infrequently, either.

Writing to EEPROM and acknowleding via MQTT is done at the same time
by design, so when you see that ack, you know it's been persisted.

The Espressif docu is not clear about the EEPROM wear impact of
writing, which is why the code doesn't just blindly write on every
invocation of `update()`. Let me know if you find more in-depth docu
(or have investigated Espressif's code, which I'm too lazy to do).

## Shortcomings

This does not hold *all* configuration. Namely, the WiFi and MQTT
configs can't be stored, as the `IOTConfig` object needs to have MQTT available, which needs WiFi.

I'm not sure if I even want to tackle this, as this would lead to some
builtin AP mode when there's no configured WiFi available, and the
handling of changing of the MQTT server over MQTT... and currently
that's way out of scope.

Error handling/reporting: hmmm... I need to think about that.

Also, this is my first C++ project and my first published library, so
there'll be dragons. Really obviously stupid dragons probably...

## License

This code is released under the MIT License.
