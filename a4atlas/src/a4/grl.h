#ifndef _A4ATLAS_GRL_H_
#define _A4ATLAS_GRL_H_

#include <cstdint>
#include <set>
#include <string>
#include <map>
#include <utility>

namespace a4{
    /// Namespace for ATLAS-specific utilities and helper classes
    namespace atlas{

        /// Helper class for ATLAS Good Run Lists in (run, lumiblock) format.
        /// WARNING: This class cannot be used for reading standard XML GRLS!
        class GRL {
            public:
                virtual bool pass(const uint32_t &run, const uint32_t &lb) const = 0;
        };

        class FileGRL : public GRL {
            public:
                FileGRL(const std::string & fn);
                virtual bool pass(const uint32_t &run, const uint32_t &lb) const;

            private:
                typedef std::pair<uint32_t,uint32_t> LBRange; // <end lb, start lb>
                std::map<uint32_t, std::set<LBRange> > _data;

        };

        class NoGRL : public GRL {
            public:
                NoGRL() {};
                bool pass(const uint32_t &, const uint32_t &) const { return true; };
        };
    };
};

#endif
