#ifndef _A4_TYPES_H_
#define _A4_TYPES_H_

#ifndef shared
#include <memory>
#define shared std::shared_ptr
#define static_shared_cast std::static_pointer_cast
#define dynamic_shared_cast std::dynamic_pointer_cast
#define reinterpret_shared_cast std::reinterpret_pointer_cast
#endif
#ifndef foreach
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#endif

#include <cstdint>

#endif
