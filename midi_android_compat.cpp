/**
 * Android MIDI Compatibility Layer
 * Implements regroove's midi.c and midi_output.c API using RFX midi_handler
 */

#ifdef __ANDROID__

#include <cstdio>
#include <jni.h>

#include "midi.h"
#include "midi_output.h"
#include "regroove_metadata.h"
#include "midi_handler.h"

// SDL JNI function
extern "C" void* SDL_GetAndroidJNIEnv();

extern "C" {

// MIDI Input API (from midi.c)

int midi_list_ports(void) {
    return midi_handler_get_device_count();
}

int midi_get_port_name(int port, char* name_out, int bufsize) {
    if (!name_out || bufsize == 0) {
        return -1;
    }

    // Directly call Java to get name to avoid dangling pointer issues
    // Get JNI environment from SDL
    JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
    if (!env) {
        snprintf(name_out, bufsize, "Port %d", port);
        return 0;
    }

    jclass midiHandlerClass = env->FindClass("nl/gbraad/regroove/MidiHandler");
    if (!midiHandlerClass) {
        snprintf(name_out, bufsize, "Port %d", port);
        return 0;
    }

    // Call static method to get cached device info from Java side
    // This avoids race conditions with the C++ device vector
    jmethodID getNameMethod = env->GetStaticMethodID(midiHandlerClass, "getDeviceNameByIndex", "(I)Ljava/lang/String;");
    if (!getNameMethod) {
        env->DeleteLocalRef(midiHandlerClass);
        snprintf(name_out, bufsize, "Port %d", port);
        return 0;
    }

    jstring jname = (jstring)env->CallStaticObjectMethod(midiHandlerClass, getNameMethod, port);
    if (jname) {
        const char* name_str = env->GetStringUTFChars(jname, nullptr);
        if (name_str) {
            snprintf(name_out, bufsize, "%s", name_str);
            env->ReleaseStringUTFChars(jname, name_str);
        }
        env->DeleteLocalRef(jname);
    } else {
        snprintf(name_out, bufsize, "Port %d", port);
    }

    env->DeleteLocalRef(midiHandlerClass);
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
    // Initialize handler only if not already initialized
    // (Java enumeration happens once at app startup)
    if (!midi_handler_init()) {
        return -1;
    }

    g_midi_callback = cb;
    g_midi_userdata = userdata;
    midi_handler_set_callback(internal_midi_callback, nullptr);

    // Open first device if specified and valid
    if (num_ports > 0 && ports && ports[0] >= 0) {
        int device_count = midi_handler_get_device_count();
        if (ports[0] < device_count) {
            midi_handler_open_device(ports[0]);
        } else {
            fprintf(stderr, "MIDI device %d out of range (have %d devices)\n", ports[0], device_count);
        }
    }

    return num_ports;
}

void midi_deinit(void) {
    // Just close the current device, but keep the handler initialized
    // and device list intact (Java enumeration only happens once at startup)
    midi_handler_close_device();
    // DON'T call midi_handler_cleanup() - it destroys the device list!
    // The handler stays initialized for the lifetime of the app
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
    // MIDI output supported - return same device count as input
    // (Android MIDI devices can both send and receive)
    return midi_handler_get_device_count();
}

int midi_output_get_port_name(int port, char* name_out, int bufsize) {
    if (!name_out || bufsize == 0) {
        return -1;
    }

    // Use same device names as input
    const char* name = midi_handler_get_device_name(port);
    if (!name) {
        name_out[0] = '\0';
        return -1;
    }

    snprintf(name_out, bufsize, "%s", name);
    return 0;
}

int midi_output_init(int device_id) {
    // Output uses the same device - no separate initialization needed
    (void)device_id;
    return 0;
}

void midi_output_deinit(void) {
    // Don't close - input might still be using it
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
