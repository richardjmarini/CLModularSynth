#pragma once

#include <vector>
#include <optional>
#include <stdexcept>
#include <string>

using namespace std;

class Filter {
public:
    enum class Type { LOWPASS, HIGHPASS };

    Filter(double fs, Type type, double fc, int rolloff_db);
    double process(double x);

private:
    struct FirstOrder {
        double b0, b1, a1;
        double z1{0.0};
        double process(double x);
    };

    struct Biquad {
        double b0, b1, b2, a1, a2;
        double z1{0.0}, z2{0.0};
        double process(double x);
    };

    FirstOrder computeFirstOrder(Type type, double fc) const;
    Biquad computeBiquad(Type type, double fc) const;

    double fs_;
    optional<FirstOrder> first_;
    vector<Biquad> biquads_;
};

Filter::Type parseFilterType(const string &s);

