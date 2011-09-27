// $Id: MultiReaderAlg.cxx 27698 2011-02-02 17:12:25Z krasznaa $

// Gaudi/Athena include(s):
#include "AthenaKernel/errorcheck.h"

// Local include(s):
#include "MultiReaderAlg.h"
#include "IReaderD3PD.h"

namespace D3PD {

   MultiReaderAlg::MultiReaderAlg( const std::string& name, ISvcLocator* svcloc )
      : AthAlgorithm( name, svcloc ),
        m_d3pdSvc( "D3PD::RootReaderD3PDSvc", name ),
        m_tools( this ),
        m_metadataTools( this ),
        m_d3pds(),
        m_booked( false ) {

        declareProperty( "D3PDSvc", m_d3pdSvc,
                         "The D3PD creation service." );
        declareProperty( "Tools", m_tools,
                         "List of IObjFillerTool instances to run." );
        declareProperty( "MetadataTools", m_metadataTools,
                         "List of IMetadataTool instances to run." );
        declareProperty( "TuplePath", m_tuplePath = "dummy",
                         "The name of the tuple. The interpretation of this "
                         "depends on the D3PDSvc." );

        declareProperty( "Prefixes", m_prefixes,
                         "Common prefix to the D3PD variable names" );
        declareProperty( "ClassNames", m_classnames,
                         "Name of the C++ classes to be generated" );

        declareProperty( "Directory", m_dir = "./",
                         "Output directory for the generated sources" );
   }

   StatusCode MultiReaderAlg::initialize() {

      ATH_MSG_INFO( "Initializing - Package version: " << PACKAGE_VERSION );

      CHECK( m_d3pdSvc.retrieve() );
      CHECK( m_tools.retrieve() );

      // Check if the configuration looks okay:
      if( ( m_tools.size() != m_prefixes.size() ) ||
          ( m_tools.size() != m_classnames.size() ) ) {
         REPORT_MESSAGE( MSG::ERROR ) << "The configuration is inconsistent: "
                                      << m_tools.size() << " tools, "
                                      << m_prefixes.size() << " prefixes"
                                      << m_classnames.size() << " classnames";
         for (size_t i = 0; i < m_prefixes.size(); i++)
         {
            REPORT_MESSAGE( MSG::ERROR ) << " " << i << " "
                << (i < m_tools.size() ? m_tools[i].name() : std::string("")) << " : "
                << (i < m_prefixes.size() ? m_prefixes[i] : std::string("")) << " : "
                << (i < m_classnames.size() ? m_classnames[i] : std::string("")) << " : ";
         }
         return StatusCode::FAILURE;
      }

      // Create a new D3PD for each object filled tool:
      for( size_t i = 0; i < m_tools.size(); ++i ) {

         // Generate a name for the class if it doesn't have one yet:
         if( m_classnames[ i ] == "" ) {
            static int counter = 1;
            std::ostringstream classname;
            classname << "D3PDObject" << counter;
            m_classnames[ i ] = classname.str();
            ++counter;
         }

         ID3PD* d3pd = 0;
         CHECK( m_d3pdSvc->make( m_tuplePath + m_classnames[ i ], d3pd ) );

         ATH_MSG_INFO( "Creating D3PD object with name: " << m_tuplePath
                       << m_classnames[ i ] );

         // Check that the service created the correct type of D3PD object:
         IReaderD3PD* rd3pd = dynamic_cast< IReaderD3PD* >( d3pd );
         if( ! rd3pd ) {
            REPORT_MESSAGE( MSG::ERROR ) << "The configured service ("
                                         << m_d3pdSvc << ") did not create a "
                                         << "D3PD::IReaderD3PD object!";
            return StatusCode::FAILURE;
         }

         // Configure the D3PD object:
         rd3pd->setIsContainer( m_tools[ i ]->isContainerFiller() );

         // Remember this D3PD object:
         m_d3pds.push_back( rd3pd );

      }

      // Configure each tool.
      for( size_t i = 0; i < m_tools.size(); ++i ) {
         if( m_classnames[ i ] == "" ) continue;
         CHECK( m_tools[ i ]->configureD3PD( m_d3pds[ i ] ) );
      }

      m_booked = false;
      return StatusCode::SUCCESS;
   }

   StatusCode MultiReaderAlg::finalize() {

      // Let the D3PD generate the source code that can read it back:
      for( size_t i = 0; i < m_tools.size(); ++i ) {
         if( m_classnames[ i ] == "" ) continue;
         CHECK( m_d3pds[ i ]->createReader( m_classnames[ i ], m_dir, m_prefixes[ i ] ) );
      }

      return StatusCode::SUCCESS;
   }

   StatusCode MultiReaderAlg::execute() {

      // Instruct all the tools to declare their variables to the D3PD object:
      if( ! m_booked ) {
         m_booked = true;
         for( size_t i = 0; i < m_tools.size(); ++i ) {
            if( m_classnames[ i ] == "" ) continue;
            CHECK( m_tools[ i ]->book() );
         }
      }

      return StatusCode::SUCCESS;
   }

} // namespace D3PD
