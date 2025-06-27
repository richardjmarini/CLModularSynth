#include <iostream>
#include <cmath>

#include <argparse/argparse.hpp>

#include "vco.hpp"

using namespace std;

Vco::WaveType parseWaveType(const string& value) {

    cout << "wave form" << value << endl;

    if(value == "sine")
        return  Vco::WaveType::SINE;
    else if(value == "triangle") 
        return Vco::WaveType::TRIANGLE;
    else if(value == "square")
        return Vco::WaveType::SQUARE;
    else
        throw runtime_error("Invalid wave type");
}

Vco::Vco(double sampleRate, double sensitivity, double amplitude) {

    sampleRate_= sampleRate;
    sensitivity_= sensitivity;
    amplitude_= amplitude;
}

double Vco::generateSineWave(double frequency) {

    phase_+= fmod((2.0 * M_PI * frequency) / sampleRate_, 2.0 * M_PI);

    double sine_wave= sin(phase_);

    return sine_wave;
}

double Vco::generateTriangleWave(double frequency) {

    phase_+= (2.0 * frequency) / sampleRate_;
    if(phase_ >= 1.0)
        phase_-= 1.0;

    double triangle_wave = 4.0 * fabs(phase_ - 0.5) - 1.0;

    return triangle_wave;
}

double Vco::generateSquareWave(double frequency) {

    phase_+= frequency / sampleRate_;
    if (phase_ >= 1.0) 
        phase_-= 1.0;

    double square_wave= (phase_ < 0.5) ? 1.0 : -1.0;

    return square_wave;
}

double Vco::generateWaveForm(double controlVoltage, Vco::WaveType waveType) {
    
    double waveForm;
    double frequency= sensitivity_ * controlVoltage;

    switch(waveType) {
        case Vco::WaveType::SINE:
            waveForm= generateSineWave(frequency);
            break;
        case Vco::WaveType::TRIANGLE:
            waveForm= generateTriangleWave(frequency);
            break;
        case Vco::WaveType::SQUARE:
            waveForm= generateSquareWave(frequency);
            break;
        default:
            cerr << "Unknown Wave Type" << endl;
            waveForm= (double)NULL;
            break;
    }
 
    if(waveForm != (double)NULL) 
        return amplitude_ * waveForm;

    return (double)NULL;
}

int main(int argc, char *argv[]) {

   double controlVoltage;
   double sampleRate;
   double sensitivity;
   double amplitude;

   argparse::ArgumentParser args("Vco");
   args.add_argument("--sensitivity").default_value(1.0).help("control voltage sensitivity").scan<'g', double>();
   args.add_argument("--sample_rate").default_value(48000.0).help("sampling rate").scan<'g', double>();
   args.add_argument("--amplitude").default_value(1.0).help("amplitude").scan<'g', double>();
   args.add_argument("--wave_type").default_value(string("sine")).help("sine, triangle, square").action([](const string &value) {

       if (value != "sine" && value != "triangle" && value != "square") 
           throw std::runtime_error("Invalid wave type: must be 'sine', 'triangle', or 'square'");

      return value;
   });


   try {
       args.parse_args(argc, argv);
   } catch (const exception &err) {
      cerr << err.what() << endl;
      cerr << args;
      return -1;
   }

   sensitivity= args.get<double>("sensitivity");
   sampleRate= args.get<double>("sample_rate");
   amplitude= args.get<double>("amplitude");
   Vco::WaveType waveType= parseWaveType(args.get<string>("wave_type"));

   Vco vco(sampleRate, sensitivity, amplitude);
   while(cin >> controlVoltage) {
       cout << vco.generateWaveForm(controlVoltage, waveType) << endl;
   }

   return 0;
}
