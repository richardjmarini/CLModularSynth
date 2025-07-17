#include <iostream>
#include <argparse/argparse.hpp>
#include <cmath>

#include "filter.hpp"

using namespace std;

Filter::Filter(double fs, Type type, double fc, int rolloff_db)
    : fs_(fs)
{
    int order= rolloff_db / 6;
    bool odd= order & 1;
    int n_biquads= order / 2;

    if (order < 1)
        throw runtime_error("rolloff must be ≥6 dB");

    if (odd)
        first_.emplace(computeFirstOrder(type, fc));

    for (int i= 0; i < n_biquads; ++i)
        biquads_.push_back(computeBiquad(type, fc));
}

Filter::Biquad Filter::computeBiquad(Type type, double fc) const
{
    static constexpr double Q= sqrt(2.0) / 2.0;
    double w0= 2.0 * M_PI * fc / fs_;
    double sinW= std::sin(w0);
    double cosW= std::cos(w0);
    double alpha= sinW / (2.0 * Q);
    double b0, b1, b2, a0, a1, a2;

    if (type == Type::LOWPASS) {
        b0= (1.0 - cosW) / 2.0;
        b1= 1.0 - cosW;
        b2= (1.0 - cosW) / 2.0;
    } else {
        b0= (1.0 + cosW) / 2.0;
        b1= -(1.0 + cosW);
        b2= (1.0 + cosW) / 2.0;
    }

    a0= 1.0 + alpha;
    a1= -2.0 * cosW;
    a2= 1.0 - alpha;

    Biquad bq;
    bq.b0= b0 / a0;
    bq.b1= b1 / a0;
    bq.b2= b2 / a0;
    bq.a1= a1 / a0;
    bq.a2= a2 / a0;

    return bq;
}

Filter::FirstOrder Filter::computeFirstOrder(Type type, double fc) const
{
    double w0= std::tan(M_PI * fc / fs_);
    double a0= 1.0 + w0;
    FirstOrder f;

    if (type == Type::LOWPASS) {
        f.b0= w0 / a0;
        f.b1= w0 / a0;
    } else {
        f.b0= 1.0 / a0;
        f.b1= -1.0 / a0;
    }
    f.a1= (w0 - 1.0) / a0;

    return f;
}


double Filter::Biquad::process(double x)
{
    double y= b0 * x + z1;
    z1= b1 * x - a1 * y + z2;
    z2= b2 * x - a2 * y;

    return y;
}

double Filter::FirstOrder::process(double x)
{
    double y= b0 * x + z1;
    z1= b1 * x - a1 * y;

    return y;
}

double Filter::process(double x)
{
    if (first_)
        x= first_->process(x);

    for (auto &bq : biquads_)
        x= bq.process(x);

    return x;
}

Filter::Type parseFilterType(const string &s)
{
    if (s == "lowpass")
        return Filter::Type::LOWPASS;

    if (s == "highpass") 
        return Filter::Type::HIGHPASS;

    throw runtime_error("filter_type must be 'lowpass' or 'highpass'");
}


int main(int argc, char *argv[])
{
    argparse::ArgumentParser args("Filter");
    args.add_argument("--filter_type").default_value(string("lowpass")).help("lowpass | highpass").action([](const string &v){ return v; });
    args.add_argument("--cutoff").required().help("cutoff frequency in Hz") .scan<'g',double>();
    args.add_argument("--rolloff").default_value(12).help("rolloff in dB/oct (multiple of 6)").scan<'i', int>();
    args.add_argument("--sample_rate").default_value(48000.0).help("sampling rate (Hz)").scan<'g', double>();

    string line;
    double sample;

    try {
        args.parse_args(argc, argv);
    } catch (const exception &e) {
        cerr << e.what() << endl << args << endl;
        return EXIT_FAILURE;
    }

    const auto type= parseFilterType(args.get<string>("filter_type"));
    const auto cutoff= args.get<double>("cutoff");
    const auto rolloff_db= args.get<int>("rolloff");
    const auto fs= args.get<double>("sample_rate");
    if (rolloff_db % 6 != 0) {
        cerr << "rolloff must be an integer multiple of 6 dB" << endl;
        return EXIT_FAILURE;
    }

    Filter filter(fs, type, cutoff, rolloff_db);

    while (std::getline(cin, line)) {
        istringstream iss(line);
        if (iss >> sample) {
            cout << filter.process(sample) << '\n';
        } else {
            cerr << "Skipping non-numeric line: " << line << endl;
        }
    }    

    return EXIT_SUCCESS;
}

