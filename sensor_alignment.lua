GPIO_PIN_F = 1
GPIO_PIN_S = 2

local time = 0 
gpio.mode(GPIO_PIN_F,gpio.INT)
gpio.mode(GPIO_PIN_S,gpio.INT)

function interrupt_f(level, stamp)
gpio.trig(GPIO_PIN_F)

tmr.delay(700000)
print('level:',level)
print('stamp(us):',stamp)
print('interrupt on pin:', GPIO_PIN_F)
print('time: ', time)
time = 0

gpio.trig(GPIO_PIN_F,"both", interrupt_f)
end

function interrupt_s(level, stamp)
gpio.trig(GPIO_PIN_S)

tmr.delay(700000)
print('level:',level)
print('stamp(us):',stamp)
print('interrupt on pin:', GPIO_PIN_S)
print('time: ', time)
time = 0

gpio.trig(GPIO_PIN_S,"both", interrupt_s)
end

gpio.trig(GPIO_PIN_F,"both", interrupt_f)
gpio.trig(GPIO_PIN_S,"both", interrupt_s)

local interval = 100
tmr.create():alarm(interval, tmr.ALARM_AUTO, function()
    time = time + 1
end)