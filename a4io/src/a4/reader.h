#ifndef PROTOBUF_READER_H
#define PROTOBUF_READER_H

#include <fstream>
#include <string>

typedef enum {
    READ_ITEM = 0,
    NEW_METADATA = 1,
    STREAM_END = 2,
    FAIL = 3,
} ReadResult;

template <class Item, class MetaData>
class Reader
{
    public:
        Reader(const string & input_file);
        ~Reader() {};

        // read the next item
        // returns ("got another item", "changed_metadata")
        ReadResult read(Item &);
        uint64_t items_read() const {return _items_read;};

        // returns last encountered MetaData
        const MetaData * last_meta_data() {return _last_meta_data.get(); };

    private:
        std::fstream _input;

        boost::shared_ptr< ::google::protobuf::io::ZeroCopyInputStream>
            _raw_in;

        boost::shared_ptr< ::google::protobuf::io::CodedInputStream>
            _coded_in;

        int _read_header();

        uint64_t _items_read;
        uint64_t _content_type;
        boost::shared_ptr<MetaData> _last_meta_data;
};

#endif
