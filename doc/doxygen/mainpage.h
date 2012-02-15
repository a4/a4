/*! \mainpage A4

  Inspired and building on Samvel Khalatyan's excellent demonstration "RTvsPBThreads"
   - http://indico.cern.ch/contributionDisplay.py?contribId=29&confId=141309
   - https://github.com/ksamdev/RTvsPBThreads

  A4 aims to provide a set of loosely coupled, fast libraries for HEP data analysis.
  The current A4 libraries are:
   - a4io: Input/Output Streams
   - a4process: Tools to easily process events to histograms
 
  Concepts (not yet finalized):
   - Event Stream: list of <Event> objects, with <MetaData> objects following after groups of events
   - Results: list of pairs (<Key>, <Result>), with <MetaData> following
   - Processing: Analyzing events and producing Results and/or another Stream of Events

  
   - <Event> objects can have arbitrary format
   - <Result> objects _must_ be addable (operator + / __add__, can be a logical merge)
   - <MetaData> objects must be addable and can define a difference measure, which makes it
    possibe to merge metadata depending on different factors,
    e.g per lumiblock, per run or per stream

   Streams of Events are saved in .a4 files.
   Streams of Results are saved in .results files (which are also in the "a4" format).
 
   Processing is done in executable files with the following syntax:

   \code
   ./analysis -c config_hww.ini --results my.results --output skim.a4 events.a4
   \endcode

*/


