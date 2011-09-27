// This file's extension implies that it's C, but it's really -*- C++ -*-.
// $Id$
/**
 * @file D3PDMakerA4/src/A4ProtoDumpD3PD.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2009
 * @brief A4-based D3PD tree.
 */

#ifndef D3PDMAKERA4_A4ProtoDumpD3PD_H
#define D3PDMAKERA4_A4ProtoDumpD3PD_H


#include "D3PDMakerInterfaces/ID3PD.h"
class TTree;
class TClass;


namespace D3PD {


// Helper for clearing tuple variables.
namespace A4 {
class Cleartable;
}


/**
 * @brief A4-based D3PD tree.
 *
 * This provides an @c ID3PD interface for a A4 tree.
 * The tree is passed to the constructor.  Ownership of the tree remains
 * with A4.
 *
 * If there are several trees being made, one may be designated as the master;
 * the other trees will be made friends of it.  (For AANT, this is generally
 * CollectionTree.)
 */
class A4ProtoDumpD3PD
  : public ID3PD
{
public:
  /**
   * @brief Constructor.
   * @param tree The underlying A4 tree.
   * @param master The name of the master tree.  Null if no master.
   * @param basketSize The branch basket size.  -1 to use A4 default.
   * @param entryOffsetLen The branch entry offset buffer size.
   *                       -1 to use A4 default.
   */
  A4ProtoDumpD3PD ();


  /// Destructor.
  ~A4ProtoDumpD3PD();


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
  virtual StatusCode addVariable (const std::string& name,
                                  const std::type_info& ti,
                                  void* & ptr,
                                  const std::string& docstring = "",
                                  const void* defval = 0);


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
  virtual StatusCode
  addDimensionedVariable (const std::string& name,
                          const std::type_info& ti,
                          void* & ptr,
                          const std::string& dim,
                          const std::string& docstring = "",
                          const void* defval = 0);


  /**
   * @brief Capture the current state of all variables and write to the tuple.
   */
  virtual StatusCode capture ();


  /**
   * @brief Clear all the tuple variables.
   */
  virtual StatusCode clear ();


  /// Currently unimplemented --- see design note.
  virtual StatusCode redim (const Dim_t* ptr);


  /**
   * @brief Return the underlying A4 tree.
   */
  TTree* tree() const;


  /**
   * @brief Return the name of the master tree.
   */
  const std::string& master() const;


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
  virtual StatusCode addMetadata (const std::string& key,
                                  const void* obj,
                                  const std::type_info& ti);


private:
  /// Try to convert from a std::type_info to a TClass.
  /// On failure, write an error and return null.
  TClass* getClass (const std::type_info& ti);

  /// The underlying A4 tree.
  TTree* m_tree;

  /// The name of the master tree.
  std::string m_master;

  /// Specified basket size.
  int m_basketSize;

  /// Specified entry offset buffer length.
  int m_entryOffsetLen;
};


} // namespace D3PD


#endif // not D3PDMAKERA4_A4ProtoDumpD3PD_H

