#ifndef _A4_STREAMABLE_H_
#define _A4_STREAMABLE_H_

#include <google/protobuf/message.h>
#include <boost/shared_ptr.hpp>

using google::protobuf::Message;
typedef boost::shared_ptr<Message> MessagePtr;

class Streamable {
    virtual void from_message(google::protobuf::Message &) = 0;
    virtual MessagePtr get_message() = 0;
};

#endif
