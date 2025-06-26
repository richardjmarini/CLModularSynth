#pragma once

class Cv {
public:
    Cv(double sampleRate,  double amplitude);
    void process(double duration);
private:
    double sampleRate_;
    double amplitude_;
};


