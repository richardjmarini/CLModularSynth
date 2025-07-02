#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <argparse/argparse.hpp>
#include <SDL2/SDL.h>

#include "ringbuffer.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    argparse::ArgumentParser args("Scope");
    args.add_argument("--sample_rate").default_value(48000).help("sample rate (e.g, 48000 == 48KHz").scan<'i', int>();
    args.add_argument("--trigger").default_value(false).implicit_value(true).help("enable trigger mode");
    args.add_argument("--trigger_threshold").default_value(0.01f).help("trigger threshold").scan<'g', float>();
    args.add_argument("--trigger_offset").default_value(0).help("trigger offset").scan<'i', int>();
    args.add_argument("--window_width").default_value(800).help("window width").scan<'i', int>();
    args.add_argument("--window_height").default_value(400).help("window height").scan<'i', int>();
    args.add_argument("--voltage_per_division").default_value(0.1f).help("vols per division (e.g, 0.1 == 1v)").scan<'g', float>();
    args.add_argument("--time_per_division").default_value(0.001f).help("time per division in seconds (e.g, 0.001 == 1ms)").scan<'g', float>();
    args.add_argument("--time_divisions").default_value(10).help("total time divisions to display (e.g, 10)").scan<'i', int>();
    args.add_argument("--voltage_divisions").default_value(10).help("total voltage divisions to display (e.g, 10)").scan<'i', int>();

    try {
        args.parse_args(argc, argv);
    } catch (const exception &err) {
        cerr << err.what() << endl << args;
        return -1;
    }

    int windowWidth= args.get<int>("window_width");
    int windowHeight= args.get<int>("window_height");
    int sampleRate= args.get<int>("sample_rate");
    float timePerDivision= args.get<float>("time_per_division");
    float voltagePerDivision= args.get<float>("voltage_per_division");
    int timeDivisions= args.get<int>("time_divisions");
    int voltageDivisions= args.get<int>("voltage_divisions");

    float totalTime= timePerDivision * timeDivisions;  
    float voltageFullScale= voltagePerDivision * voltageDivisions;
    float voltageHalfScale= voltageFullScale / 2.0f;
    size_t displayBufferSize= static_cast<size_t>(sampleRate * totalTime);
    int ringBufferSize= displayBufferSize * 4;

    RingBuffer<float> ringBuffer(ringBufferSize);
    vector<float> displayBuffer(displayBufferSize, 0.0f);

    atomic<bool> running(true);
    mutex bufferMutex;
    condition_variable dataReady;

    bool trigger= args.get<bool>("trigger");
    float triggerThreshold= args.get<float>("trigger_threshold");
    int triggerOffset= args.get<int>("trigger_offset");

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window= SDL_CreateWindow(
        "Triggered Scope",
         SDL_WINDOWPOS_CENTERED,
         SDL_WINDOWPOS_CENTERED,
         windowWidth,
         windowHeight,
         SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer= SDL_CreateRenderer(
        window, 
        -1,
         SDL_RENDERER_ACCELERATED
    );

    SDL_Texture* waveformTexture= SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        windowWidth,
        windowHeight
    );

    // Producer thread
    thread inputThread([&]() {
        string line;
        float sample;
        float previousSample= 0.0f;

        while (running) {
            if (getline(cin, line)) {

                if(!(istringstream(line) >> sample))
                    continue;

                ringBuffer.push(sample);

                lock_guard<mutex> lock(bufferMutex);
                if (trigger) {

                    if (ringBuffer.size() >= static_cast<size_t>(triggerOffset + displayBufferSize)) {
                        bool fireTrigger = trigger && previousSample < triggerThreshold && sample >= triggerThreshold;
                        if (fireTrigger) {
                            ringBuffer.copyFromTail(triggerOffset, displayBuffer.data(), displayBufferSize);
                            dataReady.notify_one();
                        }
                    }

                }
                else {
                    ringBuffer.copyFromTail(0, displayBuffer.data(), displayBufferSize);
                    dataReady.notify_one();
                }
                previousSample= sample;
            }
        }
    });

    // Main render loop
    SDL_Event event;
    bool quit= false;
    uint32_t *pixels;
    int pitch;
    auto lastDrawTime= chrono::steady_clock::now();
    const auto frameInterval= chrono::milliseconds(16);

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit= true;
        }

        auto now= chrono::steady_clock::now();
        if (now - lastDrawTime >= frameInterval) {
            lastDrawTime= now;

            SDL_LockTexture(waveformTexture, nullptr, (void**)&pixels, &pitch);
            memset(pixels, 0, pitch * windowHeight);
            uint32_t pitchFactor= pitch / 4;

            float scale= static_cast<float>(displayBufferSize) / windowWidth;
            float yCenter= windowHeight / 2.0f;
            int yLimit= windowHeight - 1;

            {
                lock_guard<mutex> lock(bufferMutex);

                // x-axis
                int yAxis= static_cast<int>(windowHeight / 2);
                for(int x=0; x < windowWidth; ++x) {
                    pixels[yAxis * pitchFactor + x]= 0x444444;
                }
    
                // time divisions
                for(int i=1; i <= timeDivisions; ++i) {
                    int x= i * windowWidth / timeDivisions;
                    for(int y= 0; y < windowHeight; ++y)
                        pixels[y * pitchFactor + x]= 0x222222;
                    }

                // voltage divisions (y-axis)
                for(int i= 0; i <= voltageDivisions; ++i) {
                    int y= i * (windowHeight / voltageDivisions);
                    for(int x= 0; x < windowWidth; ++x) 
                        pixels[y * pitchFactor + x]= (i == voltageDivisions / 2) ? 0x444444: 0x222222;
                }


                // waveform
                for (int i= 1; i < windowWidth; ++i) {

                    int idx1= static_cast<int>((i - 1) * scale) % displayBufferSize;
                    int idx2= static_cast<int>(i * scale) % displayBufferSize;
                 

                    int y1= clamp(static_cast<int>(yCenter - (displayBuffer[idx1] / voltageHalfScale) * yCenter), 0, yLimit);
                    int y2= clamp(static_cast<int>(yCenter - (displayBuffer[idx2] / voltageHalfScale) * yCenter), 0, yLimit);

                    if (y1 > y2) swap(y1, y2);
                    for (int y= y1; y <= y2; ++y)
                        pixels[y * pitchFactor + i]= 0x00FF00;
                }

                // trigger indicator
                if (trigger) {
                    int markerX = windowWidth - static_cast<int>((displayBufferSize - triggerOffset) / scale);
                    if (markerX >= 0 && markerX < windowWidth) {
                        for (int y = 0; y < windowHeight; ++y) {
                            pixels[y * pitchFactor + markerX] = 0xFF0000; 
                        }
                    }
                }
            }
                    
            SDL_UnlockTexture(waveformTexture);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, waveformTexture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }

        this_thread::sleep_for(frameInterval);
    }

    running= false;
    inputThread.join();

    SDL_DestroyTexture(waveformTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

