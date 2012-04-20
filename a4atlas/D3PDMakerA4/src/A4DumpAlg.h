// Dear emacs, this is -*- c++ -*-
#ifndef D3PDMAKERREADER_A4DumpALG_H
#define D3PDMAKERREADER_A4DumpALG_H

#include <string>

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"

#include "D3PDMakerInterfaces/ID3PDSvc.h"
#include "D3PDMakerInterfaces/IObjFillerTool.h"
#include "D3PDMakerInterfaces/IMetadataTool.h"


namespace D3PD {


    // class IReaderD3PD;

    class A4DumpAlg : public AthAlgorithm {

    public:
        A4DumpAlg( const std::string& name, ISvcLocator* svcloc );

        virtual StatusCode initialize();
        virtual StatusCode finalize();
        virtual StatusCode execute();

    private:      
        ServiceHandle< ID3PDSvc > m_d3pdSvc; /// Property: The D3PD creation service.
        ToolHandleArray< IObjFillerTool > m_tools; /// Property: List of object filler tools to run.
        ToolHandleArray< IMetadataTool > m_metadataTools; /// Property: List of metadata tools to run.

        std::string m_filename; ///< The output filename
        std::string m_tuplePath; ///< The D3PD base name
        std::vector< std::string > m_prefixes; ///< Variable name prefixes
        std::vector< std::string > m_classnames; ///< Reader class names

        std::string m_dir; ///< Directory where the sources should be put

        std::vector< ID3PD* > m_d3pds;

        bool m_booked; /// Flag that we've called book().

    }; 


}


#endif
