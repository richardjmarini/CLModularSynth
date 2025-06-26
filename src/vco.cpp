#include <iostream>
#include <cmath>

#include <argparse/argparse.hpp>

#include "vco.hpp"

using namespace std;

Vco::Vco(double sampleRate, double sensitivity, double amplitude) {
    sampleRate_= sampleRate;
    sensitivity_= sensitivity;
    amplitude_= amplitude;
}

double Vco::process(double controlVoltage) {

    double frequency= sensitivity_ * controlVoltage;

    phase_+= fmod((2.0 * M_PI * frequency) / sampleRate_, 2.0 * M_PI);

    return amplitude_ * sin(phase_);
}

int main(int argc, char *argv[]) {

   double controlVoltage;
   double sampleRate;
   double sensitivity;
   double amplitude;

   argparse::ArgumentParser args("Vco");
   args.add_argument("--sensitivity").default_value(1.0).help("control voltage sensitivity").scan<'g', double>();
   args.add_argument("--sampleRate").default_value(48000.0).help("sampling rate").scan<'g', double>();
   args.add_argument("--amplitude").default_value(1.0).help("amplitude").scan<'g', double>();


   try {
       args.parse_args(argc, argv);
   } catch (const exception &err) {
      cerr << err.what() << endl;
      cerr << args;
      return -1;
   }

   sensitivity= args.get<double>("sensitivity");
   sampleRate= args.get<double>("sampleRate");
   amplitude= args.get<double>("amplitude");

   Vco vco(sampleRate, sensitivity, amplitude);
   while(cin >> controlVoltage) {
       cout << vco.process(controlVoltage) << endl;
   }

   return 0;
}
