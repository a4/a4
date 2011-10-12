#ifndef _A4_TYPES_H_
#define _A4_TYPES_H_

#if HAVE_CONFIG_H
#include <a4/config.h>
#endif

#if HAVE_CSTDINT
#include <cstdint>
#elif HAVE_TR1_CSTDINT
#include <tr1/cstdint>
#elif HAVE_STDINT_H
#include <stdint.h>
#else
#error "No Standard int type header found!"
#endif

#include <a4/exceptions.h>

#if HAVE_STD_SMART_PTR

#include <memory>
// Since we are aiming for gcc 4.3, 
// no template aliases yet.
namespace std_memory_prefix = std;
#define shared std::shared_ptr
#define unique std::unique_ptr

#elif HAVE_STD_TR1_SMART_PTR

#include <tr1/memory>
namespace std_memory_prefix = std::tr1;
#define shared std::tr1::shared_ptr
#define unique std::tr1::unique_ptr

#else
#error "No implementation of C++11 smart pointers found!"
#endif

#ifndef foreach
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#endif

using std_memory_prefix::static_pointer_cast;
using std_memory_prefix::dynamic_pointer_cast;
// also, no reinterpret_cast until gcc 4.4...

template<typename T, typename V>
unique<T> && static_pointer_cast(unique<V> && p) {
    return unique<T>(static_cast<T>(p));
};

template<typename T, typename V>
unique<T> && dynamic_pointer_cast(unique<V> && p) {
    return unique<T>(dynamic_cast<T>(p.release()));
};

template<typename T, typename V>
unique<T> && reinterpret_pointer_cast(unique<V> && p) {
    return unique<T>(reinterpret_cast<T>(p.release()));
};

// Provide an array deleter for shared and unique pointers
template<typename T> struct array_delete {
   void operator()(T* p) {
      delete [] p;
   }
};


#endif
