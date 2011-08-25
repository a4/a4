#ifndef AXIS_H
#define AXIS_H
#include <iosfwd>
#include <inttypes.h>

class Axis
{
    public:
        Axis(const uint32_t &bins, const double &min, const double &max);
        Axis(const Axis &);
        ~Axis();

        double min() const {return _min;};
        double max() const {return _max;};
        uint32_t bins() const {return _bins;};
        uint32_t find_bin(const double &x) const;

    private:
        bool sane() const;
        double _min;
        double _max;
        uint32_t _bins;
        double _delta;
};

std::ostream &operator<<(std::ostream &, const Axis &);

#endif
