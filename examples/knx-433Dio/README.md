Control Chacon/DIO 433Mhz plug like that: (not an affiliated link just for info)
https://www.amazon.fr/DiO-Connected-Home-t%C3%A9l%C3%A9command%C3%A9es-t%C3%A9l%C3%A9commande/dp/B005LKMAW0


Hardware:
- Samd21 (gy-samd21)
- FS 1000A (cheap 433 transmitter)
- TpUart KNX
- logic level converter
- Chacon/DIO 433Mhz plug (must work with every dumb 433mhz plug/remote that support the HomeEasy protocol)

Software
- change the rfPin variable, compile and tranfert into the samd21 (or any microcontroller)
- (maybe) adapt THIGH and TLOW in rfCode class for your case.

Before configuring ETS, you need to receive the 433mhz code for each button of your remote.
Take an arduino UNO, plug the FS1000a receiver, look on github/google to receive the code rc-switch or https://charleslabs.fr/fr/project-Contr%C3%B4le+de+prises+DiO+avec+Arduino (in french), or https://caron.ws/diy-cartes-microcontroleurs/piloter-vos-prises-chacon-rf433mhz-arduino/ ( in french too)


for me it's : 1806947472 and 1806947456 for the channel 1 on/off, etc...

Now in ETS put the received 433Mhz code into parameters.
-> Full Download in ETS and enjoy.



Feature:
This code is delay() free, blocking code free (ie: no long while or for loop), to call the knx.loop() as fast as possible.




