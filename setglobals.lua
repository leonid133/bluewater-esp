global_status=1
sensor_f = 1
sensor_s = 1
id=0

local wifi_connector = "wifi.lua"
local status_sender = "send_status.lua"
local sensor_scaner = "sensor.lua"

dofile(wifi_connector)
dofile(sensor_scaner)
dofile(status_sender)

