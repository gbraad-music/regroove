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
    SYSEX_CMD_NEXT_ORDER        = 0x24,  // Queue next order (beat-synced)
    SYSEX_CMD_PREV_ORDER        = 0x25,  // Queue previous order (beat-synced)
    SYSEX_CMD_CHANNEL_MUTE      = 0x30,  // Mute/unmute channel
    SYSEX_CMD_CHANNEL_SOLO      = 0x31,  // Solo/unsolo channel
    SYSEX_CMD_CHANNEL_VOLUME    = 0x32,  // Set channel volume
    SYSEX_CMD_MASTER_VOLUME     = 0x33,  // Set master volume
    SYSEX_CMD_MASTER_MUTE       = 0x34,  // Set master mute
    SYSEX_CMD_INPUT_VOLUME      = 0x35,  // Set input volume
    SYSEX_CMD_INPUT_MUTE        = 0x36,  // Set input mute
    SYSEX_CMD_FX_SET_ROUTE      = 0x37,  // Set FX routing (0=none, 1=master, 2=playback, 3=input)
    SYSEX_CMD_STEREO_SEPARATION = 0x57,  // Set stereo separation (0-200, where 100=normal)
    SYSEX_CMD_CHANNEL_PANNING   = 0x58,  // Set channel panning (0=left, 64=center, 127=right)
    SYSEX_CMD_MASTER_PANNING    = 0x59,  // Set master panning (0=left, 64=center, 127=right)
    SYSEX_CMD_INPUT_PANNING     = 0x5A,  // Set input panning (0=left, 64=center, 127=right)
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
    SYSEX_CMD_GET_PLAYER_STATE = 0x60,  // Request complete player state
    SYSEX_CMD_PLAYER_STATE_RESPONSE = 0x61,  // Response with player state data
} SysExCommand;

// SysEx message callback
// Called when a valid Regroove SysEx message is received
// device_id: sender's device ID
// command: command code
// data: command data bytes
// data_len: length of data
typedef void (*SysExCallback)(uint8_t device_id, SysExCommand command,
                              const uint8_t *data, size_t data_len, void *userdata);

// Initialize SysEx system with pointer to device ID
// The pointer must remain valid for the lifetime of the SysEx system
// This allows the device ID to be updated without additional setter calls
void sysex_init(const uint8_t *device_id_ptr);

// Get current device ID (reads from the pointer passed to sysex_init)
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

// Build MASTER_VOLUME message
// volume: master volume level (0-127)
size_t sysex_build_master_volume(uint8_t target_device_id, uint8_t volume,
                                  uint8_t *buffer, size_t buffer_size);

// Build MASTER_MUTE message
// mute: 1 = mute, 0 = unmute
size_t sysex_build_master_mute(uint8_t target_device_id, uint8_t mute,
                                uint8_t *buffer, size_t buffer_size);

// Build INPUT_VOLUME message
// volume: input volume level (0-127)
size_t sysex_build_input_volume(uint8_t target_device_id, uint8_t volume,
                                 uint8_t *buffer, size_t buffer_size);

// Build INPUT_MUTE message
// mute: 1 = mute, 0 = unmute
size_t sysex_build_input_mute(uint8_t target_device_id, uint8_t mute,
                               uint8_t *buffer, size_t buffer_size);

// Build FX_SET_ROUTE message
// route: FX routing (0=none, 1=master, 2=playback, 3=input)
size_t sysex_build_fx_set_route(uint8_t target_device_id, uint8_t route,
                                 uint8_t *buffer, size_t buffer_size);

// Build STEREO_SEPARATION message
// separation: stereo separation (0-200, where 0=mono, 100=normal, 200=wide)
size_t sysex_build_stereo_separation(uint8_t target_device_id, uint8_t separation,
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

// Build NEXT_ORDER message
// Queue jump to next order (beat-synced)
size_t sysex_build_next_order(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build PREV_ORDER message
// Queue jump to previous order (beat-synced)
size_t sysex_build_prev_order(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build GET_PLAYER_STATE message
// Requests complete player state for visualization
size_t sysex_build_get_player_state(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build PLAYER_STATE_RESPONSE message
// Sends complete player state data for visualization (Meister compatibility)
// Format:
//   - byte 0: playback flags
//       bit 0: playing (0=stopped, 1=playing)
//       bit 1-2: mode (00=song, 01=pattern/loop, 10=performance, 11=record)
//       bits 3-7: reserved (set to 0)
//   - byte 1: current order (0-127)
//   - byte 2: current row (0-127)
//   - byte 3: current pattern number (0-127)
//   - byte 4: total rows in pattern (0-127)
//   - byte 5: number of channels (1-127)
//   - byte 6: master volume (0-127, where 127 = 100% = 1.0)
//   - byte 7: mixer flags
//       bit 0: master mute (0=unmuted, 1=muted)
//       bit 1: input mute (0=unmuted, 1=muted)
//       bits 2-7: reserved (set to 0)
//   - byte 8: input volume (0-127, where 127 = 100% = 1.0)
//   - byte 9: FX routing
//       0=none, 1=master, 2=playback, 3=input
//   - byte 10: stereo separation (0-127, maps to 0-200 where 64≈100=normal)
//   - byte 11: BPM LSB (lower 7 bits)
//   - byte 12: BPM MSB (upper 7 bits)
//       BPM value = LSB | (MSB << 7), supports 0-16383 BPM
//   - byte 13: master pan (0-127, where 0=left, 64=center, 127=right)
//   - byte 14: input pan (0-127, where 0=left, 64=center, 127=right)
//   - byte 15+: bit-packed channel mute states (ceil(num_channels/8) bytes)
//       channel 0 = bit 0 of byte 15, channel 1 = bit 1 of byte 15, etc.
//       1=muted, 0=unmuted
//   - byte N+: channel volumes (num_channels bytes, each 0-127)
//       one byte per channel, where 127 = 100% volume = 1.0
//   - byte N+: channel panning (num_channels bytes, each 0-127)
//       one byte per channel, where 0=left, 64=center, 127=right
//
// Size examples:
//   - 4 channels: 15 header + 1 mute + 4 vol + 4 pan = 24 bytes
//   - 16 channels: 15 header + 2 mute + 16 vol + 16 pan = 49 bytes
//   - 64 channels: 15 header + 8 mute + 64 vol + 64 pan = 151 bytes
//
// mute_bits: pointer to bit-packed mute states (ceil(num_channels/8) bytes)
// channel_volumes: pointer to channel volume array (num_channels bytes, each 0-127)
// master_volume: 0-127 (where 127 = 100% volume = 1.0)
// master_mute: 0=unmuted, 1=muted
// input_volume: 0-127 (where 127 = 100% = 1.0)
// input_mute: 0=unmuted, 1=muted
// fx_route: 0=none, 1=master, 2=playback, 3=input
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
                                          uint8_t *buffer, size_t buffer_size);

// Helper: Parse PLAYER_STATE_RESPONSE message
// Returns 1 on success, 0 on failure
// Extracts player state from a received PLAYER_STATE_RESPONSE message data payload
// out_mute_bits must be allocated with at least ceil(num_channels/8) bytes
// out_channel_volumes must be allocated with at least num_channels bytes
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
                                       uint8_t *out_channel_panning);

// --- Helper Functions ---

// Get command name for debugging
const char* sysex_command_name(SysExCommand cmd);

// Validate device ID
int sysex_is_valid_device_id(uint8_t device_id);

#endif // SYSEX_H
