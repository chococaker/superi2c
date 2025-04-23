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



#include "aus1.h"

#pragma once

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif

// Packet IDs
#define AUS1_TYPE_PING_FIELD            0xA0
#define AUS1_TYPE_PING_RESPONSE_FIELD   0xA1
#define AUS1_TYPE_START_OF_STREAM_FIELD 0xA2

void aus1_encode_ping(uint8_t *buf) {
    buf[0] = AUS1_TYPE_PING_FIELD;
}
bool aus1_decode_ping(uint8_t *buf) {
    return buf[0] == AUS1_TYPE_PING_FIELD;
}

void aus1_encode_ping_response(uint8_t *buf, const aus1_ping_response_packet *packet) {
    buf[0] = AUS1_TYPE_PING_RESPONSE_FIELD;
    write_uint32(buf + sizeof(uint8_t) /* packet type */, packet->peripheral_type);
    write_uint16(buf + sizeof(uint8_t) /* packet type */ + sizeof(uint32_t) /* peripheral type */, packet->peripheral_version);
}
aus1_ping_response_packet aus1_decode_ping_response(uint8_t *buf) {
    aus1_ping_response_packet packet = {0};
    if (buf[0] != AUS1_TYPE_PING_RESPONSE_FIELD) return packet;

    packet.peripheral_type = read_uint32(buf + sizeof(uint8_t) /* packet type */);
    packet.peripheral_version = read_uint16(buf + sizeof(uint8_t) /* packet type */ + sizeof(uint32_t) /* peripheral type */);

    return packet;
}

void aus1_encode_start_of_stream(uint8_t *buf, const aus1_start_of_stream_packet *packet) {
    buf[0] = AUS1_TYPE_PING_RESPONSE_FIELD;
    write_uint16_raw(buf + sizeof(uint16_t) /* packet type */, packet->data_size);
    memcpy(buf + sizeof(uint8_t) /* packet type */ + sizeof(uint16_t) /* data size */, packet->crc_hash, crc_hash_SIZE);
}
aus1_start_of_stream_packet aus1_decode_start_of_stream(uint8_t *buf) {
    aus1_start_of_stream_packet packet = {0};
    if (buf[0] != AUS1_TYPE_START_OF_STREAM_FIELD) return packet;

    packet.data_size = read_uint16_raw(buf + sizeof(uint8_t) /* packet type */);
    memcpy(packet.crc_hash, buf + sizeof(uint8_t) /* packet type */ + sizeof(uint16_t) /* data size */, crc_hash_SIZE);

    return packet;
}

/**
 * @brief Writes a 16-bit unsigned host int into a network buffer
 * @note Requires the buffer to be at least 2 bytes in size
 * 
 * @param buf The buffer to write into
 * @param val The value to write into the buffer
 */
void write_uint16(uint8_t* buf, uint16_t val) {
    write_uint16_raw(buf, htons(val));
}

/**
 * @brief Writes a 16-bit unsigned int into a buffer without modifying its endianness
 * @note Requires the buffer to be at least 2 bytes in size
 * 
 * @param buf The buffer to write into
 * @param val The value to write into the buffer
 */
void write_uint16_raw(uint8_t* buf, uint16_t val) {
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)(val & 0xFF);
}

/**
 * @brief Writes a 32-bit unsigned host int into a network buffer
 * @note Requires the buffer to be at least 4 bytes in size
 * 
 * @param buf The buffer to write into
 * @param val The value to write into the buffer
 */
void write_uint32(uint8_t *buf, uint32_t val) {
    write_uint16_raw(buf, htonl(val));
}

/**
 * @brief Writes a 16-bit unsigned int into a buffer without modifying its endianness
 * @note Requires the buffer to be at least 2 bytes in size
 * 
 * @param buf The buffer to write into
 * @param val The value to write into the buffer
 */
void write_uint32_raw(uint8_t* buf, uint16_t val) {
    buf[0] = (uint8_t)(val >> 24);
    buf[1] = (uint8_t)(val >> 16);
    buf[2] = (uint8_t)(val >> 8);
    buf[3] = (uint8_t)(val & 0xFF);
}

/**
 * @brief Reads a 16-bit unsigned host int from a network buffer
 * @note Requires the buffer to be at least 2 in size
 * 
 * @param buf The buffer to read from
 * @return The host-endian read value
 */
uint16_t read_uint16(uint8_t *buf) {
    return ntohs(read_uint16_raw(buf));
}

/**
 * @brief Reads a 16-bit unsigned int from a buffer without changing its endianness
 * @note Requires the buffer to be at least 2 in size
 * 
 * @param buf The buffer to read from
 * @return The read value
 */
uint16_t read_uint16_raw(uint8_t *buf) {
    return (buf[0] << 8) | buf[1];
}

/**
 * @brief Reads a 32-bit unsigned host int from a network buffer
 * @note Requires the buffer to be at least 4 in size
 * 
 * @param buf The buffer to read from
 * @return The host-endian read value
 */
uint32_t read_uint32(uint8_t *buf) {
    return ntohl(read_uint32_raw(buf));
}

/**
 * @brief Reads a 32-bit unsigned host int from a network buffer
 * @note Requires the buffer to be at least 4 in size
 * 
 * @param buf The buffer to read from
 * @return uint32_t The host-endian read value
 */
uint32_t read_uint32_raw(uint8_t *buf) {
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

