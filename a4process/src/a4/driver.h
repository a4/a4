#ifndef _A4_DRIVER_H_
#define _A4_DRIVER_H_

#include <a4/processor.h>

namespace a4{ namespace process{

class Driver {
    protected:
        void set_instream(Processor* p, shared<a4::io::InputStream> instream) { p->_instream = instream; }
        void set_outstream(Processor* p, shared<a4::io::OutputStream> outstream, bool fw=true) { 
            if (fw) outstream->set_forward_metadata();
            p->_outstream = outstream;
        }
        void set_backstore(Processor* p, shared<ObjectBackStore> bs) { p->_backstore = bs; }
        void set_store_prefix(Processor* p, const char * dir = "") { p->S = (*p->_backstore)(dir); }
        void set_metadata(Processor* p, A4Message md) { p->metadata_message = md; }
        static bool get_auto_metadata(Processor* p) { return p->auto_metadata; }
        static const google::protobuf::Message * get_out_metadata(Processor* p) { return p->out_metadata; }
        static void reset_out_metadata(Processor* p) { p->out_metadata = NULL; }

        static void process_rerun_channels(Processor* p, A4Message msg) {
            p->process_message(msg);
            ObjectStore S = p->S;
            std::set<const char *> finished_channels;
            do {
                std::set<const char *> channels = p->rerun_channels;
                foreach (const char * c, channels) {
                    if (finished_channels.find(c) != finished_channels.end()) continue;
                    p->rerun_channels_current = c;
                    p->S = S("channel/", c, "/");
                    p->process_message(msg);
                }
                finished_channels.swap(channels);
            } while (finished_channels.size() != p->rerun_channels.size());
            p->S = S;
            p->rerun_channels_current = NULL;
            p->rerun_channels.clear();
        }

        static void process_rerun_systematics(Processor* p, A4Message msg) {
            ObjectStore S = p->S;
            process_rerun_channels(p, msg);
            foreach (const char * c, p->rerun_systematics) {
                p->rerun_systematics_current = c;
                p->S = S("systematic/", c, "/");
                process_rerun_channels(p, msg);
            }
            p->S = S;
            p->rerun_systematics_current = NULL;
            p->rerun_systematics.clear();
        }
};

};}; // namespace a4::process

#endif
