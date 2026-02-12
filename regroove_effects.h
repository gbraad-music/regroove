#ifndef REGROOVE_EFFECTS_H
#define REGROOVE_EFFECTS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration - opaque structure (defined in regroove_effects.c)
typedef struct RegrooveEffects RegrooveEffects;

// Initialize effects with default parameters
RegrooveEffects* regroove_effects_create(void);

// Free effects
void regroove_effects_destroy(RegrooveEffects* fx);

// Reset effect state (clear filter memory, etc.)
void regroove_effects_reset(RegrooveEffects* fx);

// Process audio buffer through effects chain
// buffer: interleaved stereo int16 samples (L, R, L, R, ...)
// frames: number of stereo frames
// sample_rate: sample rate in Hz
void regroove_effects_process(RegrooveEffects* fx, int16_t* buffer, int frames, int sample_rate);

// Parameter setters (normalized 0.0 - 1.0 for MIDI mapping)
void regroove_effects_set_distortion_enabled(RegrooveEffects* fx, int enabled);
void regroove_effects_set_distortion_drive(RegrooveEffects* fx, float drive);   // 0.0 - 1.0
void regroove_effects_set_distortion_mix(RegrooveEffects* fx, float mix);       // 0.0 - 1.0

void regroove_effects_set_filter_enabled(RegrooveEffects* fx, int enabled);
void regroove_effects_set_filter_cutoff(RegrooveEffects* fx, float cutoff);     // 0.0 - 1.0
void regroove_effects_set_filter_resonance(RegrooveEffects* fx, float resonance); // 0.0 - 1.0

void regroove_effects_set_eq_enabled(RegrooveEffects* fx, int enabled);
void regroove_effects_set_eq_low(RegrooveEffects* fx, float gain);     // 0.0 - 1.0
void regroove_effects_set_eq_mid(RegrooveEffects* fx, float gain);     // 0.0 - 1.0
void regroove_effects_set_eq_high(RegrooveEffects* fx, float gain);    // 0.0 - 1.0

void regroove_effects_set_compressor_enabled(RegrooveEffects* fx, int enabled);
void regroove_effects_set_compressor_threshold(RegrooveEffects* fx, float threshold);
void regroove_effects_set_compressor_ratio(RegrooveEffects* fx, float ratio);
void regroove_effects_set_compressor_attack(RegrooveEffects* fx, float attack);
void regroove_effects_set_compressor_release(RegrooveEffects* fx, float release);
void regroove_effects_set_compressor_makeup(RegrooveEffects* fx, float makeup);

void regroove_effects_set_phaser_enabled(RegrooveEffects* fx, int enabled);
void regroove_effects_set_phaser_rate(RegrooveEffects* fx, float rate);
void regroove_effects_set_phaser_depth(RegrooveEffects* fx, float depth);
void regroove_effects_set_phaser_feedback(RegrooveEffects* fx, float feedback);

void regroove_effects_set_reverb_enabled(RegrooveEffects* fx, int enabled);
void regroove_effects_set_reverb_room_size(RegrooveEffects* fx, float size);
void regroove_effects_set_reverb_damping(RegrooveEffects* fx, float damping);
void regroove_effects_set_reverb_mix(RegrooveEffects* fx, float mix);

void regroove_effects_set_delay_enabled(RegrooveEffects* fx, int enabled);
void regroove_effects_set_delay_time(RegrooveEffects* fx, float time);
void regroove_effects_set_delay_feedback(RegrooveEffects* fx, float feedback);
void regroove_effects_set_delay_mix(RegrooveEffects* fx, float mix);

// Parameter getters (normalized 0.0 - 1.0)
int regroove_effects_get_distortion_enabled(RegrooveEffects* fx);
float regroove_effects_get_distortion_drive(RegrooveEffects* fx);
float regroove_effects_get_distortion_mix(RegrooveEffects* fx);

int regroove_effects_get_filter_enabled(RegrooveEffects* fx);
float regroove_effects_get_filter_cutoff(RegrooveEffects* fx);
float regroove_effects_get_filter_resonance(RegrooveEffects* fx);

int regroove_effects_get_eq_enabled(RegrooveEffects* fx);
float regroove_effects_get_eq_low(RegrooveEffects* fx);
float regroove_effects_get_eq_mid(RegrooveEffects* fx);
float regroove_effects_get_eq_high(RegrooveEffects* fx);

int regroove_effects_get_compressor_enabled(RegrooveEffects* fx);
float regroove_effects_get_compressor_threshold(RegrooveEffects* fx);
float regroove_effects_get_compressor_ratio(RegrooveEffects* fx);
float regroove_effects_get_compressor_attack(RegrooveEffects* fx);
float regroove_effects_get_compressor_release(RegrooveEffects* fx);
float regroove_effects_get_compressor_makeup(RegrooveEffects* fx);

int regroove_effects_get_phaser_enabled(RegrooveEffects* fx);
float regroove_effects_get_phaser_rate(RegrooveEffects* fx);
float regroove_effects_get_phaser_depth(RegrooveEffects* fx);
float regroove_effects_get_phaser_feedback(RegrooveEffects* fx);

int regroove_effects_get_reverb_enabled(RegrooveEffects* fx);
float regroove_effects_get_reverb_room_size(RegrooveEffects* fx);
float regroove_effects_get_reverb_damping(RegrooveEffects* fx);
float regroove_effects_get_reverb_mix(RegrooveEffects* fx);

int regroove_effects_get_delay_enabled(RegrooveEffects* fx);
float regroove_effects_get_delay_time(RegrooveEffects* fx);
float regroove_effects_get_delay_feedback(RegrooveEffects* fx);
float regroove_effects_get_delay_mix(RegrooveEffects* fx);

#ifdef __cplusplus
}
#endif

#endif // REGROOVE_EFFECTS_H
