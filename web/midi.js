let midiAccess;
let outputDevice;

/**
 * Utils
 */
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

// SysEx
const showHideSysExHelp = () => {
  const isChecked = document.getElementById("sysexHelp").checked;

  document.querySelectorAll(".help").forEach((el) => {
    el.style.display = isChecked ? "inline-block" : "none";
  });
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

const payloadToColoredSpans = (message) => {
  const messageId = message[2];
  const payloadLength = message.length - 4;

  const validateArgumentsCount = (expected) => {
    if (payloadLength !== expected) {
      const expectedLength = expected + 4;
      throw `Invalid SysEx message: message ID ${messageId} must have ${expectedLength} bytes, got ${message.length}`;
    }
  };

  const validateGraphPayload = () => {
    if ((payloadLength - 2) % 6 !== 0) {
      throw `Invalid SysEx message: message ID ${messageId} must have a payload that is a multiple of 6 bytes`;
    }
  };

  switch (messageId) {
    case 1:
    case 3:
      validateArgumentsCount(1);
      return [
        `<span class="help">${messageId === 1 ? "Led ID" : "Scene ID"}</span>`,
        `<span class="second">${decToHex(message[3])}</span>`,
      ].join(" ");
    case 2:
      validateArgumentsCount(5);

      return [
        `<span class="help">Light ID</span>`,
        `<span class="second">${decToHex(message[3])}</span>`,
        `<span class="help">Pin R</span>`,
        `<span class="third">${decToHex(message[4])}</span>`,
        `<span class="help">Pin G</span>`,
        `<span class="fourth">${decToHex(message[5])}</span>`,
        `<span class="help">Pin B</span>`,
        `<span class="fifth">${decToHex(message[6])}</span>`,
        `<span class="help">Pin W</span>`,
        `<span class="sixth">${decToHex(message[7])}</span>`,
      ].join(" ");
    case 4:
      validateGraphPayload();

      return [
        `<span class="help">Scene ID</span>`,
        `<span class="second">${decToHex(message[3])}</span>`,
        `<span class="help">Graph ID</span>`,
        `<span class="second">${decToHex(message[4])}</span>`,
      ]
        .concat(
          message
            .filter((_, i) => i > 4 && i < message.length - 1)
            .map((val, i) => {
              if (i % 6 === 0) {
                return [
                  `<span class="help">Keyframe ${i / 6 + 1}</span>`,
                  `(<span class="third">${decToHex(val)}</span>`,
                ].join(" ");
              }

              if (i % 6 === 5) {
                return `<span class="third">${decToHex(val)}</span>)`;
              }

              if (i % 2 === 1) {
                return `<span class="third">${decToHex(val)}</span>,`;
              }

              return `<span class="third">${decToHex(val)}</span>`;
            })
        )
        .join(" ");
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      const mode = message[5];

      if (mode === undefined) {
        throw `Invalid SysEx message: message ID ${messageId} must have a mode`;
      }

      if (messageId === 9) {
        validateArgumentsCount(11);

        return [
          `<span class="help">Scene ID</span>`,
          `<span class="second">${decToHex(message[3])}</span>`,
          `<span class="help">Light ID</span>`,
          `<span class="second">${decToHex(message[4])}</span>`,
          `<span class="help">Mode</span>`,
          `<span class="third">${decToHex(message[5])}</span>`,
          `<span class="help">Trigger note</span>`,
          `<span class="fourth">${decToHex(message[6])}</span>`,
          `<span class="help">Graph</span>`,
          `<span class="fifth">${decToHex(message[7])}</span>`,
          `<span class="help">Min</span>`,
          `<span class="sixth">${decToHex(message[8])}</span>`,
          `<span class="help">Max</span>`,
          `<span class="seventh">${decToHex(message[9])}</span>`,
          `<span class="help">Frequency</span>`,
          `<span class="eighth">${decToHex(message[10])}</span>`,
          `<span class="eighth">${decToHex(message[11])}</span>`,
          `<span class="help">Period</span>`,
          `<span class="ninth">${decToHex(message[12])}</span>`,
          `<span class="ninth">${decToHex(message[13])}</span>`,
        ].join(" ");
      }

      if (mode === 0 || mode === 1) {
        validateArgumentsCount(12);

        return [
          `<span class="help">Scene ID</span>`,
          `<span class="second">${decToHex(message[3])}</span>`,
          `<span class="help">Light ID</span>`,
          `<span class="second">${decToHex(message[4])}</span>`,
          `<span class="help">Mode</span>`,
          `<span class="third">${decToHex(message[5])}</span>`,
          `<span class="help">Trigger note</span>`,
          `<span class="fourth">${decToHex(message[6])}</span>`,
          `<span class="help">Graph</span>`,
          `<span class="fifth">${decToHex(message[7])}</span>`,
          `<span class="help">Min</span>`,
          `<span class="sixth">${decToHex(message[8])}</span>`,
          `<span class="help">Max</span>`,
          `<span class="seventh">${decToHex(message[9])}</span>`,
          `<span class="help">Duration</span>`,
          `<span class="eighth">${decToHex(message[10])}</span>`,
          `<span class="eighth">${decToHex(message[11])}</span>`,
          `<span class="eighth">${decToHex(message[12])}</span>`,
          `<span class="help">Period</span>`,
          `<span class="ninth">${decToHex(message[13])}</span>`,
          `<span class="ninth">${decToHex(message[14])}</span>`,
        ].join(" ");
      }

      if (mode === 2) {
        validateArgumentsCount(4);

        return [
          `<span class="help">Scene ID</span>`,
          `<span class="second">${decToHex(message[3])}</span>`,
          `<span class="help">Light ID</span>`,
          `<span class="second">${decToHex(message[4])}</span>`,
          `<span class="help">Control note</span>`,
          `<span class="third">${decToHex(message[5])}</span>`,
        ].join(" ");
      }

      throw `Invalid SysEx message: message ID ${messageId} must have a mode of 0, 1 or 2`;
  }

  return message
    .filter((_, i) => i > 2 && i < message.length - 1)
    .map(decToHex)
    .map((val) => `<span class="second">${val}</span>`)
    .join(" ");
};

const messageToColoredSpans = (message) => {
  const [first, second] = message;
  const last = message[message.length - 1];
  const startsWithSysExStart = first === 0xf0 && second === 0x7e;
  const endsWithSysExEnd = last === 0xf7;

  let res = [];

  if (!startsWithSysExStart || !endsWithSysExEnd) {
    return 'Invalid SysEx message: must start with "0xf0 0x7e" and end with "0xf7"';
  }

  const messageId = message[2];
  if (!messageId || messageId <= 0 || messageId > 9) {
    return "Invalid SysEx message: must have a valid message ID";
  }

  res.push(
    ...[
      `<span class="help">SysEx start</span>`,
      `<span class="start">${decToHex(first)} ${decToHex(second)}</span>`,
      `<span class="help">Message ID</span>`,
      `<span class="first">${decToHex(messageId)}</span>`,
    ]
  );

  try {
    res.push(payloadToColoredSpans(message));
  } catch (error) {
    return error;
  }

  res.push(
    ...[
      `<span class="help">SysEx end</span>`,
      `<span class="end">${decToHex(last)}</span>`,
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

  messages.forEach((message) => {
    try {
      outputDevice.send(message);

      log(`Sent Sysex: ${message.map(decToHex).join(", ")}`);
    } catch (error) {
      log(error);
    }
  });
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

/**
 * Main
 */
const initProgramChange = () => {
  document.getElementById("next-scene").addEventListener("click", () => {
    next();
  });
  document.getElementById("restart").addEventListener("click", () => {
    restart();
  });
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
  });
};

const initSysEx = () => {
  document.getElementById("sysexMessage").value = `0xf0 0x7e 0x01 0x3e 0xf7
0xf0 0x7e 0x02 0x01 0x04 0x05 0x06 0x07 0xf7
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

  document.getElementById("sendSysex").addEventListener("submit", (e) => {
    e.preventDefault();
    sendSysex();
  });

  colorizeSysExMessage();
  showHideSysExHelp();
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

const init = () => {
  initControlChange();
  initProgramChange();
  initSendNote();
  initSysEx();
  initMidi();
};

document.addEventListener("DOMContentLoaded", init);
