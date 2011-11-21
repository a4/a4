// This file's extension implies that it's C, but it's really -*- C++ -*-.
// $Id$
/**
 * @file D3PDMakerA4/src/A4D3PDSvc.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2009
 * @brief Service to create a A4-based D3PD tree.
 */


#ifndef A4DUMPER_A4DUMPERSVC_H
#define A4DUMPER_A4DUMPERSVC_H

#include <vector>
#include <string>

#include "GaudiKernel/Service.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

namespace A4Dumper {

class A4DumperSvc : public Service
{
public:
    A4DumperSvc(const std::string& name,
                ISvcLocator* svcloc);
    
    virtual StatusCode initialize();
    virtual StatusCode finalize();

    /// Standard Gaudi @c queryInterface method.
    virtual StatusCode queryInterface(const InterfaceID& riid,
                                      void** ppvIf);


private:
    std::string m_mode;

    // Disallow copying.
    A4D3PDSvc (const A4D3PDSvc&);
    A4D3PDSvc& operator= (const A4D3PDSvc&);
};


}


#endif
