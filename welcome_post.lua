local http_welcome_post = "welcome_post.lua"
local http_endpoint='http://bluewater.k8s.hydrosphere.io/welcome'

 http.post(http_endpoint,
  'Content-Type: application/json\r\n',
  '{"hello":"vertuhai is ready"}',
  function(code, data)
    if (code < 0) then
      print("HTTP request failed")
      dofile(http_welcome_post)
    else
      print(code, data)
    end
  end)