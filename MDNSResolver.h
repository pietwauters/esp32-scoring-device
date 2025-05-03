#ifndef MDNS_RESOLVER_H
#define MDNS_RESOLVER_H

#include <Arduino.h>
#include <WiFi.h>
#include "Singleton.h"
class MDNSResolver: public SingletonMixin<MDNSResolver> {
public:
  //virtual ~MDNSResolver();
  void begin(const char* espHostname = "esp32");
  IPAddress lookupService(const char* service, const char* proto, IPAddress fallback, const char* targetHostname);
private:
// private methods
/** Default constructor */
friend class SingletonMixin<MDNSResolver>;
  MDNSResolver(){};  // tickPeriod in miliseconds

};

#endif
