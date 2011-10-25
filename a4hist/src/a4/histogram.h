#ifndef _A4_HISTOGRAM_H_
#define _A4_HISTOGRAM_H_

#include <string>

#include <a4/storable.h>
#include <a4/axis.h>

#include <a4/hist/Histograms.pb.h>

namespace a4{ namespace hist{

class H1 : public a4::process::StorableAs<H1, pb::H1>
{
    public:
        H1();
        H1(const H1 &);
        ~H1();

        // Implements StorableAs
        virtual void to_pb(bool blank_pb);
        virtual void from_pb();
        virtual H1 & operator+=(const H1 &other);

        void constructor(const char * _title) {
            _initializations_remaining++;
            title = _title;
        };
        void constructor(const uint32_t &bins, const double &min, const double &max);
        void constructor(const uint32_t &bins, const double &min, const double &max, const char * _label) {
            constructor(bins, min, max);
            _axis->label = _label;
        };

        void fill(const double &, const double &weight = 1);
        H1 & __add__(const H1 &);
        H1 & __mul__(const double &);

        uint64_t entries() const {return _entries;};
        uint64_t bins() const {return _axis->bins();};
        double integral() const;

        const Axis & x() const {return *_axis;};

        void print(std::ostream &) const;

        const shared_array<double> data() const {return _data;} //TODO: only for copyin into TH1D
        const shared_array<double> weights_squared() const {return _weights_squared;} //TODO: only for copyin into TH1D

        std::string title;

    private:
        // Prevent copying by assignment
        H1 &operator =(const H1 &);

        unique<Axis> _axis;
        shared_array<double> _data;
        shared_array<double> _weights_squared;
        uint64_t _entries;
};

std::ostream &operator<<(std::ostream &, const H1 &);

class H2 : public a4::process::StorableAs<H2, pb::H2>
{
    public:
        H2();
        H2(const H2 &);
        ~H2();

        // Implements StorableAs
        virtual void to_pb(bool blank_pb);
        virtual void from_pb();
        virtual H2 & operator+=(const H2 &other);

        void constructor(const char * _title) {
            _initializations_remaining++;
            title = _title;
        };
        void constructor(const uint32_t &bins, const double &min, const double &max, const char * _label);
        void constructor(const uint32_t &bins, const double &min, const double &max) {
            constructor(bins, min, max, "");
        };

        void fill(const double &, const double &, const double &weight = 1);
        H2 & __add__(const H2 &);
        H2 & __mul__(const double &);

        uint64_t entries() const {return _entries;};
        double integral() const;

        const Axis & x() const {return *_x_axis;};
        const Axis & y() const {return *_y_axis;};

        void print(std::ostream &) const;

        const shared_array<double> data() const {return _data;} //TODO: only for copyin into TH2D
        const shared_array<double> weights_squared() const {return _weights_squared;} //TODO: only for copyin into TH1D

        std::string title;

    private:
        // Prevent copying by assignment
        H2 &operator =(const H2 &);

        unique<Axis> _x_axis;
        unique<Axis> _y_axis;
        
        shared_array<double> _data;
        shared_array<double> _weights_squared;
        uint64_t _entries;
};

std::ostream &operator<<(std::ostream &, const H2 &);

class H3 : public a4::process::StorableAs<H3, pb::H3>
{
    public:
        H3();
        H3(const H3 &);
        ~H3();

        // Implements StorableAs
        virtual void to_pb(bool blank_pb);
        virtual void from_pb();
        virtual H3 & operator+=(const H3 &other);

        void constructor(const char * _title) {
            _initializations_remaining++;
            title = _title;
        };
        void constructor(const uint32_t &bins, const double &min, const double &max, const char * _label);
        void constructor(const uint32_t &bins, const double &min, const double &max) {
            constructor(bins, min, max, "");
        };

        void fill(const double &, const double &, const double &, const double &weight = 1);
        H3 & __add__(const H3 &);
        H3 & __mul__(const double &);

        uint64_t entries() const {return _entries;}
        double integral() const;

        const Axis & x() const {return *_x_axis;}
        const Axis & y() const {return *_y_axis;}
        const Axis & z() const {return *_z_axis;}

        void print(std::ostream &) const;

        const shared_array<double> data() const {return _data;}; //TODO: only for copyin into TH3D
        const shared_array<double> weights_squared() const {return _weights_squared;}; //TODO: only for copyin into TH1D

        std::string title;

    private:
        // Prevent copying by assignment
        H3 &operator =(const H3 &);

        unique<Axis> _x_axis;
        unique<Axis> _y_axis;
        unique<Axis> _z_axis;
        
        shared_array<double> _data;
        shared_array<double> _weights_squared;
        uint64_t _entries;
};

std::ostream &operator<<(std::ostream &, const H3 &);

};}; //namespace a4::hist

#endif

