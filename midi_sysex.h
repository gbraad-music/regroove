#ifndef SYSEX_H
#define SYSEX_H

#include <stddef.h>
#include <stdint.h>

// SysEx Message Format for Regroove Inter-Instance Communication
// F0 7D <device_id> <command> [<data>...] F7
//
// F0 = SysEx Start
// 7D = Manufacturer ID (Educational/Research use)
// <device_id> = 0-127, identifies target Regroove instance
// <command> = Command byte (see below)
// [<data>...] = Variable-length command data
// F7 = SysEx End

// Manufacturer ID for educational/research/non-commercial use
#define SYSEX_MANUFACTURER_ID 0x7D

// SysEx Start/End bytes
#define SYSEX_START 0xF0
#define SYSEX_END   0xF7

// Special device IDs
#define SYSEX_DEVICE_BROADCAST 0x7F  // Broadcast to all devices
#define SYSEX_DEVICE_ANY       0x7E  // Accept from any device (for receiving)

// SysEx Command Codes
typedef enum {
    SYSEX_CMD_PING              = 0x01,  // Device discovery/heartbeat
    SYSEX_CMD_FILE_LOAD         = 0x10,  // Load file by name
    SYSEX_CMD_PLAY              = 0x20,  // Start playback
    SYSEX_CMD_STOP              = 0x21,  // Stop playback
    SYSEX_CMD_PAUSE             = 0x22,  // Pause/Continue
    SYSEX_CMD_RETRIGGER         = 0x23,  // Retrigger current pattern
    SYSEX_CMD_CHANNEL_MUTE      = 0x30,  // Mute/unmute channel
    SYSEX_CMD_CHANNEL_SOLO      = 0x31,  // Solo/unsolo channel
    SYSEX_CMD_CHANNEL_VOLUME    = 0x32,  // Set channel volume
    // Position/playback control
    SYSEX_CMD_JUMP_TO_ORDER_ROW   = 0x40,  // Jump to order + row (immediate)
    SYSEX_CMD_JUMP_TO_PATTERN_ROW = 0x46,  // Jump to pattern + row (immediate)
    SYSEX_CMD_SET_LOOP_RANGE      = 0x41,  // Set loop: start_order, start_row, end_order, end_row
    SYSEX_CMD_SET_TEMPO           = 0x42,  // Set playback tempo (pitch multiplier)
    SYSEX_CMD_SET_LOOP_CURRENT  = 0x43,  // Loop current pattern (1=enable, 0=disable)
    SYSEX_CMD_SET_LOOP_ORDER    = 0x44,  // Loop specific order number
    SYSEX_CMD_SET_LOOP_PATTERN  = 0x45,  // Loop specific pattern number
    // Performance triggers
    SYSEX_CMD_TRIGGER_PHRASE    = 0x50,  // Trigger phrase by index
    SYSEX_CMD_TRIGGER_LOOP      = 0x51,  // Trigger saved loop range by index
    SYSEX_CMD_TRIGGER_PAD       = 0x52,  // Trigger application/song pad by index
    // State query/response (for visualization)
    SYSEX_CMD_GET_CHANNEL_STATE = 0x60,  // Request channel state (all channels)
    SYSEX_CMD_CHANNEL_STATE_RESPONSE = 0x61,  // Response with channel state data
} SysExCommand;

// SysEx message callback
// Called when a valid Regroove SysEx message is received
// device_id: sender's device ID
// command: command code
// data: command data bytes
// data_len: length of data
typedef void (*SysExCallback)(uint8_t device_id, SysExCommand command,
                              const uint8_t *data, size_t data_len, void *userdata);

// Initialize SysEx system with this device's ID
void sysex_init(uint8_t device_id);

// Set device ID (0-127)
void sysex_set_device_id(uint8_t device_id);

// Get current device ID
uint8_t sysex_get_device_id(void);

// Register callback for incoming SysEx commands
void sysex_register_callback(SysExCallback callback, void *userdata);

// Parse incoming MIDI message - returns 1 if it was a valid Regroove SysEx message
int sysex_parse_message(const uint8_t *msg, size_t msg_len);

// --- SysEx Message Building Functions ---

// Build PING message
// Returns message length, fills buffer
size_t sysex_build_ping(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build FILE_LOAD message
// filename: null-terminated filename string
size_t sysex_build_file_load(uint8_t target_device_id, const char *filename,
                              uint8_t *buffer, size_t buffer_size);

// Build PLAY message
size_t sysex_build_play(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build STOP message
size_t sysex_build_stop(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build PAUSE message
size_t sysex_build_pause(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build CHANNEL_MUTE message
// channel: channel index (0-63)
// mute: 1 = mute, 0 = unmute
size_t sysex_build_channel_mute(uint8_t target_device_id, uint8_t channel, uint8_t mute,
                                 uint8_t *buffer, size_t buffer_size);

// Build CHANNEL_SOLO message
// channel: channel index (0-63)
// solo: 1 = solo, 0 = unsolo
size_t sysex_build_channel_solo(uint8_t target_device_id, uint8_t channel, uint8_t solo,
                                 uint8_t *buffer, size_t buffer_size);

// Build CHANNEL_VOLUME message
// channel: channel index (0-63)
// volume: volume level (0-127)
size_t sysex_build_channel_volume(uint8_t target_device_id, uint8_t channel, uint8_t volume,
                                   uint8_t *buffer, size_t buffer_size);

// Build JUMP_TO_ORDER_ROW message
// order: pattern order number (0-255)
// row: row within pattern (0-255)
size_t sysex_build_jump_to_order_row(uint8_t target_device_id, uint8_t order, uint8_t row,
                                      uint8_t *buffer, size_t buffer_size);

// Build JUMP_TO_PATTERN_ROW message
// pattern: pattern number (0-255)
// row: row within pattern (0-255)
size_t sysex_build_jump_to_pattern_row(uint8_t target_device_id, uint8_t pattern, uint8_t row,
                                        uint8_t *buffer, size_t buffer_size);

// Build SET_LOOP_RANGE message
// Sets the loop range for playback
// start_order, start_row: loop start position
// end_order, end_row: loop end position
size_t sysex_build_set_loop_range(uint8_t target_device_id,
                                   uint8_t start_order, uint8_t start_row,
                                   uint8_t end_order, uint8_t end_row,
                                   uint8_t *buffer, size_t buffer_size);

// Build SET_TEMPO message (sets playback speed/pitch)
// bpm: tempo in BPM (16-bit value, sent as two 7-bit bytes)
size_t sysex_build_set_bpm(uint8_t target_device_id, uint16_t bpm,
                           uint8_t *buffer, size_t buffer_size);

// Build SET_LOOP_CURRENT message
// enable: 1=enable loop on current pattern, 0=disable loop
size_t sysex_build_set_loop_current(uint8_t target_device_id, uint8_t enable,
                                     uint8_t *buffer, size_t buffer_size);

// Build SET_LOOP_ORDER message
// order_number: order index to loop (0-255)
size_t sysex_build_set_loop_order(uint8_t target_device_id, uint8_t order_number,
                                   uint8_t *buffer, size_t buffer_size);

// Build SET_LOOP_PATTERN message
// pattern_number: pattern index to loop (0-255)
size_t sysex_build_set_loop_pattern(uint8_t target_device_id, uint8_t pattern_number,
                                     uint8_t *buffer, size_t buffer_size);

// Build TRIGGER_PHRASE message
// phrase_index: phrase index to trigger (0-255)
size_t sysex_build_trigger_phrase(uint8_t target_device_id, uint8_t phrase_index,
                                   uint8_t *buffer, size_t buffer_size);

// Build TRIGGER_LOOP message
// loop_index: saved loop range index (0-255)
size_t sysex_build_trigger_loop(uint8_t target_device_id, uint8_t loop_index,
                                 uint8_t *buffer, size_t buffer_size);

// Build TRIGGER_PAD message
// pad_index: pad number (0-31)
size_t sysex_build_trigger_pad(uint8_t target_device_id, uint8_t pad_index,
                                uint8_t *buffer, size_t buffer_size);

// Build RETRIGGER message
// Retriggers the current pattern from the beginning
size_t sysex_build_retrigger(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build GET_CHANNEL_STATE message
// Requests channel state information for visualization
size_t sysex_build_get_channel_state(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build CHANNEL_STATE_RESPONSE message
// Sends channel state data for visualization
// Format: For each channel (up to num_channels):
//   - byte 0: channel index
//   - byte 1: flags (bit 0: mute, bit 1: solo)
//   - byte 2: volume (0-127)
size_t sysex_build_channel_state_response(uint8_t target_device_id,
                                           const uint8_t *channel_data,
                                           size_t num_channels,
                                           uint8_t *buffer, size_t buffer_size);

// --- Helper Functions ---

// Get command name for debugging
const char* sysex_command_name(SysExCommand cmd);

// Validate device ID
int sysex_is_valid_device_id(uint8_t device_id);

#endif // SYSEX_H
