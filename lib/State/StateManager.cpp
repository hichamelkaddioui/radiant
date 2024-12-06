#include <Utils.h>
#include <StateManager.h>

void StateManager::setup()
{
    lb.setup();
}

void StateManager::update()
{
    sb.getCurrentScene()->update();
}

void StateManager::createAndSaveStubs(RP2040Flash flash)
{
    // Create banks
    LedBank localLedBank = LedBank::createDummy();
    GraphBank localGraphBank = GraphBank::createDummy();
    SceneBank localSceneBank = SceneBank::createDummy(localLedBank, localGraphBank);
    StateManager localStateManager(localLedBank, localGraphBank, localSceneBank);

    // Serialize
    localStateManager.serialize(flash);
}

Scene *StateManager::getCurrentScene()
{
    return sb.getCurrentScene();
}

int StateManager::getCurrentSceneId()
{
    return sb.currentSceneId;
}

void StateManager::serialize(RP2040Flash flash)
{
    size_t offset = 0;
    uint8_t buffer[FLASH_BUFFER_SIZE]{};

    offset += lb.serialize(buffer + offset);
    offset += gb.serialize(buffer + offset);
    offset += sb.serialize(buffer + offset, lb, gb);

    flash.write(0x0, buffer, offset);
}

void StateManager::deserialize(RP2040Flash flash)
{
    size_t offset = 0;
    uint8_t buffer[FLASH_BUFFER_SIZE]{};

    flash.read(0x0, buffer, FLASH_BUFFER_SIZE);

    // Deserialize
    offset += lb.deserialize(buffer);
    offset += gb.deserialize(buffer + offset);
    offset += sb.deserialize(buffer + offset, lb, gb);

    debug(1, "[flash] deserialized %d bytes", offset);
}

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
    case SysExMessage::SET_PARAMS:
    case SysExMessage::SET_STROBE_A:
    case SysExMessage::SET_STROBE_B:
    default:
        debugByteArray(buffer, length);
        break;
    }
}

void StateManager::handleOledButtonPress()
{
    sb.next();
}