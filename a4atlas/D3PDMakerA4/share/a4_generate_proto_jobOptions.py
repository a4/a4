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

from D3PDMakerA4.A4DumpAlg import A4DumpAlg
from D3PDMakerA4.D3PDMakerA4Conf import D3PD__A4D3PDSvc
service = D3PD__A4D3PDSvc(
    "A4ReaderD3PDSvc",
    Mode="A4ProtoDumpD3PD",
    OutputLevel=INFO,
    #Mode="A4DumpD3PD", # Generate A4 itself (not yet implemented)
)
ServiceMgr += service

d3pdalg = A4DumpAlg("A4DumpAlg", Directory="./", TuplePath="./", D3PDSvc=service)

from PhotonD3PDMaker.PhotonD3PD import PhotonD3PD
PhotonD3PD(d3pdalg)

