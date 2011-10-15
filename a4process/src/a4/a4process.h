
namespace a4{
    /// Processing utilities for A4.
    ///
    /// To implement an Analysis that processes MyEvents, and has MyMetaData,
    /// derive a class from ProcessorOf<MyEvent, MyMetaData>.
    /// If you want to process more than one type of Events, or a4 files 
    /// containing histograms, derive directly from Processor.
    ///
    /// If your analysis needs configuration or setup (command-line options, external
    /// smearing classes, ...) derive a configuration class from Configuration
    /// 
    namespace process{
    }
}
