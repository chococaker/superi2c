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

#include <stdint.h>
#include <stdbool.h>

#define MD5_HASH_SIZE 16

#define AUS1_I2C_ADDRESS 0x0A

// Utility macros for allocating packets
#define AUS1_PING_PACKET_SIZE            1
#define AUS1_PING_RESPONSE_PACKET_SIZE   7
#define AUS1_START_OF_STREAM_PACKET_SIZE 19

#define AUS1_DATA_REQUEST_SIZE 19

#define AUS1_DATA_PACKET_SIZE 32

typedef struct {
    uint32_t peripheral_type;
    uint16_t peripheral_version;
} aus1_ping_response_packet;

typedef struct {
    uint16_t data_size;
    uint8_t md5_hash[MD5_HASH_SIZE];
} aus1_start_of_stream_packet;

/**
 * @brief Writes an AUS1 PING packet into a buffer
 * 
 * @param buf The buffer to write into
 */
void aus1_encode_ping(uint8_t *buf);
/**
 * @brief Decodes an AUS1 PING packet from a buffer
 * 
 * @param buf 
 * @return Whether the packet was a ping packet
 */
bool aus1_decode_ping(uint8_t *buf);

/**
 * @brief Writes an AUS1 PING RESPONSE packet into a buffer
 * 
 * @param buf The buffer to write into
 * @param packet The packet to write into the buffer
 */
void aus1_encode_ping_response(uint8_t *buf, const aus1_ping_response_packet *packet);
/**
 * @brief Decodes an AUS1 PING-RESPONSE packet from a buffer
 * 
 * @param buf 
 * @return The decoded packet
 * @return {0} If the packet was invalid
 */
aus1_ping_response_packet aus1_decode_ping_response(uint8_t *buf);

/**
 * @brief Writes an AUS1 START-OF-STREAM packet into a buffer
 * 
 * @param buf The buffer to write into
 * @param packet The packet to write into the buffer
 */
void aus1_encode_start_of_stream(uint8_t *buf, const aus1_start_of_stream_packet *packet);
/**
 * @brief Decodes an AUS1 START-OF-STREAM packet from a buffer
 * 
 * @param buf 
 * @return The decoded packet
 * @return {0} If the packet was invalid
 */
aus1_start_of_stream_packet aus1_decode_start_of_stream(uint8_t *buf);
