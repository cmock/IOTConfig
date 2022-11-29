/*
  IOTConfig
   christian mock
   https://www.tahina.priv.at/
*/

#include <functional>
#include "IOTConfig.h"

IOTConfig::IOTVar::IOTVar(String name, bool storePrefs, bool mqttWriteable, 
			  IOTConfig *iotc) {
  this->name = name;
  this->storePrefs = storePrefs;
  this->mqttWriteable = mqttWriteable;
  this->iotc = iotc;
}

bool IOTConfig::IOTVar::updateMQTT() {
  String topic = iotc->baseTopic + "/" + name;
  iotc->mqttClient->publish(topic.c_str(), toString().c_str(), iotc->retain);
  // TODO: error handling
  return true;
}

// Integer

IOTConfig::IOTVarInt::IOTVarInt(int *var, String name, bool storePrefs,
				bool mqttWriteable, int dflt, IOTConfig *iotc)
  : IOTVar(name, storePrefs, mqttWriteable, iotc) {
  this->ptr = var;
  *var = dflt;
  if(storePrefs) {
    iotc->prefs.begin(iotc->prefsNS.c_str(), false);
    if(iotc->prefs.isKey(name.c_str())) {
      *var = iotc->prefs.getInt(name.c_str());
    } else {
      if(!iotc->prefs.putInt(name.c_str(), *var)) {
	// TODO error handling
      }
    }
    prev_value = *ptr;
    iotc->prefs.end();
  }
}

String IOTConfig::IOTVarInt::toString() {
  return String(*ptr);
}

// true: updated, false: not updated
bool IOTConfig::IOTVarInt::updatePrefs() {
  int stored;
  bool found = false, changed = false;

  if(*ptr == prev_value)
    return false; // unchanged
  prev_value = *ptr;
  
  if(!storePrefs)
    return true; // or else it wouldn't publish to MQTT...

  iotc->prefs.begin(iotc->prefsNS.c_str(), false);
  if(iotc->prefs.isKey(name.c_str())) {
    stored = iotc->prefs.getInt(name.c_str());
    found = true;
  }

  if(!found || *ptr != stored) {
    changed = true;
    if(!iotc->prefs.putInt(name.c_str(), *ptr)) {
      // TODO error handling
    }
  }
  iotc->prefs.end();
  return changed;
}

bool IOTConfig::IOTVarInt::setFromString(String string) {
  *this->ptr = string.toInt();
  return true;
}

// Float
IOTConfig::IOTVarFloat::IOTVarFloat(float *var, String name,
				    bool storePrefs, bool mqttWriteable,
				    float dflt, IOTConfig *iotc)
  : IOTVar(name, storePrefs, mqttWriteable, iotc) {
  this->ptr = var;
  *var = dflt;
  if(storePrefs) {
    iotc->prefs.begin(iotc->prefsNS.c_str(), false);
    if(iotc->prefs.isKey(this->name.c_str())) {
      *var = iotc->prefs.getFloat(this->name.c_str());
    } else {
      if(!iotc->prefs.putFloat(this->name.c_str(), *var)) {
	// TODO error handling
      }
    }
    iotc->prefs.end();
  }
  prev_value = *ptr;
}

String IOTConfig::IOTVarFloat::toString() {
  return String(*ptr);
}

bool IOTConfig::IOTVarFloat::updatePrefs() {
  float stored;
  bool found = false, changed = false;
  
  if(*ptr == prev_value)
    return false; // unchanged
  prev_value = *ptr;

  if(!this->storePrefs)
    return true; 

  iotc->prefs.begin(iotc->prefsNS.c_str(), false);
  if(iotc->prefs.isKey(this->name.c_str())) {
    stored = iotc->prefs.getFloat(this->name.c_str());
    found = true;
  }

  if(!found || *this->ptr != stored) {
    changed = true;
    if(!iotc->prefs.putFloat(this->name.c_str(), *this->ptr)) {
      // TODO error handling
    }
  }
  iotc->prefs.end();
  return changed;
}

bool IOTConfig::IOTVarFloat::setFromString(String string) {
  *this->ptr = string.toFloat();
  return true;
}

// String
IOTConfig::IOTVarString::IOTVarString(String *var, String name,
				      bool storePrefs, bool mqttWriteable,
				      String dflt, IOTConfig *iotc)
  : IOTVar(name, storePrefs, mqttWriteable, iotc) {
  ptr = var;
  if(storePrefs) {
    iotc->prefs.begin(iotc->prefsNS.c_str(), false);
    if(iotc->prefs.isKey(name.c_str())) {
      *var = iotc->prefs.getString(name.c_str());
    } else {
      if(!iotc->prefs.putString(name.c_str(), *var)) {
	// TODO error handling
      }
    }
    iotc->prefs.end();
  }
  prev_value = *ptr;
}

String IOTConfig::IOTVarString::toString() {
  return *ptr;
}

bool IOTConfig::IOTVarString::updatePrefs() {
  String stored;
  bool found = false, changed = false;
  
  if(*ptr == prev_value)
    return false; // unchanged
  prev_value = *ptr;

  if(!storePrefs)
    return true;

  iotc->prefs.begin(iotc->prefsNS.c_str(), false);
  if(iotc->prefs.isKey(name.c_str())) {
    stored = iotc->prefs.getString(name.c_str());
    found = true;
  }

  if(!found || *ptr != stored) {
    changed = true;
    if(!iotc->prefs.putString(name.c_str(), *ptr)) {
      // TODO error handling
    }
  }
  iotc->prefs.end();
  return changed;
}

bool IOTConfig::IOTVarString::setFromString(String string) {
  *ptr = string;
  return true;
}

bool IOTConfig::begin(PubSubClient &psc, String prefsNS, String baseTopic, 
		 bool retain) {
  this->mqttClient = &psc;
  this->prefsNS = prefsNS;
  this->baseTopic = baseTopic;
  this->retain = retain;
  // This is way over my head, but cf the definition of MQTT_CALLBACK_SIGNATURE
  // in PubSubClient.h and
  // https://blog.mbedded.ninja/programming/languages/c-plus-plus/callbacks/
  // case 8, and pray...
  mqttClient->setCallback(std::bind(&IOTConfig::mqttCallback, this,
				   std::placeholders::_1,
				   std::placeholders::_2,
				   std::placeholders::_3));
  return true;
}

bool IOTConfig::update() {
  for(auto itr : vars)
    if(itr->updatePrefs())
      if(!itr->updateMQTT())
	return false;
  return true;
}

void IOTConfig::mqttCallback(char *topic, uint8_t *msg, unsigned int len) {
  // find topic
  auto tstr = String(topic);
  auto bstr = baseTopic + MQTT_CONFIG_SUBTOPIC;
  if(!tstr.startsWith(bstr))
    return; // not relevant
  auto vstr = tstr.substring(bstr.length());
  for(auto itr : vars)
    if(itr->getName() == vstr) {
      auto mstr = String();
      mstr.reserve(len+1);
      for(int i = 0; i < len; i++)
	mstr.concat((char)msg[i]);
      itr->setFromString(mstr);
      return;
    }
}

void IOTConfig::resubscribe() {
  for(auto itr : vars) 
    mqttClient->subscribe(String(baseTopic + MQTT_CONFIG_SUBTOPIC + 
				itr->getName()).c_str());
}


bool IOTConfig::addVar_common(IOTVar *iotvar) {
  vars.push_back(iotvar);
  mqttClient->subscribe(String(baseTopic + MQTT_CONFIG_SUBTOPIC + 
			      iotvar->getName()).c_str());
  return iotvar->updateMQTT();
}

bool IOTConfig::addVar(int *var, String name, 
		       bool storePrefs, bool mqttWriteable, int dflt) {
  auto iotvar = new IOTVarInt(var, name, storePrefs, mqttWriteable, dflt, this);
  return addVar_common(iotvar);
}  

bool IOTConfig::addVar(float *var, String name, 
		       bool storePrefs, bool mqttWriteable, float dflt) {
  auto iotvar = 
    new IOTVarFloat(var, name, storePrefs, mqttWriteable, dflt, this);
  return addVar_common(iotvar);
}

bool IOTConfig::addVar(String *var, String name,
		       bool storePrefs, bool mqttWriteable, String dflt) {
  auto iotvar = 
    new IOTVarString(var, name, storePrefs, mqttWriteable, dflt, this);
  return addVar_common(iotvar);
}

