#pragma once
#include <stdint.h>

/**
 * Interface for classes that can save and restore data from a buffer. 
 */
class SaveRestore
{
  public:
    /**
     * This method is called when the object should save its state.
     */
    virtual void save() = 0;
    /**
     * This method is called when the object should restore its state from the given address.
     *  
     * @param startAddr The startAddr the object should restore its state from.
     * 
     */
    virtual void restore(uint8_t* startAddr) = 0;
    virtual uint32_t size() = 0;
    void memoryID (uint32_t ID){_ID = ID;}
  protected:
    uint32_t _ID;
};
