#include "MDNSResolver.h"
#include "esp_netif.h"
#include "mdns.h"
#include "esp_log.h"
#include <IPAddress.h>  // Arduino-compatible IPAddress class
#include <string.h>


void MDNSResolver::begin(const char* espHostname) {
  // Prerequisite: WiFi must already be started
  ESP_ERROR_CHECK(mdns_init());
  ESP_ERROR_CHECK(mdns_hostname_set(espHostname));
  ESP_ERROR_CHECK(mdns_instance_name_set(espHostname));
}
/*
IPAddress MDNSResolver::lookupService(const char* service, const char* proto, IPAddress fallback, const char* targetHostname) {
  int n = MDNS.queryService(service, proto);
  if (n == 0) {
    Serial.println("No mDNS services found, using fallback");
    return fallback;
  }

  for (int i = 0; i < n; ++i) {
    Serial.print("Found service: ");
    Serial.println(MDNS.hostname(i));

    if (targetHostname && strcmp(MDNS.hostname(i).c_str(), targetHostname) == 0) {
      Serial.print("Match found at IP: ");
      IPAddress ip = MDNS.queryHost(targetHostname);
      Serial.println(ip);
      return ip;
    }
  }

  Serial.println("No matching mDNS service found, using fallback");
  return fallback;
}
*/

IPAddress MDNSResolver::lookupService(const char* service, const char* proto, IPAddress fallback, const char* targetHostname) {
    mdns_result_t* results = nullptr;

    esp_err_t err = mdns_query_ptr(service, proto, 3000, 10, &results);
    if (err != ESP_OK || results == nullptr) {
        ESP_LOGW("mDNS", "Service query failed or no results, using fallback");
        return fallback;
    }

    mdns_result_t* r = results;
    while (r != nullptr) {
        if (r->instance_name && strcmp(r->instance_name, targetHostname) == 0) {
            if (r->addr && r->addr->addr.type == ESP_IPADDR_TYPE_V4) {
                esp_ip4_addr_t ip4 = r->addr->addr.u_addr.ip4;
                mdns_query_results_free(results);
                return IPAddress(ip4.addr);  // Return found IP
            }
        }
        r = r->next;
    }

    mdns_query_results_free(results);
    ESP_LOGW("mDNS", "Target host not found, using fallback");
    return fallback;
}
