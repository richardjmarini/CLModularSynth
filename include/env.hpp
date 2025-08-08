#pragma once

enum class Stage {
    Idle,
    Attack,
    Decay,
    Sustain,
    Release
};

struct ADSR {
    float attack;
    float decay;
    float sustain;
    float release;

    int sample_counter;

    float level= 0.0f;
    Stage stage= Stage::Idle;

    ADSR(float a, float d, float s, float r, int sampleRate);
    void note_on();
    void note_off();
    float update();
};
