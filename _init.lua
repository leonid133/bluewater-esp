local runfile = "setglobals.lua"
tmr.create():alarm(15000, 0, function()
  if file.exists(runfile) then
      dofile(runfile)
  else
      print("No ".. runfile..", Rename init.lua!")
      if file.exists("init.lua") then
          file.rename("init.lua","_init.lua")
          node.restart()
      end
  end
end)