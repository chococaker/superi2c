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
#include "aus1.h"

namespace superi2c {
    /**
     * @brief Defines the state of an AUS1 controller
     */
    enum class aus1_controller_state {
        AWAITING_PING_RESPONSE,
        AWAITING_START_OF_STREAM,
        RECEIVING_DATA,
        IDLE
    };

    /**
     * @brief A function that is called when data is received by a controller
     */
    typedef void (*receiver_function)(uint8_t *buf, size_t data_size, size_t buf_size);

    class aus1_controller {
        /**
         * @brief Construct a new aus1 controller object
         * 
         * @param wire The I2C wire to take control of
         */
        explicit aus1_controller(TwoWire *wire);

        /**
         * @brief Gets the connection status of the controller wire
         *
         * 
         * @return Whether the wire is connected to an AUS1 peripheral
         */
        bool connected() const;
        
        /**
         * @brief Sets the period to wait before the peripheral is assumed to be disconnected
         * 
         * @param period The timeout period in milliseconds
         */
        void set_timeout_period(unsigned long period);

        /**
         * @brief Requests data from the peripheral
         * 
         * @param receiver The function to call when requested data is received.
         */
        void request_data(receiver_function receiver);

        /**
         * @brief Get the state object
         * 
         * @return aus1_controller_state The current state of this controller
         */
        aus1_controller_state get_state() const;

        /**
         * @brief Performs operations (reading, pinging, etc) that should be called every loop
         */
        void update();

    private:
        /**
         * @brief The wire that is being commandeered by this controller
         */
        TwoWire *wire;
        /**
         * @brief The current state of the controller
         */
        aus1_controller_state state;
        /**
         * @brief Whether a peripheral is connected
         */
        bool is_connected;
        
        /**
         * @brief The type of connected peripheral
         */
        uint32_t device_type;
        /**
         * @brief The version of the connected peripheral
         */
        uint16_t device_version;

        /**
         * @brief The function to be called when data is received after a request from a peripheral. `nullptr` when no data is being requested.
         */
        receiver_function receiver;
        /**
         * @brief The checksum CRC32 hash for the data packets
         */
        uint32_t data_crc_hash;
        /**
         * @brief Size of the data being receieved
         */
        uint16_t received_data_size;

        /**
         * @brief The data buffer
         */
        uint8_t *data;
        /**
         * @brief The size of the data
         */
        size_t data_buffer_size;
        /**
         * @brief Current location of byte writer in the data buffer
         */
        size_t data_loc;

        /**
         * @brief The time it takes for the controller to time out and assume the peripheral to be disconnected
         */
        unsigned long timeout_period;

        /**
         * @brief The previous millisecond the peripheral was pinged
         */
        unsigned long last_ping_ms;
        /**
         * @brief The last millisecond data was receieved by the controller
         */
        unsigned long last_bytes_received_ms;
        
        /**
         * @brief Deletes the data buffer, cleans up, and remakes it
         * 
         * @param new_buffer_size The size of the new data buffer
         */
        void reset(size_t new_buffer_size);
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
    };
}
