#include <iostream>

#include <a4/application.h>

#include <a4/processor.h>
#include <a4/message.h>

class CopyProcessor : public a4::process::Processor {
public:
    bool _metadata_only;
    uint64_t _start_index;
    
    CopyProcessor() : _metadata_only(false), _start_index(0) {}

    virtual void process_message(shared<const a4::io::A4Message> m) {
        if (_metadata_only) {
            skip_to_next_metadata = true;
            return;
        }
    
        // Not a thread safe static but the worst is that we only emit too few
        // events (we only used it to emit a message every 1000 events)
        static size_t i = 0;
        if (i++ < _start_index)
            return;
        if (i % 1000 == 0)
            std::cout << "Copied " << i << " events.." << std::endl;
        write(m);
    }
};

class CopyConfiguration : public a4::process::ConfigurationOf<CopyProcessor> {
public:

    bool _metadata_only;
    uint64_t _start_index;

    CopyConfiguration() : _metadata_only(false), _start_index(0) {}

    void add_options(po::options_description_easy_init opt) {
        opt("metadata-only,M", po::bool_switch(&_metadata_only), "Only copy metadata");
        opt("start-index", po::value(&_start_index), "Start copy from a particular index");
    }

    void setup_processor(CopyProcessor& p) {
        if (_start_index) {
            static int processor_count = 0;
            if (processor_count++)
                FATAL("--start-index only works with one processor (-t1)");
            p._start_index = _start_index;
        }
        p._metadata_only = _metadata_only;
    }
};

int main(int argc, const char* argv[]) {
    return a4::process::a4_main_configuration<CopyConfiguration>(argc, argv);
}
