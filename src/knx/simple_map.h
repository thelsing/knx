#pragma once

// Provides a simple unordered map which is based on two arrays of different data types, namely K and V.
// One array is used for the keys, the other array is used for the values.
// Tracking of free/occupied slots in the arrays is realized by a bitmask of size uint64_t.
// As a result the maximum size of the map is 64 entries.
// If a non-primitive data type is required for the key by using a "class" or "struct",
// then the operator== has to be provided by that class/struct:
// bool operator ==(const K&) const { return true/false; }

template <typename K, typename V, int SIZE>
class Map
{
public:
    Map()
    {
        static_assert (SIZE <= 64, "Map is too big! Max. 64 elements.");
    }

    void clear()
    {
        _validEntries = 0;
    }

    bool empty()
    {
        return (_validEntries == 0);
    }

    uint8_t size()
    {
        uint8_t size = 0;

        for (uint8_t i = 0; i < SIZE; i++)
        {
            size += (((_validEntries >> i) & 0x01) == 0x01) ? 1 : 0;
        }

        return size;
    }

    bool insert(K key, V value)
    {
        uint8_t index = getNextFreeIndex();
        if (index != noFreeEntryFoundIndex)
        {
            keys[index] = key;
            values[index] = value;

            _validEntries |= 1 << index;
            return true;
        }

        // No free space
        return false;
    }

    bool insertOrAssign(K key, V value)
    {
        // Try to find the key
        for (uint8_t i = 0; i < SIZE; i++)
        {
            // Check if this array slot is occupied
            if ((_validEntries >> i) & 0x01)
            {
                // Key found?
                if (keys[i] == key)
                {
                    values[i] = value;
                    return true;
                }
            }
        }

        // Key does not exist, add it if enough space
        return insert(key, value);
    }

    bool erase(K key)
    {
        for (uint8_t i = 0; i < SIZE; i++)
        {
            if ((_validEntries >> i) & 0x01)
            {
                if (keys[i] == key)
                {
                    _validEntries &= ~(1 << i);
                    return true;
                }
            }
        }
        return false;
    }

    V* get(K key)
    {
        // Try to find the key
        for (uint8_t i = 0; i < SIZE; i++)
        {
            // Check if this array slot is occupied
            if ((_validEntries >> i) & 0x01)
            {
                // Key found?
                if (keys[i] == key)
                {
                    return &values[i];
                }
            }
        }
        return nullptr;
    }

private:
    uint8_t getNextFreeIndex()
    {
        for (uint8_t i = 0; i < SIZE; i++)
        {
            if (((_validEntries >> i) & 0x01) == 0)
            {
                return i;
            }
        }

        return noFreeEntryFoundIndex;
    }

    uint64_t _validEntries{0};
    K keys[SIZE];
    V values[SIZE];
    static constexpr uint8_t noFreeEntryFoundIndex = 255;
};
