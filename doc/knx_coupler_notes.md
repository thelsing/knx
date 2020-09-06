KNX Coupler
===========

Implementation Notes
--------------------
* Add support for coupler model 1.x and coupler model 2.0 (only used for TP1/RF coupler so far)
* currently implemented mask versions: 091A (IP/TP1 coupler) and 2920 (TP1/RF coupler)

ToDo:
-----
* class NetworkLayerCoupler: add support for all ACK modes for medium TP1, currently ALL received frames are ACK'ed (mode 2).
* handle MasterReset according to spec. for router object

Development environment
-----------------------
* see linux coupler example

