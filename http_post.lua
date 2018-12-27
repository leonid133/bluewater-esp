--local http_endpoint='http://95f3f29d.ngrok.io/sensor'
local http_endpoint='http://bluewater.k8s.hydrosphere.io/sensor'
id = id + 1
sec, usec, rate = rtctime.get()
 http.post(http_endpoint,
  'Content-Type: application/json\r\n',
  '{"id":'..id..
  ',"sec":'..sec..
  ',"usec":'..usec..
  ',"status":'..global_status..
  ',"sensor_f_sec":'..sensor_f_sec..
  ',"sensor_f_usec":'..sensor_f_usec..
  ',"sensor_s_sec":'..sensor_s_sec..
  ',"sensor_s_usec":'..sensor_s_usec..
  '}',
  function(code, data)
    if (code < 0) then
      print("HTTP request failed")
    else
      print(code, data)
    end
  end)
