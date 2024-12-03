let midiAccess;
let outputDevice;

/**
 * Utils
 */
const defaultSysExMessage = [
  0xf0, 0x7e, 0x7f, 0x00, 0x00, 0x00, 0x13, 0x12, 0x13, 0x12, 0x13, 0x13, 0x12,
  0x13, 0x12, 0x13, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12,
  0x13, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x13,
  0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x13, 0x12, 0x13,
  0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x13, 0x12, 0x13, 0x12, 0x13,
  0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13,
  0x12, 0x13, 0x12, 0x13, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0xf7,
];

const decToHex = (dec) => "0x" + dec.toString(16).padStart(2, "0");

let logArray = [];

const log = (message) => {
  console.log(message);

  const now = new Date();
  const time = now.toLocaleTimeString();
  const ms = now.getMilliseconds();

  logArray.push(`[${time}.${String(ms).padStart(3, "0")}] ${message}`);

  document.getElementById("log").innerText = [...logArray].reverse().join("\n");
};

const midiNoteToABC = (note) => {
  if (note < 1 || note > 127) {
    return "";
  }

  // Define the note names in order starting from C
  const noteNames = [
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B",
  ];

  // MIDI note number for middle C is 60, so we calculate the offset
  const octave = Math.floor((note - 12) / 12);
  const noteIndex = note % 12;

  // Get the note name (C, C#, D, etc.)
  const noteName = noteNames[noteIndex];

  // Format the ABC notation with note and octave
  return `${noteName}${octave}`;
};

/**
 * MIDI messages functions
 */

// Program Change
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

// Note On
const sendNoteOn = () => {
  if (!outputDevice) {
    alert("No MIDI device connected.");
    return;
  }

  const note = parseInt(document.getElementById("note").value);
  const velocity = parseInt(document.getElementById("velocity").value);
  const duration = parseInt(document.getElementById("duration").value);

  // MIDI message format: [statusByte, dataByte1, dataByte2]
  // Note On for the specified note number with maximum velocity (127)
  let noteOnMessage = [0x90, note, velocity];
  outputDevice.send(noteOnMessage);

  const noteAbc = midiNoteToABC(note);

  log(`Sent Note On: ${noteAbc}, velocity ${velocity}`);

  // Optionally, send a Note Off after 1 second (to stop the note)
  setTimeout(() => {
    let noteOffMessage = [0x80, note, 0];
    outputDevice.send(noteOffMessage);
    log(`Sent Note Off: ${noteAbc}`);
  }, duration);
};

// SysEx
const sendSysex = () => {
  if (!outputDevice) {
    alert("No MIDI device connected.");
    return;
  }

  const message = document
    .getElementById("sysexMessage")
    .value.trim()
    .split("")
    .filter((c) => /[0-9a-fA-Fx\s]/.test(c))
    .join("")
    .split(" ")
    .map((hex) => Number(hex));

  try {
    outputDevice.send(message);

    log(`Sent Sysex: ${message.map(decToHex).join(", ")}`);
  } catch (error) {
    log(error);
  }
};

const logMidiState = (state, isError = false) => {
  const message = `<kbd>${isError ? "🔴" : "🟢"} ${state}</kbd>`;

  document.getElementById("connection-info").innerHTML = message;

  log(state);
};

/**
 * MIDI I/O
 */
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

/**
 * Main
 */
const initMidi = () => {
  // Display default SysEx message
  document.getElementById("sysexMessage").value = defaultSysExMessage
    .map(decToHex)
    .join(", ");

  // Check for Web MIDI API support
  if (!navigator.requestMIDIAccess) {
    logMidiState("Web MIDI API not supported.", true);

    return;
  }

  log("Web MIDI API is supported!");

  requestMidiDevice();

  document.getElementById("sendAB").addEventListener("submit", (e) => {
    e.preventDefault();
    sendAB();
  });
  document.getElementById("sendNoteOn").addEventListener("submit", (e) => {
    e.preventDefault();
    sendNoteOn();
  });
  document.getElementById("sendSysex").addEventListener("submit", (e) => {
    e.preventDefault();
    sendSysex();
  });
};

document.addEventListener("DOMContentLoaded", initMidi);
