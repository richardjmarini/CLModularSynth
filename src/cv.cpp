#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>

#include <argparse/argparse.hpp>

#include "cv.hpp"

using namespace std;

Cv::Cv(double sampleRate, double amplitude) {
    sampleRate_= sampleRate;
    amplitude_= amplitude;
}

void Cv::process(double duration) {

    using clock= chrono::high_resolution_clock;
    auto start= clock::now();
    size_t totalSamples= static_cast<size_t>(duration * sampleRate_);

    for(size_t i= 0; i < totalSamples; ++i) {

        double sample= amplitude_;
        std::cout << sample << std::endl;

        auto next_time= start + chrono::duration<double>(i * (1.0 / sampleRate_));
        while(clock::now() < next_time) {
           // do nothing until we should process the next sample
        }
    }
}

int main(int argc, char *argv[]) {

   double sampleRate;
   double amplitude;
   double duration;

   argparse::ArgumentParser args("Cv");
   args.add_argument("--sampleRate").default_value(48000.0).help("sampling rate").scan<'g', double>();
   args.add_argument("--amplitude").default_value(1.0).help("amplitude").scan<'g', double>();
   args.add_argument("--duration").default_value(1.0).help("duratione").scan<'g', double>();


   try {
       args.parse_args(argc, argv);
   } catch (const std::exception &err) {
      std::cerr << err.what() << std::endl;
      std::cerr << args;
      return -1;
   }

   sampleRate= args.get<double>("sampleRate");
   amplitude= args.get<double>("amplitude");
   duration= args.get<double>("duration");

   Cv cv(sampleRate, amplitude);
   cv.process(duration);

   return 0;
}
