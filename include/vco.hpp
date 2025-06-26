#pragma once

class Vco {
public:
    Vco(double sampleRate, double sensitivity, double amplitude);
    double process(double controlVoltage);
private:
    double sampleRate_;
    double sensitivity_;
    double amplitude_;
    double phase_;
};
