#include <string>
#include <vector>

#include <boost/function.hpp>
using boost::function;

#include <a4/types.h>

class Message;
class TBranchElement;
class FieldDescriptor;


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

typedef function<shared<Message> ()> RootToMessageFactory;
typedef function<void (Message*)>    Copier;
typedef std::vector<Copier>          Copiers;

typedef Message* MessageP;

typedef function<void (Message**, size_t)> SubmessageSetter;
typedef std::vector<SubmessageSetter> SubmessageSetters;

RootToMessageFactory make_message_factory(TTree* tree, const Descriptor* desc, 
    const std::string& prefix="");
