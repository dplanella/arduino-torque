# arduino-torque

## Protocol: serial communication trace 

This trace shows the commands sent by the torque app (preceded by a prompt character `>`) and the response from the Arduino. Essentially, there is a one-off handshake protocol, in which the Torque app sends a series of commands (AT and custom ones) to the Arduino, which in turn sends a reply, followed by a loop where Torque requests the sensor values with the `G` command, and Arduino sends the values:

```
>ATZ
Torque Protocol Interface v0.0.1
OK
>ATE0
OK
>ATM0
OK
>ATL0
OK
>ATST62
OK
>ATS0
OK
>AT@1
Torque For Android Protocol Interface
OK
>ATI
Torque Protocol Interface v0.0.1
OK
>GETCONFIGURATION
NO_CAR_SENSORS,NO_DEVICE_SENSORS
>GETDEFINITIONS
00aaii--Pot 1Pot 1Potentiometer 1Potentiometer 1vv0055
11aaii--LDR 1LDR 1Light Sensor 1Light Sensor 1vv0055
44ddoo00Dout1Dout1Digital Out 1Digital Out 1bitbit001

>G
0:a:484
1:a:404

>G
0:a:421
1:a:406

```
