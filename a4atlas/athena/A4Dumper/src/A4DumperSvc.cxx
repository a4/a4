#include <cstdlib>

//#include "AnalysisTools/AANTupleParams.h"
#include "AthenaKernel/errorcheck.h"

#include "TROOT.h"

#include "A4DumperSvc.h"

namespace A4Dumper {


A4DumperSvc::A4DumperSvc(const std::string& name,
                     ISvcLocator* svcloc)
    : Service(name, svcloc)
{  
    declareProperty("Mode", m_mode = "testing");
}

StatusCode A4DumperSvc::initialize()
{
    CHECK(Service::initialize());
    return StatusCode::SUCCESS;
}

StatusCode A4DumperSvc::finalize()
{
    return StatusCode::SUCCESS;
}

StatusCode A4DumperSvc::queryInterface(const InterfaceID& riid, void** ppvIf)
{
    if (riid == ID3PDSvc::interfaceID())  {
        *ppvIf = static_cast<ID3PDSvc*>(this);
        addRef();
        return StatusCode::SUCCESS;
    }

    return Service::queryInterface(riid, ppvIf);
}


}
