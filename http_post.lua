--local http_endpoint='http://95f3f29d.ngrok.io/sensor'
local http_endpoint='http://bluewater.k8s.hydrosphere.io/sensor'
id = id + 1
 http.post(http_endpoint,
  'Content-Type: application/json\r\n',
  '{"id":'..id..',"status":'..global_status..',"sensor_f":'..sensor_f..',"sensor_s":'..sensor_s..'}',
  function(code, data)
    if (code < 0) then
      print("HTTP request failed")
    else
      print(code, data)
    end
  end)
