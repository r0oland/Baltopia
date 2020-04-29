import os
import time
import sys
import paho.mqtt.client as mqtt
import json
from subprocess import check_output
from re import findall

def get_temp():
    temp = check_output(["vcgencmd","measure_temp"]).decode("UTF-8")
    return(findall("\d+\.\d+",temp)[0])


THINGSBOARD_HOST = '192.168.1.2'
ACCESS_TOKEN = 'NcMx4LQQIKUby0bsvEcz'

# Data capture and upload interval in seconds. Less interval will eventually hang the DHT22.
INTERVAL = 2

sensor_data = {'temperature': 0}

next_reading = time.time() 

client = mqtt.Client()

# Set access token
client.username_pw_set(ACCESS_TOKEN)

# Connect to ThingsBoard using default MQTT port and 60 seconds keepalive interval
client.connect(THINGSBOARD_HOST, 1883, 60)

client.loop_start()

try:
    while True:
        temperature = get_temp()
        # temperature = round(temperature, 2)
        # print(u"Temperature: {:g}\u00b0C".format(temperature))
        sensor_data['temperature'] = temperature

        # Sending humidity and temperature data to ThingsBoard
        client.publish('v1/devices/me/telemetry', json.dumps(sensor_data), 1)

        next_reading += INTERVAL
        sleep_time = next_reading-time.time()
        if sleep_time > 0:
            time.sleep(sleep_time)
except KeyboardInterrupt:
    pass

client.loop_stop()
client.disconnect()