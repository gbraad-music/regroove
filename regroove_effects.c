#include "regroove_effects.h"
#include "fx_delay.h"
#include "fx_distortion.h"
#include "fx_filter.h"
#include "fx_eq.h"
#include "fx_compressor.h"
#include "fx_reverb.h"
#include <stdlib.h>
#include <string.h>

// RegrooveEffects structure - wrapper around individual RFX effects
struct RegrooveEffects {
    // Individual RFX effect instances
    FXDistortion* distortion;
    FXFilter* filter;
    FXEqualizer* eq;
    FXCompressor* compressor;
    FXReverb* reverb;
    FXDelay* delay;

    // Phaser state (kept for compatibility but not used)
    int phaser_enabled;
    float phaser_rate;
    float phaser_depth;
    float phaser_feedback;
};

RegrooveEffects* regroove_effects_create(void) {
    RegrooveEffects* fx = (RegrooveEffects*)calloc(1, sizeof(RegrooveEffects));
    if (!fx) return NULL;

    // Create individual effect instances
    fx->distortion = fx_distortion_create();
    fx->filter = fx_filter_create();
    fx->eq = fx_eq_create();
    fx->compressor = fx_compressor_create();
    fx->reverb = fx_reverb_create();
    fx->delay = fx_delay_create();

    // Check if all effects were created successfully
    if (!fx->distortion || !fx->filter || !fx->eq ||
        !fx->compressor || !fx->reverb || !fx->delay) {
        regroove_effects_destroy(fx);
        return NULL;
    }

    // RFX effects already initialize with sensible defaults in their _create() functions
    // We only need to ensure they're all disabled
    // (The RFX defaults are stable and tested - don't override them unless necessary!)

    // Phaser (kept for compatibility but not used)
    fx->phaser_enabled = 0;
    fx->phaser_rate = 0.3f;
    fx->phaser_depth = 0.5f;
    fx->phaser_feedback = 0.3f;

    return fx;
}

void regroove_effects_destroy(RegrooveEffects* fx) {
    if (fx) {
        if (fx->distortion) fx_distortion_destroy(fx->distortion);
        if (fx->filter) fx_filter_destroy(fx->filter);
        if (fx->eq) fx_eq_destroy(fx->eq);
        if (fx->compressor) fx_compressor_destroy(fx->compressor);
        if (fx->reverb) fx_reverb_destroy(fx->reverb);
        if (fx->delay) fx_delay_destroy(fx->delay);
        free(fx);
    }
}

void regroove_effects_reset(RegrooveEffects* fx) {
    if (!fx) return;

    if (fx->distortion) fx_distortion_reset(fx->distortion);
    if (fx->filter) fx_filter_reset(fx->filter);
    if (fx->eq) fx_eq_reset(fx->eq);
    if (fx->compressor) fx_compressor_reset(fx->compressor);
    if (fx->reverb) fx_reverb_reset(fx->reverb);
    if (fx->delay) fx_delay_reset(fx->delay);
}

void regroove_effects_process(RegrooveEffects* fx, int16_t* buffer, int frames, int sample_rate) {
    if (!fx || !buffer || frames <= 0) return;

    // Convert to float ONCE to avoid repeated conversions causing distortion
    // Allocate float buffer for processing
    float* float_buffer = (float*)malloc(frames * 2 * sizeof(float));
    if (!float_buffer) return;

    // Convert int16 to float
    const float scale_to_float = 1.0f / 32768.0f;
    for (int i = 0; i < frames * 2; i++) {
        float_buffer[i] = (float)buffer[i] * scale_to_float;
    }

    // Process effects in order using RFX library process_f32 functions
    // Order: Distortion -> Filter -> EQ -> Compressor -> Reverb -> Delay

    if (fx->distortion && fx_distortion_get_enabled(fx->distortion)) {
        fx_distortion_process_f32(fx->distortion, float_buffer, frames, sample_rate);
    }

    if (fx->filter && fx_filter_get_enabled(fx->filter)) {
        fx_filter_process_f32(fx->filter, float_buffer, frames, sample_rate);
    }

    if (fx->eq && fx_eq_get_enabled(fx->eq)) {
        fx_eq_process_f32(fx->eq, float_buffer, frames, sample_rate);
    }

    if (fx->compressor && fx_compressor_get_enabled(fx->compressor)) {
        fx_compressor_process_f32(fx->compressor, float_buffer, frames, sample_rate);
    }

    if (fx->reverb && fx_reverb_get_enabled(fx->reverb)) {
        fx_reverb_process_f32(fx->reverb, float_buffer, frames, sample_rate);
    }

    if (fx->delay && fx_delay_get_enabled(fx->delay)) {
        fx_delay_process_f32(fx->delay, float_buffer, frames, sample_rate);
    }

    // Convert back to int16 with clamping
    const float scale_to_int16 = 32767.0f;
    for (int i = 0; i < frames * 2; i++) {
        float sample = float_buffer[i] * scale_to_int16;
        if (sample > 32767.0f) sample = 32767.0f;
        if (sample < -32768.0f) sample = -32768.0f;
        buffer[i] = (int16_t)sample;
    }

    free(float_buffer);
}

// ============================================================================
// Parameter setters/getters - forward to RFX effects
// ============================================================================

// Distortion
void regroove_effects_set_distortion_enabled(RegrooveEffects* fx, int enabled) {
    if (fx && fx->distortion) fx_distortion_set_enabled(fx->distortion, enabled);
}

void regroove_effects_set_distortion_drive(RegrooveEffects* fx, float drive) {
    if (fx && fx->distortion) fx_distortion_set_drive(fx->distortion, drive);
}

void regroove_effects_set_distortion_mix(RegrooveEffects* fx, float mix) {
    if (fx && fx->distortion) fx_distortion_set_mix(fx->distortion, mix);
}

int regroove_effects_get_distortion_enabled(RegrooveEffects* fx) {
    return (fx && fx->distortion) ? fx_distortion_get_enabled(fx->distortion) : 0;
}

float regroove_effects_get_distortion_drive(RegrooveEffects* fx) {
    return (fx && fx->distortion) ? fx_distortion_get_drive(fx->distortion) : 0.0f;
}

float regroove_effects_get_distortion_mix(RegrooveEffects* fx) {
    return (fx && fx->distortion) ? fx_distortion_get_mix(fx->distortion) : 0.0f;
}

// Filter
void regroove_effects_set_filter_enabled(RegrooveEffects* fx, int enabled) {
    if (fx && fx->filter) fx_filter_set_enabled(fx->filter, enabled);
}

void regroove_effects_set_filter_cutoff(RegrooveEffects* fx, float cutoff) {
    if (fx && fx->filter) fx_filter_set_cutoff(fx->filter, cutoff);
}

void regroove_effects_set_filter_resonance(RegrooveEffects* fx, float resonance) {
    if (fx && fx->filter) fx_filter_set_resonance(fx->filter, resonance);
}

int regroove_effects_get_filter_enabled(RegrooveEffects* fx) {
    return (fx && fx->filter) ? fx_filter_get_enabled(fx->filter) : 0;
}

float regroove_effects_get_filter_cutoff(RegrooveEffects* fx) {
    return (fx && fx->filter) ? fx_filter_get_cutoff(fx->filter) : 0.0f;
}

float regroove_effects_get_filter_resonance(RegrooveEffects* fx) {
    return (fx && fx->filter) ? fx_filter_get_resonance(fx->filter) : 0.0f;
}

// EQ
void regroove_effects_set_eq_enabled(RegrooveEffects* fx, int enabled) {
    if (fx && fx->eq) fx_eq_set_enabled(fx->eq, enabled);
}

void regroove_effects_set_eq_low(RegrooveEffects* fx, float gain) {
    if (fx && fx->eq) fx_eq_set_low(fx->eq, gain);
}

void regroove_effects_set_eq_mid(RegrooveEffects* fx, float gain) {
    if (fx && fx->eq) fx_eq_set_mid(fx->eq, gain);
}

void regroove_effects_set_eq_high(RegrooveEffects* fx, float gain) {
    if (fx && fx->eq) fx_eq_set_high(fx->eq, gain);
}

int regroove_effects_get_eq_enabled(RegrooveEffects* fx) {
    return (fx && fx->eq) ? fx_eq_get_enabled(fx->eq) : 0;
}

float regroove_effects_get_eq_low(RegrooveEffects* fx) {
    return (fx && fx->eq) ? fx_eq_get_low(fx->eq) : 0.5f;
}

float regroove_effects_get_eq_mid(RegrooveEffects* fx) {
    return (fx && fx->eq) ? fx_eq_get_mid(fx->eq) : 0.5f;
}

float regroove_effects_get_eq_high(RegrooveEffects* fx) {
    return (fx && fx->eq) ? fx_eq_get_high(fx->eq) : 0.5f;
}

// Compressor
void regroove_effects_set_compressor_enabled(RegrooveEffects* fx, int enabled) {
    if (fx && fx->compressor) fx_compressor_set_enabled(fx->compressor, enabled);
}

void regroove_effects_set_compressor_threshold(RegrooveEffects* fx, float threshold) {
    if (fx && fx->compressor) fx_compressor_set_threshold(fx->compressor, threshold);
}

void regroove_effects_set_compressor_ratio(RegrooveEffects* fx, float ratio) {
    if (fx && fx->compressor) fx_compressor_set_ratio(fx->compressor, ratio);
}

void regroove_effects_set_compressor_attack(RegrooveEffects* fx, float attack) {
    if (fx && fx->compressor) fx_compressor_set_attack(fx->compressor, attack);
}

void regroove_effects_set_compressor_release(RegrooveEffects* fx, float release) {
    if (fx && fx->compressor) fx_compressor_set_release(fx->compressor, release);
}

void regroove_effects_set_compressor_makeup(RegrooveEffects* fx, float makeup) {
    if (fx && fx->compressor) fx_compressor_set_makeup(fx->compressor, makeup);
}

int regroove_effects_get_compressor_enabled(RegrooveEffects* fx) {
    return (fx && fx->compressor) ? fx_compressor_get_enabled(fx->compressor) : 0;
}

float regroove_effects_get_compressor_threshold(RegrooveEffects* fx) {
    return (fx && fx->compressor) ? fx_compressor_get_threshold(fx->compressor) : 0.7f;
}

float regroove_effects_get_compressor_ratio(RegrooveEffects* fx) {
    return (fx && fx->compressor) ? fx_compressor_get_ratio(fx->compressor) : 0.5f;
}

float regroove_effects_get_compressor_attack(RegrooveEffects* fx) {
    return (fx && fx->compressor) ? fx_compressor_get_attack(fx->compressor) : 0.1f;
}

float regroove_effects_get_compressor_release(RegrooveEffects* fx) {
    return (fx && fx->compressor) ? fx_compressor_get_release(fx->compressor) : 0.3f;
}

float regroove_effects_get_compressor_makeup(RegrooveEffects* fx) {
    return (fx && fx->compressor) ? fx_compressor_get_makeup(fx->compressor) : 0.5f;
}

// Phaser (kept for compatibility but not used - stored locally)
void regroove_effects_set_phaser_enabled(RegrooveEffects* fx, int enabled) {
    if (fx) fx->phaser_enabled = enabled;
}

void regroove_effects_set_phaser_rate(RegrooveEffects* fx, float rate) {
    if (fx) fx->phaser_rate = rate;
}

void regroove_effects_set_phaser_depth(RegrooveEffects* fx, float depth) {
    if (fx) fx->phaser_depth = depth;
}

void regroove_effects_set_phaser_feedback(RegrooveEffects* fx, float feedback) {
    if (fx) fx->phaser_feedback = feedback;
}

int regroove_effects_get_phaser_enabled(RegrooveEffects* fx) {
    return fx ? fx->phaser_enabled : 0;
}

float regroove_effects_get_phaser_rate(RegrooveEffects* fx) {
    return fx ? fx->phaser_rate : 0.3f;
}

float regroove_effects_get_phaser_depth(RegrooveEffects* fx) {
    return fx ? fx->phaser_depth : 0.5f;
}

float regroove_effects_get_phaser_feedback(RegrooveEffects* fx) {
    return fx ? fx->phaser_feedback : 0.3f;
}

// Reverb
void regroove_effects_set_reverb_enabled(RegrooveEffects* fx, int enabled) {
    if (fx && fx->reverb) fx_reverb_set_enabled(fx->reverb, enabled);
}

void regroove_effects_set_reverb_room_size(RegrooveEffects* fx, float size) {
    if (fx && fx->reverb) fx_reverb_set_size(fx->reverb, size);
}

void regroove_effects_set_reverb_damping(RegrooveEffects* fx, float damping) {
    if (fx && fx->reverb) fx_reverb_set_damping(fx->reverb, damping);
}

void regroove_effects_set_reverb_mix(RegrooveEffects* fx, float mix) {
    if (fx && fx->reverb) fx_reverb_set_mix(fx->reverb, mix);
}

int regroove_effects_get_reverb_enabled(RegrooveEffects* fx) {
    return (fx && fx->reverb) ? fx_reverb_get_enabled(fx->reverb) : 0;
}

float regroove_effects_get_reverb_room_size(RegrooveEffects* fx) {
    return (fx && fx->reverb) ? fx_reverb_get_size(fx->reverb) : 0.5f;
}

float regroove_effects_get_reverb_damping(RegrooveEffects* fx) {
    return (fx && fx->reverb) ? fx_reverb_get_damping(fx->reverb) : 0.5f;
}

float regroove_effects_get_reverb_mix(RegrooveEffects* fx) {
    return (fx && fx->reverb) ? fx_reverb_get_mix(fx->reverb) : 0.3f;
}

// Delay
void regroove_effects_set_delay_enabled(RegrooveEffects* fx, int enabled) {
    if (fx && fx->delay) fx_delay_set_enabled(fx->delay, enabled);
}

void regroove_effects_set_delay_time(RegrooveEffects* fx, float time) {
    if (fx && fx->delay) fx_delay_set_time(fx->delay, time);
}

void regroove_effects_set_delay_feedback(RegrooveEffects* fx, float feedback) {
    if (fx && fx->delay) fx_delay_set_feedback(fx->delay, feedback);
}

void regroove_effects_set_delay_mix(RegrooveEffects* fx, float mix) {
    if (fx && fx->delay) fx_delay_set_mix(fx->delay, mix);
}

int regroove_effects_get_delay_enabled(RegrooveEffects* fx) {
    return (fx && fx->delay) ? fx_delay_get_enabled(fx->delay) : 0;
}

float regroove_effects_get_delay_time(RegrooveEffects* fx) {
    return (fx && fx->delay) ? fx_delay_get_time(fx->delay) : 0.375f;
}

float regroove_effects_get_delay_feedback(RegrooveEffects* fx) {
    return (fx && fx->delay) ? fx_delay_get_feedback(fx->delay) : 0.4f;
}

float regroove_effects_get_delay_mix(RegrooveEffects* fx) {
    return (fx && fx->delay) ? fx_delay_get_mix(fx->delay) : 0.3f;
}
