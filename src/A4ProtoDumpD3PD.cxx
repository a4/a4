#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cassert>

using std::endl;

#include <boost/foreach.hpp>

#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/ToolHandle.h"

#include "TString.h"
#include "TObjString.h"
#include "TROOT.h"
#include "TDataType.h"

#include "D3PDMakerInterfaces/IObjFillerTool.h"

#include "A4ProtoDumpD3PD.h"

namespace {

char find_typecode(const std::type_info& ti)
{
    if (ti == typeid(char*))
        return 'C';
    else if (ti == typeid(Char_t))
        return 'B';
    else if (ti == typeid(UChar_t))
        return 'b';
    else if (ti == typeid(Short_t))
        return 'S';
    else if (ti == typeid(UShort_t))
        return 's';
    else if (ti == typeid(Int_t))
        return 'I';
    else if (ti == typeid(UInt_t))
        return 'i';
    else if (ti == typeid(Float_t))
        return 'F';
    else if (ti == typeid(Double_t))
        return 'D';
    else if (ti == typeid(Long64_t))
        return 'L';
    else if (ti == typeid(ULong64_t))
        return 'l';
    else if (ti == typeid(Bool_t))
        return 'O';

    return '\0';
}


}

namespace D3PD {

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

StatusCode A4ProtoDumpD3PD::recordInfo(
    std::ofstream&          output, 
    const std::string&      prefix, 
    const std::string&      classname, 
    const ToolHandle<IObjFillerTool>&   tool) const
{
    output << "---" << endl
           << "prefix: " << prefix << endl
           << "classname: " << classname << endl
           << "tool: " << tool->name() << endl
           << "is_container: " << tool->isContainerFiller() << endl
           << "variables: " << endl;
    
    BOOST_FOREACH( const Variable& v, m_variables ) {
        output << "  - ["
               << "'" << v.type << "', "
               << "'" << v.fullname.substr(prefix.length()) << "', "
               //<< "'" << v.name << "', "
               //<< "'" << v.varname << "', "
               //<< "'" << v.doc << "', "
               << "" << (v.primitive ? "True" : "False") << ", "
               << "" << (v.cls ? "True" : "False")
               << "]"
               << endl;
    }
    
    return StatusCode::SUCCESS;
}

StatusCode A4ProtoDumpD3PD::addVariable(
    const std::string& name,
    const std::type_info& ti,
    void* & ptr,
    const std::string& docstring,
    const void* defval)
{
    REPORT_MESSAGE (MSG::INFO) << "Added variable " << name << " " << ti.name() << " " 
                               << ptr << " " << docstring << " " << defval;

    // Store the full description of the variable:
    A4ProtoDumpD3PD::Variable var;
    var.fullname = name;
    var.name = name;
    var.varname = name;
    var.doc = docstring;  
    
    char typecode = find_typecode(ti);
    if (typecode != '\0') {
        // Let's use the ROOT native types for the primitives:
        var.type = typecode;
        var.primitive = true;
        var.cls = NULL;
    } else {
        // When we'll read the object, we'll have to use a pointer:
        var.type = System::typeinfoName( ti.name() ) + "*";
        var.primitive = false;
        var.cls = getClass(ti);
    }

    /*
    // docstring.
    docstring.c_str();

    char typecode = find_typecode(ti);
    if (typecode != '\0') {
        // Basic type
        
        EDataType dt = TDataType::GetType(ti);
        TDataType* tdt = gROOT->GetType(TDataType::GetTypeName(dt));
        assert (tdt != 0);
        
        size_t defsize = tdt->Size();
        ptr = new unsigned char[defsize];
        
        // Use this immediately
        const char* default_string = tdt->AsString(ptr);
    }
    else {
        // A class type.  Try to find the class.
        TClass* cls = getClass(ti);
    }
    */

    m_variables.push_back(var);

    return StatusCode::SUCCESS;
}
StatusCode A4ProtoDumpD3PD::addMetadata (
    const std::string& /*key*/,
    const void* /*obj*/,
    const std::type_info& /*ti*/)
{
    /*
  TObjString ostmp;

  TClass* cls = getClass (ti);
  if (!cls)
    return StatusCode::RECOVERABLE;

  if (ti == typeid(TString)) {
    ostmp.String() = *reinterpret_cast<const TString*> (obj);
    obj = &ostmp;
    cls = gROOT->GetClass ("TObjString");
  }
  else if (ti == typeid(std::string)) {
    ostmp.String() = *reinterpret_cast<const std::string*> (obj);
    obj = &ostmp;
    cls = gROOT->GetClass ("TObjString");
  }

  std::string metaname;
  TDirectory* dir = m_tree->GetDirectory();
  std::string thekey = key;

  if (key.size() > 0 && key[key.size()-1] == '/') {
    // Go to the root directory of the file.
    while (dynamic_cast<TDirectoryFile*> (dir) != 0 &&
           dir->GetMotherDir())
      dir = dir->GetMotherDir();

    metaname = key.substr (0, key.size()-1);
    thekey = m_tree->GetName();
  }
  else {
    metaname = m_tree->GetName();
    metaname += "Meta";
  }

  TDirectory::TContext ctx (dir);
  TDirectory* metadir = dir->GetDirectory (metaname.c_str());
  if (!metadir) {
    metadir = dir->mkdir (metaname.c_str());
    if (!metadir) {
      REPORT_MESSAGE (MSG::ERROR)
        << "Can't create metadata dir " << metaname
        << "in dir " << dir->GetName();
      return StatusCode::RECOVERABLE;
    }
  }

  // If we're writing in a common directory, make sure name is unique.
  if (key.size() > 0 && key[key.size()-1] == '/') {
    int i = 1;
    while (metadir->FindObject (thekey.c_str())) {
      ++i;
      std::ostringstream ss;
      ss << m_tree->GetName() << "-" << i;
      thekey = ss.str();
    }
  }

  if (metadir->WriteObjectAny (obj, cls, thekey.c_str(), "overwrite") == 0) {
    REPORT_MESSAGE (MSG::ERROR)
      << "Can't write metadata object " << thekey
      << " for tree " << m_tree->GetName();
    return StatusCode::RECOVERABLE;
  }
    */
  return StatusCode::SUCCESS;
}

StatusCode
A4ProtoDumpD3PD::addDimensionedVariable(
    const std::string& /*name*/,
    const std::type_info& /*ti*/,
    void* & /*ptr*/,
    const std::string& /*dim*/,
    const std::string& /*docstring = ""*/,
    const void* /*defval = 0*/)
{
    // Not sure if this is actually used by D3PDMaker yet (unlikely)
    REPORT_MESSAGE (MSG::ERROR) << "addDimensionedVariable not yet implemented.";
    return StatusCode::FAILURE;
}


TClass* A4ProtoDumpD3PD::getClass (const std::type_info& ti)
{
  TClass* cls = gROOT->GetClass (ti);
  if (!cls) {
    // Can't do autoloading via type_info.
    // Try to convert back to a name...
    std::string tiname = System::typeinfoName (ti);
    cls = gROOT->GetClass (tiname.c_str());
  }

  if (!cls) {
    REPORT_MESSAGE (MSG::ERROR) << "Can't find class for typeinfo "
                                << ti.name();
  }

  return cls;
}


}

