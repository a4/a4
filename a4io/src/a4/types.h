#ifndef _A4_TYPES_H_
#define _A4_TYPES_H_

// Needs to be first so that we eliminate these 
// "warning: ‘boost::system::posix_category’ defined but not used" type errors
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED 1
#endif

#include <a4/config.h>

#if HAVE_CSTDINT
#include <cstdint>
#elif HAVE_TR1_CSTDINT
#include <tr1/cstdint>
#elif HAVE_STDINT_H
#include <stdint.h>
#else
#error "No Standard int type header found!"
#endif

#if HAVE_CSTRING
#include <cstring>
#elif HAVE_TR1_CSTRING
#include <tr1/cstring>
#elif HAVE_STRING_H
#include <string.h>
#else
#error "No Standard cstring header found!"
#endif

#include <a4/exceptions.h>
#include <a4/string.h>
#include <a4/debug.h>

// The shared_ptr include must come before the unique<> #define.
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
using boost::shared_array;

#if __clang__

    // GCC <=4.6's implementation of shared_ptr is broken under the new C++0x 
    // standard. See good explanation here. Might be fixed one day.
    // http://stackoverflow.com/questions/7964360/using-stdshared-ptr-with-clang-and-libstdc

    #include <boost/weak_ptr.hpp>
    #include <boost/shared_ptr.hpp>
    #define shared boost::shared_ptr
    #define weak_shared boost::weak_ptr

    // Boost doesn't define a unique<> so we use shared instead, which can lead
    // to incorrect code being allowed.
    #define unique shared

    namespace std_memory_prefix = std;

#elif HAVE_STD_SMART_PTR

    #include <memory>
    // Since we are aiming for gcc 4.3, 
    // no template aliases yet.
    namespace std_memory_prefix = std;
    #define shared std::shared_ptr
    #define unique std::unique_ptr
    #define weak_shared std::weak_ptr

#elif HAVE_STD_TR1_SMART_PTR

    #include <tr1/memory>
    namespace std_memory_prefix = std::tr1;
    #define shared std::tr1::shared_ptr
    #define unique std::tr1::unique_ptr
    #define weak_shared std::tr1::weak_ptr

#else
    #error "No implementation of C++11 smart pointers found!"
#endif

#ifndef foreach
    #include <boost/foreach.hpp>
    #define foreach BOOST_FOREACH

/// Loop with variable `index` which gives the index of `value` into `iterable`
#define foreach_enumerate(index, value_def, iterable)                        \
    for(unsigned int index = static_cast<unsigned int>(-1), index##run_once = true; \
        index##run_once; index##run_once = false)            \
            foreach(value_def, iterable) if(++index,true)
#endif

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

using std_memory_prefix::static_pointer_cast;
using std_memory_prefix::dynamic_pointer_cast;
// also, no reinterpret_cast until gcc 4.4...

template<typename T, typename V>
unique<T>&& static_pointer_cast(unique<V>&& p) {
    return unique<T>(static_cast<T>(p));
}

template<typename T, typename V>
unique<T>&& dynamic_pointer_cast(unique<V>&& p) {
    return unique<T>(dynamic_cast<T>(p.release()));
}

template<typename T, typename V>
unique<T>&& reinterpret_pointer_cast(unique<V>&& p) {
    return unique<T>(reinterpret_cast<T>(p.release()));
}

// Provide an array deleter for shared and unique pointers
template<typename T> 
struct array_delete {
    void operator()(T* p) {
        delete [] p;
    }
};

#endif
