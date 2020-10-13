#pragma once
/** \page Classdiagramm KNX device
 * This diagramm shows the most important classes of a normal KNX device.

@startuml

skinparam monochrome true
skinparam componentStyle uml2

note top of knx
  Class diagram for a
  <b><color:royalBlue>normal KNX device</color>
end note
package "knx" {
class BusAccessUnit [[class_bus_access_unit.html]]
class DeviceObject [[class_device_object.html]]
class BauSystemBDevice [[class_bau_system_b.html]]
BusAccessUnit<|--BauSystemB
BauSystemB<|--BauSystemBDevice
class ApplicationProgramObject [[class_application_program_object.html]]
BauSystemB*--ApplicationProgramObject
DeviceObject--*BauSystemB
class AddressTableObject [[class_address_table_object.html]]
BauSystemBDevice*--AddressTableObject
class AssociationTableObject [[class_association_table_object.html]]
BauSystemBDevice*--AssociationTableObject
class GroupObjectTableObject [[class_group_object_table_object.html]]
BauSystemBDevice*--GroupObjectTableObject
class GroupObject [[class_group_object.html]]
GroupObject<--GroupObjectTableObject
GroupObjectTableObject<--GroupObject
class ApplicationLayer [[class_application_layer.html]]

package knx-data-secure
{
class SecureApplicationLayer [[class_secure_application_layer.html]]
class SecurityInterfaceObject [[class_security_interface_object.html]]
SecureApplicationLayer-->SecurityInterfaceObject
SecurityInterfaceObject-->SecureApplicationLayer
BauSystemBDevice*--"SecurityInterfaceObject"
BauSystemBDevice*--"SecureApplicationLayer"
}
BauSystemBDevice*--"ApplicationLayer"
ApplicationLayer<|--SecureApplicationLayer
SecureApplicationLayer--*BauSystemBDevice
class TransportLayer [[class_transport_layer.html]]
TransportLayer--*BauSystemBDevice
class NetworkLayerDevice [[class_network_layer.html]]
NetworkLayerDevice--*BauSystemBDevice
class NetworkLayerEntity [[class_network_layer_entity.html]]
NetworkLayerEntity--*NetworkLayerDevice
class DataLinkLayer [[class_data_link_layer.html]]
ApplicationLayer-->SecureApplicationLayer
SecureApplicationLayer-->ApplicationLayer
ApplicationLayer-->BusAccessUnit
ApplicationLayer-->TransportLayer
SecureApplicationLayer-->TransportLayer
TransportLayer-->ApplicationLayer
TransportLayer-->SecureApplicationLayer
TransportLayer-->NetworkLayerDevice
NetworkLayerDevice-->TransportLayer
NetworkLayerEntity-->DataLinkLayer
DataLinkLayer-->NetworkLayerEntity
TransportLayer-->AddressTableObject
SecureApplicationLayer-->AddressTableObject
SecureApplicationLayer-->DeviceObject
DataLinkLayer-->DeviceObject
ApplicationLayer-->AssociationTableObject
class Dpt [[class_dpt.html]]
GroupObject->Dpt

package knx-ip
{
class IpDataLinkLayer [[class_ip_data_link_layer.html]]
IpDataLinkLayer--|>DataLinkLayer
class Bau57B0 [[class_bau57_b0.html]]
Bau57B0--|>BauSystemBDevice
Bau57B0*--IpDataLinkLayer
class IpParameterObject [[class_ip_parameter_object.html]]
IpParameterObject-->DeviceObject
Bau57B0*--IpParameterObject
IpDataLinkLayer-->IpParameterObject
}
package knx-tp
{
class TpUartDataLinkLayer [[class_tp_uart_data_link_layer.html]]
TpUartDataLinkLayer--|>DataLinkLayer
class Bau07B0 [[class_bau07_b0.html]]
Bau07B0*--TpUartDataLinkLayer
Bau07B0--|>BauSystemBDevice
}
package knx-rf
{
class RfDataLinkLayer [[class_rf_data_link_layer.html]]
RfDataLinkLayer--|>DataLinkLayer
class Bau27B0 [[class_bau27_b0.html]]
Bau27B0*--"RfDataLinkLayer"
Bau27B0--|>BauSystemBDevice
class RfMediumObject [[class_rf_medium_object.html]]
Bau27B0*--"RfMediumObject"
class RfPhysicalLayer [[class_rf_physical_layer.html]]
"RfDataLinkLayer"*--"RfPhysicalLayer"
}
package knx-cemi-server
{
class CemiServer [[class_cemi_server.html]]
class CemiServerObject [[class_cemi_server_object.html]]
class UsbTunnelInterface [[class_usb_tunnel_inerface.html]]
CemiServer*--"UsbTunnelInterface"
Bau57B0*--"CemiServer"
Bau57B0*--"CemiServerObject"
Bau27B0*--"CemiServer"
Bau27B0*--"CemiServerObject"
Bau07B0*--"CemiServer"
Bau07B0*--"CemiServerObject"
}
CemiServer-->DataLinkLayer
DataLinkLayer-->CemiServer
}

package platform
{
class Platform [[class_platform.html]]
class ArduinoPlatform [[class_arduino_platform.html]]
class SamdPlatform [[class_samd_platform.html]]
Platform<|--ArduinoPlatform
ArduinoPlatform<|--SamdPlatform
class EspPlatform [[class_esp_platform.html]]
ArduinoPlatform<|--EspPlatform
class Esp32Platform [[class_esp32_platform.html]]
ArduinoPlatform<|--Esp32Platform
class Stm32Platform [[class_stm32_platform.html]]
ArduinoPlatform<|--Stm32Platform
class LinuxPlatform [[class_linux_platform.html]]
LinuxPlatform--|>Platform
}
package frontend
{
class KnxFacade [[class_knx_facade.html]]
BauSystemBDevice<--KnxFacade
}
knx-->Platform
@enduml

* \page Classdiagramm KNX coupler
 * This diagramm shows the most important classes of a KNX coupler.
@startuml

skinparam monochrome true
skinparam componentStyle uml2

note top of knx
  Class diagram for a
  <b><color:royalBlue>KNX coupler</color>
end note
package "knx" {
class BusAccessUnit [[class_bus_access_unit.html]]
class DeviceObject [[class_device_object.html]]
class BauSystemBCoupler [[class_bau_system_b.html]]
BusAccessUnit<|--BauSystemB
BauSystemB<|--BauSystemBCoupler
class ApplicationProgramObject [[class_application_program_object.html]]
BauSystemB*--ApplicationProgramObject
DeviceObject--*BauSystemB
class ApplicationLayer [[class_application_layer.html]]

package knx-data-secure
{
class SecureApplicationLayer [[class_secure_application_layer.html]]
class SecurityInterfaceObject [[class_security_interface_object.html]]
SecureApplicationLayer-->SecurityInterfaceObject
SecurityInterfaceObject-->SecureApplicationLayer
BauSystemBCoupler*--"SecurityInterfaceObject"
BauSystemBCoupler*--"SecureApplicationLayer"
}
BauSystemBCoupler*--"ApplicationLayer"
ApplicationLayer<|--SecureApplicationLayer
SecureApplicationLayer--*BauSystemBCoupler
class TransportLayer [[class_transport_layer.html]]
TransportLayer--*BauSystemBCoupler
class NetworkLayerEntity [[class_network_layer_entity.html]]
class NetworkLayerCoupler [[class_network_layer.html]]
{
NetworkLayerEntity[] _networkLayerEntities
}
NetworkLayerCoupler*--"NetworkLayerEntity"
NetworkLayerCoupler--*BauSystemBCoupler
class DataLinkLayer [[class_data_link_layer.html]]
ApplicationLayer-->SecureApplicationLayer
SecureApplicationLayer-->ApplicationLayer
ApplicationLayer-->BusAccessUnit
ApplicationLayer-->TransportLayer
SecureApplicationLayer-->TransportLayer
TransportLayer-->ApplicationLayer
TransportLayer-->SecureApplicationLayer
TransportLayer-->NetworkLayerCoupler
NetworkLayerCoupler-->TransportLayer
NetworkLayerEntity-->DataLinkLayer
DataLinkLayer-->NetworkLayerEntity
SecureApplicationLayer-->DeviceObject
DataLinkLayer-->DeviceObject

package knx-ip-tp
{
class IpDataLinkLayer [[class_ip_data_link_layer.html]]
IpDataLinkLayer--|>DataLinkLayer
class TpUartDataLinkLayer [[class_tp_uart_data_link_layer.html]]
TpUartDataLinkLayer--|>DataLinkLayer
class Bau091A [[class_bau09_1a.html]]
Bau091A--|>BauSystemBCoupler
Bau091A*--IpDataLinkLayer
Bau091A*--TpUartDataLinkLayer
class RouterObject [[class_router_object.html]]
class IpParameterObject [[class_ip_parameter_object.html]]
IpParameterObject-->DeviceObject
Bau091A*--"RouterObject"
Bau091A*--IpParameterObject
IpDataLinkLayer-->IpParameterObject
}
}

package platform
{
class Platform [[class_platform.html]]
class ArduinoPlatform [[class_arduino_platform.html]]
class SamdPlatform [[class_samd_platform.html]]
Platform<|--ArduinoPlatform
ArduinoPlatform<|--SamdPlatform
class EspPlatform [[class_esp_platform.html]]
ArduinoPlatform<|--EspPlatform
class Esp32Platform [[class_esp32_platform.html]]
ArduinoPlatform<|--Esp32Platform
class Stm32Platform [[class_stm32_platform.html]]
ArduinoPlatform<|--Stm32Platform
class LinuxPlatform [[class_linux_platform.html]]
LinuxPlatform--|>Platform
}
package frontend
{
class KnxFacade [[class_knx_facade.html]]
BauSystemBCoupler<--KnxFacade
}
knx-->Platform
@enduml

 */
#include "knx_facade.h"
