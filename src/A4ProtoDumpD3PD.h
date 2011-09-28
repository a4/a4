#ifndef D3PDMAKERA4_A4ProtoDumpD3PD_H
#define D3PDMAKERA4_A4ProtoDumpD3PD_H

#include <fstream>

#include "GaudiKernel/ToolHandle.h"

#include "D3PDMakerInterfaces/ID3PD.h"
#include "IA4D3PD.h"

class TClass;


namespace D3PD {


class A4ProtoDumpD3PD : public IA4D3PD
{
public:

    A4ProtoDumpD3PD() {};
    ~A4ProtoDumpD3PD() {};

    virtual StatusCode recordInfo(std::ofstream& output, const std::string& prefix, const std::string& classname, const ToolHandle<IObjFillerTool>& tool) const;

    virtual StatusCode addVariable(const std::string& name,
                                   const std::type_info& ti,
                                   void* & ptr,
                                   const std::string& docstring = "",
                                   const void* defval = 0);

    virtual StatusCode
    addDimensionedVariable(const std::string& name,
                           const std::type_info& ti,
                           void* & ptr,
                           const std::string& dim,
                           const std::string& docstring = "",
                           const void* defval = 0);

    virtual StatusCode addMetadata(const std::string& key,
                                   const void* obj,
                                   const std::type_info& ti);


    virtual StatusCode capture () { return StatusCode::SUCCESS; }
    virtual StatusCode clear () { return StatusCode::SUCCESS; }

    /// Currently unimplemented --- see design note.
    /// UNUSED by D3PDMaker
    virtual StatusCode redim (const Dim_t* /*ptr*/) { return StatusCode::FAILURE; }


private:
    TClass* getClass (const std::type_info& ti);
};


}


#endif

