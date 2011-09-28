#ifndef D3PDMAKERA4_A4ProtoDumpD3PD_H
#define D3PDMAKERA4_A4ProtoDumpD3PD_H

#include <fstream>

#include "GaudiKernel/ToolHandle.h"

#include <TClass.h>

#include "D3PDMakerInterfaces/ID3PD.h"
#include "IA4D3PD.h"

class TClass;


namespace D3PD {


class A4ProtoDumpD3PD : public IA4D3PD
{
    class Variable
    {
        friend class A4ProtoDumpD3PD;
        
        std::string type; ///< Full type name of the variable
        std::string fullname; ///< Full name of the variable in the D3PD
        std::string doc; ///< Variable documentation string
        std::string name; ///< Variable name without prefix
        std::string varname; ///< Variable name without prefix and whitespaces
        bool primitive; ///< Flag showing whether variable is a primitive
        TClass* cls;
    };

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
    std::vector<Variable> m_variables;
};


}


#endif

