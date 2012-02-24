#ifndef _A4_CUTFLOW_H_
#define _A4_CUTFLOW_H_

#include <string>

#include <a4/storable.h>
#include <a4/axis.h>
#include <a4/hash_lookup.h>

#include <a4/hist/Cutflow.pb.h>

namespace a4{ namespace hist{

class Cutflow : public a4::process::StorableAs<Cutflow, pb::Cutflow>
{
    public:
        class CutNameCount {
            public: 
                std::string name; 
                double count, weights_squared; 
                CutNameCount(std::string n, double c, double w2) : name(n), count(c), weights_squared(w2) {};
        };

        Cutflow();
        Cutflow(const Cutflow &);
        ~Cutflow();
        
        std::string title;

        // Implements StorableAs
        virtual void constructor();
        virtual void constructor(const char* _title) { title = _title; constructor();};

        virtual void to_pb(bool blank_pb);
        virtual void from_pb();
        virtual Cutflow& operator+=(const Cutflow &other);
        virtual Cutflow& operator*=(const double &v) { return __mul__(v); };

        std::vector<CutNameCount> content() const;

        Cutflow& __add__(const Cutflow &);
        Cutflow& __mul__(const double &);

        void print(std::ostream &) const;

        template <typename... Args>
        inline void order(const Args& ...args) { 
            void *& res = _fast_access->lookup(args...);
            if (res == NULL) res = (void*)new_bin(str_cat(args...));
        }

        template <typename... Args>
        inline void passed(const Args& ...args) { fillw(_current_weight, args...); }

        template <typename... Args>
        inline void passedw(const double& w, const Args& ...args) { fillw(w, args...); }

        template <typename... Args>
        inline void fill(const Args& ...args) { fillw(_current_weight, args...); }

        template <typename... Args>
        void fillw(const double& w, const Args& ...args) {
            assert_initialized();
            void *& res = _fast_access->lookup(args...);
            if (res != NULL) return fill_internal(uintptr_t(res)-1, w);
            res = (void*)new_bin(str_cat(args...));
            fill_internal(uintptr_t(res)-1, w);
        };
    private:
        void fill_internal(const uintptr_t& idx, const double& w);
        uintptr_t new_bin(std::string name);
        unique<hash_lookup> _fast_access;
        std::vector<double> _bin;
        shared<std::vector<double> > _weights_squared;
        std::vector<std::string> _cut_names;
};

};}; //namespace a4::hist

#endif

