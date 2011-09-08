#ifndef _A4_INTERFACES_
#define _A4_INTERFACES_

#include <string>
#include <iostream>
#include <sstream>

#include "a4/streamable.h"

class Cloneable {
    public:
        virtual ~Cloneable() {};
        virtual Cloneable * clone() const = 0;
};

class Addable {
    public:
        virtual ~Addable() {};
        virtual Addable & __add__(const Addable &) = 0;
        Addable & operator+=(const Addable & rhs) { return __add__(rhs); };
};

class Scalable {
    public:
        virtual ~Scalable() {};
        virtual Scalable & __mul__(const double &) = 0;
        Scalable & operator*=(const double & rhs) { return __mul__(rhs); };
};

class Printable {
    public:
        virtual ~Printable() {};

        virtual std::string __repr__() const { 
            std::stringstream ss;
            ss << "<" << typeid(this).name() << "> at " << std::hex << this;
            return ss.str();
        };
        virtual std::string __str__() const { return __repr__() + "\n"; };

        virtual void dump(std::ostream & o) const { o << __str__() << std::endl;};
        virtual void print(std::ostream & o) const { o << __repr__() << std::endl;};

        virtual void dump() const { dump(std::cout); };
        virtual void print() const { print(std::cout); };
};

class Named : public Printable {
    public:
        virtual ~Named() {};

        std::string get_name() {return name;};
        std::string get_title() {return title;};

        void set_name(std::string new_name) { name = new_name; }
        void set_title(std::string new_title) { title = new_title; }

        virtual std::string __repr__() const { 
            if (name.size()+title.size() != 0) {
                std::stringstream ss;
                ss << Printable::__repr__() << ", Name: \"" << name << "\", Title: \"" << title << "\"";
                return ss.str();
            }
            return Printable::__repr__();
        }

    private:
        std::string name, title;
};

class CallConstructible {
    public:
        CallConstructible() : _initialized(false) {};
        virtual ~CallConstructible() {};

    protected:
        bool _initialized;
};

class MetaData : public Printable, public Addable, virtual public Streamable {
    public:
        // Difference of two Metadata objects
        // Return values:
        // 0 = indentical
        // 1 = should always be merged (same run, same stream)
        // 2 = could be merged (same stream in Data)
        // 3 = should not usually be merged without preprocessing
        //     (different run in MC, different stream in Data)
        // 4 = very unlikely to be merged - Data / MC mixing
        // ... can be extended

        typedef enum {
            IDENTICAL = 0,
            SHOULD_MERGE = 1,
            COULD_MERGE = 2,
            SHOULD_NOT_MERGE = 3,
            NO_MERGE = 4
        } MetaDataDifference;
        
        virtual ~MetaData() {};
        virtual MetaDataDifference difference(MetaData & rhs) = 0;
};

#endif
