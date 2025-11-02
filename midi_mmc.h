#ifndef MMC_H
#define MMC_H

#include <stddef.h>
#include <stdint.h>

// MIDI Machine Control (MMC) Implementation
// Standard: MMA/AMEI RP-013 / RP-032
// Format: F0 7F <device_id> 06 <command> [<data>...] F7

// MMC Universal SysEx IDs
#define MMC_SYSEX_START        0xF0
#define MMC_SYSEX_END          0xF7
#define MMC_UNIVERSAL_RT       0x7F  // Universal Real-Time SysEx ID
#define MMC_SUB_ID_MMC         0x06  // MMC Sub-ID

// MMC Device IDs
#define MMC_DEVICE_ALL         0x7F  // All devices
#define MMC_DEVICE_DEFAULT     0x00  // Default device

// MMC Command Codes
typedef enum {
    MMC_CMD_STOP           = 0x01,  // Stop
    MMC_CMD_PLAY           = 0x02,  // Play
    MMC_CMD_DEFERRED_PLAY  = 0x03,  // Deferred Play
    MMC_CMD_FAST_FORWARD   = 0x04,  // Fast Forward
    MMC_CMD_REWIND         = 0x05,  // Rewind
    MMC_CMD_RECORD_STROBE  = 0x06,  // Record Strobe (Punch In)
    MMC_CMD_RECORD_EXIT    = 0x07,  // Record Exit (Punch Out)
    MMC_CMD_RECORD_PAUSE   = 0x08,  // Record Pause
    MMC_CMD_PAUSE          = 0x09,  // Pause
    MMC_CMD_EJECT          = 0x0A,  // Eject
    MMC_CMD_CHASE          = 0x0B,  // Chase
    MMC_CMD_LOCATE         = 0x44,  // Locate (Go to position)
    MMC_CMD_SHUTTLE        = 0x47,  // Shuttle
} MMCCommand;

// MMC Locate Information Type (for LOCATE command)
typedef enum {
    MMC_LOCATE_TARGET      = 0x01,  // Target position (where to go)
    MMC_LOCATE_LOOP_START  = 0x04,  // Loop start position
    MMC_LOCATE_LOOP_END    = 0x05,  // Loop end position
} MMCLocateType;

// MMC Position Format (Regroove uses custom: Order + Row)
// We'll encode as: HH:MM:SS:FF format where:
// - HH = Order (0-255)
// - MM = Row (0-255)
// - SS:FF = 00:00 (unused)
typedef struct {
    uint8_t hours;      // Order number
    uint8_t minutes;    // Row number
    uint8_t seconds;    // Reserved (0)
    uint8_t frames;     // Reserved (0)
} MMCPosition;

// MMC message callback
typedef void (*MMCCallback)(uint8_t device_id, MMCCommand command,
                            const uint8_t *data, size_t data_len, void *userdata);

// Initialize MMC system
void mmc_init(uint8_t device_id);

// Set device ID (0-127, or 0x7F for all devices)
void mmc_set_device_id(uint8_t device_id);

// Get current device ID
uint8_t mmc_get_device_id(void);

// Register callback for incoming MMC commands
void mmc_register_callback(MMCCallback callback, void *userdata);

// Parse incoming MIDI message - returns 1 if it was a valid MMC message
int mmc_parse_message(const uint8_t *msg, size_t msg_len);

// --- MMC Message Building Functions ---

// Build STOP command
size_t mmc_build_stop(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build PLAY command
size_t mmc_build_play(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build PAUSE command
size_t mmc_build_pause(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build RECORD_STROBE command (start recording)
size_t mmc_build_record_start(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build RECORD_EXIT command (stop recording)
size_t mmc_build_record_stop(uint8_t target_device_id, uint8_t *buffer, size_t buffer_size);

// Build LOCATE command (go to position: order + row)
// order: Pattern order number (0-255)
// row: Row within pattern (0-255)
// loop_mode: 0 = one-shot, 1 = loop
size_t mmc_build_locate(uint8_t target_device_id, uint8_t order, uint8_t row,
                        uint8_t loop_mode, uint8_t *buffer, size_t buffer_size);

// Helper: Convert Regroove position (order, row) to MMC position
void mmc_position_from_order_row(MMCPosition *pos, uint8_t order, uint8_t row);

// Helper: Convert MMC position to Regroove position (order, row)
void mmc_position_to_order_row(const MMCPosition *pos, uint8_t *order, uint8_t *row);

#endif // MMC_H
