#include <boost/shared_ptr.hpp>
#include <inttypes.h>

#include <map>
#include <set>
#include <utility>

class GRL {
    public:
        typedef std::pair<uint32_t,uint32_t> LBRange; // WARNING: order end, start

        GRL(std::string fn);
        bool pass(uint32_t run, uint32_t lb);

    private:
        std::map<uint32_t, std::set<LBRange> > _data;

};

typedef boost::shared_ptr<GRL> GRLPtr;
