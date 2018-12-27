local http_post = "http_post.lua"
local status_freq = 1000
global_status = 1
id = 0
sensor_f_sec = 0
sensor_f_usec = 0
sensor_s_sec = 0
sensor_s_usec = 0

local http_welcome_post = "welcome_post.lua"
dofile(http_welcome_post)

tmr.create():alarm(status_freq, tmr.ALARM_AUTO, function()
    print("sensor f: "..sensor_f_sec)
    print("sensor f u: "..sensor_f_usec)
    print("sensor s: "..sensor_s_sec)
    print("sensor s u: "..sensor_s_usec)
    print("status: "..global_status)
    print("id: "..id)
    tm = rtctime.epoch2cal(rtctime.get())
    print(string.format("%04d/%02d/%02d %02d:%02d:%02d", tm["year"], tm["mon"], tm["day"], tm["hour"], tm["min"], tm["sec"]))
    sec, usec = rtctime.get()
    print("sec: "..sec)
    print("usec: "..usec)
    dofile(http_post)
end)
