#pragma once
#include <stdint.h>

/**
 * Interface for classes that can save and restore data from a buffer. 
 */
class SaveRestore
{
  public:
    /**
     * This method is called when the object should save its state to the buffer.
     *  
     * @param buffer The buffer the object should save its state to.
     * 
     * @return The buffer plus the size of the object state. The next object will use this value as 
     * the start of its buffer.
     */
    virtual uint8_t* save(uint8_t* buffer) = 0;
    /**
     * This method is called when the object should restore its state from the buffer.
     *  
     * @param buffer The buffer the object should restore its state from.
     * 
     * @return The buffer plus the size of the object state. The next object will use this value as 
     * the start of its buffer.
     */
    virtual uint8_t* restore(uint8_t* buffer) = 0;
};