#include <Utils.h>
#include <StateManager.h>

void StateManager::setupFlash()
{
    flash.begin();
}

void StateManager::setupLeds()
{
    lb.setup();
}

void StateManager::loop()
{
    sb.getCurrentScene()->update();
}

// Scene
void StateManager::createAndSaveStubs()
{
    // Create banks
    LedBank localLedBank = LedBank::createDummy();
    GraphBank localGraphBank = GraphBank::createDummy();
    SceneBank localSceneBank = SceneBank::createDummy(localLedBank, localGraphBank);
    StateManager localStateManager(localLedBank, localGraphBank, localSceneBank);
    localStateManager.params.abNote = 62;

    // Serialize
    localStateManager.setupFlash();
    localStateManager.serialize();
}

Scene *StateManager::getCurrentScene()
{
    return sb.getCurrentScene();
}

// MIDI
void StateManager::handleProgramChange(byte value)
{
    debug(1, "[midi] received program change: 0x%02X", value);

    switch (value)
    {
    case 0x00:
        sb.restart();
        break;
    case 0x01:
        sb.next();
        break;
    default:
        break;
    }
}

void StateManager::handleControlChange(byte type, byte value)
{
    debug(1, "[midi] received control change: 0x%02X 0x%02X", type, value);

    switch (type)
    {
    // Balance
    case 0x08:
        sb.getCurrentScene()->_ab = value / 127.0f;
        break;
    default:
        break;
    }
}

void StateManager::handleNoteOn(byte note, byte velocity)
{
    if (note == params.abNote)
    {
        sb.getCurrentScene()->_ab = (velocity - 1) / 126.0f;
        debug(1, "[midi] received ab control with value %0.2f", sb.getCurrentScene()->_ab);
    }

    Scene *currentScene = sb.getCurrentScene();

    if (!currentScene)
    {
        debug(1, "[midi] no current scene");
        return;
    }

    currentScene->onNotePlayed(note, velocity);
}

void StateManager::handleSystemExclusive(const byte *buffer, unsigned long length)
{
    debug(1, "[midi] received System Exclusive");

    // Read message id
    byte messageId = buffer[2];

    switch (messageId)
    {
    case SysExMessage::SET_PARAMS:
        params.abNote = static_cast<int>(buffer[3]);
        debug(1, "[SysEx] [params] abNote set to %d", params.abNote);
        break;
    case SysExMessage::CREATE_LIGHT:
        lb.sysExCreate(buffer + 3, length - 3);
        break;
    case SysExMessage::CREATE_SCENE:
        sb.sysExCreate(buffer + 3, length - 3);
        break;
    case SysExMessage::CREATE_GRAPH:
        gb.sysExCreate(buffer + 3, length - 3);
        break;
    case SysExMessage::SET_HUE_A:
    case SysExMessage::SET_BRIGHTNESS_A:
    case SysExMessage::SET_HUE_B:
    case SysExMessage::SET_BRIGHTNESS_B:
        sb.sysExSetHueBrightness(buffer + 2, length - 2, lb, gb);
        break;
    case SysExMessage::SET_STROBE_A:
    case SysExMessage::SET_STROBE_B:
    default:
        debugByteArray(buffer, length);
        break;
    }

    serialize();
}

// OLED
void StateManager::handleOledButtonPress()
{
    sb.next();
}

// Serialization
int StateManager::getCurrentSceneId()
{
    return sb.currentSceneId;
}

size_t StateManager::serialize()
{
    size_t offset = 0;
    uint8_t buffer[FLASH_BUFFER_SIZE]{};

    // Serialize params
    memcpy(buffer + offset, &params.abNote, sizeOfByte);
    offset += sizeOfByte;
    debug(1, "[state manager] serializing, abNote: %d", params.abNote);

    // Serialize bank
    offset += lb.serialize(buffer + offset);
    offset += gb.serialize(buffer + offset);
    offset += sb.serialize(buffer + offset, lb, gb);

    // Write
    flash.write(0x0, buffer, offset);

    debug(1, "[state manager] serialized %d bytes", offset);

    return offset;
}

size_t StateManager::deserialize()
{
    size_t offset = 0;
    uint8_t buffer[FLASH_BUFFER_SIZE]{};

    // Read
    flash.read(0x0, buffer, FLASH_BUFFER_SIZE);

    // Deserialize params
    memcpy(&params.abNote, buffer + offset, sizeOfByte);
    offset += sizeOfByte;
    debug(1, "[state manager] deserializing, abNote: %d", params.abNote);

    // Deserialize bank
    offset += lb.deserialize(buffer + offset);
    offset += gb.deserialize(buffer + offset);
    offset += sb.deserialize(buffer + offset, lb, gb);

    debug(1, "[state manager] deserialized %d bytes", offset);

    return offset;
}
