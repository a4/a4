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
#endif

#if HAVE_MEMORY
#include <memory>
#elif HAVE_TR1_MEMORY
#include <tr1/memory>
#endif

#ifndef foreach
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#endif

#include <a4/exceptions.h>

// Since we are aiming for gcc 4.3, 
// no template aliases yet.
#define shared std::shared_ptr
using std::static_pointer_cast;
using std::dynamic_pointer_cast;
// also, no reinterpret_cast until gcc 4.4...

// No template alias again...
#define unique std::unique_ptr

template<typename T, typename V>
std::unique_ptr<T> && static_pointer_cast(std::unique_ptr<V> && p) {
    return std::unique_ptr<T>(static_cast<T>(p));
};

template<typename T, typename V>
std::unique_ptr<T> && dynamic_pointer_cast(std::unique_ptr<V> && p) {
    return std::unique_ptr<T>(dynamic_cast<T>(p.release()));
};

template<typename T, typename V>
std::unique_ptr<T> && reinterpret_pointer_cast(std::unique_ptr<V> && p) {
    return std::unique_ptr<T>(reinterpret_cast<T>(p.release()));
};

// Provide an array deleter for shared and unique pointers
template<typename T> struct array_delete {
   void operator()(T* p) {
      delete [] p;
   }
};


#endif
