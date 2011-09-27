// $Id$
/**
 * @file D3PDMakerA4/src/A4ProtoDumpD3PD.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2009
 * @brief A4-based D3PD tree.
 */

#include "A4ProtoDumpD3PD.h"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/System.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TBranch.h"
#include "TBranchElement.h"
#include "TVirtualCollectionProxy.h"
#include "TMethodCall.h"
#include "TString.h"
#include "TObjString.h"
#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TROOT.h"
#include "TDataType.h"
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace {

/**
 * @brief Helper: map from a @c std::type_info to a root type code.
 * @param ti The @c type_info to translate.
 */
char find_typecode (const std::type_info& ti)
{
  if (ti == typeid (char*))
    return 'C';
  else if (ti == typeid (Char_t))
    return 'B';
  else if (ti == typeid (UChar_t))
    return 'b';
  else if (ti == typeid (Short_t))
    return 'S';
  else if (ti == typeid (UShort_t))
    return 's';
  else if (ti == typeid (Int_t))
    return 'I';
  else if (ti == typeid (UInt_t))
    return 'i';
  else if (ti == typeid (Float_t))
    return 'F';
  else if (ti == typeid (Double_t))
    return 'D';
  else if (ti == typeid (Long64_t))
    return 'L';
  else if (ti == typeid (ULong64_t))
    return 'l';
  else if (ti == typeid (Bool_t))
    return 'O';

  return '\0';
}


} // anonymous namespace

namespace D3PD {


/****************************************************************************
 * A4ProtoDumpD3PD class.
 */


/**
 * @brief Constructor.
 * @param tree The underlying A4 tree.
 * @param master The name of the master tree.  Null if no master.
 * @param basketSize The branch basket size.  -1 to use A4 default.
 * @param entryOffsetLen The branch entry offset buffer size.
 *                       -1 to use A4 default.
 */
A4ProtoDumpD3PD::A4ProtoDumpD3PD ()
{
}


/**
 * @brief Destructor.
 */
A4ProtoDumpD3PD::~A4ProtoDumpD3PD()
{
}


/**
 * @brief Add a variable to the tuple.
 * @param name The name of the variable.
 * @param type The type of the variable.
 * @param ptr Pointer to the type of the variable.
 *            The pointer need not be initialized;
 *            the D3PD software will set the pointer
 *            prior to calling @c fill().
 * @param docstring Documentation string for this variable.
 * @param defval Pointer to the default value to use for this variable.
 *               Null for no default (generally means to fill with zeros).
 *               Of the type given by @c ti.
 *               Only works for basic types.
 */
StatusCode A4ProtoDumpD3PD::addVariable (const std::string& name,
                                  const std::type_info& ti,
                                  void* & ptr,
                                  const std::string& docstring /*= ""*/,
                                  const void* defval /*= 0*/)
{

  EDataType dt = TDataType::GetType (ti);
  TDataType* tdt = gROOT->GetType (TDataType::GetTypeName (dt));
  assert (tdt != 0);
  // docstring.
  docstring.c_str();
  
  size_t defsize = tdt->Size();
  
  ptr = new unsigned char[defsize];
  
  // Use this immediately
  const char* default_string = tdt->AsString(ptr);

  char typecode = find_typecode(ti);
  if (typecode != '\0') {
    // Basic type
  }
  else {
    // A class type.  Try to find the class.
    TClass* cls = getClass (ti);
  }

    

  REPORT_MESSAGE (MSG::ERROR)  << "I AM HERE!" << name << " -- " << docstring;
    
  return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}


/**
 * @brief Add a variable to the tuple.
 * @param name The name of the variable.
 * @param type The type of the variable.
 * @param ptr Pointer to the type of the variable.
 *            The pointer need not be initialized;
 *            the D3PD software will set the pointer
 *            prior to calling @c fill().
 * @param dim Dimension for the variable.
 *            (Presently unimplemented!)
 * @param docstring Documentation string for this variable.
 * @param defval Pointer to the default value to use for this variable.
 *               Null for no default (generally means to fill with zeros).
 *               Of the type given by @c ti.
 *               Only works for basic types.
 */
StatusCode
A4ProtoDumpD3PD::addDimensionedVariable (const std::string& /*name*/,
                                  const std::type_info& /*ti*/,
                                  void* & /*ptr*/,
                                  const std::string& /*dim*/,
                                  const std::string& /*docstring = ""*/,
                                  const void* /*defval = 0*/)
{
  REPORT_MESSAGE (MSG::ERROR)
    << "addDimensionedVariable not yet implemented.";
  return StatusCode::FAILURE;
}


/**
 * @brief Capture the current state of all variables and write to the tuple.
 */
StatusCode A4ProtoDumpD3PD::capture ()
{
  if (m_tree->Fill() < 0)
    return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}


/**
 * @brief Clear all the tuple variables.
 */
StatusCode A4ProtoDumpD3PD::clear ()
{
  return StatusCode::SUCCESS;
}


/// Currently unimplemented --- see design note.
StatusCode A4ProtoDumpD3PD::redim (const Dim_t* /*ptr*/)
{
  return StatusCode::FAILURE;
}


/**
 * @brief Return the underlying root tree.
 */
TTree* A4ProtoDumpD3PD::tree() const
{
  return m_tree;
}


/**
 * @brief Return the name of the master tree.
 */
const std::string& A4ProtoDumpD3PD::master() const
{
  return m_master;
}


/**
 * @brief Add a new piece of metadata to the tuple.
 * @param key - The key for this object.
 *              Any existing object will be overwritten.
 * @param obj - Pointer to the object to write.
 * @param ti - Type of the object to write.
 *
 * The interpretation of the @c key parameter is up to the concrete
 * D3PD implementation.  However, a key name with a trailing slash
 * NAME/ indicates that all metadata items with this name should
 * be grouped together in a collection called NAME (for example,
 * in a A4 directory with that name).
 */
StatusCode A4ProtoDumpD3PD::addMetadata (const std::string& key,
                                  const void* obj,
                                  const std::type_info& ti)
{
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

  return StatusCode::SUCCESS;
}


/**
 * @brief Try to convert from a std::type_info to a TClass.
 * @param ti The type to convert.
 *
 * On failure, write an error and return null.
 */
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


} // namespace D3PD

