#ifndef _A4_DRIVER_H_
#define _A4_DRIVER_H_

#include <a4/a4process.h>

namespace a4{ namespace process{

class Driver {
    protected:
        void set_instream(Processor* p, shared<a4::io::A4InputStream> instream) { p->_instream = instream; };
        void set_outstream(Processor* p, shared<a4::io::A4OutputStream> outstream, bool fw=true) { 
            if (fw) outstream->set_forward_metadata();
            outstream->content_cls(p->content_class_id());
            outstream->metadata_cls(p->metadata_class_id());
            p->_outstream = outstream;
        };
        void set_backstore(Processor* p, shared<ObjectBackStore> bs) { p->_backstore = bs; };
        void set_store_prefix(Processor* p, const char * dir = "") { p->S = (*p->_backstore)(dir); };
};

};}; // namespace a4::process

#endif
