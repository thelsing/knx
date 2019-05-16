#pragma once

#include "interface_object.h"
#include "platform.h"

/**
 * @brief This class provides common functionality for interface objects that are configured by ETS with MemorWrite.
 */
class TableObject: public InterfaceObject
{
public:
    /**
     * @brief The constuctor.
     * @param platform the Platform on which the software runs. The class uses the memory management features of Platform.
     */
    TableObject(Platform& platform);
    virtual void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    virtual void writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count);
    virtual uint8_t propertySize(PropertyID id);
    /**
     * @brief The destructor.
     */
    virtual ~TableObject();
    /**
     * @brief This method returns the ::LoadState of the interface object.
     */
    LoadState loadState();
    virtual uint8_t* save(uint8_t* buffer);
    virtual uint8_t* restore(uint8_t* buffer);
protected:
    /**
     * @brief This method is called before the interface object enters a new ::Loadstate.
     * If there is a error changing the state newState should be set to ::LS_ERROR and errorCode() 
     * to a reason for the failure.
     */
    virtual void beforeStateChange(LoadState& newState) {}
    
    /**
     * @brief returns the internal data of the interface object. This pointer belongs to the TableObject class and 
     * must not be freed.
     */
    uint8_t* data();
    /**
     * @brief returns the size of the internal data of the interface object int byte.
     */
    uint32_t size();
    /**
     * @brief Set the reason for a state change failure.
     */
    void errorCode(ErrorCode errorCode);

  private:
    uint32_t tableReference();
    bool allocTable(uint32_t size, bool doFill, uint8_t fillByte);
    void loadEvent(uint8_t* data);
    void loadEventUnloaded(uint8_t* data);
    void loadEventLoading(uint8_t* data);
    void loadEventLoaded(uint8_t* data);
    void loadEventError(uint8_t* data);
    void additionalLoadControls(uint8_t* data);
    void loadState(LoadState newState);
    LoadState _state = LS_UNLOADED;
    Platform& _platform;
    uint8_t *_data = 0;
    uint32_t _size = 0;
    ErrorCode _errorCode = E_NO_FAULT;
};
