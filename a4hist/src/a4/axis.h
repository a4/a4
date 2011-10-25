#ifndef AXIS_H
#define AXIS_H

#include <a4/types.h>

namespace a4{ namespace hist{

namespace pb{class SimpleAxis;};

class Axis {
    public:
        virtual unique<pb::SimpleAxis> get_proto() = 0;
        static unique<Axis> from_proto(const pb::SimpleAxis &);
        virtual double min() const = 0;
        virtual double max() const = 0;
        virtual uint32_t bins() const = 0;
        virtual uint32_t find_bin(const double &x) const = 0;
        std::string label;
};

class SimpleAxis : public Axis {
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
