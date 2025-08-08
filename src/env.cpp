#include <iostream>
#include <exception>
#include <thread>
#include <chrono>

#include <argparse/argparse.hpp>

#include "env.hpp"

using namespace std;

ADSR::ADSR(float a, float d, float s, float r, int sample_rate)
    : attack(a * sample_rate), decay(d * sample_rate), sustain(s), release(r * sample_rate),
    sample_counter(0),
    level(0.0f),
    stage(Stage::Idle) {

}

void ADSR::note_on() {
    stage= Stage::Attack;
    sample_counter= 0; 
}

void ADSR::note_off() {
    if(stage != Stage::Idle) {
        stage= Stage::Release;
        sample_counter= 0; 
    }
}

float ADSR::update() {

    switch(stage) {

        case Stage::Attack:
            if(attack <= 0) {
                level= 1.0f;
                stage= Stage::Decay;
                sample_counter= 0;
            } else {
                level= min(1.0f, static_cast<float>(sample_counter) / attack);
                if(++sample_counter >= attack) {
                    stage= Stage::Decay;
                    sample_counter= 0;
                    level= 1.0f;
                }
            }
            break;
                    
        case Stage::Decay:
            if(decay <= 0) {
                level= sustain;
                stage= Stage::Sustain;
            } else {
                float decay_progress= min(1.0f, static_cast<float>(sample_counter) / decay);
                level= 1.0f - (1.0f - sustain) * decay_progress;
                if(++sample_counter >= decay) { 
                    stage= Stage::Sustain;
                    level= sustain;
                }
            }
            break; 

        case Stage::Sustain:
            level= sustain;
            break;

        case Stage::Release:

            if(release <= 0) {
                level= 0.0f;
                stage= Stage::Idle;
            } else {
                float release_progress= min(1.0f, static_cast<float>(sample_counter) / release);
                level= sustain * (1.0f - release_progress);
                if(++sample_counter >= release || level <= 0.001f) {
                   stage= Stage::Idle;  
                   level= 0.0f;
                }
            }
            break; 

        default:
            level= 0.0f;
            break;
    }

    return level;
}

int main(int argc, char **argv) {

    argparse::ArgumentParser args("env");
    string line;
    bool lastGate= false;

    args.add_argument("--attack").default_value(0.01f).help("attack").scan<'g', float>();
    args.add_argument("--decay").default_value(0.1f).help("decay").scan<'g', float>();
    args.add_argument("--sustain").default_value(0.7f).help("sustain").scan<'g', float>();
    args.add_argument("--release").default_value(0.3f).help("release").scan<'g', float>();
    args.add_argument("--sample_rate").default_value(48000).help("sample_rate").scan<'i', int>();

    try {
        args.parse_args(argc, argv);
    } catch (const exception &err) {
       cerr << err.what() << endl;
       cerr << args;
       return EXIT_FAILURE;
    }

    float attack= args.get<float>("attack");
    float decay= args.get<float>("decay");
    float sustain= args.get<float>("sustain");
    float release= args.get<float>("release");
    int sampleRate= args.get<int>("sample_rate");

    const auto sampleTime= chrono::microseconds(1000000 / sampleRate);
    
 
    ADSR env {attack, decay, sustain, release, sampleRate};
    env.note_on();
    

    while(getline(cin, line)) {
        bool gate= (stof(line) > 0.5);

        if(gate && !lastGate) {
            env.note_on();
        } else if(!gate && lastGate) {
            env.note_off();
        }

        lastGate= gate;
 
        float sample= env.update();
        cout << sample << "\n";

        this_thread::sleep_for(sampleTime);
    }
    return EXIT_SUCCESS;
} 
