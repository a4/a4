from logging import getLogger; log = getLogger("a4")
from a4 import A4WriterStream
from a4.messages import Trigger, Isolation, TrackHits, MuonTrackHits
from a4.messages import Lepton, Photon, Jet, Event
from a4.messages import LorentzVector, Vertex, MissingEnergy

from AthenaPython import PyAthena

def athena_setup(input = None, max_events = None):
    # use closest DB replica
    #from AthenaCommon.AppMgr import ServiceMgr
    #from PoolSvc.PoolSvcConf import PoolSvc
    #from DBReplicaSvc.DBReplicaSvcConf import DBReplicaSvc
    #ServiceMgr+=PoolSvc(SortReplicas=True)
    #ServiceMgr+=DBReplicaSvc(UseCOOLSQLite=False)
    #ServiceMgr+=DBReplicaSvc(UseCOOLSQLite=True)

    # This import makes Athena read Pool files.
    import AthenaPoolCnvSvc.ReadAthenaPool

    # setup autoconfiguration to deal also with DAODs
    from RecExConfig.RecFlags import rec
    #rec.doApplyAODFix.set_Value_and_Lock(False) ## Uncomment for data from v15 skimmed with v16
    rec.readRDO.set_Value_and_Lock(False)
    rec.readESD.set_Value_and_Lock(False)
    rec.readAOD.set_Value_and_Lock(True)
    rec.doCBNT       = False
    rec.doWriteESD   = False
    rec.doWriteAOD   = False
    rec.doAOD        = False
    rec.doWriteTAG   = False 
    rec.doPerfMon    = False
    rec.doHist       = False
    rec.doTruth      = False
    rec.LoadGeometry = True
    rec.AutoConfiguration.set_Value_and_Lock(['ProjectName', 'RealOrSim', 'FieldAndGeo', 'BeamType', 'ConditionsTag', 'DoTruth', 'BeamEnergy', 'LumiFlags', 'TriggerStream'])

    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags

    if not input is None:
        # Set input
        athenaCommonFlags.FilesInput = input

    if not max_events is None:
        athenaCommonFlags.EvtMax = max_events


class AOD2A4(PyAthena.Alg):
    def __init__(self, name, options = {}):
        super(AnalysisAlgorithm,self).__init__(name)
        self.options = options
        self.file_name = options.get("file_name", "events.a4")
        self.is_mc = None
        self.event_info_key = None
        
    def initialize(self):
        log.info("Initialize AOD2A4")
        self.sg = PyAthena.py_svc("StoreGateSvc")
        self.sum_mc_event_weights = 0.0
        self.number_events = 0
        self.a4 = A4WriterStream(open(self.file_name, "w"), "Event", Event)

        return PyAthena.StatusCode.Success


    def load_event_info(self):
        if self.event_info_key is None:
            if self.sg.contains("EventInfo", "ByteStreamEventInfo"):
                # EventInfo in data
                self.event_info_key = "ByteStreamEventInfo"
                self.is_mc = False
            elif self.sg.contains("EventInfo", "MyEvent"):
                # EventInfo in pileup monte carlo
                self.event_info_key = "MyEvent"
                self.is_mc = True
            elif self.sg.contains("EventInfo", "McEventInfo"):
                # EventInfo in pileup monte carlo
                self.event_info_key = "McEventInfo"
                self.is_mc = True
            else:
                self.sg.dump()
                raise RuntimeError("EventInfo not found in StoreGate!") 
        self.event_info = self.sg[self.event_info_key]

        self.event.event_number = self.event_info.event_ID().event_number()
        self.event.run_number = self.event_info.event_ID().run_number()
        self.event.lumi_block = self.event_info.event_ID().lumi_block()

        if self.is_mc:
            event_weight = 1.0
            try:
                truth = self.sg["GEN_AOD"]
                try:
                    truth = truth[0]
                    weights = truth.weights()
                    event_weight = weights[0]
                except IndexError, x:
                    print("STRANGE: No MC Weight in this event - set to 1.0 (%s)" % x)
            except KeyError:
                self.sg.dump()
                raise RuntimeError("MC weight not found in StoreGate (GEN_AOD)!") 
            self.event.event_weight = event_weight
            self.sum_mc_event_weights += self.event_weight
        self.number_events += 1

    def finalize(self):
        log.info("Finalizing AOD2A4")
        self.histogram_manager.write_parameter("sum_mc_event_weights", self.sum_mc_event_weights)
        self.histogram_manager.write_parameter("initial_events", self.number_events)
        return PyAthena.StatusCode.Success
 
    def electrons(event):
        return list(event.sg["ElectronAODCollection"])

    def muons(event):
        return list(event.sg["%sMuonCollection" % event.muon_algo])

    def execute(event):
        # note that "self" is named "event" here for semantic reasons
        self.event = Event()
        event.load_event_info()
        
        return PyAthena.StatusCode.Success
