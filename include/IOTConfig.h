/*
  IOTConfig
   christian mock
   https://www.tahina.priv.at/
*/

#ifndef IOTConfig_h
#define IOTConfig_h

#include <PubSubClient.h>
#include <Preferences.h>
#include <vector>

#ifndef MQTT_CONFIG_SUBTOPIC
#define MQTT_CONFIG_SUBTOPIC "/config/"
#endif

class IOTConfig {
 public:
  class IOTVar;
  class IOTVarInt;
  class IOTVarFloat;
  class IOTVarString;
  IOTConfig() = default;
  ~IOTConfig() = default;
  String prefsNS;
  Preferences prefs;
  
  bool begin(PubSubClient &psc, String prefsNS, String baseTopic, 
	     bool retain);
  bool addVar(int *var, String name, bool storePrefs, bool mqttWriteable, 
	      int dflt);
  bool addVar(float *var, String name, bool storePrefs, bool mqttWriteable,
	      float dflt);
  bool addVar(String *var, String name, bool storePrefs, bool mqttWriteable,
	      String dflt);
  bool update();
  bool addVar_common(IOTVar *iotvar);
  void resubscribe();
 private:
  PubSubClient *mqttClient;
  String baseTopic;
  bool retain;
  std::vector<IOTVar *> vars;
  void mqttCallback(char *topic, uint8_t *msg, unsigned int len);
};

class IOTConfig::IOTVar {
 public:
  IOTVar(String name, bool storePrefs, bool mqttWriteable, IOTConfig *iotc);
  ~IOTVar() = default;
  virtual String toString() = 0; // for MQTT publish
  virtual bool setFromString(String string) = 0;
  String getName() { return name; }
  bool updateMQTT();
  virtual bool updatePrefs() = 0;
 protected:
  String name;
  IOTConfig *iotc;
  bool storePrefs, mqttWriteable;
};

class IOTConfig::IOTVarInt : public IOTConfig::IOTVar {
 public:
  IOTVarInt(int *var, String name, bool storePrefs, bool mqttWriteable,
	    int dflt, IOTConfig *iotc);
  String toString();
  bool setFromString(String string);
  bool updatePrefs();
 private:
  int *ptr;
  int prev_value;
};

class IOTConfig::IOTVarFloat : public IOTConfig::IOTVar {
 public:
  IOTVarFloat(float *var, String name, bool storePrefs, bool mqttWriteable,
	    float dflt, IOTConfig *iotc);
  String toString();
  bool setFromString(String string);
  bool updatePrefs();
private:
  float *ptr;
  float prev_value;
};

class IOTConfig::IOTVarString : public IOTConfig::IOTVar {
 public:
  IOTVarString(String *var, String name, bool storePrefs, bool mqttWriteable,
	    String dflt, IOTConfig *iotc);
  String toString();
  bool setFromString(String string);
  bool updatePrefs();
 private:
  String *ptr;
  String prev_value;
};


#endif // IOTConfg_h
