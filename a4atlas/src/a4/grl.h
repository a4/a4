#include <boost/shared_ptr.hpp>
#include <inttypes.h>

#include <map>
#include <set>
#include <utility>

class GRL {
    public:
        GRL(const std::string & fn);
        bool pass(const uint32_t &run, const uint32_t &lb) const;

    private:
        typedef std::pair<uint32_t,uint32_t> LBRange; // <end lb, start lb>
        std::map<uint32_t, std::set<LBRange> > _data;

};

typedef boost::shared_ptr<GRL> GRLPtr;
