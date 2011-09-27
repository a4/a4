# $Id$

# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )

from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.MessageSvc = Service("MessageSvc")
ServiceMgr.MessageSvc.OutputLevel = ERROR

#
# Define an input file. To know which variables the D3PDObject-s would've
# created, we have to use an actual input file...
#
if not "InputFiles" in dir():
    InputFiles = [ "AOD.pool.root" ]

# We need to process exactly one event:
EvtMax = 1

# Set up the needed RecEx flags:
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.FilesInput.set_Value_and_Lock( InputFiles )
athenaCommonFlags.EvtMax.set_Value_and_Lock( EvtMax )

# Set up what the RecExCommon job should and shouldn't do:
from RecExConfig.RecFlags import rec
rec.AutoConfiguration = [ "everything" ]
rec.doCBNT.set_Value_and_Lock( False )
rec.doWriteESD.set_Value_and_Lock( False )
rec.doWriteAOD.set_Value_and_Lock( False )
rec.doWriteTAG.set_Value_and_Lock( False )
rec.doESD.set_Value_and_Lock( False )
rec.doAOD.set_Value_and_Lock( False )
rec.doDPD.set_Value_and_Lock( False )
rec.doHist.set_Value_and_Lock( False )
rec.doPerfMon.set_Value_and_Lock( False )
rec.doForwardDet.set_Value_and_Lock( False )
rec.doEdmMonitor.set_Value_and_Lock( False )

# Let RecExCommon set everything up:
include( "RecExCommon/RecExCommon_topOptions.py" )

#########################################################################
#                                                                       #
#                     Now set generate the sources                      #
#                                                                       #
#########################################################################

import D3PDMakerReader
from D3PDMakerA4.D3PDMakerA4Conf import D3PD__A4D3PDSvc
service = D3PD__A4D3PDSvc("A4ReaderD3PDSvc")
# Set the dumper into proto dumping mode
service.FormatDumping = True
#service = D3PDMakerReader.RootReaderD3PDSvc( "RootReaderD3PDSvc" )
#service.Version = 2
#service.OutputLevel = 1
ServiceMgr += service

d3pdalg = D3PDMakerReader.MultiReaderAlg("ReaderAlg", Directory = ".",
                                         TuplePath = "Test",
                                         D3PDSvc = service)

old_iadd = D3PDMakerReader.MultiReaderAlg.__iadd__
def monkeypatch__iadd__(self, config):
    """
    A fixup required to MultiReaderAlg to prevent a mismatch between its data structures and what Gaudi's PrivateToolHandleArray
    
    1) Loop over all configs
    2) Don't try to insert configs which are already in the tools list
    """
    if type(config) != type([]):
        config = [config]
        
    for c in config:
        # if c.ObjectName is empty then it gets called a D3PDObjectN
        try:
            
            self.Tools[c.getName()]
        except IndexError:
            # References
            # http://alxr.usatlas.bnl.gov/lxr/source/Gaudi/GaudiKernel/python/GaudiKernel/GaudiHandles.py#182
            # It's not yet in there, so add it.
            old_iadd(self, c)
    return self
        
D3PDMakerReader.MultiReaderAlg.__iadd__ = monkeypatch__iadd__

from PhotonD3PDMaker.PhotonD3PD import PhotonD3PD
PhotonD3PD(d3pdalg)

#for name, tool in zip(d3pdalg.ClassNames, d3pdalg.Tools):
#    print name, tool.getName()

#raise
