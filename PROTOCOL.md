# Romi Control Protocol

The Romi uses a websocket to communicate all of these packets. The address of the websocket 
is `ws://{ROMI_ADDRESS}/test` where `ROMI_ADDRESS` depends on the connection method. It is 
`192.168.4.1` when running in AP mode.

## Data Types
All data sent or received should be [Little Endian](https://en.wikipedia.org/wiki/Endianness).

| Name         	| Size (bytes) 	|
|--------------	|--------------	|
| Unsigned Int 	| 4            	|
| Float        	| 4            	|
| String       	| >4           	|
| Bytes         | 0+            |

### Strings
Strings are encoded as an int representing the length followed 
by the character data.

## Server(Romi)bound

### Joystick Update
Updates the position of the "virtual joystick" in the Romi. The position determines movement
of the Romi.

The web client implementation is to dispatch this packet a maximum of 1000 times per second. 
The packet is _not_ sent if the position has not changed.

Note: The X and Y positions in the packet are ignored by the Romi. The web client always sets them to 1.0.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th>Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td rowspan="4">0x20</td>
    <td>Position X</td>
    <td>Float</td>
    <td>0.0 - 1.0</td>
  </tr>
  <tr>
    <td>Position Y</td>
    <td>Float</td>
    <td>0.0 - 1.0</td>
  </tr>
  <tr>
    <td>Angle</td>
    <td>Float</td>
    <td>Polar angle, in radians.</td>
  </tr>
  <tr>
    <td>Magnitude</td>
    <td>Float</td>
    <td>Polar magnitude, 0.0 - 1.0</td>
  </tr>
</tbody>
</table>

### Slider Update
Updates the position of a slider with the provided ID.

The web client implementation always sends ID 0.
The Romi has 4 slider slots, 0 corresponds to the lifting arm.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th>Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td rowspan="2">0x30</td>
    <td>Slider Number</td>
    <td>Unsigned Int</td>
    <td>The slider to update</td>
  </tr>
  <tr>
    <td>Value</td>
    <td>Float</td>
    <td>0.0 - 1.0</td>
  </tr>
</tbody>
</table>

### Button Update
Updates the button state for the given ID.

The web client implementation does not dispatch this packet.
The Romi does nothing when it receives this packet.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th>Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td rowspan="2">0x40</td>
    <td>Button ID</td>
    <td>Unsigned Int</td>
    <td>The button to update</td>
  </tr>
  <tr>
    <td>State</td>
    <td>Unsigned Int</td>
    <td>0 is up, 1 is down</td>
  </tr>
</tbody>
</table>

### Heartbeat
Keeps the websocket alive.

The web client implementation sends a heartbeat every 1 second.
The Romi keeps track of the latest uuid sent by the client.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th>Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td>0x50</td>
    <td>Random "UUID"</td>
    <td>Unsigned Int</td>
    <td>A random integer, cannot be zero.</td>
  </tr>
</tbody>
</table>

## Clientbound

### Value Update (Single)
This packet is designed to update a single data value on the client.

Note: This packet is not implemented by the web client, and it is never dispatched by the Romi.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th>Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td rowspan=2>0x10</td>
    <td>Index</td>
    <td>Unsigned Int</td>
    <td>The index to update.</td>
  </tr>
  <tr>
    <td>Data</td>
    <td>Unsigned Int | Float</td>
    <td>The new data.</td>
  </tr>
</tbody>
</table>

### PID Values Update
Updates the client display for PID values. These are separate from the dynamic values.

The web client implementation is to log the values to the console.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th>Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td rowspan="4">0x60</td>
    <td>PID Channel</td>
    <td>Unsigned Int</td>
    <td>The motor relevant to the PID values.</td>
  </tr>
  <tr>
    <td>P</td>
    <td>Float</td>
    <td></td>
  </tr>
  <tr>
    <td>I</td>
    <td>Float</td>
    <td></td>
  </tr>
  <tr>
    <td>D</td>
    <td>Float</td>
    <td></td>
  </tr>
</tbody>
</table>

### Heartbeat
Keeps the websocket alive, and proves that the Romi is still active.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th>Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td>0x50</td>
    <td>Random "UUID"</td>
    <td>Unsigned Int</td>
    <td>A random integer, cannot be zero.</td>
  </tr>
</tbody>
</table>

### Console Data
Used to send an arbitrary log string to the client.

The web client implementation is to `console.log` the data.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th>Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td>0x11</td>
    <td>Data</td>
    <td>String</td>
    <td>See String encoding above.</td>
  </tr>
</tbody>
</table>

### Bulk Label Update
Updates or creates a variable number of value indices in the client value dictionary.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th colspan="2">Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td rowspan="6">0x1d</td>
    <td colspan="2">Number of labels (n)</td>
    <td>Unsigned Int</td>
    <td></td>
  </tr>
  <tr>
    <td colspan="2">Start of string data</td>
    <td>Unsigned Int</td>
    <td>The start position of the Raw String Data, relative to this position, AKA n * 12.</td>
  </tr>
  <tr>
    <td rowspan="3">Once for each n</td>
    <td>Index</td>
    <td>Unsigned Int</td>
    <td>The label index to update.</td>
  </tr>
  <tr>
    <td>String Offset</td>
    <td>Unsigned Int</td>
    <td>The start position of the String, relative to the start of the Raw String Data.</td>
  </tr>
  <tr>
    <td>String Length</td>
    <td>Unsigned Int</td>
    <td></td>
  </tr>
  <tr>
    <td colspan="2">Raw String Data</td>
    <td>Bytes</td>
    <td>All of the labels concatenated.</td>
  </tr>
</tbody>
</table>

### Bulk Value Update
Updates a variable number of values in the client value dictionary.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th colspan="2">Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td rowspan="3">0x1e</td>
    <td colspan="2">Number of updates (n)</td>
    <td>Unsigned Int</td>
    <td>The number (n) of updates to perform.</td>
  </tr>
  <tr>
    <td rowspan="2">Once for each n.</td>
    <td>Index</td>
    <td>Unsigned Int</td>
    <td></td>
  </tr>
  <tr>
    <td>Value</td>
    <td>Unsigned Int</td>
    <td>The new value for the index.</td>
  </tr>
</tbody>
</table>

### New Value Index
Creates a new value index in the client value dictionary.

Note: This packet is not implemented by the web client, or dispatched by the Romi.

<table>
<thead>
  <tr>
    <th>Packet ID</th>
    <th>Field Name</th>
    <th>Field Type</th>
    <th>Notes</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td rowspan="2">0x1F</td>
    <td>Index</td>
    <td>Unsigned Int</td>
    <td></td>
  </tr>
  <tr>
    <td>Name</td>
    <td>String</td>
    <td>See String encoding above.</td>
  </tr>
</tbody>
</table>

