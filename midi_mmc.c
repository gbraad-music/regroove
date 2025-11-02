#include "midi_mmc.h"
#include <stdio.h>
#include <string.h>

// Global state
static uint8_t mmc_device_id = MMC_DEVICE_DEFAULT;
static MMCCallback mmc_callback = NULL;
static void *mmc_callback_userdata = NULL;

void mmc_init(uint8_t device_id) {
    mmc_device_id = device_id;
    printf("[MMC] Initialized with device ID: %d\n", device_id);
}

void mmc_set_device_id(uint8_t device_id) {
    mmc_device_id = device_id;
}

uint8_t mmc_get_device_id(void) {
    return mmc_device_id;
}

void mmc_register_callback(MMCCallback callback, void *userdata) {
    mmc_callback = callback;
    mmc_callback_userdata = userdata;
}

int mmc_parse_message(const uint8_t *msg, size_t msg_len) {
    // Minimum MMC message: F0 7F <dev> 06 <cmd> F7 = 6 bytes
    if (!msg || msg_len < 6) return 0;

    // Check for MMC message format: F0 7F <device_id> 06 <command> [data...] F7
    if (msg[0] != MMC_SYSEX_START ||
        msg[1] != MMC_UNIVERSAL_RT ||
        msg[3] != MMC_SUB_ID_MMC ||
        msg[msg_len - 1] != MMC_SYSEX_END) {
        return 0;
    }

    uint8_t device_id = msg[2];
    uint8_t command = msg[4];
    const uint8_t *data = (msg_len > 6) ? &msg[5] : NULL;
    size_t data_len = (msg_len > 6) ? (msg_len - 6) : 0;

    // Check if message is for us (our device ID or broadcast)
    if (device_id != mmc_device_id && device_id != MMC_DEVICE_ALL) {
        printf("[MMC] Message for device %d (we are %d), ignoring\n", device_id, mmc_device_id);
        return 1;  // Valid MMC message, just not for us
    }

    printf("[MMC] Received command 0x%02X from device %d, data_len=%zu\n",
           command, device_id, data_len);

    // Dispatch to callback
    if (mmc_callback) {
        mmc_callback(device_id, (MMCCommand)command, data, data_len, mmc_callback_userdata);
    }

    return 1;
}

size_t mmc_build_stop(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = MMC_SYSEX_START;
    buffer[1] = MMC_UNIVERSAL_RT;
    buffer[2] = target_device_id;
    buffer[3] = MMC_SUB_ID_MMC;
    buffer[4] = MMC_CMD_STOP;
    buffer[5] = MMC_SYSEX_END;

    return 6;
}

size_t mmc_build_play(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = MMC_SYSEX_START;
    buffer[1] = MMC_UNIVERSAL_RT;
    buffer[2] = target_device_id;
    buffer[3] = MMC_SUB_ID_MMC;
    buffer[4] = MMC_CMD_PLAY;
    buffer[5] = MMC_SYSEX_END;

    return 6;
}

size_t mmc_build_pause(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = MMC_SYSEX_START;
    buffer[1] = MMC_UNIVERSAL_RT;
    buffer[2] = target_device_id;
    buffer[3] = MMC_SUB_ID_MMC;
    buffer[4] = MMC_CMD_PAUSE;
    buffer[5] = MMC_SYSEX_END;

    return 6;
}

size_t mmc_build_record_start(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = MMC_SYSEX_START;
    buffer[1] = MMC_UNIVERSAL_RT;
    buffer[2] = target_device_id;
    buffer[3] = MMC_SUB_ID_MMC;
    buffer[4] = MMC_CMD_RECORD_STROBE;
    buffer[5] = MMC_SYSEX_END;

    return 6;
}

size_t mmc_build_record_stop(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 6) return 0;

    buffer[0] = MMC_SYSEX_START;
    buffer[1] = MMC_UNIVERSAL_RT;
    buffer[2] = target_device_id;
    buffer[3] = MMC_SUB_ID_MMC;
    buffer[4] = MMC_CMD_RECORD_EXIT;
    buffer[5] = MMC_SYSEX_END;

    return 6;
}

size_t mmc_build_locate(uint8_t target_device_id, uint8_t order, uint8_t row,
                        uint8_t loop_mode, uint8_t *buffer, size_t buffer_size) {
    // MMC LOCATE format: F0 7F <dev> 06 44 06 01 <hr> <mn> <sc> <fr> <sf> F7
    // 06 = Information Field length (6 bytes)
    // 01 = Standard time (could be 0x04 for loop start, 0x05 for loop end)
    // We use Order=hours, Row=minutes, loop_mode determines if it's loop point
    if (!buffer || buffer_size < 13) return 0;

    MMCPosition pos;
    mmc_position_from_order_row(&pos, order, row);

    buffer[0] = MMC_SYSEX_START;
    buffer[1] = MMC_UNIVERSAL_RT;
    buffer[2] = target_device_id;
    buffer[3] = MMC_SUB_ID_MMC;
    buffer[4] = MMC_CMD_LOCATE;
    buffer[5] = 0x06;  // Information field length
    buffer[6] = loop_mode ? MMC_LOCATE_LOOP_START : MMC_LOCATE_TARGET;
    buffer[7] = pos.hours;    // Order
    buffer[8] = pos.minutes;  // Row
    buffer[9] = pos.seconds;  // 0
    buffer[10] = pos.frames;  // 0
    buffer[11] = 0x00;        // Subframes
    buffer[12] = MMC_SYSEX_END;

    return 13;
}

void mmc_position_from_order_row(MMCPosition *pos, uint8_t order, uint8_t row) {
    if (!pos) return;
    pos->hours = order;
    pos->minutes = row;
    pos->seconds = 0;
    pos->frames = 0;
}

void mmc_position_to_order_row(const MMCPosition *pos, uint8_t *order, uint8_t *row) {
    if (!pos) return;
    if (order) *order = pos->hours;
    if (row) *row = pos->minutes;
}
