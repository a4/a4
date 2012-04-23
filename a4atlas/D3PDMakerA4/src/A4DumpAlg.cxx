#include <fstream>

#include "AthenaKernel/errorcheck.h"

#include "D3PDMakerInterfaces/ID3PD.h"

#include "A4DumpAlg.h"
#include "IA4D3PD.h"


namespace D3PD {


A4DumpAlg::A4DumpAlg( const std::string& name, ISvcLocator* svcloc )
    : AthAlgorithm( name, svcloc ),
    m_d3pdSvc("D3PD::A4D3PDSvc", name),
    m_tools( this ),
    m_metadataTools( this ),
    m_d3pds(),
    m_booked( false ) 
{

    declareProperty("D3PDSvc", m_d3pdSvc, "The D3PD creation service.");
    declareProperty("Tools", m_tools, "List of IObjFillerTool instances to run.");
    declareProperty("MetadataTools", m_metadataTools, "List of IMetadataTool instances to run.");
    declareProperty("TuplePath", m_tuplePath = "dummy", "The name of the tuple. The interpretation of this depends on the D3PDSvc.");

    declareProperty("Prefixes", m_prefixes, "Common prefix to the D3PD variable names");
    declareProperty("ClassNames", m_classnames, "Name of the C++ classes to be generated");

    declareProperty("Directory", m_dir = "./", "Output directory for the generated sources");
    
    declareProperty("Filename", m_filename = "a4info.yaml", "Output filename");
}

StatusCode A4DumpAlg::initialize()
{

    ATH_MSG_INFO( "Initializing - Package version: " << PACKAGE_VERSION );

    CHECK( m_d3pdSvc.retrieve() );
    CHECK( m_tools.retrieve() );

    if ( ( m_tools.size() != m_prefixes.size()   ) ||
         ( m_tools.size() != m_classnames.size() ) ) 
    {
        REPORT_MESSAGE( MSG::ERROR ) << "The configuration is inconsistent: "
              << m_tools.size() << " tools, "
              << m_prefixes.size() << " prefixes "
              << m_classnames.size() << " classnames";
              
        for (size_t i = 0; i < m_prefixes.size(); i++)
        {
            REPORT_MESSAGE( MSG::ERROR ) << " " << i << " "
                << (i < m_tools.size()      ? m_tools[i].name() : std::string("")) << " : "
                << (i < m_prefixes.size()   ? m_prefixes[i]     : std::string("")) << " : "
                << (i < m_classnames.size() ? m_classnames[i]   : std::string("")) << " : ";
        }
        return StatusCode::FAILURE;
    }

    // Create a new D3PD for each object filled tool:
    for( size_t i = 0; i < m_tools.size(); ++i ) 
    {

        // Generate a name for the class if it doesn't have one yet:
        if( m_classnames[ i ] == "" ) 
        {
            static int counter = 1;
            std::ostringstream classname;
            classname << "D3PDObject" << counter;
            m_classnames[ i ] = classname.str();
            ++counter;
        }

        ATH_MSG_INFO("Creating D3PD object with name: " << m_tuplePath
                     << m_classnames[ i ] );

        ID3PD* d3pd = NULL;
        CHECK( m_d3pdSvc->make( m_tuplePath + m_classnames[ i ], d3pd ) );

        // Check that the service created the correct type of D3PD object:
        IA4D3PD* rd3pd = dynamic_cast<IA4D3PD*>(d3pd);
        if( ! rd3pd ) 
        {
            REPORT_MESSAGE( MSG::ERROR ) << 
                "The configured service (" << m_d3pdSvc << ") did not create a "
                << "D3PD::IA4D3PD object!";
            return StatusCode::FAILURE;
        }

        // Configure the D3PD object:
        // rd3pd->setIsContainer( m_tools[ i ]->isContainerFiller() );

        // Remember this D3PD object:
        m_d3pds.push_back(rd3pd);

        CHECK(m_tools[i]->configureD3PD(d3pd));
    }

    m_booked = false;
    return StatusCode::SUCCESS;
}

StatusCode A4DumpAlg::finalize() 
{
    std::ofstream output(m_filename.c_str());
    
    for (size_t i = 0; i < m_tools.size(); ++i) 
    {
        IA4D3PD* d = dynamic_cast<IA4D3PD*>(m_d3pds[i]);
        if (!d)
            return StatusCode::FAILURE;
        CHECK(d->recordInfo(output, m_prefixes[i], m_classnames[i], m_tools[i]));
    }

    return StatusCode::SUCCESS;
}

StatusCode A4DumpAlg::execute() 
{
    // Instruct all the tools to declare their variables to the D3PD object:
    if (!m_booked) 
    {
        m_booked = true;
        for (size_t i = 0; i < m_tools.size(); ++i) 
        {
            CHECK(m_tools[ i ]->book());
        }
    }

    return StatusCode::SUCCESS;
}

} // namespace D3PD

