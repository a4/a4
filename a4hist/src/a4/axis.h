#ifndef AXIS_H
#define AXIS_H

#include <a4/types.h>

namespace a4{ namespace hist{

namespace pb{
    class Axis;
};

class Axis {
    public:
        virtual unique<pb::Axis> get_proto() = 0;
        static unique<Axis> from_proto(const pb::Axis &);
        virtual double min() const = 0;
        virtual double max() const = 0;
        virtual uint32_t bins() const = 0;
        virtual uint32_t find_bin(const double &x) const = 0;
        virtual bool variable() const = 0;
        virtual double bin_edge(const int& i) const = 0;
        std::string label;
};

class SimpleAxis : public Axis {
    public:
        SimpleAxis();
        SimpleAxis(const uint32_t &bins, const double &min, const double &max);
        SimpleAxis(const SimpleAxis &);
        SimpleAxis(const pb::Axis &);
        ~SimpleAxis();

        unique<pb::Axis> get_proto();

        double min() const {return _min;};
        double max() const {return _max;};
        uint32_t bins() const {return _bins;};
        uint32_t find_bin(const double &x) const;
        bool variable() const { return false; }
        double bin_edge(const int& i) const {
            if (i < 0 || uint32_t(i) > bins())
                throw a4::Fatal("Tried to request bin ", i, " of a ", 
                                 bins(), "-bin axis");
            return _min + i*_delta;
        }

    protected:
        bool sane() const;
        double _min;
        double _max;
        uint32_t _bins;
        double _delta;

};

std::ostream &operator<<(std::ostream &, const SimpleAxis &);

class VariableAxis : public SimpleAxis {
    public:
        VariableAxis();
        VariableAxis(const std::vector<double>& bins);
        VariableAxis(const VariableAxis&);
        VariableAxis(const pb::Axis&);
        ~VariableAxis();

        unique<pb::Axis> get_proto();

        double min() const {return _bin_bounds[1];};
        double max() const {return _bin_bounds[bins()];};
        uint32_t bins() const {return _bin_bounds_end - _bin_bounds.get() - 2;};
        uint32_t find_bin(const double&) const;
        bool variable() const { return true; }
        double bin_edge(const int& i) const {
            if (i < 0 || uint32_t(i) > bins())
                throw a4::Fatal("Tried to request bin ", i, " of a ", 
                                 bins(), "-bin axis");
            return _bin_bounds[i+1];
        }

    protected:
        bool sane() const;
        unique<double[]> _bin_bounds;
        double* _bin_bounds_end;
        
        void _init_bins(const uint32_t bins);
};

std::ostream &operator<<(std::ostream &, const VariableAxis &);

};};

#endif
