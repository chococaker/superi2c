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



#include "aus1_controller.h"

#include "../aus1.h"
#include "crc32.h"

#include <cstdint>

#define WIRE_TIMEOUT_ERR_CODE 5

namespace superi2c {
    aus1_controller::aus1_controller(TwoWire *wire)
        : wire(wire),
          state(aus1_controller_state::IDLE),
          is_connected(false),
          device_type(0),
          device_version(0),
          receiver(nullptr),
          data(nullptr),
          data_loc(0),
          last_ping_ms(0),
          last_bytes_received_ms(0) {}

    bool aus1_controller::connected() const { return is_connected; }

    void aus1_controller::set_timeout_period(unsigned long period) { this->timeout_period = period; }

    void aus1_controller::request_data(receiver_function receiver) { this->receiver = receiver; }

    aus1_controller_state aus1_controller::get_state() const { return state; }

    void aus1_controller::update() {
        unsigned long current_time = millis();

        // Write wire data
        if (wire->available()) {
            last_bytes_received_ms = millis();
        }
        while (wire->available()) {
            if (data_loc == data_buffer_size) break; // Discard overflowing buffer data
            data[data_loc++] = wire->read(); 
        }

        // Failsafe: if state is IDLE and wire is receieving data, something has gone wrong.
        // Await for the stream of data to end (hopefully it does) by waiting 10ms

        if (state == aus1_controller_state::IDLE && current_time - last_bytes_received_ms <= 10) return;

        // If the controller is not IDLE, then it must be receiving data.
        // Assume the module was disconnected if the time the last bytes were receieved exceeds the timeout
        if (state != aus1_controller_state::IDLE && current_time - last_bytes_received_ms > timeout_period) {
            state = aus1_controller_state::IDLE;
            is_connected = false;
            reset(0);
            return;
        }
        
        switch (state) {
            case aus1_controller_state::AWAITING_PING_RESPONSE:
                if (data_loc == AUS1_PING_RESPONSE_PACKET_SIZE) {
                    aus1_ping_response_packet packet = aus1_decode_ping_response(data);

                    if (packet.peripheral_type == 0) { // invalid packet
                        is_connected = false;
                        receiver = nullptr;
                        reset(0);
                    } else {
                        device_type = packet.peripheral_type;
                        device_version = packet.peripheral_version;
                        is_connected = true;
                        last_ping_ms = current_time;
                    }

                    state = aus1_controller_state::IDLE;
                }

            break;

            case aus1_controller_state::AWAITING_START_OF_STREAM:
                if (data_loc == AUS1_START_OF_STREAM_PACKET_SIZE) {
                    aus1_start_of_stream_packet packet = aus1_decode_start_of_stream(data);
                    
                    if (packet.data_size == 0) { // invalid packet
                        state = aus1_controller_state::IDLE;
                        is_connected = false;
                        receiver = nullptr;
                        reset(0);
                        break;
                    }

                    received_data_size = packet.data_size;
                    data_crc_hash = packet.crc_hash;

                    reset(packet.data_size + (packet.data_size % AUS1_DATA_PACKET_SIZE)); // set buffer to new size
                    state = aus1_controller_state::RECEIVING_DATA;
                }

            break;
            
            case aus1_controller_state::RECEIVING_DATA:
                if (data_buffer_size == data_loc) {
                    // Check CRC checksum
                    if (crc32buf(data, data_buffer_size) == data_crc_hash) {
                        receiver(data, received_data_size + (received_data_size % AUS1_DATA_PACKET_SIZE), data_buffer_size);
                    } else {
                        receiver(nullptr, 0, 0);
                    }

                    // Remove data buffer without deleting it (which is what reset() does)
                    data = nullptr;
                    data_loc = 0;
                    data_buffer_size = 0;

                    receiver = nullptr;

                    state = aus1_controller_state::IDLE;

                    break;
                }

                if ((data_loc - 1) % AUS1_DATA_PACKET_SIZE == 0) {
                    wire->requestFrom(AUS1_I2C_ADDRESS, AUS1_DATA_REQUEST_SIZE);
                }
                
            break;

            case aus1_controller_state::IDLE:
                if (receiver != nullptr) { // a data retrieval is requested
                    wire->requestFrom(AUS1_I2C_ADDRESS, AUS1_DATA_REQUEST_SIZE);
                    reset(AUS1_START_OF_STREAM_PACKET_SIZE);
                    state = aus1_controller_state::AWAITING_START_OF_STREAM;
                } else if (current_time - last_ping_ms > 20) { // interval to ping is up
                    uint8_t *packet = new uint8_t[AUS1_PING_PACKET_SIZE];
                    aus1_encode_ping(packet);

                    if (send_transmission(packet, AUS1_PING_PACKET_SIZE) == WIRE_TIMEOUT_ERR_CODE) {
                        is_connected = false;
                        break;
                    }

                    reset(AUS1_PING_RESPONSE_PACKET_SIZE);
                    state = aus1_controller_state::AWAITING_PING_RESPONSE;
                }

            break;

        }
    }

    void aus1_controller::reset(size_t new_buffer_size) {
        data_loc = 0;
        if (!data) delete[] data;
        data = new uint8_t[new_buffer_size];
        data_buffer_size = new_buffer_size;
    }

    int aus1_controller::send_transmission(uint8_t *buf, size_t len) {
        wire->beginTransmission(AUS1_I2C_ADDRESS);
        wire->write(buf, len);
        return wire->endTransmission();
    }
}
