#ifndef PROTOBUF_READER_H
#define PROTOBUF_READER_H

#include <fstream>
#include <string>
#include <utility>

#include "a4/interfaces.h"

typedef struct RR {
    RR(uint32_t cls, boost::shared_ptr<Streamable> obj) : class_id(cls), object(obj) {};
    uint32_t class_id; 
    boost::shared_ptr<Streamable> object;
    bool operator==(const RR & rhs) {return rhs.class_id==class_id && rhs.object==object;}
} ReadResult;

const ReadResult END_OF_STREAM = {0, boost::shared_ptr<Streamable>()};
const ReadResult READ_ERROR = {1, boost::shared_ptr<Streamable>()};

class Reader
{
    public:
        Reader(const std::string & input_file);
        ~Reader() {};

        bool is_good() {return _is_good;};
        ReadResult read();
        uint64_t items_read() const {return _items_read;};
        const boost::shared_ptr<MetaData> last_meta_data() {return _last_meta_data; };

    private:
        std::fstream _input;

        boost::shared_ptr< ::google::protobuf::io::ZeroCopyInputStream>
            _raw_in;

        boost::shared_ptr< ::google::protobuf::io::CodedInputStream>
            _coded_in;

        int _read_header();
    
        bool _is_good;
        uint64_t _items_read;
        uint32_t _content_class_id;
        uint32_t _metadata_class_id;
        from_msg_func _content_func;
        boost::shared_ptr<MetaData> _last_meta_data;
};

#endif
