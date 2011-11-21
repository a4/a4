// Dear emacs, this is -*- c++ -*-
#ifndef D3PDMAKERREADER_A4DumpALG_H
#define D3PDMAKERREADER_A4DumpALG_H

#include <string>
#include <map>
#include <set>

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
        int m_year; /// Property: The year of data being processed
        std::string m_filename; ///< The output filename

        bool m_is_mc;
        std::string m_event_info_key;

        std::list<std::string> m_possible_streams;
        std::map<int, int> m_runs_encountered;
        std::map<int, double> m_runs_encountered_w;
        std::map<int, std::set<int> > m_lbs_encountered;
        ServiceHandle< A4DumperSvc > m_dumperSvc; /// Property: The Dumper service.

    }; 


}


#endif
