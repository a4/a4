#ifndef HAVE_A4IO
#define HAVE_A4IO

#include <a4/types.h>

#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <string>
#include <iostream>
namespace google{namespace protobuf{ class Message; };};

namespace a4{
    /// A4IO

    ///   The A4IO package provides A4 Streams - reading and writing
    ///
    ///   Concepts:
    ///       * Event Stream: list of <Event> objects, with <MetaData> objects following after groups of events
    ///       * Results: list of pairs (<Key>, <Result>), with <MetaData> following
    ///
    ///    <Event> objects can have arbitrary format
    ///    <Result> objects _must_ be addable (operator + / __add__, can be a logical merge)
    ///    <MetaData> objects must be addable and can define a difference measure, which makes it
    ///       possibe to merge metadata depending on different factors,
    ///       e.g per lumiblock, per run or per stream
    ///
    ///
    ///    Streams of Events should be saved in .a4 files.
    ///    Results are saved in .results files (which are also a4 streams).
    ///
    namespace io{

        class A4Message;
        class A4Input;
        class A4Output;
        class A4InputStream;
        class A4OutputStream;

        template <typename ProtoClass> class RegisterClassID;

        using google::protobuf::Message;
        
        static const int FIRST_CUSTOM_MESSAGE_CLASS = 300;
        
        inline void dump_message(const google::protobuf::Message* message)
        {
            std::string str;
            google::protobuf::TextFormat::PrintToString(*message, &str);
            std::cout << str << std::endl;
        }
    }
}

#endif
