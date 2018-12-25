local http_post = "http_post.lua"
local status_freq = 1000
global_status = 1
sensor_f = 1
sensor_s = 1
id = 0

local http_welcome_post = "welcome_post.lua"
dofile(http_welcome_post)

tmr.create():alarm(status_freq, tmr.ALARM_AUTO, function()
    print("sensor f: "..sensor_f)
    print("sensor s: "..sensor_s)
    print("status: "..global_status)
    print("id: "..id)
    dofile(http_post)
end)
