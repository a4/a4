# $Id$

# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )

from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.MessageSvc = Service("MessageSvc")
ServiceMgr.MessageSvc.OutputLevel = ERROR

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

#
# Define an input file
#
if not "InputFiles" in dir():
    InputFiles = [ "AOD.pool.root" ]

# EvtMax = 1

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

from A4Dumper.A4DumperAlg import A4DumperAlg
from A4Dumper.A4DumperConf import A4Dumper__A4DumperSvc

service = A4Dumper__A4DumperSvc(
    "A4DumperSvc",
    OutputLevel=INFO,
    #Mode="A4DumpD3PD", # Generate A4 itself (not yet implemented)
)
ServiceMgr += service

dumper = A4DumperAlg("A4DumperAlg", A4DumperSvc=service)
topSequence += [ dumper ]
