# AUS1 Packet Format

A simple format for **one-to-one controller-to-peripheral** communication, built on top of the I2C protocol (7-8 bit).

The AUS1 format enables a controller to search for, identify, and request data from, a peripheral.

## Communication

### Ping/Discovery

At an interval, the controller will send out a `PING` packet (while a transmission is not in progress) to `0x0A`, an address meant for devices using the AUS1 format. The peripheral device (if it exists) is expected to respond with a `PING-RESPONSE` packet, which contains information about the device.

If the controller does not receive a `PING-RESPONSE` packet, it should assume that there is no AUS1 peripheral on the other side of the connection.

**`PING` packet** (1 byte):

| Field         | Length   | Description              |
|---------------|----------|--------------------------|
| Packet Type   | 1 byte   | `0xA0` for AUS1 `PING`   |

**`PING-RESPONSE` packet** (7 bytes):

| Field                | Length    | Description                                              |
|----------------------|-----------|----------------------------------------------------------|
| Packet Type          | 1 byte    | `0xA1` for AUS1 `PING-RESPONSE`                          |
| Peripheral Type      | 4 bytes   | A numeric ID for the type of device the peripheral       |
| Peripheral Version   | 2 bytes   | A numeric ID for the current version of the peripheral   |

### Retreiving Data from Peripheral

If the peripheral is known to exist, controller may now ask for data from a peripheral. The data can be of any size up to 65535, as it is split into 32-byte chunks by the peripheral.

This interaction begins with the controller sending an I2C request of 19 bytes.

The peripheral responds with a start-of-stream packet indicating the size of the payload it is about to send.

**`START-OF-STREAM` Packet** (7 bytes):

| Field            | Length    | Description                      |
|------------------|-----------|----------------------------------|
| Packet Type      | 1 byte    | `0xA2` for AUS1 Data Reponse     |
| Data size        | 2 bytes   | Size of data buffer (in bytes)   |
| CRC32 Checksum   | 4 bytes   | Checksum for data                |

Once this packet is receieved by the controller, it will continuously send 32-byte I2C requests which should be responded by chunks of data starting from the top of the buffer.

If the data size is not divisble by 32, excess bytes the peripheral should pad the last bytes of the final packet, which the controller should discard.

Once the bytes have been received by the controller and have been properly written, the controller may send another data request.

Any packet that fails the checksum should be discarded.

## Failsafes

In the event that a peripheral fails to finish sending its byte buffer, the controller should wait .5 seconds before assuming that the peripheral has been disconnected, and should resume sending out `PING` packets.

Additionally, when a transmission is not occurring, the controller should be regularly sending out `PING` packets to make sure that the peripheral has not changed or disconnected.
