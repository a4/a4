
#include <algorithm>
#include <vector>

/// Sort `container` by a function fragment `key` which operates on the variable
/// `itemname`
#define SORT_KEY(container, itemname, key) \
    { \
        typedef decltype(container) container##_type; \
        typedef container##_type::value_type value_type; \
        auto key_function = [](const value_type& itemname) \
                              { return key; }; \
        auto key_predicate = [&](const value_type& lhs, const value_type& rhs) \
                                { return key_function(lhs) < key_function(rhs); }; \
        std::sort(container.begin(), container.end(), key_predicate); \
    }

template<class Container, class Predicate>
bool any(const Container& container, const Predicate& pred) {
    foreach (const auto& item, container)
        if (pred(item)) 
            return true;
    return false;
}

template<class Container, class Predicate>
bool all(const Container& container, const Predicate& pred) {
    foreach (const auto& item, container)
        if (!pred(item)) 
            return false;
    return true;
}

/// Remove elements from  `container` which satisfy the result of a function
/// fragment `condition` which operates on the variable `itemname`
#define REMOVE_IF(container, itemname, condition) \
    { \
        typedef decltype(container) container##_type; \
        typedef container##_type::value_type value_type; \
        auto container##_eraseto = \
            std::remove_if(container.begin(), container.end(), \
                           [](const value_type& itemname) \
                             { return condition; }); \
        container.erase(container##_eraseto, container.end()); \
    }

namespace a4 {
namespace process {
namespace utility {

/// Builds a std::vector of pointer to elements from the `in` container.
template<class ContainerType>
std::vector<const typename ContainerType::value_type*> 
  vector_of_ptr(const ContainerType& in) {
    std::vector<const typename ContainerType::value_type*> out;
    out.reserve(in.size());
    foreach (const auto& value, in) {
        out.push_back(&value);
    }
    return out; // std::move(out);
}


template<class ContainerType, class Cmp>
typename ContainerType::value_type& 
  max_element(const ContainerType& container, const Cmp& cmp) {
    return max_element(container.begin(), container.end(), cmp);
}

template<class Map, typename T>
bool in_map(const Map& input, const T& value) {
    return input.find(value) != input.end();
}

};};};
