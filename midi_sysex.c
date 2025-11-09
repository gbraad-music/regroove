#include "midi_sysex.h"
#include <stdio.h>
#include <string.h>

// Pointer to device ID (managed externally, typically in device config)
static const uint8_t *local_device_id_ptr = NULL;

// Callback for incoming messages
static SysExCallback message_callback = NULL;
static void *callback_userdata = NULL;

void sysex_init(const uint8_t *device_id_ptr) {
    local_device_id_ptr = device_id_ptr;
    if (device_id_ptr) {
        printf("[SysEx] Initialized with device ID pointer (current value: %d)\n", *device_id_ptr);
    } else {
        printf("[SysEx] Warning: Initialized with NULL device ID pointer\n");
    }
}

uint8_t sysex_get_device_id(void) {
    return local_device_id_ptr ? *local_device_id_ptr : 0;
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
    uint8_t our_device_id = local_device_id_ptr ? *local_device_id_ptr : 0;
    if (device_id != our_device_id && device_id != SYSEX_DEVICE_BROADCAST) {
        return 0;  // Not for us
    }

    // Extract data (everything between command and end byte)
    const uint8_t *data = (msg_len > 5) ? &msg[4] : NULL;
    size_t data_len = (msg_len > 5) ? (msg_len - 5) : 0;

    // Call registered callback (debug output moved to callback for selective verbosity)
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

size_t sysex_build_retrigger(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 5) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_RETRIGGER;
    buffer[4] = SYSEX_END;

    return 5;
}

size_t sysex_build_next_order(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 5) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_NEXT_ORDER;
    buffer[4] = SYSEX_END;

    return 5;
}

size_t sysex_build_prev_order(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 5) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_PREV_ORDER;
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

size_t sysex_build_master_volume(uint8_t target_device_id, uint8_t volume,
                                  uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_MASTER_VOLUME;
    buffer[4] = volume & 0x7F;
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_master_mute(uint8_t target_device_id, uint8_t mute,
                                uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_MASTER_MUTE;
    buffer[4] = mute ? 1 : 0;
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_input_volume(uint8_t target_device_id, uint8_t volume,
                                 uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_INPUT_VOLUME;
    buffer[4] = volume & 0x7F;
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_input_mute(uint8_t target_device_id, uint8_t mute,
                               uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_INPUT_MUTE;
    buffer[4] = mute ? 1 : 0;
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_fx_set_route(uint8_t target_device_id, uint8_t route,
                                 uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_FX_SET_ROUTE;
    buffer[4] = route & 0x03;  // 0-3 only
    buffer[5] = SYSEX_END;

    return 6;
}

size_t sysex_build_stereo_separation(uint8_t target_device_id, uint8_t separation,
                                      uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_STEREO_SEPARATION;
    buffer[4] = separation & 0x7F;  // 0-127, maps to 0-200 (multiply by 200/127)
    buffer[5] = SYSEX_END;

    return 6;
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

size_t sysex_build_get_player_state(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 5) return 0;

    buffer[0] = SYSEX_START;
    buffer[1] = SYSEX_MANUFACTURER_ID;
    buffer[2] = target_device_id & 0x7F;
    buffer[3] = SYSEX_CMD_GET_PLAYER_STATE;
    buffer[4] = SYSEX_END;

    return 5;
}

size_t sysex_build_player_state_response(uint8_t target_device_id,
                                          uint8_t playback_flags,
                                          uint8_t order, uint8_t row,
                                          uint8_t pattern, uint8_t total_rows,
                                          uint8_t num_channels,
                                          uint8_t master_volume,
                                          uint8_t master_mute,
                                          uint8_t input_volume,
                                          uint8_t input_mute,
                                          uint8_t fx_route,
                                          uint8_t stereo_separation,
                                          uint16_t bpm,
                                          uint8_t master_pan,
                                          uint8_t input_pan,
                                          const uint8_t *mute_bits,
                                          const uint8_t *channel_volumes,
                                          const uint8_t *channel_panning,
                                          uint8_t *buffer, size_t buffer_size) {
    if (!buffer || !mute_bits || !channel_volumes || !channel_panning || num_channels == 0 || num_channels > 127) return 0;

    // Calculate required size: F0 7D <dev> <cmd> <15 header bytes> <mute_bytes> <num_channels vol bytes> <num_channels pan bytes> F7
    size_t mute_bytes = (num_channels + 7) / 8;  // ceil(num_channels / 8)
    size_t required = 5 + 15 + mute_bytes + num_channels + num_channels + 1;  // start + manufacturer + device + cmd + data + end
    if (buffer_size < required) return 0;

    size_t pos = 0;
    buffer[pos++] = SYSEX_START;
    buffer[pos++] = SYSEX_MANUFACTURER_ID;
    buffer[pos++] = target_device_id & 0x7F;
    buffer[pos++] = SYSEX_CMD_PLAYER_STATE_RESPONSE;

    // Header data
    buffer[pos++] = playback_flags & 0x7F;
    buffer[pos++] = order & 0x7F;
    buffer[pos++] = row & 0x7F;
    buffer[pos++] = pattern & 0x7F;
    buffer[pos++] = total_rows & 0x7F;
    buffer[pos++] = num_channels & 0x7F;
    buffer[pos++] = master_volume & 0x7F;

    // Mixer flags (byte 7)
    uint8_t mixer_flags = 0;
    if (master_mute) mixer_flags |= 0x01;  // bit 0: master mute
    if (input_mute) mixer_flags |= 0x02;   // bit 1: input mute
    buffer[pos++] = mixer_flags & 0x7F;

    buffer[pos++] = input_volume & 0x7F;   // byte 8
    buffer[pos++] = fx_route & 0x7F;       // byte 9
    buffer[pos++] = stereo_separation & 0x7F;  // byte 10
    buffer[pos++] = bpm & 0x7F;            // byte 11: BPM LSB
    buffer[pos++] = (bpm >> 7) & 0x7F;     // byte 12: BPM MSB
    buffer[pos++] = master_pan & 0x7F;     // byte 13: master pan
    buffer[pos++] = input_pan & 0x7F;      // byte 14: input pan

    // Mute bits (byte 15+)
    memcpy(&buffer[pos], mute_bits, mute_bytes);
    pos += mute_bytes;

    // Channel volumes
    memcpy(&buffer[pos], channel_volumes, num_channels);
    pos += num_channels;

    // Channel panning
    memcpy(&buffer[pos], channel_panning, num_channels);
    pos += num_channels;

    buffer[pos++] = SYSEX_END;

    return pos;
}

int sysex_parse_player_state_response(const uint8_t *data, size_t data_len,
                                       uint8_t *out_playback_flags,
                                       uint8_t *out_order, uint8_t *out_row,
                                       uint8_t *out_pattern, uint8_t *out_total_rows,
                                       uint8_t *out_num_channels,
                                       uint8_t *out_master_volume,
                                       uint8_t *out_master_mute,
                                       uint8_t *out_input_volume,
                                       uint8_t *out_input_mute,
                                       uint8_t *out_fx_route,
                                       uint8_t *out_stereo_separation,
                                       uint16_t *out_bpm,
                                       uint8_t *out_master_pan,
                                       uint8_t *out_input_pan,
                                       uint8_t *out_mute_bits,
                                       uint8_t *out_channel_volumes,
                                       uint8_t *out_channel_panning) {
    // Minimum: 15 header bytes + at least 1 mute byte + at least 1 channel volume + at least 1 channel pan
    if (!data || data_len < 18) return 0;

    // Extract header
    if (out_playback_flags) *out_playback_flags = data[0];
    if (out_order) *out_order = data[1];
    if (out_row) *out_row = data[2];
    if (out_pattern) *out_pattern = data[3];
    if (out_total_rows) *out_total_rows = data[4];

    uint8_t num_channels = data[5];
    if (out_num_channels) *out_num_channels = num_channels;

    if (out_master_volume) *out_master_volume = data[6];

    // Extract mixer flags (byte 7)
    uint8_t mixer_flags = data[7];
    if (out_master_mute) *out_master_mute = (mixer_flags & 0x01) ? 1 : 0;
    if (out_input_mute) *out_input_mute = (mixer_flags & 0x02) ? 1 : 0;

    if (out_input_volume) *out_input_volume = data[8];
    if (out_fx_route) *out_fx_route = data[9];
    if (out_stereo_separation) *out_stereo_separation = data[10];

    // Extract BPM (bytes 11-12)
    if (out_bpm) {
        uint8_t bpm_lsb = data[11];
        uint8_t bpm_msb = data[12];
        *out_bpm = bpm_lsb | (bpm_msb << 7);
    }

    // Extract panning (bytes 13-14)
    if (out_master_pan) *out_master_pan = data[13];
    if (out_input_pan) *out_input_pan = data[14];

    // Validate data length
    size_t mute_bytes = (num_channels + 7) / 8;
    if (data_len < 15 + mute_bytes + num_channels + num_channels) return 0;

    // Extract mute bits (byte 15+)
    if (out_mute_bits) {
        memcpy(out_mute_bits, &data[15], mute_bytes);
    }

    // Extract channel volumes
    if (out_channel_volumes) {
        memcpy(out_channel_volumes, &data[15 + mute_bytes], num_channels);
    }

    // Extract channel panning
    if (out_channel_panning) {
        memcpy(out_channel_panning, &data[15 + mute_bytes + num_channels], num_channels);
    }

    return 1;
}

// --- Helper Functions ---

const char* sysex_command_name(SysExCommand cmd) {
    switch (cmd) {
        case SYSEX_CMD_PING:           return "PING";
        case SYSEX_CMD_FILE_LOAD:      return "FILE_LOAD";
        case SYSEX_CMD_PLAY:           return "PLAY";
        case SYSEX_CMD_STOP:           return "STOP";
        case SYSEX_CMD_PAUSE:          return "PAUSE";
        case SYSEX_CMD_RETRIGGER:      return "RETRIGGER";
        case SYSEX_CMD_NEXT_ORDER:     return "NEXT_ORDER";
        case SYSEX_CMD_PREV_ORDER:     return "PREV_ORDER";
        case SYSEX_CMD_CHANNEL_MUTE:        return "CHANNEL_MUTE";
        case SYSEX_CMD_CHANNEL_SOLO:        return "CHANNEL_SOLO";
        case SYSEX_CMD_CHANNEL_VOLUME:      return "CHANNEL_VOLUME";
        case SYSEX_CMD_MASTER_VOLUME:       return "MASTER_VOLUME";
        case SYSEX_CMD_MASTER_MUTE:         return "MASTER_MUTE";
        case SYSEX_CMD_INPUT_VOLUME:        return "INPUT_VOLUME";
        case SYSEX_CMD_INPUT_MUTE:          return "INPUT_MUTE";
        case SYSEX_CMD_FX_SET_ROUTE:        return "FX_SET_ROUTE";
        case SYSEX_CMD_STEREO_SEPARATION:   return "STEREO_SEPARATION";
        case SYSEX_CMD_CHANNEL_PANNING:     return "CHANNEL_PANNING";
        case SYSEX_CMD_MASTER_PANNING:      return "MASTER_PANNING";
        case SYSEX_CMD_INPUT_PANNING:       return "INPUT_PANNING";
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
        case SYSEX_CMD_GET_PLAYER_STATE:      return "GET_PLAYER_STATE";
        case SYSEX_CMD_PLAYER_STATE_RESPONSE: return "PLAYER_STATE_RESPONSE";
        default:                       return "UNKNOWN";
    }
}

int sysex_is_valid_device_id(uint8_t device_id) {
    return device_id <= 0x7F;
}
