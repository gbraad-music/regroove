#include "midi_sysex.h"
#include <stdio.h>
#include <string.h>

// Current device ID
static uint8_t local_device_id = 0;

// Callback for incoming messages
static SysExCallback message_callback = NULL;
static void *callback_userdata = NULL;

void sysex_init(uint8_t device_id) {
    local_device_id = device_id & 0x7F;  // Ensure 7-bit
    printf("[SysEx] Initialized with device ID: %d\n", local_device_id);
}

void sysex_set_device_id(uint8_t device_id) {
    local_device_id = device_id & 0x7F;
    printf("[SysEx] Device ID set to: %d\n", local_device_id);
}

uint8_t sysex_get_device_id(void) {
    return local_device_id;
}

void sysex_register_callback(SysExCallback callback, void *userdata) {
    message_callback = callback;
    callback_userdata = userdata;
}

int sysex_parse_message(const uint8_t *msg, size_t msg_len) {
    // Minimum valid message: F0 7D <dev> <cmd> F7 = 5 bytes
    if (!msg || msg_len < 5) return 0;

    // Check for SysEx start
    if (msg[0] != SYSEX_START) return 0;

    // Check for our manufacturer ID
    if (msg[1] != SYSEX_MANUFACTURER_ID) return 0;

    // Check for SysEx end
    if (msg[msg_len - 1] != SYSEX_END) return 0;

    // Extract device ID and command
    uint8_t device_id = msg[2];
    uint8_t command = msg[3];

    // Check if message is for us (or broadcast)
    if (device_id != local_device_id && device_id != SYSEX_DEVICE_BROADCAST) {
        return 0;  // Not for us
    }

    // Extract data (everything between command and end byte)
    const uint8_t *data = (msg_len > 5) ? &msg[4] : NULL;
    size_t data_len = (msg_len > 5) ? (msg_len - 5) : 0;

    printf("[SysEx] Received %s from device %d (data_len=%zu)\n",
           sysex_command_name((SysExCommand)command), device_id, data_len);

    // Call registered callback
    if (message_callback) {
        message_callback(device_id, (SysExCommand)command, data, data_len, callback_userdata);
    }

    return 1;  // Message was handled
}

// --- Message Building Functions ---

size_t sysex_build_ping(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 5) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_PING;
    buffer[4] = SYSEX_END;

    return 5;
}

size_t sysex_build_file_load(uint8_t target_device_id, const char *filename,
                              uint8_t *buffer, size_t buffer_size) {
    if (!buffer || !filename) return 0;

    size_t filename_len = strlen(filename);
    if (filename_len == 0 || filename_len > 255) return 0;

    // Calculate required size: F0 7D <dev> <cmd> <len> <name...> F7
    size_t required = 6 + filename_len;
    if (buffer_size < required) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_FILE_LOAD;
    buffer[4] = (uint8_t)filename_len;
    memcpy(&buffer[5], filename, filename_len);
    buffer[5 + filename_len] = SYSEX_END;

    return required;
}

size_t sysex_build_play(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 5) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_PLAY;
    buffer[4] = SYSEX_END;

    return 5;
}

size_t sysex_build_stop(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 5) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_STOP;
    buffer[4] = SYSEX_END;

    return 5;
}

size_t sysex_build_pause(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 5) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_PAUSE;
    buffer[4] = SYSEX_END;

    return 5;
}

size_t sysex_build_channel_mute(uint8_t target_device_id, uint8_t channel, uint8_t mute,
                                 uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 7) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_CHANNEL_MUTE;
    buffer[4] = channel & 0x7F;
    buffer[5] = mute ? 1 : 0;
    buffer[6] = SYSEX_END;

    return 7;
}

size_t sysex_build_channel_solo(uint8_t target_device_id, uint8_t channel, uint8_t solo,
                                 uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 7) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_CHANNEL_SOLO;
    buffer[4] = channel & 0x7F;
    buffer[5] = solo ? 1 : 0;
    buffer[6] = SYSEX_END;

    return 7;
}

size_t sysex_build_channel_volume(uint8_t target_device_id, uint8_t channel, uint8_t volume,
                                   uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 7) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_CHANNEL_VOLUME;
    buffer[4] = channel & 0x7F;
    buffer[5] = volume & 0x7F;
    buffer[6] = SYSEX_END;

    return 7;
}

size_t sysex_build_jump_to_order_row(uint8_t target_device_id, uint8_t order, uint8_t row,
                                      uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 7) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_JUMP_TO_ORDER_ROW;
    buffer[4] = order & 0x7F;  // Order (0-127, but tracker supports 0-255)
    buffer[5] = row & 0x7F;    // Row (0-127, but tracker supports 0-255)
    buffer[6] = SYSEX_END;

    return 7;
}

size_t sysex_build_jump_to_pattern_row(uint8_t target_device_id, uint8_t pattern, uint8_t row,
                                        uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 7) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_JUMP_TO_PATTERN_ROW;
    buffer[4] = pattern & 0x7F;  // Pattern (0-127, but tracker supports 0-255)
    buffer[5] = row & 0x7F;      // Row (0-127, but tracker supports 0-255)
    buffer[6] = SYSEX_END;

    return 7;
}

size_t sysex_build_set_loop_range(uint8_t target_device_id,
                                   uint8_t start_order, uint8_t start_row,
                                   uint8_t end_order, uint8_t end_row,
                                   uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 9) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_SET_LOOP_RANGE;
    buffer[4] = start_order & 0x7F;
    buffer[5] = start_row & 0x7F;
    buffer[6] = end_order & 0x7F;
    buffer[7] = end_row & 0x7F;
    buffer[8] = SYSEX_END;

    return 9;
}

size_t sysex_build_set_bpm(uint8_t target_device_id, uint16_t bpm,
                           uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 7) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_SET_TEMPO;
    buffer[4] = bpm & 0x7F;        // LSB
    buffer[5] = (bpm >> 7) & 0x7F; // MSB
    buffer[6] = SYSEX_END;

    return 7;
}

size_t sysex_build_set_loop_current(uint8_t target_device_id, uint8_t enable,
                                     uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_SET_LOOP_CURRENT;
    buffer[4] = enable ? 1 : 0;
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_set_loop_order(uint8_t target_device_id, uint8_t order_number,
                                   uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_SET_LOOP_ORDER;
    buffer[4] = order_number & 0x7F;
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_set_loop_pattern(uint8_t target_device_id, uint8_t pattern_number,
                                     uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_SET_LOOP_PATTERN;
    buffer[4] = pattern_number & 0x7F;
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_trigger_phrase(uint8_t target_device_id, uint8_t phrase_index,
                                   uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_TRIGGER_PHRASE;
    buffer[4] = phrase_index & 0x7F;
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_trigger_loop(uint8_t target_device_id, uint8_t loop_index,
                                 uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_TRIGGER_LOOP;
    buffer[4] = loop_index & 0x7F;
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_trigger_pad(uint8_t target_device_id, uint8_t pad_index,
                                uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_TRIGGER_PAD;
    buffer[4] = pad_index & 0x7F;
    buffer[5] = SYSEX_END;

    return 6;
}

// --- Helper Functions ---

const char* sysex_command_name(SysExCommand cmd) {
    switch (cmd) {
        case SYSEX_CMD_PING:           return "PING";
        case SYSEX_CMD_FILE_LOAD:      return "FILE_LOAD";
        case SYSEX_CMD_PLAY:           return "PLAY";
        case SYSEX_CMD_STOP:           return "STOP";
        case SYSEX_CMD_PAUSE:          return "PAUSE";
        case SYSEX_CMD_CHANNEL_MUTE:        return "CHANNEL_MUTE";
        case SYSEX_CMD_CHANNEL_SOLO:        return "CHANNEL_SOLO";
        case SYSEX_CMD_CHANNEL_VOLUME:      return "CHANNEL_VOLUME";
        case SYSEX_CMD_JUMP_TO_ORDER_ROW:   return "JUMP_TO_ORDER_ROW";
        case SYSEX_CMD_JUMP_TO_PATTERN_ROW: return "JUMP_TO_PATTERN_ROW";
        case SYSEX_CMD_SET_LOOP_RANGE:      return "SET_LOOP_RANGE";
        case SYSEX_CMD_SET_TEMPO:         return "SET_TEMPO";
        case SYSEX_CMD_SET_LOOP_CURRENT:  return "SET_LOOP_CURRENT";
        case SYSEX_CMD_SET_LOOP_ORDER:    return "SET_LOOP_ORDER";
        case SYSEX_CMD_SET_LOOP_PATTERN:  return "SET_LOOP_PATTERN";
        case SYSEX_CMD_TRIGGER_PHRASE:    return "TRIGGER_PHRASE";
        case SYSEX_CMD_TRIGGER_LOOP:      return "TRIGGER_LOOP";
        case SYSEX_CMD_TRIGGER_PAD:       return "TRIGGER_PAD";
        default:                       return "UNKNOWN";
    }
}

int sysex_is_valid_device_id(uint8_t device_id) {
    return device_id <= 0x7F;
}
