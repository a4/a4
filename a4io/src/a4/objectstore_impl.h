
#include <sstream>
#include <iostream>
#include <fstream>


bool is_writeable_pointer(const char * _p) {
    uintptr_t p = reinterpret_cast<uintptr_t>(_p);
    std::ifstream map("/proc/self/maps");
    uintptr_t start, end; char minus, rflag, wflag;
    std::string line;
    while(getline(map,line)) {
        std::stringstream(line) >> std::hex >> start >> minus >> end >> rflag >> wflag;
        //std::cerr << std::hex << start << " to " << end << " flag " << wflag << std::endl;
        if (start < p && p < end) return (wflag == 'w');
    }
    std::cerr << "ptr at " << std::hex << p << " not in any mapped region!" << std::endl;
    return true;
}


hash_lookup::hash_lookup() {
    cc_key = NULL;
    ui_key = 0;
    for (int i = 0; i < size; i++) {
        data[i].value = NULL;
        data[i].cc_key = NULL;
        data[i].ui_key = 0;
        subdir[i] = NULL;
    };
}

void * & hash_lookup::lookup(const char * const index) {
    uintptr_t idx = reinterpret_cast<uintptr_t>(index) % size;
    uintptr_t idx0 = idx - 1;
    while (idx != idx0) {
        hash_lookup::hash_lookup_data & d = data[idx];
        if (d.cc_key == index && d.ui_key == 0) return d.value;
        if (d.value == NULL) { 
            d.ui_key = 0;
            d.cc_key = index; 
            if (is_writeable_pointer(index)) {
                std::cerr << "ERROR: Detected dynamically generated string in object lookup! "\
                             "Change it to a static string or use the slower get() lookup." << std::endl;
                assert(false);
            }
            return d.value; 
        };
        idx = (idx+1) % size;
    }
    std::cerr << "ERROR: Hash table of strings full - what are you doing???" << std::endl;
}

void * & hash_lookup::lookup(uint32_t index) {
    uintptr_t idx = index % size;
    uintptr_t idx0 = idx - 1;
    while (idx != idx0) {
        hash_lookup::hash_lookup_data & d = data[idx];
        if (d.ui_key == index && d.cc_key == NULL) return d.value;
        if (d.value == NULL) { d.ui_key = index; d.cc_key = NULL; return d.value; };
        idx = (idx+1) % size;
    }
    std::cerr << "ERROR: Hash table of strings full - what are you doing???" << std::endl;
}

template <typename... Args>
void * & hash_lookup::lookup(const char * const index, const Args& ...args) {
    uintptr_t idx = reinterpret_cast<uintptr_t>(index) % size;
    uintptr_t idx0 = idx - 1;
    while (idx != idx0) {
        hash_lookup * & sd = subdir[idx];
        if (sd == NULL) {
            sd = new hash_lookup(); 
            sd->cc_key = index;
            if (is_writeable_pointer(index)) {
                std::cerr << "ERROR: Detected dynamically generated string in object lookup!\
                              Change it to a static string or use the slower get() lookup." << std::endl;
                assert(false);
            }
            sd->lookup(args...); };
        if (sd->cc_key == index && sd->ui_key == 0) return sd->lookup(args...);
        idx = (idx+1) % size;
    }
    std::cerr << "ERROR: Hash table full - what are you doing???" << std::endl;
}

template <typename... Args>
void * & hash_lookup::lookup(uint32_t index, const Args& ...args) {
    uintptr_t idx = index % size;
    uintptr_t idx0 = idx - 1;
    while (idx != idx0) {
        hash_lookup * & sd = subdir[idx];
        if (sd == NULL) { sd = new hash_lookup(); sd->ui_key = index; sd->lookup(args...); };
        if (sd->ui_key == index && sd->cc_key == NULL) return sd->lookup(args...);
        idx = (idx+1) % size;
    }
    std::cerr << "ERROR: Hash table full - what are you doing???" << std::endl;
}


std::string str_printf(const char * s) { return std::string (s); };
template<typename T, typename... Args, int N = sizeof...(Args)>
std::string str_printf(const char * s, const T& value, const Args&... args) {
    std::string res;
    while (*s) {
        if (*s == '%' && *++s != '%') {
            res += std::string(value);
            return res + str_printf(++s, args...);
        }
        res += char(*s++);
    }
    // Append extra arguments
    return res + std::string(value) + str_printf("", args...);
}

std::string str_cat() { return std::string(""); };

template<typename T, typename... Args, int N = sizeof...(Args)>
std::string str_cat(const T& s, const Args&... args) {
    std::stringstream ss;
    ss << s << str_cat(args...);
    return ss.str();
}


// Retrieve a pointer to a T from the store with no checks, using its full name
template <class STORE>
template <class T> 
boost::shared_ptr<T> ObjectStore<STORE>::get(const std::string &name) {
    boost::shared_ptr<STORE> & res = main_storage[name];
    if (!res) res.reset(new T());
    return boost::static_pointer_cast<T>(res);
}

// Map "name" to a pointer to T. Passes ownership of T to the Store.
template <class STORE>
void ObjectStore<STORE>::set(const std::string &name, STORE* newly_created) {
    boost::shared_ptr<STORE> p(newly_created);
    main_storage[name] = p;
}

// Map "name" to a pointer
template <class STORE>
void ObjectStore<STORE>::set(const std::string &name, boost::shared_ptr<STORE> obj) {
    main_storage[name] = obj;
}

// Retrieve a pointer to T from the store with type checked
template <class STORE>
template <class T> 
boost::shared_ptr<T> ObjectStore<STORE>::get_checked(const std::string &name) const {
    typename std::map<std::string, boost::shared_ptr<STORE> >::const_iterator obj;
    obj = main_storage.find(name);
    if (obj == main_storage.end()) return boost::shared_ptr<T>();
    return boost::dynamic_pointer_cast<T>(obj->second);
}

// List all names in the Store
template <class STORE>
std::vector<std::string> ObjectStore<STORE>::list() const {
    std::vector<std::string> res;
    for (typename std::map<std::string, boost::shared_ptr<STORE> >::const_iterator it = main_storage.begin(), end = main_storage.end(); it != end; it++) {
        res.push_back(it->first);
    }
    return res;
}

// List all names of a given class in the store
template <class STORE>
template <class T>
std::vector<std::string> ObjectStore<STORE>::list() const {
    std::vector<std::string> res;
    for (typename std::map<std::string, boost::shared_ptr<STORE> >::const_iterator it = main_storage.begin(), end = main_storage.end(); it != end; it++) {
        if ((boost::dynamic_pointer_cast<T>(it->second)).get())
            res.push_back(it->first);
    }
    return res;
}

template <class STORE>
template <class T, typename... Args>
T & ObjectStore<STORE>::get_fmt(const char * name_fmt, const Args& ...args) {
    void * & res = fast_lookup.lookup(args...);
    if (res != NULL) return *(static_cast<T*>(res));
    boost::shared_ptr<T> t = get<T>(str_printf(args...));
    res = t.get();
    return *t.get();
}

template <class STORE>
template <class T, typename... Args>
T & ObjectStore<STORE>::get_cat(const Args& ...args) {
    void * & res = fast_lookup.lookup(args...);
    if (res != NULL) return *(static_cast<T*>(res));
    boost::shared_ptr<T> t = get<T>(str_cat(args...));
    res = t.get();
    return *t.get();
}

template <class STORE>
template <class T, typename... Args>
T & ObjectStore<STORE>::get_hint(uint32_t hint, const Args& ...args) {
    void * & res = fast_lookup.lookup(hint, args...);
    if (res != NULL) return *(static_cast<T*>(res));
    boost::shared_ptr<T> t = get<T>(str_cat(args...));
    res = t.get();
    return *t.get();
}


