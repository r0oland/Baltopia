// ThingsBoardSized client class
template <size_t PayloadSize, size_t MaxFieldsAmt, typename Logger>
class ThingsBoardSized
{
public:
  // Initializes ThingsBoardSized class with network client.
  inline ThingsBoardSized(Client &client) :m_client(client) { }

  // Destroys ThingsBoardSized class with network client.
  inline ~ThingsBoardSized() { }

  // Connects to the specified ThingsBoard server and port.
  // Access token is used to authenticate a client.
  // Returns true on success, false otherwise.
  bool connect(const char *host, const char *access_token, int port = 1883) {
    if (!host || !access_token) {
      return false;
    }
    this->RPC_Unsubscribe(); // Cleanup any subscriptions
    m_client.setServer(host, port);
    return m_client.connect("TbDev", access_token, NULL);
  }

  // Disconnects from ThingsBoard. Returns true on success.
  inline void disconnect() {
    m_client.disconnect();
  }

  // Returns true if connected, false otherwise.
  inline bool connected() {
    return m_client.connected();
  }

  // Executes an event loop for PubSub client.
  inline void loop() {
    m_client.loop();
  }

  //----------------------------------------------------------------------------
  // Telemetry API

  // Sends integer telemetry data to the ThingsBoard, returns true on success.
  inline bool sendTelemetryInt(const char *key, int value) {
    return sendKeyval(key, value);
  }

  // Sends boolean telemetry data to the ThingsBoard, returns true on success.
  inline bool sendTelemetryBool(const char *key, bool value) {
    return sendKeyval(key, value);
  }

  // Sends float telemetry data to the ThingsBoard, returns true on success.
  inline bool sendTelemetryFloat(const char *key, float value) {
    return sendKeyval(key, value);
  }

  // Sends string telemetry data to the ThingsBoard, returns true on success.
  inline bool sendTelemetryString(const char *key, const char *value) {
    return sendKeyval(key, value);
  }

  // Sends aggregated telemetry to the ThingsBoard.
  inline bool sendTelemetry(const Telemetry *data, size_t data_count) {
    return sendDataArray(data, data_count);
  }

  // Sends custom JSON telemetry string to the ThingsBoard.
  inline bool sendTelemetryJson(const char *json) {
    return m_client.publish("v1/devices/me/telemetry", json);
  }

  //----------------------------------------------------------------------------
  // Attribute API

  // Sends integer attribute with given name and value.
  inline bool sendAttributeInt(const char *attrName, int value) {
    return sendKeyval(attrName, value, false);
  }

  // Sends boolean attribute with given name and value.
  inline bool sendAttributeBool(const char *attrName, bool value) {
    return sendKeyval(attrName, value, false);
  }

  // Sends float attribute with given name and value.
  inline bool sendAttributeFloat(const char *attrName, float value) {
    return sendKeyval(attrName, value, false);
  }

  // Sends string attribute with given name and value.
  inline bool sendAttributeString(const char *attrName, const char *value) {
    return sendKeyval(attrName, value, false);
  }

  // Sends aggregated attributes to the ThingsBoard.
  inline bool sendAttributes(const Attribute *data, size_t data_count) {
    return sendDataArray(data, data_count, false);
  }

  // Sends custom JSON with attributes to the ThingsBoard.
  inline bool sendAttributeJSON(const char *json) {
    return m_client.publish("v1/devices/me/attributes", json);
  }

  //----------------------------------------------------------------------------
  // Server-side RPC API

  // Subscribes multiple RPC callbacks with given size
  bool RPC_Subscribe(const RPC_Callback *callbacks, size_t callbacks_size) {
    if (callbacks_size > sizeof(m_rpcCallbacks) / sizeof(*m_rpcCallbacks)) {
      return false;
    }
    if (ThingsBoardSized::m_subscribedInstance) {
      return false;
    }

    if (!m_client.subscribe("v1/devices/me/rpc/request/+")) {
      return false;
    }

    ThingsBoardSized::m_subscribedInstance = this;
    for (size_t i = 0; i < callbacks_size; ++i) {
      m_rpcCallbacks[i] = callbacks[i];
    }

    m_client.setCallback(ThingsBoardSized::on_message);

    return true;
}

  inline bool RPC_Unsubscribe() {
    ThingsBoardSized::m_subscribedInstance = NULL;
    return m_client.unsubscribe("v1/devices/me/rpc/request/+");
  }

private:
  // Sends single key-value in a generic way.
  template<typename T>
  bool sendKeyval(const char *key, T value, bool telemetry = true) {
    Telemetry t(key, value);

    char payload[PayloadSize];
    {
      Telemetry t(key, value);
      StaticJsonDocument<JSON_OBJECT_SIZE(1)>jsonBuffer;
      JsonVariant object = jsonBuffer.template to<JsonVariant>();
      if (t.serializeKeyval(object) == false){
        Logger::log("unable to serialize data");
        return false;
      }

      if (measureJson(jsonBuffer) > PayloadSize - 1) {
        Logger::log("too small buffer for JSON data");
        return false;
      }
      serializeJson(object, payload, sizeof(payload));
    }
    return telemetry ? sendTelemetryJson(payload) : sendAttributeJSON(payload);
  }

  // Processes RPC message
  void process_message(char* topic, uint8_t* payload, unsigned int length) {
    RPC_Response r;
    {
      StaticJsonDocument<JSON_OBJECT_SIZE(MaxFieldsAmt)> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, payload, length);
      if (error) {
        Logger::log("unable to de-serialize RPC");
        return;
      }
      const JsonObject &data = jsonBuffer.template as<JsonObject>();

      const char *methodName = data["method"];

      if (methodName) {
        Logger::log("received RPC:");
        Logger::log(methodName);
      } else {
        Logger::log("RPC method is NULL");
        return;
      }

      for (size_t i = 0; i < sizeof(m_rpcCallbacks) / sizeof(*m_rpcCallbacks); ++i) {
        if (m_rpcCallbacks[i].m_cb && !strcmp(m_rpcCallbacks[i].m_name, methodName)) {

          Logger::log("calling RPC:");
          Logger::log(methodName);

          // Do not inform client, if parameter field is missing for some reason
          if (!data.containsKey("params")) {
            Logger::log("no parameters passed with RPC, passing null JSON");
          }
          // Getting non-existing field from JSON should automatically
          // set JSONVariant to null
          r = m_rpcCallbacks[i].m_cb(data["params"]);
          break;
        }
      }
    }
    {
      // Fill in response
      char payload[PayloadSize] = {0};
      StaticJsonDocument<JSON_OBJECT_SIZE(1)> respBuffer;
      JsonVariant resp_obj = respBuffer.template to<JsonVariant>();

      if (r.serializeKeyval(resp_obj) == false) {
        Logger::log("unable to serialize data");
        return;
      }

      if (measureJson(respBuffer) > PayloadSize - 1) {
        Logger::log("too small buffer for JSON data");
        return;
      }
      serializeJson(resp_obj, payload, sizeof(payload));

      String responseTopic = String(topic);
      responseTopic.replace("request", "response");
      Logger::log("response:");
      Logger::log(payload);
      m_client.publish(responseTopic.c_str(), payload);
    }
  }

  // Sends array of attributes or telemetry to ThingsBoard
  bool sendDataArray(const Telemetry *data, size_t data_count, bool telemetry = true) {
    if (MaxFieldsAmt < data_count) {
      Logger::log("too much JSON fields passed");
      return false;
    }
    char payload[PayloadSize];
    {
      StaticJsonDocument<JSON_OBJECT_SIZE(MaxFieldsAmt)> jsonBuffer;
      JsonVariant object = jsonBuffer.template to<JsonVariant>();

      for (size_t i = 0; i < data_count; ++i) {
        if (data[i].serializeKeyval(object) == false) {
          Logger::log("unable to serialize data");
          return false;
        }
      }
      if (measureJson(jsonBuffer) > PayloadSize - 1) {
        Logger::log("too small buffer for JSON data");
        return false;
      }
      serializeJson(object, payload, sizeof(payload));
    }

    return telemetry ? sendTelemetryJson(payload) : sendAttributeJSON(payload);
  }

  PubSubClient m_client;              // PubSub MQTT client instance.
  RPC_Callback m_rpcCallbacks[8];     // RPC callbacks array

  // PubSub client cannot call a method when message arrives on subscribed topic.
  // Only free-standing function is allowed.
  // To be able to forward event to an instance, rather than to a function, this pointer exists.
  static ThingsBoardSized *m_subscribedInstance;

  // The callback for when a PUBLISH message is received from the server.
  static void on_message(char* topic, uint8_t* payload, unsigned int length) {
    if (!ThingsBoardSized::m_subscribedInstance) {
      return;
    }

    ThingsBoardSized::m_subscribedInstance->process_message(topic, payload, length);
  }
};
