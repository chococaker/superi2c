/**
 * Copyright 2025 John Jerney
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#pragma once

#include "Wire.h"

extern "C" {
    #include "../aus1.h"
}

namespace superi2c {
    enum class aus1_peripheral_state {
        SENDING_DATA,
        IDLE
    };

    struct buf {
        uint8_t* data;
        size_t size;

        ~buf() {
            delete[] data;
        }
    };

    typedef buf (*provide_data_response)();

    class aus1_peripheral {
        /**
         * @brief Construct a new aus1 peripheral object
         * 
         * @param wire The I2C wire to take control of
         */
        explicit aus1_peripheral(TwoWire *wire,
                                 uint32_t peripheral_type,
                                 uint16_t peripheral_version,
                                 provide_data_response *data_response);

        /**
         * @brief Performs operations that should be called every loop
         */
        void update();

    private:
        TwoWire* wire;
        aus1_peripheral_state state;

        uint32_t peripheral_type;
        uint16_t peripheral_version;

        provide_data_response data;
        buf* data_being_sent;
        size_t data_loc;

        /**
         * @brief Transmits some data to an AUS1 device across an I2C wire
         * 
         * @param wire The wire
         * @param buf The data to write
         * @param len The length of the data
         * 
         * @return The status of the transmission
         */
        int send_transmission(uint8_t *buf, size_t len);
    }
}
