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



#include "aus1_peripheral.h"

#include "../aus1.h"
#include "../util/crc32.h"

#include <cstdint>

#define WIRE_TIMEOUT_ERR_CODE 5

namespace superi2c {
    aus1_peripheral::aus1_peripheral(TwoWire *wire,
                                     uint32_t peripheral_type,
                                     uint16_t peripheral_version,
                                     provide_data_response *data_response)
        : wire(wire),
          state(aus1_peripheral_state::IDLE),
          peripheral_type(peripheral_type),
          peripheral_version(peripheral_version) {
        
        wire->onRequest([peripheral_type, peripheral_version, this]() { // presumably a data request
            data_being_sent = new buf(data());

            uint32_t hash = crc32buf(data_being_sent->data, data_being_sent->size);

            aus1_start_of_stream_packet packet = { data_being_sent->size, hash };
            uint8_t buf[] = new uint8_t[AUS1_START_OF_STREAM_PACKET_SIZE];
            aus1_encode_start_of_stream(buf, &packet);
            this->send_transmission(buf, AUS1_START_OF_STREAM_PACKET_SIZE);

            delete[] buf;
        });
    }

    void aus1_peripheral::update() {
        if (data_being_sent) { // currently sending data
            if (data_loc < data_being_sent->size - 32) { // last segment of data, need a special size for the buffer
                send_transmission(data_being_sent->data + data_loc, data_being_sent->size - data_loc - 32);
                delete data_being_sent;
                data_being_sent = nullptr;
                data_loc = 0;
            } else { // middle of the packet, can send full 32 bytes safely
                send_transmission(data_being_sent->data + data_loc, AUS1_DATA_PACKET_SIZE);
                data_loc += AUS1_DATA_PACKET_SIZE;
            }
        } else if (wire->available() == AUS1_PING_PACKET_SIZE) { // currently receiving data
            // decode ping packet
            uint8_t ping_byte = (uint8_t) wire->read();
            if (!aus1_decode_ping(&ping_byte)) {
                return;
            }

            // send packet
            aus1_ping_response_packet packet = { peripheral_type, peripheral_version };
            uint8_t *bres = new uint8_t[AUS1_PING_RESPONSE_PACKET_SIZE];
            aus1_encode_ping_response(bres, &packet);
            this->send_transmission(bres, AUS1_PING_RESPONSE_PACKET_SIZE);

            delete[] bres;
        } else { // more data in wire than there should be
            // clear buffer; there shouldn't be data besides a one-byte ping packet
            while (wire->available()) {
                wire->read();
            }
        }
    }
    
    int aus1_peripheral::send_transmission(uint8_t *buf, size_t len) {
        wire->beginTransmission(AUS1_I2C_ADDRESS);
        wire->write(buf, len);
        return wire->endTransmission();
    }
}
