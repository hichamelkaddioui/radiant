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
<span class="second">...</span>
<span class="third">...</span>
<span class="fourth">...</span>
<span class="fifth">...</span>
<span class="sixth">...</span>
<span class="seventh">...</span>
<span class="eighth">...</span>
<span class="ninth">...</span>
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

|                               | Parameter          | Value  | Min | Max   |
| ----------------------------- | ------------------ | ------ | --- | ----- |
| <span class="first">■</span>  | Message ID         | `1`    |
| <span class="second">■</span> | A / B control note | 1 byte | `0` | `127` |

**Set A / B control note to `0x3e` = 62 (D4)**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x01</span>
<span class="second">0x3e</span>
<span class="end">0xf7</span>

### Create light

#### LED strip RGBW

|                               | Parameter  | Value  | Min | Max   |
| ----------------------------- | ---------- | ------ | --- | ----- |
| <span class="first">■</span>  | Message ID | `2`    |
| <span class="second">■</span> | Light id   | 1 byte | `0` | `127` |
| <span class="third">■</span>  | Pin R      | 1 byte | `0` | `127` |
| <span class="fourth">■</span> | Pin G      | 1 byte | `0` | `127` |
| <span class="fifth">■</span>  | Pin B      | 1 byte | `0` | `127` |
| <span class="sixth">■</span>  | Pin W      | 1 byte | `0` | `127` |

**Create light with id `1` and pins R: `0x04`, G: `0x05`, B: `0x06`, W: `0x07`**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x02</span>
<span class="second">0x01</span>
<span class="third">0x04</span>
<span class="fourth">0x05</span>
<span class="fifth">0x06</span>
<span class="sixth">0x07</span>
<span class="end">0xf7</span>

### Create scene

|                               | Parameter  | Value  | Min | Max   |
| ----------------------------- | ---------- | ------ | --- | ----- |
| <span class="first">■</span>  | Message ID | `3`    |
| <span class="second">■</span> | Scene id   | 1 byte | `1` | `127` |

**Create scene with id `2`**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x03</span>
<span class="second">0x02</span>
<span class="end">0xf7</span>
</p>

### Create graph

|                                | Parameter    | Value   | Min | Max     |
| ------------------------------ | ------------ | ------- | --- | ------- |
| <span class="first">■</span>   | Message ID   | `4`     |
| <span class="second">■</span>  | Scene id     | 1 byte  | `1` | `127`   |
| <span class="second">■</span>  | Graph id     | 1 byte  | `1` | `4`     |
| <span class="third">■ ■</span> | Keyframe 1 x | 2 bytes | `0` | `16384` |
| <span class="third">■ ■</span> | Keyframe 1 y | 2 bytes | `0` | `16384` |
| <span class="third">■ ■</span> | Keyframe 1 c | 2 bytes | `0` | `16384` |
| ...                            | ...          | ...     | ... | ...     |
| <span class="third">■ ■</span> | Keyframe N x | 2 bytes | `0` | `16384` |
| <span class="third">■ ■</span> | Keyframe N y | 2 bytes | `0` | `16384` |
| <span class="third">■ ■</span> | Keyframe N c | 2 bytes | `0` | `16384` |

**Create graph with id `4` in scene `2` defined by three keyframes: (0.0, 0.0, 0.0), (0.5, 0.5, -1.0) and (1.0, 1.0, 1.0)**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x04</span>
<span class="second">0x02 0x04</span>
<span class="third">
(0x00 0x00, 0x00 0x00, 0x40 0x00) (0x40 0x00, 0x40 0x00, 0x00 0x00) (0x7f 0x7f, 0x7f 0x7f, 0x7f 0x7f)
</span>
<span class="end">0xf7</span>
</p>

### Hue and brightness

#### Graph mode

|                                   | Parameter     | Value                    | Min | Max         |
| --------------------------------- | ------------- | ------------------------ | --- | ----------- |
| <span class="first">■</span>      | Message ID    | `5`, `6`, `7`, `8`       |
| <span class="second">■</span>     | Scene id      | 1 byte                   | `1` | `127`       |
| <span class="second">■</span>     | Light id      | 1 byte                   | `1` | `127`       |
| <span class="third">■</span>      | Mode          | `0` (once), `1` (repeat) |
| <span class="fourth">■</span>     | Trigger       | 1 byte                   | `1` | `127`       |
| <span class="fifth">■</span>      | Graph         | 1 byte                   | `1` | `127`       |
| <span class="sixth">■</span>      | Min           | 1 byte                   | `0` | `127`       |
| <span class="seventh">■</span>    | Max           | 1 byte                   | `0` | `127`       |
| <span class="eighth">■ ■ ■</span> | Duration (ms) | 3 bytes                  | `0` | `268435455` |
| <span class="ninth">■ ■</span>    | Period        | 2 bytes                  | `0` | `16384`     |

**Set hue A of scene `2`, light `1` to mode "once", trigger note `0x3c` = 60 (C4), graph id #8, min 0, max 127, duration 30 000 ms, period 1.0**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x05</span>
<span class="second">0x02 0x01</span>
<span class="third">0x00</span>
<span class="fourth">0x3c</span>
<span class="fifth">0x08</span>
<span class="sixth">0x00</span>
<span class="seventh">0x7f</span>
<span class="eighth"> 0x01 0x6a 0x30</span>
<span class="ninth">0x7f 0x7f</span>
<span class="end">0xf7</span>
</p>

**Set brightness A of scene `3`, light `1` to mode "repeat", no trigger note (`0x00`), graph id #9, min 0, max 127, duration 1000 ms, period 1.0**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x06</span>
<span class="second">0x03 0x01</span>
<span class="third">0x01</span>
<span class="fourth">0x00</span>
<span class="fifth">0x09</span>
<span class="sixth">0x00</span>
<span class="seventh">0x7f</span>
<span class="eighth">0x00 0x07 0x68</span>
<span class="ninth">0x7f 0x7f</span>
<span class="end">0xf7</span>
</p>

#### External control mode

|                               | Parameter  | Value              | Min | Max   |
| ----------------------------- | ---------- | ------------------ | --- | ----- |
| <span class="first">■</span>  | Message ID | `5`, `6`, `7`, `8` |
| <span class="second">■</span> | Scene id   | 1 byte             | `1` | `127` |
| <span class="second">■</span> | Light id   | 1 byte             | `1` | `127` |
| <span class="third">■</span>  | Mode       | `2` (external)     |
| <span class="fourth">■</span> | Control    | 1 byte             | `0` | `127` |

**Set hue B of scene `2`, light `1` to mode "external", control note `0x3e` = 62 (D4)**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x07</span>
<span class="second">0x02 0x01</span>
<span class="third">0x02</span>
<span class="fourth">0x3e</span>
<span class="end">0xf7</span>
</p>

**Set brightness B of scene `2`, light `1` to mode "external", control note `0x3e` = 62 (D4)**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x08</span>
<span class="second">0x02 0x01</span>
<span class="third">0x02</span>
<span class="fourth">0x3e</span>
<span class="end">0xf7</span>
</p>

### Set strobe A / B

|                                 | Parameter  | Value                    | Min | Max     |
| ------------------------------- | ---------- | ------------------------ | --- | ------- |
| <span class="first">■</span>    | Message ID | `9`, `10`                |
| <span class="second">■</span>   | Scene id   | 1 byte                   | `1` | `127`   |
| <span class="second">■</span>   | Light id   | 1 byte                   | `1` | `127`   |
| <span class="third">■</span>    | Mode       | `0` (once), `1` (repeat) |
| <span class="fourth">■</span>   | Trigger    | 1 byte                   | `0` | `127`   |
| <span class="fifth">■</span>    | Graph      | 1 byte                   | `1` | `127`   |
| <span class="sixth">■</span>    | Min        | 1 byte                   | `0` | `127`   |
| <span class="seventh">■</span>  | Max        | 1 byte                   | `0` | `127`   |
| <span class="eighth">■ ■</span> | Frequency  | 2 bytes                  | `0` | `16384` |
| <span class="ninth">■ ■</span>  | Period     | 2 bytes                  | `0` | `16384` |

**Set strobe A of scene `4`, light `3` to mode "repeat", no trigger note (`0x00`), graph id #12, min 0, max 127, frequency 1.0, period 1.0**

<p class="code">
<span class="start">0xf0 0x7e</span>
<span class="first">0x09</span>
<span class="second">0x04 0x03</span>
<span class="third">0x01</span>
<span class="fourth">0x00</span>
<span class="fifth">0x0c</span>
<span class="sixth">0x00</span>
<span class="seventh">0x7f</span>
<span class="eighth">0x07 0x68</span>
<span class="ninth">0x7f 0x7f</span>
<span class="end">0xf7</span>
</p>

---

## References

- [MIDI System Exclusive](https://www.recordingblogs.com/wiki/midi-system-exclusive-message)
- [SysEx](http://midi.teragonaudio.com/tech/midispec/sysex.htm)

<style>
.code { font-family: "Courier New", monospace; font-weight: bold; background-color: #011627 }
.start, .end { color: #FFC8DD; }
.first { color: #CDB4DB; }
.second { color: #BDE0FE; }
.third { color: #B2FF9E; }
.fourth { color: #9EFFEC; }
.fifth { color: #F4FF9E; }
.sixth { color: #FF9EB1; }
.seventh { color: #FF9EB1; }
.eighth { color: #FF9EF2; }
.ninth { color: #9EB1FF; }
</style>
