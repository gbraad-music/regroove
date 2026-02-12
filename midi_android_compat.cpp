/**
 * Android MIDI Compatibility Layer
 * Implements regroove's midi.c and midi_output.c API using RFX midi_handler
 */

#ifdef __ANDROID__

#include <cstdio>

#include "midi.h"
#include "midi_output.h"
#include "regroove_metadata.h"
#include "midi_handler.h"

extern "C" {

// MIDI Input API (from midi.c)

int midi_list_ports(void) {
    return midi_handler_get_device_count();
}

int midi_get_port_name(int port, char* name_out, int bufsize) {
    const char* name = midi_handler_get_device_name(port);
    if (!name || !name_out || bufsize == 0) {
        if (name_out && bufsize > 0) name_out[0] = '\0';
        return -1;
    }
    snprintf(name_out, bufsize, "%s", name);
    return 0;
}

static MidiEventCallback g_midi_callback = nullptr;
static void* g_midi_userdata = nullptr;

static void internal_midi_callback(const midi_message_t* msg, void* userdata) {
    (void)userdata;
    if (g_midi_callback) {
        g_midi_callback(msg->status, msg->data1, msg->data2, 0, g_midi_userdata);
    }
}

int midi_init_multi(MidiEventCallback cb, void *userdata, const int *ports, int num_ports) {
    if (!midi_handler_init()) {
        return -1;
    }

    g_midi_callback = cb;
    g_midi_userdata = userdata;
    midi_handler_set_callback(internal_midi_callback, nullptr);

    // Open first device if specified
    if (num_ports > 0 && ports && ports[0] >= 0) {
        midi_handler_open_device(ports[0]);
    }

    return num_ports;
}

void midi_deinit(void) {
    midi_handler_close_device();
    midi_handler_cleanup();
    g_midi_callback = nullptr;
    g_midi_userdata = nullptr;
}

void midi_set_clock_sync_enabled(int enabled) {
    // Not implemented on Android
    (void)enabled;
}

int midi_is_clock_sync_enabled(void) {
    return 0;
}

double midi_get_clock_tempo(void) {
    return 120.0;
}

void midi_set_transport_callback(MidiTransportCallback callback, void* userdata) {
    // Not implemented on Android
    (void)callback;
    (void)userdata;
}

void midi_set_spp_callback(MidiSPPCallback callback, void* userdata) {
    // Not implemented on Android
    (void)callback;
    (void)userdata;
}

void midi_set_transport_control_enabled(int enabled) {
    // Not implemented on Android
    (void)enabled;
}

void midi_set_input_channel_filter(int channel) {
    // Not implemented on Android
    (void)channel;
}

// MIDI Output API (from midi_output.c)

int midi_output_list_ports(void) {
    return 0;  // MIDI output not supported on Android yet
}

int midi_output_get_port_name(int port, char* name_out, int bufsize) {
    (void)port;
    if (name_out && bufsize > 0) name_out[0] = '\0';
    return -1;
}

int midi_output_init(int device_id) {
    (void)device_id;
    return 0;
}

void midi_output_deinit(void) {
}

void midi_output_note_on(int channel, int note, int velocity) {
    midi_message_t msg = {};
    msg.status = 0x90 | (channel & 0x0F);
    msg.data1 = note & 0x7F;
    msg.data2 = velocity & 0x7F;
    midi_handler_send_message(&msg);
}

void midi_output_note_off(int channel, int note) {
    midi_message_t msg = {};
    msg.status = 0x80 | (channel & 0x0F);
    msg.data1 = note & 0x7F;
    msg.data2 = 0;
    midi_handler_send_message(&msg);
}

void midi_output_cc(int channel, int cc, int value) {
    midi_message_t msg = {};
    msg.status = 0xB0 | (channel & 0x0F);
    msg.data1 = cc & 0x7F;
    msg.data2 = value & 0x7F;
    midi_handler_send_message(&msg);
}

void midi_output_program_change(int channel, int program) {
    midi_message_t msg = {};
    msg.status = 0xC0 | (channel & 0x0F);
    msg.data1 = program & 0x7F;
    msg.data2 = 0;
    midi_handler_send_message(&msg);
}

static bool g_clock_running = false;

void midi_output_start_clock(float bpm) {
    (void)bpm;
    g_clock_running = true;
}

void midi_output_stop_clock(void) {
    g_clock_running = false;
}

void midi_output_update_clock(double bpm, double row_fraction) {
    // Send MIDI clock messages (0xF8) at 24 PPQN
    // This is a simplified implementation
    (void)bpm;
    (void)row_fraction;

    if (g_clock_running) {
        // Send clock tick
        midi_message_t msg = {};
        msg.status = 0xF8;  // MIDI Clock
        midi_handler_send_message(&msg);
    }
}

int midi_output_is_clock_master(void) {
    return 0;
}

void midi_output_send_start(void) {
    midi_message_t msg = {};
    msg.status = 0xFA;  // MIDI Start
    midi_handler_send_message(&msg);
}

void midi_output_send_stop(void) {
    midi_message_t msg = {};
    msg.status = 0xFC;  // MIDI Stop
    midi_handler_send_message(&msg);
}

void midi_output_send_continue(void) {
    midi_message_t msg = {};
    msg.status = 0xFB;  // MIDI Continue
    midi_handler_send_message(&msg);
}

void midi_output_send_song_position(int position) {
    midi_message_t msg = {};
    msg.status = 0xF2;  // Song Position Pointer
    msg.data1 = position & 0x7F;
    msg.data2 = (position >> 7) & 0x7F;
    midi_handler_send_message(&msg);
}

int midi_output_send_sysex(const unsigned char* data, size_t len) {
    // Sysex not implemented via simple message struct
    (void)data;
    (void)len;
    return 0;
}

void midi_output_set_spp_config(int enabled, int ppqn) {
    (void)enabled;
    (void)ppqn;
}

void midi_output_set_clock_master(int enabled) {
    (void)enabled;
}

void midi_output_set_metadata(RegrooveMetadata *metadata) {
    (void)metadata;
}

void midi_output_reset_programs(void) {
}

void midi_output_update_position(int spp_position) {
    (void)spp_position;
}

void midi_output_stop_channel(int channel) {
    // Send All Notes Off CC (123)
    midi_output_cc(channel, 123, 0);
}

int midi_output_handle_note(int tracker_channel, int note, int instrument, int volume) {
    // Send program change if instrument specified
    if (instrument >= 0) {
        midi_output_program_change(tracker_channel, instrument);
    }

    // Send note on/off based on volume
    if (volume > 0) {
        // Convert tracker volume (0-64) to MIDI velocity (0-127)
        int velocity = (volume * 127) / 64;
        if (velocity > 127) velocity = 127;
        midi_output_note_on(tracker_channel, note, velocity);
    } else {
        midi_output_note_off(tracker_channel, note);
    }

    return 0;
}

} // extern "C"

#endif // __ANDROID__
