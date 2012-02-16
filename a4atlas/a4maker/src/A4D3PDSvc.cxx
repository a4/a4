#include <cstdlib>

//#include "AnalysisTools/AANTupleParams.h"
#include "AthenaKernel/errorcheck.h"

#include "TROOT.h"

#include "A4D3PDSvc.h"
#include "A4D3PD.h"
#include "A4ProtoDumpD3PD.h"


namespace D3PD {


A4D3PDSvc::A4D3PDSvc(const std::string& name,
                     ISvcLocator* svcloc)
    : Service(name, svcloc)
{  
    declareProperty("Mode", m_mode = "generate-proto");
}

StatusCode A4D3PDSvc::initialize()
{
    CHECK(Service::initialize());
    return StatusCode::SUCCESS;
}

StatusCode A4D3PDSvc::finalize()
{
    for (size_t i = 0; i < m_d3pds.size(); i++) {
        ID3PD* d3pd = m_d3pds[i];
        delete d3pd;
    }
    return StatusCode::SUCCESS;
}

StatusCode A4D3PDSvc::make(const std::string& name, ID3PD* & d3pd)
{
    if (m_mode == "A4ProtoDumpD3PD")
    {
        IA4D3PD* a4d3pd = NULL;
        d3pd = a4d3pd = new A4ProtoDumpD3PD();
        m_d3pds.push_back(a4d3pd);
    }
    else
    {
        REPORT_MESSAGE(MSG::ERROR) << "Generate mode " << m_mode << " not yet implemented.";
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;

    /*
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
  */
  return StatusCode::SUCCESS;
}

StatusCode A4D3PDSvc::queryInterface(const InterfaceID& riid, void** ppvIf)
{
    if (riid == ID3PDSvc::interfaceID())  {
        *ppvIf = static_cast<ID3PDSvc*>(this);
        addRef();
        return StatusCode::SUCCESS;
    }

    return Service::queryInterface(riid, ppvIf);
}


}
