#ifndef AXIS_H
#define AXIS_H

#include <a4/types.h>

namespace a4{ namespace hist{

namespace pb{class SimpleAxis;};

class SimpleAxis {
    public:
        SimpleAxis();
        SimpleAxis(const uint32_t &bins, const double &min, const double &max);
        SimpleAxis(const SimpleAxis &);
        SimpleAxis(const pb::SimpleAxis &);
        ~SimpleAxis();

        unique<pb::SimpleAxis> get_proto();

        double min() const {return _min;};
        double max() const {return _max;};
        uint32_t bins() const {return _bins;};
        uint32_t find_bin(const double &x) const;

        std::string label;

    private:
        bool sane() const;
        double _min;
        double _max;
        uint32_t _bins;
        double _delta;

};

std::ostream &operator<<(std::ostream &, const SimpleAxis &);

};};

#endif
