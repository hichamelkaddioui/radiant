let midiAccess;
let outputDevice;

/**
 * Utils
 */
let logArray = [];

const log = (message) => {
  console.log(message);

  const now = new Date();
  const time = now.toLocaleTimeString();
  const ms = now.getMilliseconds();

  logArray.push(`[${time}.${String(ms).padStart(3, "0")}] ${message}`);

  document.getElementById("log").innerText = [...logArray].reverse().join("\n");
};

/**
 * Program Change
 */
const sendProgramChange = (number) => {
  if (!outputDevice) {
    alert("No MIDI device connected.");
    return;
  }

  // MIDI message format: [statusByte, programNumber]
  // Program Change message for the specified program number
  let programChangeMessage = [0xc0, number];
  outputDevice.send(programChangeMessage);

  log(`Sent Program Change (program ${number})`);
};

const restart = () => {
  sendProgramChange(0x00);
  resetAb();
};

const next = () => {
  sendProgramChange(0x01);
  resetAb();
};

const initProgramChange = () => {
  document.getElementById("next-scene").addEventListener("click", () => {
    next();
  });
  document.getElementById("restart").addEventListener("click", () => {
    restart();
  });
};

/**
 * Control Change
 */
const resetAb = () => {
  document.getElementById("ab").value = 0;
  document.getElementById("abValue").innerText = "0 %";
};

const sendAB = () => {
  if (!outputDevice) {
    alert("No MIDI device connected.");
    return;
  }

  const value = parseInt(document.getElementById("ab").value);

  // Update A/B value
  const percent = ((value * 100) / 127).toFixed(2);
  document.getElementById("abValue").innerText = `${percent} %`;

  // Send control change message for A/B value
  const controlChangeMessage = [0xb0, 0x8, value];
  outputDevice.send(controlChangeMessage);

  log(`Sent A/B value: ${value} / 127`);
};

const initControlChange = () => {
  document.getElementById("sendAB").addEventListener("submit", (e) => {
    e.preventDefault();
    sendAB();
  });
  document.getElementById("ab").addEventListener("input", () => {
    sendAB();
  });
};

/**
 * Note on
 */
const midiNoteToABC = (note) => {
  if (note < 1 || note > 127) {
    return "";
  }

  // Define the note names in order starting from C
  const noteNames = "C,C#,D,D#,E,F,F#,G,G#,A,A#,B".split(",");

  // MIDI note number for middle C is 60, so we calculate the offset
  const octave = Math.floor((note - 12) / 12);
  const noteIndex = note % 12;

  // Get the note name (C, C#, D, etc.)
  const noteName = noteNames[noteIndex];

  // Format the ABC notation with note and octave
  return `${noteName}${octave}`;
};

const sendNoteOn = () => {
  if (!outputDevice) {
    alert("No MIDI device connected.");
    return;
  }

  const note = parseInt(document.getElementById("note").value);
  const velocity = parseInt(document.getElementById("velocity").value);
  const duration = parseInt(document.getElementById("duration").value);
  const noteOnMessage = [0x90, note, velocity];

  outputDevice.send(noteOnMessage);

  const noteAbc = midiNoteToABC(note);

  log(`Sent Note On: ${noteAbc}, velocity ${velocity}`);

  // Send Note Off
  setTimeout(() => {
    const noteOffMessage = [0x80, note, 0];
    outputDevice.send(noteOffMessage);
    log(`Sent Note Off: ${noteAbc}`);
  }, duration);
};

const initSendNote = () => {
  document.getElementById("sendNoteOn").addEventListener("submit", (e) => {
    e.preventDefault();
    sendNoteOn();
  });

  document.getElementById("note").addEventListener("input", () => {
    document.getElementById("abcNotation").innerText = midiNoteToABC(
      document.getElementById("note").value
    );
  });
  document.getElementById("velocity").addEventListener("input", () => {
    document.getElementById("velocityValue").innerText =
      document.getElementById("velocity").value;

    if (document.getElementById("velocityChange").checked) {
      sendNoteOn();
    }
  });
};

/**
 * SysEx
 */
const showHideSysExHelp = () => {
  const isChecked = document.getElementById("sysexHelp").checked;

  document.querySelectorAll(".help").forEach((el) => {
    el.style.display = isChecked ? "inline-block" : "none";
  });
};

let showHex = true;

const showHideSysExHex = () => {
  showHex = document.getElementById("sysexHex").checked;

  colorizeSysExMessage();
};

const getSysExMessages = () =>
  document
    .getElementById("sysexMessage")
    .value.split("\n")
    // Filter out empty lines
    .filter((l) => l.trim().length > 0)
    .map((line) =>
      line
        // Remove spaces at start and end as they serve as delimiters
        .trim()
        .split("")
        // Keep only hex characters and spaces
        .filter((c) => /[0-9a-fA-Fx\s]/.test(c))
        .join("")
        .split(" ")
        // Filter out empty strings
        .filter((c) => c)
        // Convert hex strings to numbers
        .map((hex) => Number(hex))
        // Filter out NaN
        .filter((c) => c !== NaN)
    );

const messageIdMap = {
  1: "Set params",
  2: "Create light",
  3: "Create scene",
  4: "Create graph",
  5: "Set hue A",
  6: "Set brightness A",
  7: "Set hue B",
  8: "Set brightness B",
  9: "Set strobe A",
  10: "Set strobe B",
};

const payloadToColoredObjects = (message) => {
  const messageId = message[2];
  const payloadLength = message.length - 4;

  const validateArgumentsCount = (structure) => {
    const expected = structure.map((s) => s.length).reduce((a, b) => a + b);

    if (payloadLength !== expected) {
      throw `Invalid SysEx message: message ID ${messageId} must have a payload of ${expected} bytes, got ${payloadLength}`;
    }
  };

  const validateGraphPayload = (keyframes) => {
    if (keyframes.length % 6 !== 0) {
      throw `Invalid SysEx message: message ID ${messageId} must have a multiple of 6 keyframe bytes, got ${keyframes.length}`;
    }
  };

  const validateHueSaturation = (structure) => {
    if (payloadLength < 3) {
      throw `Invalid SysEx message: message ID ${messageId} must have at least 3 bytes, got ${payloadLength}`;
    }

    const mode = message[5];

    if (mode === undefined) {
      throw `Invalid SysEx message: message ID ${messageId} must have a mode`;
    }

    if (!(mode in structure)) {
      throw `Invalid SysEx message: unknown mode ${mode} for message ID ${messageId}`;
    }

    validateArgumentsCount(structure[mode]);
  };

  const graphMode = [
    { label: "Scene ID", length: 1 },
    { label: "Light ID", length: 1 },
    { label: "Mode", length: 1 },
    { label: "Trigger note", length: 1 },
    { label: "Graph ID", length: 1 },
    { label: "Min", length: 1 },
    { label: "Max", length: 1 },
    { label: "Duration", length: 3 },
    { label: "Period", length: 2 },
  ];

  const controlMode = [
    { label: "Scene ID", length: 1 },
    { label: "Light ID", length: 1 },
    { label: "Mode", length: 1 },
    { label: "Control note", length: 1 },
  ];

  const hueSaturation = { 0: graphMode, 1: graphMode, 2: controlMode };

  const structures = {
    "Set params": [{ label: "A/B control note", length: 1 }],
    "Create light": [
      { label: "Light ID", length: 1 },
      { label: "Pin R", length: 1 },
      { label: "Pin G", length: 1 },
      { label: "Pin B", length: 1 },
    ],
    "Create scene": [{ label: "Scene ID", length: 1 }],
    "Create graph": [
      { label: "Scene ID", length: 1 },
      { label: "Graph ID", length: 1 },
    ],
    "Set hue A": hueSaturation,
    "Set brightness A": hueSaturation,
    "Set hue B": hueSaturation,
    "Set brightness B": hueSaturation,
    "Set strobe A": [
      { label: "Scene ID", length: 1 },
      { label: "Light ID", length: 1 },
      { label: "Mode", length: 1 },
      { label: "Trigger note", length: 1 },
      { label: "Graph ID", length: 1 },
      { label: "Min", length: 1 },
      { label: "Max", length: 1 },
      { label: "Frequency", length: 2 },
      { label: "Period", length: 2 },
    ],
  };

  const messageStructure = structures[messageIdMap[messageId]];

  // Message ID is not supported
  if (!messageStructure) {
    return [{ label: "Unknown", length: payloadLength }];
  }

  // Message ID is graph keyframe
  const isGraphKeyframe = messageId === 4;

  if (isGraphKeyframe) {
    const keyframes = message.filter((_, i) => i > 4 && i < message.length - 1);

    validateGraphPayload(keyframes);

    return [
      ...messageStructure,
      ...keyframes
        .filter((_, i) => i % 6 === 0)
        .map((_, i) => ({ label: `Keyframe ${i + 1}`, length: 6 })),
    ];
  }

  // Message ID is hue/saturation
  const isHueSaturation = [5, 6, 7, 8].includes(messageId);

  if (isHueSaturation) {
    const mode = message[5];

    validateHueSaturation(messageStructure);

    return messageStructure[mode];
  }

  // Message ID is not graph keyframe or hue/saturation
  validateArgumentsCount(messageStructure);

  return messageStructure;
};

const formatByte = (dec) =>
  showHex ? "0x" + dec.toString(16).padStart(2, "0") : dec;

const messageToColoredSpans = (message) => {
  const [first, second] = message;
  const last = message[message.length - 1];
  const startsWithSysExStart = first === 0xf0 && second === 0x7e;
  const endsWithSysExEnd = last === 0xf7;
  let res = [];

  if (!startsWithSysExStart || !endsWithSysExEnd) {
    return `Invalid SysEx message: must start with "${formatByte(
      0xf0
    )} ${formatByte(0x7e)}" and end with "${formatByte(0xf7)}"`;
  }

  if (message.length < 4) {
    return `Invalid SysEx message: must have at least 4 bytes, got ${message.length}`;
  }

  const messageId = message[2];
  const messageIdName = messageIdMap[messageId] || "Unknown ID";

  res.push(
    ...[
      `<span class="help">SysEx start</span>`,
      `<span class="start">${formatByte(first)} ${formatByte(second)}</span>`,
      `<span class="help">${messageIdName}</span>`,
      `<span class="first">${formatByte(messageId)}</span>`,
    ]
  );

  try {
    let offset = 3;

    const payloadStructure = payloadToColoredObjects(message);

    const coloredObjectsToSpan = (label, length, i) => {
      const help = `<span class="help">${label}</span>`;
      const values = message.slice(offset, offset + length);
      offset += length;

      const isGraphKeyframes = messageId === 4 && i > 1;

      if (isGraphKeyframes) {
        const [x1, x2, y1, y2, c1, c2] = values.map(formatByte);

        return [
          help,
          [
            `(<span class="payload-3">${x1}</span>`,
            `<span class="payload-3">${x2}</span>, `,
            `<span class="payload-3">${y1}</span>`,
            `<span class="payload-3">${y2}</span>, `,
            `<span class="payload-3">${c1}</span>`,
            `<span class="payload-3">${c2}</span>)`,
          ],
        ];
      }

      const spanValues = values.map(
        (value) => `<span class="payload-${i + 1}">${formatByte(value)}</span>`
      );

      return [help, spanValues];
    };

    const coloredSpans = payloadStructure
      .map(({ label, length }, i) => coloredObjectsToSpan(label, length, i))
      .map(([help, values]) => [help, values.join(" ")].join(" "))
      .join(" ");

    res.push(coloredSpans);
  } catch (error) {
    console.error(error);
    return error;
  }

  res.push(
    ...[
      `<span class="help">SysEx end</span>`,
      `<span class="end">${formatByte(last)}</span>`,
    ]
  );

  return res.join(" ");
};

const colorizeSysExMessage = () => {
  const messages = getSysExMessages();

  document.getElementById("sysexPayload").innerHTML = messages
    .map(messageToColoredSpans)
    .join("<br />");

  showHideSysExHelp();
};

const sendSysex = () => {
  if (!outputDevice) {
    alert("No MIDI device connected.");
    return;
  }

  const messages = getSysExMessages();

  messages.forEach((message, index) => {
    setTimeout(() => {
      try {
        outputDevice.send(message);

        log(`Sent Sysex: ${message.map(formatByte).join(", ")}`);
      } catch (error) {
        log(error);
      }
    }, index * 1500);
  });
};

const initSysEx = () => {
  document.getElementById("sysexMessage").value = `0xf0 0x7e 0x01 0x3e 0xf7
0xf0 0x7e 0x02 0x01 0x04 0x05 0x06 0xf7
0xf0 0x7e 0x03 0x02 0xf7
0xf0 0x7e 0x04 0x02 0x04 0x00 0x00 0x00 0x00 0x40 0x00 0x40 0x00 0x40 0x00 0x00 0x00 0x7f 0x7f 0x7f 0x7f 0x7f 0x7f 0xf7
0xf0 0x7e 0x05 0x02 0x01 0x00 0x3c 0x08 0x00 0x7f 0x01 0x6a 0x30 0x7f 0x7f 0xf7
0xf0 0x7e 0x06 0x03 0x01 0x01 0x00 0x09 0x00 0x7f 0x00 0x07 0x68 0x7f 0x7f 0xf7
0xf0 0x7e 0x07 0x02 0x01 0x02 0x3e 0xf7
0xf0 0x7e 0x08 0x02 0x01 0x02 0x3e 0xf7
0xf0 0x7e 0x09 0x04 0x03 0x01 0x00 0x0c 0x00 0x7f 0x07 0x68 0x7f 0x7f 0xf7`;

  document
    .getElementById("sysexMessage")
    .addEventListener("input", colorizeSysExMessage);

  document
    .getElementById("sysexHelp")
    .addEventListener("click", showHideSysExHelp);

  document
    .getElementById("sysexHex")
    .addEventListener("click", showHideSysExHex);

  document.getElementById("sendSysex").addEventListener("submit", (e) => {
    e.preventDefault();
    sendSysex();
  });

  colorizeSysExMessage();
  showHideSysExHelp();
};

/**
 * MIDI I/O
 */
const logMidiState = (state, isError = false) => {
  const message = `<kbd>${isError ? "🔴" : "🟢"} ${state}</kbd>`;

  document.getElementById("connection-info").innerHTML = message;

  log(state);
};

const requestMidiDevice = () => {
  navigator
    .requestMIDIAccess({ sysex: true })
    .then((access) => {
      midiAccess = access;

      // List available output devices
      const outputs = Array.from(midiAccess.outputs.values());

      console.log(outputs);

      if (outputs.length == 0) {
        logMidiState("No MIDI output devices found.", true);

        return;
      }

      outputDevice = outputs[0];

      logMidiState(`${outputDevice.name} connected.`);

      outputDevice.addEventListener("midimessage", (event) => {
        log(event);
      });
    })
    .catch((err) => {
      logMidiState(`MIDI Access failed: ${err}`, true);
    });
};

const initMidi = () => {
  // Check for Web MIDI API support
  if (!navigator.requestMIDIAccess) {
    logMidiState("Web MIDI API not supported.", true);

    return;
  }

  log("Web MIDI API is supported!");

  requestMidiDevice();
};

/**
 * Init
 */
document.addEventListener("DOMContentLoaded", () => {
  initControlChange();
  initProgramChange();
  initSendNote();
  initSysEx();
  initMidi();
});
