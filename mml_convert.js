function onConvertButtonClick() {
  var inputText = document.getElementById("input_text").value;
  var notes = parseCommands(inputText);
  document.getElementById("output_text").value = notes.reduce(
    (prevVal, val, index) => prevVal + ((index % 5 == 0) ? ", \n" : ", ") + val
  )
}

function parseCommands(commandText) {
  var notes = [];
  var re = /(?:(?<octave_global>[><])|(?:(?:(?<rest>r)|(?:(?<octave>[~_]?)(?<pitch>[a-g])(?<accidental>[+\-#]?)))(?<value>\d*)(?<dot>\.{0,3})\s*(?<slur>&?)))\s*/gu
  var convertCommand = getCommandConverter();
  
  for (var matchObj of commandText.matchAll(re)) {
    var note = convertCommand(matchObj.groups);
    if (note != undefined) {
      notes.push(note);
      commandText = matchObj.groups.unparsed;
    }
  }
  notes.push('0xFFFF');

  return notes;
}

function getCommandConverter() {
  var octave_global = 0;

  function convertCommand(commandObj) {
    switch (commandObj.octave_global) {
      case '>': octave_global++;  return undefined;
      case '<': octave_global--;  return undefined;
      default:  break;
    }

    var octave = octave_global;
    switch (commandObj.octave) {
      case '~': octave++; break;
      case '_': octave--; break;
      default:  break;
    }

    var pitch;
    switch (commandObj.pitch) {
      case 'c': pitch = 1;  break;
      case 'd': pitch = 3;  break;
      case 'e': pitch = 5;  break;
      case 'f': pitch = 6;  break;
      case 'g': pitch = 8;  break;
      case 'a': pitch = 10; break;
      case 'b': pitch = 12; break;
      default:  break;
    }
    switch (commandObj.accidental) {
      case '+': pitch++;  break;
      case '-': pitch--;  break;
      case '#': pitch++;  break;
      default:  break;
    }
    switch (commandObj.rest) {
      case 'r': pitch = 0;  break;
      default:  break;
    }

    var value = 4;
    switch (commandObj.value) {
      case '1':   value = 1;  break;  // 全音符
      case '2':   value = 2;  break;  // 2分音符
      case '4':   value = 3;  break;  // 4分音符
      case '8':   value = 4;  break;  // 8分音符
      case '16':  value = 5;  break;  // 16分音符
      case '32':  value = 6;  break;  // 32分音符
      case '64':  value = 7;  break;  // 64分音符
      default:    break;
    }

    var dot = 0;
    switch (commandObj.dot) {
      case '.':   dot = 1;  break;
      case '..':  dot = 2;  break;
      default:    break;
    }

    var is_slur = (commandObj.slur == "&");

    var note = ((octave >= 0 ? octave : 16 - octave) << 12) + (pitch << 8) + (is_slur << 6) + (dot << 4) + value;
    return `0x${note.toString(16).padStart(4, "0")}`
  }

  return convertCommand;
}
