#pragma once

#include "interface_object.h"

class Memory;

typedef void (*BeforeTablesUnloadCallback)();

/**
 * This class provides common functionality for interface objects that are configured by ETS with MemorWrite.
 */
class TableObject: public InterfaceObject
{
    friend class Memory;

  public:
    /**
     * The constuctor.
     * @param memory The instance of the memory management class to use.
     */
    TableObject(Memory& memory, uint32_t staticTableAdr = 0, uint32_t staticTableSize = 0);

    /**
     * The destructor.
     */
    virtual ~TableObject();
    /**
     * This method returns the ::LoadState of the interface object.
     */
    LoadState loadState();
    uint8_t* save(uint8_t* buffer) override;
    const uint8_t* restore(const uint8_t* buffer) override;
    uint16_t saveSize() override;

    static void beforeTablesUnloadCallback(BeforeTablesUnloadCallback func);
    static BeforeTablesUnloadCallback beforeTablesUnloadCallback();

  protected:
    /**
     * This method is called before the interface object enters a new ::LoadState.
     * If there is a error changing the state newState should be set to ::LS_ERROR and errorCode() 
     * to a reason for the failure.
     */
    virtual void beforeStateChange(LoadState& newState);

    /**
     * returns the internal data of the interface object. This pointer belongs to the TableObject class and 
     * must not be written at nor freed.
     */
    uint8_t* data();
    /**
     * Set the reason for a state change failure.
     */
    void errorCode(ErrorCode errorCode);

    void initializeProperties(size_t propertiesSize, Property** properties) override;

    static BeforeTablesUnloadCallback _beforeTablesUnload;

    Memory& _memory;

  private:
    uint32_t tableReference();
    bool allocTable(uint32_t size, bool doFill, uint8_t fillByte);
    void allocTableStatic();
    void initializeDynTableProperties(size_t propertiesSize, Property** properties);
    void loadEvent(const uint8_t* data);
    void loadEventUnloaded(const uint8_t* data);
    void loadEventLoading(const uint8_t* data);
    void loadEventLoaded(const uint8_t* data);
    void loadEventError(const uint8_t* data);
    void additionalLoadControls(const uint8_t* data);
    
    /**
     * set the ::LoadState of the interface object.
     * 
     * Calls beforeStateChange().
     * 
     * @param newState the new ::LoadState 
     */
    void loadState(LoadState newState);
    LoadState _state = LS_UNLOADED;
    uint8_t *_data = 0;
    static uint8_t _tableUnloadCount;
    uint32_t _staticTableAdr;
    uint32_t _staticTableSize;

    /**
     * used to store size of data() in allocTable(), needed for calculation of crc in PID_MCB_TABLE.
     * This value is also saved and restored.
     * The size of the memory block cannot be used because it is changed during alignment to page size.
     */
    uint32_t _size = 0;
};
