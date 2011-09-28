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

#include <vector>
#include <string>

#include "GaudiKernel/Service.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include "D3PDMakerInterfaces/ID3PDSvc.h"

#include "IA4D3PD.h"


namespace D3PD {


class A4D3PD;
class ID3PD;

class A4D3PDSvc : public Service, public ID3PDSvc
{
public:
    A4D3PDSvc(const std::string& name,
              ISvcLocator* svcloc);
    
    virtual StatusCode initialize();
    virtual StatusCode finalize();

    virtual StatusCode make(const std::string& name, ID3PD* & d3pd);


    /// Standard Gaudi @c queryInterface method.
    virtual StatusCode queryInterface(const InterfaceID& riid,
                                      void** ppvIf);


private:
    std::string m_mode;
    
    std::vector<IA4D3PD*> m_d3pds;

    // Disallow copying.
    A4D3PDSvc (const A4D3PDSvc&);
    A4D3PDSvc& operator= (const A4D3PDSvc&);
};


}


#endif
