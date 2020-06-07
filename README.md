# Wireless-Fencing-Box
Arduino based Fencing box with wireless capabilities. Based on NRF24 communication between the box and the and the two carry modules. 
With RF433 transmitter, reciever pair for detection of guard hits by making them work so badly as so to only allow transmittion on direct connection.  




Parts List:

- Arduino x3
- RF433MHz transmitter reciever pair x2
- Battery NCR18659B x3
- Battery protection and charging cirquit TP4056 x3
- NRF24L01 transmiter x2
- NRF24L01+ transmiter x1
- voltage conveter for NRF24L01 module x3
- powerbank case (size 75x110x22mm) x3
- power switch x3
- LED indication lights x6 (or more)

Optional:

- buzzer  x1
- clip x2


Development Notes:

- Guard must be connected to transsmitter to prevent it as acting as part of a capacitor which charges up from one of the cables set to INPUT_PULLUP leading to its trigger when the guard touches a metal surface hence registering a false hit.
- Serial.print although important for debbugging must not be used for prper testing it slows everything down to much(from 1024 micro secondd for hit cycle to 12000!).
