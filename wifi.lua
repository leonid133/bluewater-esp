do
wifi.setmode(wifi.STATION)
--ifi.sta.clearconfig()
local scfg = {}
scfg.auto = true
scfg.save = true
scfg.ssid = 'provectus-2.4'
scfg.pwd = 'provectus'
wifi.sta.config(scfg)
wifi.sta.connect()
tmr.create():alarm(15000, tmr.ALARM_SINGLE, function() print('\n', wifi.sta.getip()) end)
end
