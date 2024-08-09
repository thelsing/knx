#include "knx_ip_search_request_extended.h"
#include "bits.h"
#include "service_families.h"
#if KNX_SERVICE_FAMILY_CORE >= 2
#ifdef USE_IP
KnxIpSearchRequestExtended::KnxIpSearchRequestExtended(uint8_t* data, uint16_t length)
    : KnxIpFrame(data, length), _hpai(data + LEN_KNXIP_HEADER)
{
    if(length == LEN_KNXIP_HEADER + LEN_IPHPAI) return; //we dont have SRPs

    int currentPos = LEN_KNXIP_HEADER + LEN_IPHPAI;
    while(currentPos < length)
    {
        switch(data[currentPos+1])
        {
            case 0x01:
                srpByProgMode = true;
                break;

            case 0x02:
                srpByMacAddr = true;
                srpMacAddr = data + currentPos + 2;
                break;

            case 0x03:
                srpByService = true;
                srpServiceFamilies = data + currentPos;
                break;

            case 0x04:
                srpRequestDIBs = true;
                for(int i = 0; i < data[currentPos]-2; i++)
                {
                    if(data[currentPos+i+2] == 0) continue;
                    if(data[currentPos+i+2] > REQUESTED_DIBS_MAX)
                    {
                        print("Requested DIBs too high ");
                        continue;
                    }
                    requestedDIBs[data[currentPos+i+2]] = true;
                }
                break;
        }
        currentPos += data[currentPos];
    };
}

IpHostProtocolAddressInformation& KnxIpSearchRequestExtended::hpai()
{
    return _hpai;
}

bool KnxIpSearchRequestExtended::requestedDIB(uint8_t code)
{
    if(code > REQUESTED_DIBS_MAX) return false;
    return requestedDIBs[code];
}
#endif
#endif