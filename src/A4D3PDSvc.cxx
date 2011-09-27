#include "A4D3PDSvc.h"
#include "A4D3PD.h"
#include "A4ProtoDumpD3PD.h"
#include "AnalysisTools/AANTupleParams.h"
#include "AthenaKernel/errorcheck.h"
#include "TTree.h"
#include "TROOT.h"
#include "TFile.h"
#include "TCollection.h"
#include <cstdlib>


namespace D3PD {


/**
 * @brief Constructor.
 * @param name Service name.
 * @param svcloc Service locator.
 */
A4D3PDSvc::A4D3PDSvc (const std::string& name,
                          ISvcLocator* svcloc)
  : Service (name, svcloc),
    m_histSvc ("THistSvc", name)
{
  // See comments on cleanup().
  std::atexit (cleanup);
  
  declareProperty ("FormatDumping",  m_format_dumping = false);
  declareProperty ("HistSvc",        m_histSvc);
  declareProperty ("DoBranchRef",    m_doBranchRef = true);
  declareProperty ("MasterTree",     m_masterTree = AANTupleParams::c_treeName);
  declareProperty ("IndexMajor",     m_indexMajor = AANTupleParams::name_RunNumber);
  declareProperty ("IndexMinor",     m_indexMinor = AANTupleParams::name_EventNumber);
  declareProperty ("BasketSize",     m_basketSize = 32768);
  declareProperty ("EntryOffsetLen", m_entryOffsetLen = 512);
  declareProperty ("AutoFlush",      m_autoFlush = -1,
                   "Value to set for A4's AutoFlush parameter. "
                   "(Tells how often the tree baskets will be flushed.) "
                   "0 disables flushing. "
                   "-1 (default) makes no changes to what THistSvc did. "
                   "Any other negative number gives the number of bytes "
                   "after which to flush. "
                   "A positive number gives the number of entries after which "
                   "to flush.");
}


/**
 * @brief Make sure all files are closed before exiting, to prevent crashes.
 *
 * A4 files are normally closed by THistSvc::finalize().
 *
 * However, if we get an error during initialization, then the program
 * will exit without running the finalize() methods.  In that case,
 * the A4 files will get closed when global destructors are run.
 * But by that time, some of the A4 objects needed to perform
 * the close may have already been deleted, leading to a crash.
 *
 * To avoid this, we register this method with atexit();
 * we just loop through all existing A4 files and try to close them all.
 */
void A4D3PDSvc::cleanup ()
{
  // Sometimes gDirectory is invalid at this point...
  gDirectory = gROOT;
  TIter it (gROOT->GetListOfFiles());
  while (TObject* o = it.Next()) {
    if (TFile* f = dynamic_cast<TFile*> (o))
      f->Close();
  }
}


/**
 * @brief Standard Gaudi initialize method.
 */
StatusCode A4D3PDSvc::initialize()
{
  CHECK( Service::initialize() );
  
  return StatusCode::SUCCESS;
}


/**
 * @brief Standard Gaudi finalize method.
 */
StatusCode A4D3PDSvc::finalize()
{

#if 0
  // Run through all the trees we've made.
  for (size_t i = 0; i < m_d3pds.size(); i++) {
    A4D3PD* d3pd = m_d3pds[i];

    // Make an index if requested.
    if (!m_indexMajor.empty())
      d3pd->tree()->BuildIndex (m_indexMajor.c_str(), m_indexMinor.c_str());

    // Was there a master tree specified?
    if (!d3pd->master().empty()) {
      // Yes --- try to find it
      TTree* master = 0;
      CHECK( m_histSvc->getTree (d3pd->master(), master) );
      if (master) {
        // Make an index for the master if needed.
        if (!master->GetTreeIndex())
          // AANTupleStream will leave branch addresses in the master
          // tree pointing at dead objects.
          master->ResetBranchAddresses();

          master->BuildIndex (m_indexMajor.c_str(), m_indexMinor.c_str());

        // Make this tree a friend of the master.
        master->AddFriend (d3pd->tree());
      }
    }

    // Get rid of the A4D3PD wrapper.
    // (Doesn't delete the A4 tree itself.)
    delete d3pd;
  }
#endif
  return StatusCode::SUCCESS;
}


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
StatusCode A4D3PDSvc::make (const std::string& name, ID3PD* & d3pd)
{

  std::string tname = name;
  std::string::size_type ipos = name.rfind ('/');
  std::string master = m_masterTree;
  if (ipos != std::string::npos) {
    tname = name.substr (ipos+1);
    if (!master.empty()) {
      std::string sname = name.substr (0, ipos+1);
      std::string::size_type jpos = sname.find (':');
      if (jpos != std::string::npos)
        sname = sname.substr (0, jpos) + sname[sname.size()-1];
      master = sname + master;
    }
  }
  
  // Format dumping mode
  if (m_format_dumping)
  {
    d3pd = new A4ProtoDumpD3PD();
    m_d3pds.push_back(d3pd);
    return StatusCode::SUCCESS;
  }
  
  TTree* tree = new TTree (tname.c_str(), tname.c_str());
  CHECK( m_histSvc->regTree (name, tree) );
  if (m_doBranchRef)
    tree->BranchRef();
  if (m_autoFlush != -1)
    tree->SetAutoFlush (m_autoFlush);
  A4D3PD* rd3pd = new A4D3PD (tree, master,
                                  m_basketSize, m_entryOffsetLen);
  m_d3pds.push_back (rd3pd);
  d3pd = rd3pd;
  return StatusCode::SUCCESS;
}


/**
 * @brief Standard Gaudi @c queryInterface method.
 */
StatusCode
A4D3PDSvc::queryInterface( const InterfaceID& riid, void** ppvIf )
{
  if ( riid == ID3PDSvc::interfaceID() )  {
    *ppvIf = static_cast<ID3PDSvc*> (this);
    addRef();
    return StatusCode::SUCCESS;
  }

  return Service::queryInterface( riid, ppvIf );
}


} // namespace D3PD


namespace {
std::string foo1 = AANTupleParams::c_tokenBranchName;
std::string foo2 = AANTupleParams::c_attributeListLayoutName;
}
