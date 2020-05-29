# Weewx Arduino Pool Temperature Sensor

This project uses a Wemos D1 mini + Thermistor to send temperature info about
my pool to Weewx. This is related to some other projects I've worked - [my
Arduino/Hubitat pool
controller](https://github.com/bdwilson/hubitat/tree/master/Arduino-Pool), my
[weewx hubitat driver](https://github.com/bdwilson/hubitat/tree/master/Weewx)
to get my temps into devices in Hubitat, and my [home weather
station](https://github.com/bdwilson/acurite) project. I was using an acurite
soil/water sensor for my pool, but after 2 seasons, it was toast - so instead
of investing more money in a crummy company like Acurite, I figured I'd build
something that can get the job done in-line to my pool 


# Requirements
1. [Inline, Waterproof pool temperature sensor](https://www.amazon.com/gp/product/B07QL9PLY9/). I used this exact
device, thus my arduino code won't change much if you buy this model. You
should measure the resistance of your 10k resistor as well as this at 20
celsius to better tweak your values. 
2. [Wemos D1
Mini](https://www.amazon.com/HiLetgo-Development-ESP8285-Wireless-Internet/dp/B07BK435ZW)
~ $5/each - this also assumes you have a USB cable & the arduino build
environment - optionally, you may want the
[WebOTA](https://github.com/scottchiefbaker/ESP-WebOTA) library as this will
enable remote updates via web should you wish to update your code (and for some
reason the native Arduino OTA method isn't working - it's finicky). 
3. 10k resistor 
4. [Weewx](http://www.weewx.com/). Not a requirement as the above sensor will
work for serving temperature to any device that can make a web request, but the
[PoolService](pool.py) module will enable this natively in Weewx.

# Setting it up

* Wire up your Wemos D1 Mini this way (yes, this isn't a Wemos D1 mini, but the
pins are the same - A0, 3v3, Ground and pins on your thermistor. Make sure your resistor added as well.
Since these connections are so simple, I didn't need a breadboard and just
soldered them to the board. 
<img src="https://bdwilson.github.io/images/NodeMCU_Thermistor_Interfacing_Diagram.png" width=400px>

* Connect to arduino via USB and load
[arduino_pool_sensor.ino](arduino_pool_sensor.ino). Adjust wifi info and any
sensor parameters at the top. I essentially tweaked the numbers based on
several days of comparisons with other thermometers to get it to be as accurate
as possible. YMMV. You can also adjust +/- the temp via the weewx
configuration, so it doesn't have to be perfect. You should see IP info via
Serial Console, so connect to http://your.ip.address/temp to see the temp. If
it's grossly off, check your resistor and make sure you're on 3.3v not 5. 

* Setup the weewx configuration as per the instructions on the top of
[pool.py](pool.py).  pool.py itself goes in the weewx user directory - this is
in /usr/share/weewx/user on my machine. make sure to add the PoolService
entries in weewx and enable pool.py in the data_service line - this may be
commented out, or you may be using other services - it really depends. For me,
I have two extra data services so mine looks like this as I have a
[bmp280a that I am using here](https://github.com/bdwilson/acurite) for
pressure:
<pre>
        data_services = user.bmp280a.bmp, user.pool.PoolService
</pre>

* When you're measuring an accurate temperature, install your sensor (note; if you have a
heater, make sure to install this before your water is heated, not after) and
install your Wemos D1 Mini into a water proof box - I just added it into my
[pool controller](https://github.com/bdwilson/hubitat/tree/master/Arduino-Pool)
box and shared the power with those relays.

# Questions/Bugs/Contactd Info
-----------------
Bug me on Twitter at [@brianwilson](http://twitter.com/brianwilson) or email me [here](http://cronological.com/comment.php?ref=bubba).