KNX Data Secure
===============

Implementation Notes
--------------------
* Implementation based on application note AN158 v07 KNX Data Security 
* AES-128 is implemented in software, no hardware acceleration used currently
* Secure device setup with ETS-5.7.x tested and working
* Secure group communication needs more testing
* Support for FDSK generation
* Support for P2P mode prepared
* No support for LTE-mode[T_Data_Tag_Group] (zone key table is already there) currently
* No support roles (and no plan to implement this in the near future)

ToDo:
-----
* Add support for AN192 v04 Coupler security extensions (a.k.a. Secure Proxy which translates between unsecured and secured devices)
* Handle S-A_Sync Service when initially the last valid sequence nummer is not known during runtime, i.e. group communication
* handle MasterReset according to spec. for security interface object

Development environment
-----------------------
* see linux example on how to generate the FDSK string which needs to be entered in the ETS
* use BAU57B0, but fake the mask by setting _deviceObj.maskVersion(0x07B0). This "emulates" a TP1 device which is reachable over an IP router
* To generate a KNX ETS product database with support for KNX Data Secure, use the latest version of the CreateKnxProd tool which supports schema version 20.
