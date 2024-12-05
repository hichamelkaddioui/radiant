# SysEx messages

## Message structure

`<SysEx start> <Manufacturer ID> <Message ID> <Message payload> <SysEx end>`

> Data is sent in bytes, bit #7 is always 0, so each byte is a 7-bit word. Hence **the max value is `0b01111111 = 0x7f = 127`**. In this document, a 7-bit word is called a **byte**.

|                              | Name            | Value              | Min | Max  |
| ---------------------------- | --------------- | ------------------ | --- | ---- |
| <span class="start">■</span> | SysEx start     | `0xf0`             |     |      |
| <span class="start">■</span> | Manufacturer ID | `0x7e` (universal) |     |      |
| <span class="first">■</span> | Message ID      | 1 byte             | `0` | `10` |
|                              | Message payload | N bytes            |     |      |
| <span class="end">■</span>   | SysEx end       | `0xf7`             |     |      |

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">...</span>
<span class="payload-1">...</span>
<span class="payload-2">...</span>
<span class="payload-3">...</span>
<span class="payload-4">...</span>
<span class="payload-5">...</span>
<span class="payload-6">...</span>
<span class="payload-7">...</span>
<span class="payload-8">...</span>
<span class="end">0xf7</span>
</p>

## Messages list

| Message ID | Message Name     | Message payload               |
| ---------- | ---------------- | ----------------------------- |
| `1`        | Set params       | General parameters            |
| `2`        | Create light     | Light id                      |
| `3`        | Create scene     | Scene id                      |
| `4`        | Create graph     | Scene id, graph id, keyframes |
| `5`        | Set hue A        | Scene id, light id, params    |
| `6`        | Set brightness A | Scene id, light id, params    |
| `7`        | Set hue B        | Scene id, light id, params    |
| `8`        | Set brightness B | Scene id, light id, params    |
| `9`        | Set strobe A     | Scene id, light id, params    |
| `10`       | Set strobe B     | Scene id, light id, params    |

## Message payload

### Set params

|                                  | Parameter          | Value  | Min | Max   |
| -------------------------------- | ------------------ | ------ | --- | ----- |
| <span class="first">■</span>     | Message ID         | `1`    |
| <span class="payload-1">■</span> | A / B control note | 1 byte | `0` | `127` |

**Set A / B control note to `0x3e` = 62 (D4)**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x01</span>
<span class="payload-1">0x3e</span>
<span class="end">0xf7</span>
</p>

### Create light

#### LED strip RGBW

|                                  | Parameter  | Value  | Min | Max   |
| -------------------------------- | ---------- | ------ | --- | ----- |
| <span class="first">■</span>     | Message ID | `2`    |
| <span class="payload-1">■</span> | Light id   | 1 byte | `0` | `127` |
| <span class="payload-2">■</span> | Pin R      | 1 byte | `0` | `127` |
| <span class="payload-3">■</span> | Pin G      | 1 byte | `0` | `127` |
| <span class="payload-4">■</span> | Pin B      | 1 byte | `0` | `127` |

**Create light with id `1` and pins R: `0x04`, G: `0x05`, B: `0x06`**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x02</span>
<span class="payload-1">0x01</span>
<span class="payload-2">0x04</span>
<span class="payload-3">0x05</span>
<span class="payload-4">0x06</span>
<span class="end">0xf7</span>
</p>

### Create scene

|                                  | Parameter  | Value  | Min | Max   |
| -------------------------------- | ---------- | ------ | --- | ----- |
| <span class="first">■</span>     | Message ID | `3`    |
| <span class="payload-1">■</span> | Scene id   | 1 byte | `1` | `127` |

**Create scene with id `2`**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x03</span>
<span class="payload-1">0x02</span>
<span class="end">0xf7</span>
</p>

### Create graph

|                                    | Parameter    | Value   | Min | Max     | Parsed min | Parsed max |
| ---------------------------------- | ------------ | ------- | --- | ------- | ---------- | ---------- |
| <span class="first">■</span>       | Message ID   | `4`     |
| <span class="payload-1">■</span>   | Scene id     | 1 byte  | `1` | `127`   |
| <span class="payload-2">■</span>   | Graph id     | 1 byte  | `1` | `4`     |
| <span class="payload-3">■ ■</span> | Keyframe 1 x | 2 bytes | `0` | `16384` | `0.0`      | `1.0`      |
| <span class="payload-3">■ ■</span> | Keyframe 1 y | 2 bytes | `0` | `16384` | `0.0`      | `1.0`      |
| <span class="payload-3">■ ■</span> | Keyframe 1 c | 2 bytes | `0` | `16384` | `-1.0`     | `1.0`      |
| ...                                | ...          | ...     | ... | ...     |
| <span class="payload-3">■ ■</span> | Keyframe N x | 2 bytes | `0` | `16384` | `0.0`      | `1.0`      |
| <span class="payload-3">■ ■</span> | Keyframe N y | 2 bytes | `0` | `16384` | `0.0`      | `1.0`      |
| <span class="payload-3">■ ■</span> | Keyframe N c | 2 bytes | `0` | `16384` | `-1.0`     | `1.0`      |

**Create graph with id `4` in scene `2` defined by three keyframes: (0.0, 0.0, 0.0), (0.5, 0.5, -1.0) and (1.0, 1.0, 1.0)**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x04</span>
<span class="payload-1">0x02</span>
<span class="payload-2">0x04</span>
<span class="payload-3">
(0x00 0x00, 0x00 0x00, 0x40 0x00) (0x40 0x00, 0x40 0x00, 0x00 0x00) (0x7f 0x7f, 0x7f 0x7f, 0x7f 0x7f)
</span>
<span class="end">0xf7</span>
</p>

### Hue and brightness

#### Graph mode

|                                      | Parameter     | Value                    | Min | Max         | Parsed min | Parsed max |
| ------------------------------------ | ------------- | ------------------------ | --- | ----------- | ---------- | ---------- |
| <span class="first">■</span>         | Message ID    | `5`, `6`, `7`, `8`       |
| <span class="payload-1">■</span>     | Scene id      | 1 byte                   | `1` | `127`       |
| <span class="payload-2">■</span>     | Light id      | 1 byte                   | `1` | `127`       |
| <span class="payload-3">■</span>     | Mode          | `0` (once), `1` (repeat) |
| <span class="payload-4">■</span>     | Trigger       | 1 byte                   | `1` | `127`       |
| <span class="payload-5">■</span>     | Graph         | 1 byte                   | `1` | `127`       |
| <span class="payload-6">■</span>     | Min           | 1 byte                   | `0` | `127`       | `0`        | `255`      |
| <span class="payload-7">■</span>     | Max           | 1 byte                   | `0` | `127`       | `0`        | `255`      |
| <span class="payload-8">■ ■ ■</span> | Duration (ms) | 3 bytes                  | `0` | `268435455` |
| <span class="payload-9">■ ■</span>   | Period        | 2 bytes                  | `0` | `16384`     | `0.0`      | `1.0`      |

**Set hue A of scene `2`, light `1` to mode "once", trigger note `0x3c` = 60 (C4), graph id #8, min 0, max 127, duration 30 000 ms, period 1.0**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x05</span>
<span class="payload-1">0x02</span>
<span class="payload-2">0x01</span>
<span class="payload-3">0x00</span>
<span class="payload-4">0x3c</span>
<span class="payload-5">0x08</span>
<span class="payload-6">0x00</span>
<span class="payload-7">0x7f</span>
<span class="payload-8">0x01 0x6a 0x30</span>
<span class="payload-9">0x7f 0x7f</span>
<span class="end">0xf7</span>
</p>

**Set brightness A of scene `3`, light `1` to mode "repeat", no trigger note (`0x00`), graph id #9, min 0, max 127, duration 1000 ms, period 1.0**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x06</span>
<span class="payload-1">0x03</span>
<span class="payload-2">0x01</span>
<span class="payload-3">0x01</span>
<span class="payload-4">0x00</span>
<span class="payload-5">0x09</span>
<span class="payload-6">0x00</span>
<span class="payload-7">0x7f</span>
<span class="payload-8">0x00 0x07 0x68</span>
<span class="payload-9">0x7f 0x7f</span>
<span class="end">0xf7</span>
</p>

#### External control mode

|                                  | Parameter  | Value              | Min | Max   |
| -------------------------------- | ---------- | ------------------ | --- | ----- |
| <span class="first">■</span>     | Message ID | `5`, `6`, `7`, `8` |
| <span class="payload-1">■</span> | Scene id   | 1 byte             | `1` | `127` |
| <span class="payload-2">■</span> | Light id   | 1 byte             | `1` | `127` |
| <span class="payload-3">■</span> | Mode       | `2` (external)     |
| <span class="payload-4">■</span> | Control    | 1 byte             | `0` | `127` |

**Set hue B of scene `2`, light `1` to mode "external", control note `0x3e` = 62 (D4)**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x07</span>
<span class="payload-1">0x02</span>
<span class="payload-2">0x01</span>
<span class="payload-3">0x02</span>
<span class="payload-4">0x3e</span>
<span class="end">0xf7</span>
</p>

**Set brightness B of scene `2`, light `1` to mode "external", control note `0x3e` = 62 (D4)**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x08</span>
<span class="payload-1">0x02</span>
<span class="payload-2">0x01</span>
<span class="payload-3">0x02</span>
<span class="payload-4">0x3e</span>
<span class="end">0xf7</span>
</p>

### Set strobe A / B

|                                    | Parameter  | Value                    | Min | Max     | Parsed min | Parsed max |
| ---------------------------------- | ---------- | ------------------------ | --- | ------- | ---------- | ---------- |
| <span class="first">■</span>       | Message ID | `9`, `10`                |
| <span class="payload-1">■</span>   | Scene id   | 1 byte                   | `1` | `127`   |
| <span class="payload-2">■</span>   | Light id   | 1 byte                   | `1` | `127`   |
| <span class="payload-3">■</span>   | Mode       | `0` (once), `1` (repeat) |
| <span class="payload-4">■</span>   | Trigger    | 1 byte                   | `0` | `127`   |
| <span class="payload-5">■</span>   | Graph      | 1 byte                   | `1` | `127`   |
| <span class="payload-6">■</span>   | Min        | 1 byte                   | `0` | `127`   | `0`        | `255`      |
| <span class="payload-7">■</span>   | Max        | 1 byte                   | `0` | `127`   | `0`        | `255`      |
| <span class="payload-8">■ ■</span> | Frequency  | 2 bytes                  | `0` | `16384` | `0.0`      | `1.0`      |
| <span class="payload-9">■ ■</span> | Period     | 2 bytes                  | `0` | `16384` | `0.0`      | `1.0`      |

**Set strobe A of scene `4`, light `3` to mode "repeat", no trigger note (`0x00`), graph id #12, min 0, max 127, frequency 1.0, period 1.0**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x09</span>
<span class="payload-1">0x04</span>
<span class="payload-2">0x03</span>
<span class="payload-3">0x01</span>
<span class="payload-4">0x00</span>
<span class="payload-5">0x0c</span>
<span class="payload-6">0x00</span>
<span class="payload-7">0x7f</span>
<span class="payload-8">0x07 0x68</span>
<span class="payload-9">0x7f 0x7f</span>
<span class="end">0xf7</span>
</p>

---

## References

- [MIDI System Exclusive](https://www.recordingblogs.com/wiki/midi-system-exclusive-message)
- [SysEx](http://midi.teragonaudio.com/tech/midispec/sysex.htm)

<style>
.code { font-family: "Courier New", monospace; font-weight: bold; background-color: #011627; padding: 0.5rem 1rem; }
.first { color: #CDB4DB; }
.payload-1 { color: #BDE0FE; }
.payload-2 { color: #B2FF9E; }
.payload-3 { color: #9EFFEC; }
.payload-4 { color: #F4FF9E; }
.payload-5 { color: #FF9EB1; }
.payload-6 { color: #FF9EB1; }
.payload-7 { color: #FF9EF2; }
.payload-8 { color: #9EB1FF; }
.start, .end { color: #8c8f9f; }
.first { color: #ff0000; }
.payload-1 { color: #ff8700; }
.payload-2 { color: #ffd300; }
.payload-3 { color: #ffff0a; }
.payload-4 { color: #a1ff0a; }
.payload-5 { color: #0aff99; }
.payload-6 { color: #0aefff; }
.payload-7 { color: #1a85ff; }
.payload-8 { color: #9f73ff; }
.payload-9 { color: #be0aff; }
</style>
