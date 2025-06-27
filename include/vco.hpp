#pragma once

class Vco {
public:
    enum WaveType { SINE, TRIANGLE, SQUARE };
    Vco(double sampleRate, double sensitivity, double amplitude);
    double generateSineWave(double controlVoltage);
    double generateTriangleWave(double controlVoltage);
    double generateSquareWave(double controlVoltage);
    double generateWaveForm(double controlVoltage, WaveType waveType);
private:
    double sampleRate_;
    double sensitivity_;
    double amplitude_;
    double phase_;
};
