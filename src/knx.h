#pragma once
/** \page Classdiagramm
 * This diagramm shows the most important classes.

@startuml
skinparam monochrome true
skinparam componentStyle uml2
package knx {
class BusAccessUnit [[class_bus_access_unit.html]]
class DeviceObject [[class_device_object.html]]
class BauSystemB [[class_bau_system_b.html]]
BusAccessUnit<|--BauSystemB
class ApplicationProgramObject [[class_application_program_object.html]]
BauSystemB*--ApplicationProgramObject
DeviceObject--*BauSystemB
class AddressTableObject [[class_address_table_object.html]]
BauSystemB*--AddressTableObject
class AssociationTableObject [[class_association_table_object.html]]
BauSystemB*--AssociationTableObject
class GroupObjectTableObject [[class_group_object_table_object.html]]
BauSystemB*--GroupObjectTableObject
class GroupObject [[class_group_object.html]]
GroupObject<--GroupObjectTableObject
GroupObjectTableObject<--GroupObject
class ApplicationLayer [[class_application_layer.html]]
ApplicationLayer--*BauSystemB
class TransportLayer [[class_transport_layer.html]]
TransportLayer--*BauSystemB
class NetworkLayer [[class_network_layer.html]]
NetworkLayer--*BauSystemB
class DataLinkLayer [[class_data_link_layer.html]]
DataLinkLayer--*BauSystemB
ApplicationLayer-->BusAccessUnit
ApplicationLayer-->TransportLayer
TransportLayer-->ApplicationLayer
TransportLayer-->NetworkLayer
NetworkLayer-->TransportLayer
NetworkLayer-->DataLinkLayer
DataLinkLayer-->NetworkLayer
TransportLayer-->AddressTableObject
DataLinkLayer-->AddressTableObject
DataLinkLayer-->DeviceObject
ApplicationLayer-->AssociationTableObject
class Dpt [[class_dpt.html]]
GroupObject->Dpt
package knx-ip
{
class IpDataLinkLayer [[class_ip_data_link_layer.html]]
IpDataLinkLayer--|>DataLinkLayer
class Bau57B0 [[class_bau57_b0.html]]
Bau57B0--|>BauSystemB
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
Bau07B0--|>BauSystemB
}
}

package platfom
{
class Platform [[class_platform.html]]
class SamdPlatform [[class_samd_platform.html]]
Platform<|--SamdPlatform
class EspPlatform [[class_esp_platform.html]]
Platform<|--EspPlatform
class LinuxPlatform [[class_linux_platform.html]]
LinuxPlatform--|>Platform
}
package frontend
{
class KnxFacade [[class_knx_facade.html]]
BauSystemB<--KnxFacade
}
knx-->Platform
@enduml
 */
#include "knx_facade.h"
