#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>

#include <argparse/argparse.hpp>
#include <SDL2/SDL.h>

using namespace std;

int main(int argc, char *argv[]) {

    argparse::ArgumentParser args("Scope");
    args.add_argument("--horizontal_scale").default_value(48000).help("horizontal time scape").scan<'i', int>();
    args.add_argument("--trigger").default_value(false).implicit_value(true).help("enable trigger mode");
    args.add_argument("--trigger_threshold").default_value(0.01f).help("trigger threshold").scan<'g', float>();
    args.add_argument("--trigger_offset").default_value(0).help("trigger offset").scan<'i', int>();
    args.add_argument("--buffer_size").default_value(800).help("buffer size").scan<'i', int>();
    args.add_argument("--window_width").default_value(800).help("window width").scan<'i', int>();
    args.add_argument("--window_height").default_value(400).help("window height").scan<'i', int>();

    try {
        args.parse_args(argc, argv);
    } catch (const exception &err) {
        cerr << err.what() << endl;
        cerr << args;
        return -1;
    }

    bool quit= false;
    string line;
    float sample;
    int circularBufferIndex= 0;
    chrono::steady_clock::time_point lastTriggerTime= chrono::steady_clock::now();
    bool triggerArmed= true;
    int triggerIndex= -1;
    SDL_Event event;
    uint32_t *pixels;
    int pitch;
    const auto frameInterval= chrono::milliseconds(16); // 1/30.0 == 0.03333 == 30 FPS
    auto lastDrawTime = chrono::steady_clock::now();

    int windowWidth= args.get<int>("window_width");
    int windowHeight= args.get<int>("window_height");
    int displayBufferSize= args.get<int>("horizontal_scale");
    int circularBufferSize= displayBufferSize * 4;
    vector<float> circularBuffer(circularBufferSize, 0.0f);
    vector<float> displayBuffer(displayBufferSize, 0.0f);
    bool trigger= args.get<bool>("trigger");
    float triggerThreshold= args.get<float>("trigger_threshold");
    int triggerOffset= args.get<int>("trigger_offset");

    cout << "Trigger Mode: " << trigger << endl
         << "Trigger Threshold: " << triggerThreshold <<  endl
         << "Trigger Offset: " << triggerOffset << endl;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "SDL init failed: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window* window= SDL_CreateWindow(
        "Real-Time Plot",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth, 
        windowHeight,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer *renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *waveformTexture= SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        windowWidth,
        windowHeight
    );

    SDL_LockTexture(waveformTexture, nullptr, (void**)&pixels, &pitch);


    while (!quit) {

        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT)
                quit = true;
        }

        // get samples
        if (getline(cin, line)) {

            istringstream iss(line);

            if (iss >> sample) {

                circularBuffer[circularBufferIndex]= sample;
                int previousCircularBufferIndex= (circularBufferIndex - 1 + circularBufferSize) % circularBufferSize;
                float previousSample= circularBuffer[previousCircularBufferIndex];

                if (trigger
                    && triggerArmed 
                    && previousSample < triggerThreshold
                    && sample >= triggerThreshold
                    && chrono::steady_clock::now() - lastTriggerTime > chrono::milliseconds(100)) {

                    // if we're in trigger mode

                    int start= (circularBufferIndex - triggerOffset + circularBufferSize) % circularBufferSize;
                    for(int i= 0; i < displayBufferSize; ++i) {
                        displayBuffer[i]= circularBuffer[(start + i) % circularBufferSize];
                    }

                    triggerArmed= false;
                    triggerIndex= start;
                    lastTriggerTime= chrono::steady_clock::now();

                } else {
                
                    // if we're NOT in trigger mode
 
                    int start= (circularBufferIndex - displayBufferSize + circularBufferSize) % circularBufferSize;
                    for(int i= 0; i < displayBufferSize; ++i) {
                        displayBuffer[i]= circularBuffer[(start + i) % circularBufferSize];
                    }
                }

                circularBufferIndex= (circularBufferIndex + 1) % circularBufferSize;
            }
        }


        // draw the display
        auto now= chrono::steady_clock::now();
        if(now - lastDrawTime >= frameInterval) {

            lastDrawTime= now;
            float scale= static_cast<float>(displayBufferSize) / windowWidth;
            float yCenter= (windowHeight / 2);
            int yLimit= windowHeight - 1;
            uint32_t pitchFactor= pitch / 4;

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); 
            SDL_LockTexture(waveformTexture, nullptr, (void **)&pixels, &pitch);
            memset(pixels, 0, pitch * windowHeight);

            for (int i= 1; i < windowWidth; ++i) {

                int idx1= (static_cast<int>((i - 1) * scale)) % displayBufferSize;
                int idx2= (static_cast<int>(i * scale)) % displayBufferSize;
                int y1= yCenter - displayBuffer[idx1] * yCenter;
                int y2= yCenter - displayBuffer[idx2] * yCenter;
                y1= clamp(y1, 0, yLimit);
                y2= clamp(y2, 0, yLimit);
                if(y1 > y2)
                    swap(y1, y2);
           
                for(int y= y1; y <= y2; ++y) 
                    pixels[y * pitchFactor + i]= 0x00FF00; // green
            }

            SDL_UnlockTexture(waveformTexture);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, waveformTexture, nullptr, nullptr);

            // draw the trigger if it was set
            if(triggerIndex >= 0) {

                int displayStartIndex = (circularBufferIndex - displayBufferSize + circularBufferSize) % circularBufferSize;
                int offset = (triggerIndex - displayStartIndex + circularBufferSize) % circularBufferSize;
                int triggerX= static_cast<int>((float)offset/displayBufferSize * windowWidth);

                if(triggerX >= 0 && triggerX < windowWidth) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    SDL_RenderDrawLine(renderer, triggerX, 0, triggerX, windowHeight);
                }
            }

            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
