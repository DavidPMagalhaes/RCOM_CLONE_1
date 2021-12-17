# RC_serial_port

## How to use

### Receiver mode
./app.exe \<port> [options]

### Transmitter mode
./app.exe \<port> \<filename> [options]

### Options
* -noAlarms
    * Disables alarms. Won't be timeouts. Program might malfunction
* -FER \<header> \<data>
    * header: Odds of there being an error in the header. 0 is no errors
    * data: Odds of there being an error in the data. 0 is no errors
* -TPROP \<time>
    * Added propagation time of the message (milliseconds) 
* -PACKETLOSS \<loss>
    * Odds of a frame not being transmitted (at the data link layer)