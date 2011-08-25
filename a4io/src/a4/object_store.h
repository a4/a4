#include <vector>
#include <map>
#include <utility>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>

using std::vector;
struct _object_store_append_to
{
    std::string * s;
    _object_store_append_to(std::string * s) : s(s) {}
    template <typename T> void operator()(T const& x) const {(*s) += x;};
};

template <typename STORE>
class ObjectStore {
    public:
        typedef const char * cstr;

        template <class T> 
        boost::shared_ptr<T> get(const std::string &name) {
            boost::shared_ptr<STORE> & res = main_storage[name];
            if (!res) res.reset(new T());
            return boost::static_pointer_cast<T>(res);
        }
        template <class T>
        void set(const std::string &name, T* new_created) {
            boost::shared_ptr<T> p(new_created);
            main_storage[name] = boost::static_pointer_cast<STORE>(p);
        }
        template <class T> 
        boost::shared_ptr<T> get_checked(const std::string &name) const {
            typename std::map<std::string, boost::shared_ptr<STORE> >::const_iterator obj;
            obj = main_storage.find(name);
            if (obj == main_storage.end()) return boost::shared_ptr<T>();
            return boost::dynamic_pointer_cast<T>(obj->second);
        }

        vector<std::string> list() const {
            vector<std::string> res;
            for (typename std::map<std::string, boost::shared_ptr<STORE> >::const_iterator it = main_storage.begin(), end = main_storage.end(); it != end; it++) {
                res.push_back(it->first);
            }
            return res;
        }
        template <class T>
        vector<std::string> list() const {
            vector<std::string> res;
            for (typename std::map<std::string, boost::shared_ptr<STORE> >::const_iterator it = main_storage.begin(), end = main_storage.end(); it != end; it++) {
                if ((boost::dynamic_pointer_cast<T>(it->second)).get())
                    res.push_back(it->first);
            }
            return res;
        }

        template <class T>
        T & get(const int & hint, cstr n1) {return get<T,N1>(hint,N1(n1),cache1);};
        template <class T>
        T & get(const int & hint, cstr n1, cstr n2) {return get<T,N2>(hint,N2(n1,n2),cache2);};
        template <class T>
        T & get(const int & hint, cstr n1, cstr n2, cstr n3) {return get<T,N3>(hint,N3(n1,n2,n3),cache3);};
        template <class T>
        T & get(const int & hint, cstr n1, cstr n2, cstr n3, cstr n4) {return get<T,N4>(hint,N4(n1,n2,n3,n4),cache4);};

    private:
        template <class T, typename N>
        T & get(const int & hint, const N & name, vector< vector< std::pair<N, boost::shared_ptr<STORE> > > > & cache) {
            if (cache.size() <= hint) cache.resize(hint+1);
            vector< std::pair<N, boost::shared_ptr<STORE> > > &hints = cache[hint];
            typedef typename vector< std::pair<N, boost::shared_ptr<STORE> > >::const_iterator hints_iterator;
            for (hints_iterator it = hints.begin(), end = hints.end(); it != end; it++)
                if (it->first == name) return *static_cast<T*>(it->second.get());
            // not found
            std::string n;
            boost::fusion::for_each(name, _object_store_append_to(&n) );
            boost::shared_ptr<T> t = get<T>(n);
            hints.push_back(std::pair<N, boost::shared_ptr<STORE> >(name, t));
            return *t.get();
        }
        std::map<std::string, boost::shared_ptr<STORE> > main_storage;
        typedef boost::fusion::tuple<cstr> N1;
        typedef boost::fusion::tuple<cstr, cstr> N2;
        typedef boost::fusion::tuple<cstr, cstr, cstr> N3;
        typedef boost::fusion::tuple<cstr, cstr, cstr, cstr> N4;
        vector< vector< std::pair<N1, boost::shared_ptr<STORE> > > > cache1;
        vector< vector< std::pair<N2, boost::shared_ptr<STORE> > > > cache2;
        vector< vector< std::pair<N3, boost::shared_ptr<STORE> > > > cache3;
        vector< vector< std::pair<N4, boost::shared_ptr<STORE> > > > cache4;
};

