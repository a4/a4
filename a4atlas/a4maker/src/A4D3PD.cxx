// $Id$
/**
 * @file D3PDMakerA4/src/A4D3PD.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2009
 * @brief A4-based D3PD tree.
 */

#include "A4D3PD.h"
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
 * We support several different ways to clear a variable, depending
 * on the type.
 * To prevent having to do dictionary lookups, etc, on each event,
 * we do it once, and save the results in a table.  The table is called
 * @c Cleartable; it is a collection of @c Clearinfo objects.
 */


namespace A4 {


/**
 * @brief Hold information on how to clear one variable.
 */
class Clearinfo
{
public:
  /// Constructor.
  Clearinfo();


  /**
   * @brief Initialize for clearing a variable.
   * @param br The branch containing the variable.
   * @param defval Pointer to the default value to use for this variable.
   *               Null for no default (generally means to fill with zeros).
   *               Of the type given by @c ti.
   *               Only works for basic types.
   *               We take ownership of this.
   * @param defsize Size of the object pointed at by defval.
   */
  StatusCode init (TBranch* br, char* defval, size_t defsize);


  /// Clear the variable.
  void clear();


  /// Free allocated memory.
  /// (Not done in the destructor to make it easier to hold these in a vector.)
  void free();


private:
  /// The method to use to clear this variable.
  enum Cleartype {
    /// Not set yet.
    INVALID,

    /// Clear variable by filling with zeros.
    ZERO,

    /// Clear variable via collection proxy.
    COLLECTION,

    /// Clear variable by calling @c clear().
    CLEAR,

    /// Clear variable by deleting and recreating.
    RESET,

    /// Copy from a default (only for basic types).
    COPY
  }
  m_type;

  /// The leaf for this variable.  Used for ZERO.
  TLeaf* m_leaf;

  /// The branch element for this variable,  Used for COLLECTION, CLEAR, RESET.
  TBranchElement *m_bre;

  /// The collection proxy for this variable.  Used for COLLECTION.
  TVirtualCollectionProxy* m_proxy;

  /// The @c clear method for this variable.  Used for CLEAR.
  TMethodCall m_meth;

  /// Default value for COPY.  We own this.
  char* m_default;

  /// Default value size for COPY.
  size_t m_defsize;
};


/**
 * @brief Default constructor.
 *
 * You must call @c init() before this object is usable.
 */
Clearinfo::Clearinfo()
  : m_type (INVALID),
    m_leaf (0),
    m_bre (0),
    m_proxy (0),
    m_default (0),
    m_defsize (0)
{
}


/**
 * @brief Initialize for clearing a variable.
 * @param br The branch containing the variable.
 * @param defval Pointer to the default value to use for this variable.
 *               Null for no default (generally means to fill with zeros).
 *               Of the type given by @c ti.
 *               Only works for basic types.
 *               We take ownership of this.
 * @param defsize Size of the object pointed at by defval.
 */
StatusCode Clearinfo::init (TBranch* br, char* defval, size_t defsize)
{
  if (typeid (*br) == typeid (TBranch)) {
    // Atomic type
    TLeaf* leaf = br->GetLeaf (br->GetName());
    if (!leaf) {
      REPORT_MESSAGE (MSG::ERROR) << "For tree " << br->GetTree()->GetName()
                                  << " can't find leaf for branch "
                                  << br->GetName();
      return StatusCode::FAILURE;
    }

    if (defval) {
      m_type = COPY;
      m_default = defval;
      m_defsize = defsize;
    }
    else
      m_type = ZERO;

    m_leaf = leaf;
  }

  else if (TBranchElement* bre = dynamic_cast<TBranchElement*> (br)) {
    assert (defval == 0);

    // Class type.  See if it seems to be a container.
    if (TVirtualCollectionProxy* collprox = bre->GetCollectionProxy()) {
      // A collection.
      m_type = COLLECTION;
      m_bre = bre;
      m_proxy = collprox;
    }
    else {
      // See if the class has a clear() method.
      TClass* cl = gROOT->GetClass (bre->GetClassName());
      if (!cl) {
        REPORT_MESSAGE (MSG::ERROR) << "For tree " << br->GetTree()->GetName()
                                    << " branch " << br->GetName()
                                    << " can't find class "
                                    << bre->GetClassName();
        return StatusCode::FAILURE;
      }

      TMethodCall meth (cl, "clear", "");
      if (meth.IsValid()) {
        // There's a @c clear() method.  Use that.
        m_type = CLEAR;
        m_bre = bre;
        m_meth = meth;
      }
      else {
        // Free and reallocate object.
        m_type = RESET;
        m_bre = bre;
      }
    }
  }
  else {
    // Someone else must have made this?
    REPORT_MESSAGE (MSG::ERROR) << "For tree " << br->GetTree()->GetName()
                                << " branch " << br->GetName()
                                << " has unknown type "
                                << typeid(*br).name();
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Clear this variable.
 */
void Clearinfo::clear()
{
  switch (m_type) {
  case ZERO:
    // Fill with zeros.
    std::memset (m_leaf->GetValuePointer(), 0,
                 m_leaf->GetLen() * m_leaf->GetLenType());
    break;

  case COLLECTION:
    // Clear via collection proxy.
    {
      void* obj = m_bre->GetObject();
      TVirtualCollectionProxy::TPushPop pushcont(m_proxy, obj);
      m_proxy->Clear();
      break;
    }

  case CLEAR:
    // Clear by calling @c clear().
    {
      void* obj = m_bre->GetObject();
      m_meth.Execute (obj);
      break;
    }

  case RESET:
    // Free and reallocate object.
    m_bre->SetAddress(0);
    break;

  case COPY:
    // Copy from default.
    std::memcpy (m_leaf->GetValuePointer(), m_default, m_defsize);
    break;

  default:
    std::abort();
    
  }
}


/// Free allocated memory.
/// (Not done in the destructor to make it easier to hold these in a vector.)
void Clearinfo::free()
{
  delete [] m_default;
}


/**
 * @brief Table giving information on how to clear all variables in a tree.
 */
class Cleartable
{
public:
  /// Destructor.
  ~Cleartable();


  /**
   * @brief Initialize for clearing a variable.
   * @param br The branch containing the variable.
   * @param defval Pointer to the default value to use for this variable.
   *               Null for no default (generally means to fill with zeros).
   *               Of the type given by @c ti.
   *               Only works for basic types.
   *               We take ownership of this.
   * @param defsize Size of the object pointed at by defval.
   */
  StatusCode add (TBranch* br, char* defval, size_t defsize);


  /// Clear all branches.
  void clear ();


private:
  std::vector<Clearinfo> m_info;
};


/**
 * @brief Destructor.
 */
Cleartable::~Cleartable()
{
  for (size_t i = 0; i < m_info.size(); i++)
    m_info[i].free();
}


/**
 * @brief Add a new branch to the table.
 * @param br The branch to be cleared.
 * @param defval Pointer to the default value to use for this variable.
 *               Null for no default (generally means to fill with zeros).
 *               Of the type given by @c ti.
 *               Only works for basic types.
 *               We take ownership of this.
 * @param defsize Size of the object pointed at by defval.
 */
StatusCode Cleartable::add (TBranch* br, char* defval, size_t defsize)
{
  m_info.push_back (Clearinfo());
  CHECK( m_info.back().init (br, defval, defsize) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Clear all variables.
 */
void Cleartable::clear()
{
  for (size_t i = 0; i < m_info.size(); i++)
    m_info[i].clear();
}


} // namespace A4


/****************************************************************************
 * A4D3PD class.
 */


/**
 * @brief Constructor.
 * @param tree The underlying A4 tree.
 * @param master The name of the master tree.  Null if no master.
 * @param basketSize The branch basket size.  -1 to use A4 default.
 * @param entryOffsetLen The branch entry offset buffer size.
 *                       -1 to use A4 default.
 */
A4D3PD::A4D3PD (TTree* tree,
                    const std::string& master,
                    int basketSize /*= -1*/,
                    int entryOffsetLen /*= -1*/)
  : m_tree (tree),
    m_master (master),
    m_basketSize (basketSize),
    m_entryOffsetLen (entryOffsetLen),
    m_cleartable (new A4::Cleartable)
{
    throw;
}


/**
 * @brief Destructor.
 */
A4D3PD::~A4D3PD()
{
  delete m_cleartable;
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
StatusCode A4D3PD::addVariable (const std::string& name,
                                  const std::type_info& ti,
                                  void* & ptr,
                                  const std::string& docstring /*= ""*/,
                                  const void* defval /*= 0*/)
{
  TBranch* br = 0;

  // Make sure that there's not already a branch with this name.
  if (m_tree->GetBranch (name.c_str()) != 0) {
    REPORT_MESSAGE (MSG::ERROR) << "Duplicate branch name " << name
                                << " in tree " << m_tree->GetName();
    return StatusCode::FAILURE;
  }

  char* defcopied = 0;
  size_t defsize = 0;

  // See if it's a basic type.
  char typecode = find_typecode (ti);
  if (typecode != '\0') {
    // Yes.  Make a simple branch for it.
    br = m_tree->Branch (name.c_str(), 0, (name + '/' + typecode).c_str());
    if (!br)
      return StatusCode::FAILURE;

    // Find the (single) leaf for the branch.
    TLeaf* leaf = br->GetLeaf (name.c_str());
    if (!leaf)
      return StatusCode::FAILURE;

    // Set the pointer.
    ptr = leaf->GetValuePointer();

    if (defval) {
      EDataType dt = TDataType::GetType (ti);
      TDataType* tdt = gROOT->GetType (TDataType::GetTypeName (dt));
      assert (tdt != 0);
      defsize = tdt->Size();
      defcopied = new char[defsize];
      std::memcpy (defcopied, defval, defsize);
    }
  }
  else {

    // Default value not supported for class types.
    if (defval) {
      REPORT_MESSAGE (MSG::ERROR)
        << "Requested a default value for variable " << name
        << " of type " << System::typeinfoName (ti)
        << "; but default values are only supported for basic types.";
      return StatusCode::FAILURE;
    }

    // A class type.  Try to find the class.
    TClass* cls = getClass (ti);
    if (!cls)
      return StatusCode::FAILURE;

    // Create the branch.
    br = m_tree->Bronch (name.c_str(), cls->GetName(), 0);
    if (!br)
      return StatusCode::FAILURE;

    TBranchElement* bre = dynamic_cast<TBranchElement*> (br);
    if (!bre) {
      REPORT_MESSAGE (MSG::ERROR) << "Unexpected branch type created for "
                                  << name;
      return StatusCode::FAILURE;
    }

    // Set the pointer.
    ptr = bre->GetObject();
  }

  // Set the branch docstring.
  br->SetTitle (docstring.c_str());

  // Set the branch buffer sizes as requested.
  if (m_basketSize != -1)
    br->SetBasketSize (m_basketSize);

  if (m_entryOffsetLen != -1)
    br->SetEntryOffsetLen (m_entryOffsetLen);

  CHECK( m_cleartable->add (br, defcopied, defsize) );

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
A4D3PD::addDimensionedVariable (const std::string& /*name*/,
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
StatusCode A4D3PD::capture ()
{
  if (m_tree->Fill() < 0)
    return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}


/**
 * @brief Clear all the tuple variables.
 */
StatusCode A4D3PD::clear ()
{
  m_cleartable->clear();
  return StatusCode::SUCCESS;
}


/// Currently unimplemented --- see design note.
StatusCode A4D3PD::redim (const Dim_t* /*ptr*/)
{
  return StatusCode::FAILURE;
}


/**
 * @brief Return the underlying root tree.
 */
TTree* A4D3PD::tree() const
{
  return m_tree;
}


/**
 * @brief Return the name of the master tree.
 */
const std::string& A4D3PD::master() const
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
StatusCode A4D3PD::addMetadata (const std::string& key,
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
TClass* A4D3PD::getClass (const std::type_info& ti)
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

