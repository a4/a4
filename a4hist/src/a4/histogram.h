#ifndef _A4_HISTOGRAM_H_
#define _A4_HISTOGRAM_H_

#include <string>

#include <a4/storable.h>
#include <a4/axis.h>

#include <a4/hist/Histograms.pb.h>

namespace a4{ namespace hist{

class H1 : public a4::store::StorableAs<H1, pb::H1>
{
    public:
        H1();
        H1(const H1 &);
        ~H1();

        // Implements StorableAs
        virtual void to_pb(bool blank_pb);
        virtual void from_pb();
        virtual H1& operator+=(const H1 &other);
        virtual H1& operator*=(const double& v) { return __mul__(v); };

        void constructor(const char* _title) {
            _initializations_remaining++;
            title = _title;
        }
        void constructor(const uint32_t &bins, const double &min, const double &max, const char* label="");
        void constructor(const std::vector<double>& bins, const char* label="");
        H1& with_axis(const Axis& axis, const char* label="") {
            if (_initializations_remaining == 0) return *this;
            _initializations_remaining--;
            _axis = std::move(axis.clone());
            _axis->label = label;
            bin_init();
            return *this;
        }


        void constructor(const std::initializer_list<double>& bins, const char* label="") {
            constructor(std::vector<double>(bins), label);
        }

        void fill(const double& x) { fill(x, _current_weight); }
        void fill(const double& x, const double& weight) {
            assert_initialized();
            int bin = _axis->find_bin(x);
            *(_data.get() + bin) += weight;
            ++_entries;

            if (_weights_squared) {
                *(_weights_squared.get() + bin) += weight*weight;
            } else if (weight != 1.0) {
                const uint32_t total_bins = _axis->bins() + 2;
                _weights_squared.reset(new double[total_bins]);
                for(uint32_t i = 0; i < total_bins; i++)
                    _weights_squared[i] = _data[i];
                *(_weights_squared.get() + bin) += weight*weight;
            }
        }
        
        H1& __add__(const H1 &);
        H1& __mul__(const double &);

        uint64_t entries() const { return _entries; }
        uint64_t bins() const { return _axis->bins(); }
        
        /// Sum of weights in _non overflow_ bins
        double integral() const;
        
        const Axis& x() const { return *_axis; }
        const Axis& axis(uint32_t i) const {
            switch (i) {
                case 0:
                    return x();
            }
            FATAL("Requested invalid axis ", i, " of H1");
        }

        void print(std::ostream &) const;

        const shared_array<double> data() const { return _data; } //TODO: only for copyin into TH1D
        const shared_array<double> weights_squared() const { return _weights_squared; } //TODO: only for copyin into TH1D

        std::string title;

    private:
        // Prevent copying by assignment
        H1 &operator =(const H1 &);
        void ensure_weights();
        void bin_init();

        UNIQUE<Axis> _axis;
        shared_array<double> _data;
        shared_array<double> _weights_squared;
        uint64_t _entries;
};

std::ostream &operator<<(std::ostream &, const H1 &);

class H2 : public a4::store::StorableAs<H2, pb::H2>
{
    public:
        H2();
        H2(const H2 &);
        ~H2();

        // Implements StorableAs
        virtual void to_pb(bool blank_pb);
        virtual void from_pb();
        virtual H2& operator+=(const H2 &other);
        virtual H2& operator*=(const double& v) { return __mul__(v); };

        void constructor(const char* _title) {
            _initializations_remaining++;
            title = _title;
        }
        void constructor(const uint32_t& bins, const double& min, const double& max, const char* _label="");
        void constructor(const std::vector<double>& bins, const char* label="");
        void constructor(const std::initializer_list<double>& bins, const char* label="") {
            constructor(std::vector<double>(bins), label);
        }
        H2& with_axis(const Axis& axis) {
            if (_initializations_remaining == 0) return *this;
            _initializations_remaining--;
            add_axis(axis.clone());
            return *this;
        }
        
        void fill(const double& x, const double& y) { fill(x, y, _current_weight); }
        void fill(const double& x, const double& y, const double& weight) {
            assert_initialized();
            int binx = _x_axis->find_bin(x);
            int biny = _y_axis->find_bin(y);

            const int skip = _x_axis->bins() + 2;
            *(_data.get() + binx + biny*skip) += weight;
            ++_entries;

            if (_weights_squared) {
                *(_weights_squared.get() + binx + biny*skip) += weight*weight;
            } else if (weight != 1.0) {
                const uint32_t total_bins = (_x_axis->bins() + 2)*(_y_axis->bins() + 2);
                _weights_squared.reset(new double[total_bins]);
                for(uint32_t i = 0; i < total_bins; i++)
                    _weights_squared[i] = _data[i];
                *(_weights_squared.get() + binx + biny*skip) += weight*weight;
            }
        }

        H2& __add__(const H2 &);
        H2& __mul__(const double &);

        uint64_t entries() const { return _entries; }
        /// Sum of weights in _non overflow_ bins
        double integral() const;

        const Axis& x() const { return *_x_axis; }
        const Axis& y() const { return *_y_axis; }
        const Axis& axis(uint32_t i) const {
            switch (i) {
                case 1: return y();
                case 0: return x();
            }
            FATAL("Requested invalid axis ", i, " of H2");
        }

        void print(std::ostream &) const;

        const shared_array<double> data() const { return _data; } //TODO: only for copyin into TH2D
        const shared_array<double> weights_squared() const { return _weights_squared; } //TODO: only for copyin into TH1D

        std::string title;

    private:
        // Prevent copying by assignment
        H2 &operator =(const H2 &);
        void ensure_weights();
        void bin_init();
        void add_axis(UNIQUE<Axis> axis);

        UNIQUE<Axis> _x_axis;
        UNIQUE<Axis> _y_axis;
        
        shared_array<double> _data;
        shared_array<double> _weights_squared;
        uint64_t _entries;
};

std::ostream &operator<<(std::ostream &, const H2 &);

class H3 : public a4::store::StorableAs<H3, pb::H3>
{
    public:
        H3();
        H3(const H3 &);
        ~H3();

        // Implements StorableAs
        virtual void to_pb(bool blank_pb);
        virtual void from_pb();
        virtual H3& operator+=(const H3 &other);
        virtual H3& operator*=(const double& v) { return __mul__(v); };

        void constructor(const char* _title) {
            _initializations_remaining++;
            title = _title;
        };
        void constructor(const uint32_t& bins, const double& min, const double& max, const char* _label="");
        void constructor(const std::vector<double>& bins, const char* label="");
        void constructor(const std::initializer_list<double>& bins, const char* label="") {
            constructor(std::vector<double>(bins), label);
        }
        template <class AxisClass>
        H3& with_axis(const AxisClass& axis) {
            if (_initializations_remaining == 0) return *this;
            _initializations_remaining--;
            add_axis(axis.clone());
            return *this;
        }

        void fill(const double& x, const double& y, const double& z) { fill(x, y, z, _current_weight); }
        void fill(const double& x, const double& y, const double& z, const double& weight) {
            assert_initialized();
            int binx = _x_axis->find_bin(x);
            int biny = _y_axis->find_bin(y);
            int binz = _z_axis->find_bin(z);
            
            const int skip_x = _x_axis->bins() + 2;
            const int skip_y = _y_axis->bins() + 2;
            *(_data.get() + binx + skip_x*(biny + skip_y*binz)) += weight;
            ++_entries;

            if (_weights_squared) {
                *(_weights_squared.get() + binx + skip_x*(biny + skip_y*binz)) += weight*weight;
            } else if (weight != 1.0) {
                const uint32_t total_bins = (_x_axis->bins() + 2)*(_y_axis->bins() + 2)*(_z_axis->bins() + 2);
                _weights_squared.reset(new double[total_bins]);
                for(uint32_t i = 0; i < total_bins; i++)
                    _weights_squared[i] = _data[i];
                *(_weights_squared.get() + binx + skip_x*(biny + skip_y*binz)) += weight*weight;
            }
        }
        
        H3& __add__(const H3 &);
        H3& __mul__(const double &);

        uint64_t entries() const { return _entries; }
        /// Sum of weights in _non overflow_ bins
        double integral() const;

        const Axis& x() const { return *_x_axis; }
        const Axis& y() const { return *_y_axis; }
        const Axis& z() const { return *_z_axis; }
        const Axis& axis(uint32_t i) const {
            switch (i) {
                case 2: return z();
                case 1: return y();
                case 0: return x();
            }
            FATAL("Requested invalid axis ", i, " of H3");
        }

        void print(std::ostream &) const;

        const shared_array<double> data() const { return _data; } //TODO: only for copyin into TH3D
        const shared_array<double> weights_squared() const { return _weights_squared; } //TODO: only for copyin into TH1D

        std::string title;

    private:
        // Prevent copying by assignment
        H3 &operator =(const H3 &);
        void ensure_weights();
        void bin_init();
        void add_axis(UNIQUE<Axis> axis);

        UNIQUE<Axis> _x_axis;
        UNIQUE<Axis> _y_axis;
        UNIQUE<Axis> _z_axis;
        
        shared_array<double> _data;
        shared_array<double> _weights_squared;
        uint64_t _entries;
};

std::ostream &operator<<(std::ostream &, const H3 &);

};}; //namespace a4::hist

#endif

