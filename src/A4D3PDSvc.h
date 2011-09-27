// This file's extension implies that it's C, but it's really -*- C++ -*-.
// $Id$
/**
 * @file D3PDMakerA4/src/A4D3PDSvc.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2009
 * @brief Service to create a A4-based D3PD tree.
 */


#ifndef D3PDMAKERROOT_ROOTD3PDSVC_H
#define D3PDMAKERROOT_ROOTD3PDSVC_H

#include "D3PDMakerInterfaces/ID3PDSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include <vector>
#include <string>


namespace D3PD {


class A4D3PD;


/**
 * @brief Service to create a root-based D3PD.
 */
class A4D3PDSvc
  : public Service, public ID3PDSvc
{
public:
  /**
   * @brief Constructor.
   * @param name Service name.
   * @param svcloc Service locator.
   */
  A4D3PDSvc (const std::string& name,
               ISvcLocator* svcloc);

  /// Standard Gaudi initialize method.
  virtual StatusCode initialize();


  /// Standard Gaudi finalize method.
  virtual StatusCode finalize();


  /**
   * @brief Create a new D3PD tree.
   * @param name The name of the new tree.
   *             If the name contains a slash, it is interpreted
   *             as STREAM/NAME.  If the stream name contains a colon,
   *             then the part of the name before the colon is the
   *             `parent' stream name; this is used to locate the
   *             master tree.
   * @param d3pd[out] The created tree.
   */
  virtual StatusCode make (const std::string& name, ID3PD* & d3pd);


  /// Standard Gaudi @c queryInterface method.
  virtual StatusCode queryInterface( const InterfaceID& riid,
                                     void** ppvIf );


private:
  /// Make sure all files are closed before exiting, to prevent crashes.
  static void cleanup();


  /// Property: If true, then add BranchRef info to the tuple.
  bool m_doBranchRef;

  /// Property: Name of the master tree.  If set, then this tree
  /// will be made a friend of the master.  The master tree will also
  /// get an index built.
  std::string m_masterTree;

  /// Property: Major variable name for index making, or null.
  std::string m_indexMajor;
  /// Property: Minor variable name for index making, or null.
  std::string m_indexMinor;

  /// Property: Basket buffer size, or -1 to use the A4 default.
  int m_basketSize;

  /// Property: Basket entry offset buffer size, or -1 to use the A4 default.
  int m_entryOffsetLen;

  /// Property: Value to set for ROOT's AutoFlush parameter.
  /// (Tells how often the tree baskets will be flushed.)
  /// 0 disables flushing.
  /// -1 (default) makes no changes to what THistSvc did.
  /// Any other negative number gives the number of bytes after which to flush.
  /// A positive number gives the number of entries after which to flush.
  long long m_autoFlush;

  /// Property: Gaudi THistSvc.
  ServiceHandle<ITHistSvc> m_histSvc;

  /// Remember all the tuples we've made.
  std::vector<A4D3PD*> m_d3pds;

  // Disallow copying.
  A4D3PDSvc (const A4D3PDSvc&);
  A4D3PDSvc& operator= (const A4D3PDSvc&);
};


} // namespace D3PD


#endif // not D3PDMAKERROOT_ROOTD3PDSVC_H
