#include <string>
#include <vector>

#include <boost/function.hpp>
using boost::function;

#include <a4/types.h>

namespace google {
namespace protobuf {
class Message;
class MessageFactory;
class FieldDescriptor;
};
};

class TBranchElement;

template<typename T>
class Setter
{
public:
    typedef function<void (Message*, T)> ProtobufSetter;
    typedef function<T (const Message&)> ProtobufGetter;
    typedef function<void (ProtobufSetter, ProtobufGetter, Message*, void*)> SetterCaller;
    
    typedef function<void (Message*, T)> ProtobufAdder;
    typedef function<void (Message*, TBranchElement*, const ProtobufAdder&, const FieldDescriptor*)> RepeatedSetterCaller;
};

typedef function<void (Message*)>    Copier;
typedef std::vector<Copier>          Copiers;

typedef Message* MessageP;

class ROOTMessageFactory {
    const Message* _default_instance;
    Copiers _copiers;
public:
    ROOTMessageFactory() : _default_instance(NULL), _copiers() {}

    ROOTMessageFactory(const Message* default_instance, const Copiers& copiers)
        : _default_instance(default_instance), _copiers(copiers) {}
    
    shared<Message> operator()() const {
        assert(_default_instance);
        
        auto message = shared<Message>(_default_instance->New());
        
        foreach (const Copier& copier, _copiers) copier(message.get());
        
        return message;
    }
};

typedef function<void (Message**, size_t)> SubmessageSetter;
typedef std::vector<SubmessageSetter> SubmessageSetters;

ROOTMessageFactory make_message_factory(TTree*, const Descriptor*,
    const std::string&, MessageFactory*);

std::vector<std::string> get_list_of_leaves();
