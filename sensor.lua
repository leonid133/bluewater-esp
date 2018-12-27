GPIO_PIN_F = 1
GPIO_PIN_S = 2
GPIO_PIN_G = 4

gpio.mode(GPIO_PIN_F,gpio.INT)
gpio.mode(GPIO_PIN_S,gpio.INT)
gpio.mode(GPIO_PIN_G,gpio.OUTPUT) 

sensor_f_sec = 0
sensor_f_usec = 0
sensor_s_sec = 0
sensor_s_usec = 0
global_status = 1

function interrupt_f(level, stamp)
    sensor_f_sec, sensor_f_usec = rtctime.get()
    local timerDelay = 3500
    tmr.alarm(GPIO_PIN_F, timerDelay, 1, function()
        gpio.trig(GPIO_PIN_F,"up", interrupt_f)
        tmr.stop(GPIO_PIN_F)
    end)    
end

function interrupt_s(level, stamp)
    sensor_s_sec, sensor_s_usec = rtctime.get()
    local timerDelay = 3500
    tmr.alarm(GPIO_PIN_S, timerDelay, 1, function()
        gpio.trig(GPIO_PIN_S,"up", interrupt_s)
        tmr.stop(GPIO_PIN_S)
    end)
end

gpio.trig(GPIO_PIN_F,"up", interrupt_f)
gpio.trig(GPIO_PIN_S,"up", interrupt_s)

tmr.create():alarm(1000, tmr.ALARM_AUTO, function()
    local udelta = (sensor_f_usec - sensor_s_usec)
    local neg_udelta = (sensor_s_usec - sensor_f_usec)
    if (sensor_f_sec>sensor_s_sec) then
        global_status = 0
    else 
        if (sensor_f_sec<sensor_s_sec) then
            global_status = 1
        else
            if (neg_udelta>300000) then
                global_status = 0
            else
                if (udelta>300000) then
                    global_status = 1
                end
            end 
        end
    end
    gpio.write(GPIO_PIN_G, global_status)
end)
