if file.exists("init.lua") then
  file.rename("init.lua","_init.lua")
  node.restart()
elseif file.exists("_init.lua") then
  print("Really rename to init.lua? \n 10 sec. delay!")
  tmr.create():alarm(10000, 0, function()
      file.rename("_init.lua","init.lua")
      node.restart()
  end)
end