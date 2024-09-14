#pragma once

#include "../knx_types.h"

#include <stddef.h>
#include <stdint.h>

#ifndef HAS_FUNCTIONAL
    #if defined(__linux__) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_STM32) || defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_RP2040)
        #define HAS_FUNCTIONAL    1
        #include <functional>
    #else
        #define HAS_FUNCTIONAL   0
    #endif
#endif

namespace Knx
{
    class GroupObject;

#if HAS_FUNCTIONAL
    typedef std::function<void(GroupObject&)> GroupObjectUpdatedHandler;
#else
    typedef void (*GroupObjectUpdatedHandler)(GroupObject& go);
#endif

    class GroupObjectTableObject;

    enum ComFlag : uint8_t
    {
        Updated = 0,      //!< Group object was updated
        ReadRequest = 1,  //!< Read was requested but was not processed
        WriteRequest = 2, //!< Write was requested but was not processed
        Transmitting = 3, //!< Group Object is processed a the moment (read or write)
        Ok = 4,           //!< read or write request were send successfully
        Error = 5,        //!< there was an error on processing a request
        Uninitialized = 6 //!< uninitialized Group Object, its value is not valid
    };

    enum Go_SizeCode
    {
        Go_1_Bit = 0,
        Go_2_Bit = 1,
        Go_3_Bit = 2,
        Go_4_Bit = 3,
        Go_5_Bit = 4,
        Go_6_Bit = 5,
        Go_7_Bit = 6,
        Go_1_Octet = 7,
        Go_2_Octets = 8,
        Go_3_Octets = 9,
        Go_4_Octets = 10,
        Go_6_Octets = 11,
        Go_8_Octets = 12,
        Go_10_Octets = 13,
        Go_14_Octets = 14,
        Go_5_Octets = 15,
        Go_7_Octets = 16,
        Go_9_Octets = 17,
        Go_11_Octets = 18,
        Go_12_Octets = 19,
        Go_13_Octets = 20,
        Go_15_Octets = 21,
        Go_16_Octets = 22,
        Go_17_Octets = 23,
        Go_18_Octets = 24,
        Go_19_Octets = 25,
        Go_20_Octets = 26,
        Go_21_Octets = 27,
        Go_22_Octets = 28,
        Go_23_Octets = 29,
        Go_24_Octets = 30,
        Go_25_Octets = 31,
        Go_26_Octets = 32,
        Go_27_Octets = 33,
        Go_28_Octets = 34,
        Go_29_Octets = 35,
        Go_30_Octets = 36,
        Go_31_Octets = 37,
        Go_32_Octets = 38,
        Go_33_Octets = 39,
        Go_34_Octets = 40,
        Go_35_Octets = 41,
        Go_36_Octets = 42,
        Go_37_Octets = 43,
        Go_38_Octets = 44,
        Go_39_Octets = 45,
        Go_40_Octets = 46,
        Go_41_Octets = 47,
        Go_42_Octets = 48,
        Go_43_Octets = 49,
        Go_44_Octets = 50,
        Go_45_Octets = 51,
        Go_46_Octets = 52,
        Go_47_Octets = 53,
        Go_48_Octets = 54,
        Go_49_Octets = 55,
        Go_50_Octets = 56,
        Go_51_Octets = 57,
        Go_52_Octets = 58,
        Go_53_Octets = 59,
        Go_54_Octets = 60,
        Go_55_Octets = 61,
        Go_56_Octets = 62,
        Go_57_Octets = 63,
        Go_58_Octets = 64,
        Go_59_Octets = 65,
        Go_60_Octets = 66,
        Go_61_Octets = 67,
        Go_62_Octets = 68,
        Go_63_Octets = 69,
        Go_64_Octets = 70,
        Go_65_Octets = 71,
        Go_66_Octets = 72,
        Go_67_Octets = 73,
        Go_68_Octets = 74,
        Go_69_Octets = 75,
        Go_70_Octets = 76,
        Go_71_Octets = 77,
        Go_72_Octets = 78,
        Go_73_Octets = 79,
        Go_74_Octets = 80,
        Go_75_Octets = 81,
        Go_76_Octets = 82,
        Go_77_Octets = 83,
        Go_78_Octets = 84,
        Go_79_Octets = 85,
        Go_80_Octets = 86,
        Go_81_Octets = 87,
        Go_82_Octets = 88,
        Go_83_Octets = 89,
        Go_84_Octets = 90,
        Go_85_Octets = 91,
        Go_86_Octets = 92,
        Go_87_Octets = 93,
        Go_88_Octets = 94,
        Go_89_Octets = 95,
        Go_90_Octets = 96,
        Go_91_Octets = 97,
        Go_92_Octets = 98,
        Go_93_Octets = 99,
        Go_94_Octets = 100,
        Go_95_Octets = 101,
        Go_96_Octets = 102,
        Go_97_Octets = 103,
        Go_98_Octets = 104,
        Go_99_Octets = 105,
        Go_100_Octets = 106,
        Go_101_Octets = 107,
        Go_102_Octets = 108,
        Go_103_Octets = 109,
        Go_104_Octets = 110,
        Go_105_Octets = 111,
        Go_106_Octets = 112,
        Go_107_Octets = 113,
        Go_108_Octets = 114,
        Go_109_Octets = 115,
        Go_110_Octets = 116,
        Go_111_Octets = 117,
        Go_112_Octets = 118,
        Go_113_Octets = 119,
        Go_114_Octets = 120,
        Go_115_Octets = 121,
        Go_116_Octets = 122,
        Go_117_Octets = 123,
        Go_118_Octets = 124,
        Go_119_Octets = 125,
        Go_120_Octets = 126,
        Go_121_Octets = 127,
        Go_122_Octets = 128,
        Go_123_Octets = 129,
        Go_124_Octets = 130,
        Go_125_Octets = 131,
        Go_126_Octets = 132,
        Go_127_Octets = 133,
        Go_128_Octets = 134,
        Go_129_Octets = 135,
        Go_130_Octets = 136,
        Go_131_Octets = 137,
        Go_132_Octets = 138,
        Go_133_Octets = 139,
        Go_134_Octets = 140,
        Go_135_Octets = 141,
        Go_136_Octets = 142,
        Go_137_Octets = 143,
        Go_138_Octets = 144,
        Go_139_Octets = 145,
        Go_140_Octets = 146,
        Go_141_Octets = 147,
        Go_142_Octets = 148,
        Go_143_Octets = 149,
        Go_144_Octets = 150,
        Go_145_Octets = 151,
        Go_146_Octets = 152,
        Go_147_Octets = 153,
        Go_148_Octets = 154,
        Go_149_Octets = 155,
        Go_150_Octets = 156,
        Go_151_Octets = 157,
        Go_152_Octets = 158,
        Go_153_Octets = 159,
        Go_154_Octets = 160,
        Go_155_Octets = 161,
        Go_156_Octets = 162,
        Go_157_Octets = 163,
        Go_158_Octets = 164,
        Go_159_Octets = 165,
        Go_160_Octets = 166,
        Go_161_Octets = 167,
        Go_162_Octets = 168,
        Go_163_Octets = 169,
        Go_164_Octets = 170,
        Go_165_Octets = 171,
        Go_166_Octets = 172,
        Go_167_Octets = 173,
        Go_168_Octets = 174,
        Go_169_Octets = 175,
        Go_170_Octets = 176,
        Go_171_Octets = 177,
        Go_172_Octets = 178,
        Go_173_Octets = 179,
        Go_174_Octets = 180,
        Go_175_Octets = 181,
        Go_176_Octets = 182,
        Go_177_Octets = 183,
        Go_178_Octets = 184,
        Go_179_Octets = 185,
        Go_180_Octets = 186,
        Go_181_Octets = 187,
        Go_182_Octets = 188,
        Go_183_Octets = 189,
        Go_184_Octets = 190,
        Go_185_Octets = 191,
        Go_186_Octets = 192,
        Go_187_Octets = 193,
        Go_188_Octets = 194,
        Go_189_Octets = 195,
        Go_190_Octets = 196,
        Go_191_Octets = 197,
        Go_192_Octets = 198,
        Go_193_Octets = 199,
        Go_194_Octets = 200,
        Go_195_Octets = 201,
        Go_196_Octets = 202,
        Go_197_Octets = 203,
        Go_198_Octets = 204,
        Go_199_Octets = 205,
        Go_200_Octets = 206,
        Go_201_Octets = 207,
        Go_202_Octets = 208,
        Go_203_Octets = 209,
        Go_204_Octets = 210,
        Go_205_Octets = 211,
        Go_206_Octets = 212,
        Go_207_Octets = 213,
        Go_208_Octets = 214,
        Go_209_Octets = 215,
        Go_210_Octets = 216,
        Go_211_Octets = 217,
        Go_212_Octets = 218,
        Go_213_Octets = 219,
        Go_214_Octets = 220,
        Go_215_Octets = 221,
        Go_216_Octets = 222,
        Go_217_Octets = 223,
        Go_218_Octets = 224,
        Go_219_Octets = 225,
        Go_220_Octets = 226,
        Go_221_Octets = 227,
        Go_222_Octets = 228,
        Go_223_Octets = 229,
        Go_224_Octets = 230,
        Go_225_Octets = 231,
        Go_226_Octets = 232,
        Go_227_Octets = 233,
        Go_228_Octets = 234,
        Go_229_Octets = 235,
        Go_230_Octets = 236,
        Go_231_Octets = 237,
        Go_232_Octets = 238,
        Go_233_Octets = 239,
        Go_234_Octets = 240,
        Go_235_Octets = 241,
        Go_236_Octets = 242,
        Go_237_Octets = 243,
        Go_238_Octets = 244,
        Go_239_Octets = 245,
        Go_240_Octets = 246,
        Go_241_Octets = 247,
        Go_242_Octets = 248,
        Go_243_Octets = 249,
        Go_244_Octets = 250,
        Go_245_Octets = 251,
        Go_246_Octets = 252,
        Go_247_Octets = 253,
        Go_248_Octets = 254,
        Go_252_Octets = 255
    };

    /**
     * This class represents a single group object. In german they are called "Kommunikationsobjekt" or "KO".
     */
    class GroupObject
    {
            friend class GroupObjectTableObject;
            GroupObject(const GroupObject& other) = delete;
        public:
            /**
             * The constructor.
             */
            GroupObject();
            /**
             * The destructor.
             */
            virtual ~GroupObject();
            // config flags from ETS
            /**
             * Check if the update flag (U) was set. (A-flag in german)
             */
            bool responseUpdateEnable();
            /**
             * Check if the transmit flag (T) was set. (UE-flag in german)
             */
            bool transmitEnable();
            /**
             * Check if the initialisation flag (I) was set.
             */
            bool valueReadOnInit();
            /**
             * Check if the write flag (W) was set. (S-flag in german)
             */
            bool writeEnable();
            /**
             * Check if the read flag (R) was set. (L-flag in german)
             */
            bool readEnable();
            /**
             * Check if the communication flag (C) was set. (K-flag in german)
             */
            bool communicationEnable();

            /**
             * Get the priority of the group object.
             */
            Priority priority();

            /**
             * Return the current state of the group object. See ::ComFlag
             */
            ComFlag commFlag();
            /**
             * Set the current state of the group object. Application code should only use this to set the state to ::Ok after
             * reading a ::Updated to mark the changed group object as processed. This is optional.
             */
            void commFlag(ComFlag value);

            /**
             * Check if the group object contains a valid value assigned from bus or from application program
             */
            bool initialized();

            /**
            * Request the read of a communication object. Calling this function triggers the
            * sending of a read-group-value telegram, to read the value of the communication
            * object from the bus.
            *
            * When the answer is received, the communication object's value will be updated.
            *
            * This sets the state of the group objecte to ::ReadRequest
            */
            void requestObjectRead();
            /**
            * Mark a communication object as written. Calling this
            * function triggers the sending of a write-group-value telegram.
            *
            * This sets the state of the group object to ::WriteRequest
            */
            void objectWritten();

            /**
             * returns the size of the group object in Byte. For Group objects with size smaller than 1 byte (for example Dpt 1) this method
             * will return 1.
             */
            size_t valueSize();
            /**
             * returns the size of the group object in Byte as it is in a telegram. For Group objects with size smaller than 1 byte (for example Dpt 1) this method
             * will return 0.
             */
            size_t sizeInTelegram();
            /**
             * returns the pointer to the value of the group object. This can be used if a datapoint type is not supported or if you want do
             * your own conversion.
             */
            uint8_t* valueRef();
            /**
             * returns the Application Service Access Point of the group object. In reality this is just the number of the group object.
             * (in german "KO-Nr")
             */
            uint16_t asap() const;

            /**
             * returns The size of the group object encoded as the size code. See knxspec 3.5.1 p. 178.
             */
            Go_SizeCode sizeCode() const;

            /**
             * return the current value of the group object.
             * If the DPT class doesn't fit to the group object size the returned value is invalid.
             */
            template<class DPT> DPT value();
            /**
             * set the current value of the group object and changes the state of the group object to ::WriteRequest.
             * @param value the value the group object is set to.
             *
             * The parameters must fit the group object. Otherwise it will stay unchanged.
             */
            template<class DPT> void value(const DPT& value);

            /**
             * Check if the value (after conversion to dpt) will differ from current value of the group object and changes the state of the group object to ::WriteRequest if different.
             * Use this method only, when the value should not be sent if it was not changed, otherwise value(const KNXValue&, const Dpt&) will do the same (without overhead for comparing)
             * @param value the value the group object is set to
             * @param type the datapoint type used for the conversion.
             *
             * The parameters must fit the group object. Otherwise it will stay unchanged.
             *
             * @returns true if the value of the group object has changed
             */
            template<class DPT> bool valueCompare(const DPT& value);

            /**
             * set the current value of the group object.
             * @param value the value the group object is set to
             *
             * The parameters must fit size of the group object. Otherwise it will stay unchanged.
             */
            template<class DPT> void valueNoSend(const DPT& value);

            /**
             * Check if the value (after conversion to dpt) will differ from current value of the group object and update if necessary.
             * Use this method only, when the value change is relevant, otherwise valueNoSend(...) will do the same (without overhead for comparing)
             * @param value the value the group object is set to.
             *
             * The parameters must fit the size of the group object. Otherwise it will stay unchanged.
             *
             * @returns true if the value of the group object has changed
             */
            template<class DPT> bool valueNoSendCompare(const DPT& value);

            /**
             * set the current value of the group object.
             * @param value the value the group object is set to
             *
             * The parameters must fit the group object. Otherwise it will stay unchanged.
             *
             * @returns true if the value of the group object was changed successfully.
             */
            template<class DPT> bool tryValue(const DPT& value);

            /**
             * Callback processing: register one global callback for all group object.
             * The registered callback will be called if any group object was changed from the bus.
             * The callback method has to dispatch to the correct handler for this group object.
             */
            static GroupObjectUpdatedHandler classCallback();
            static void classCallback(GroupObjectUpdatedHandler handler);
            static void processClassCallback(GroupObject& ko);

        private:
            // class members
            static GroupObjectTableObject* _table;
            static GroupObjectUpdatedHandler _updateHandlerStatic;

            size_t asapValueSize(uint8_t code) const;
            size_t goSize();
            uint16_t _asap = 0;
            bool _uninitialized : 1;
            ComFlag _commFlag : 7;
            uint8_t* _data = 0;
            uint8_t _dataLength = 0;
    };

    bool operator==(const GroupObject& lhs, const GroupObject& rhs);

    template<class DPT> void GroupObject::value(const DPT& value)
    {
        valueNoSend(value);
        objectWritten();
    }

    template<class DPT> DPT GroupObject::value()
    {
        DPT dpt;

        if (dpt.size() != sizeCode())
            return dpt;

        dpt.decode(_data);
        return dpt;
    }

    template<class DPT> bool GroupObject::tryValue(const DPT& value)
    {
        if (value.size() != sizeCode())
            return false;

        return value.decode(_data);
    }

    template<class DPT> void GroupObject::valueNoSend(const DPT& value)
    {
        if (value.size() != sizeCode())
            return;

        if (_uninitialized)
            commFlag(Ok);

        value.encode(_data);
    }

    template<class DPT> bool GroupObject::valueNoSendCompare(const DPT& value)
    {
        if (value.size() != sizeCode())
            return false;

        if (_uninitialized)
        {
            // always set first value
            this->valueNoSend(value);
            return true;
        }
        else
        {
            // convert new value to given dtp
            uint8_t newData[_dataLength];
            memset(newData, 0, _dataLength);
            value.encode(newData);

            // check for change in converted value / update value on change only
            const bool dataChanged = memcmp(_data, newData, _dataLength);

            if (dataChanged)
                memcpy(_data, newData, _dataLength);

            return dataChanged;
        }
    }

    template<class DPT> bool GroupObject::valueCompare(const DPT& value)
    {
        if (valueNoSendCompare(value))
        {
            objectWritten();
            return true;
        }

        return false;
    }
}