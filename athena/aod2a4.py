from logging import getLogger; log = getLogger("a4")

from a4.messages import Atlas, EventStreamInfo, RunInfo

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


class AOD2A4Base(PyAthena.Alg):
    def __init__(self, name, year, options = {}):
        super(AOD2A4Base,self).__init__(name)
        self.year = year
        self.options = options
        self.file_name = options.get("file_name", "events.a4")
        self.is_mc = None
        self.event_info_key = None

        self.possible_streams = None
        self.runs_encountered = dict()
        self.runs_encountered_w = dict()
        self.lbs_encountered = dict()
        
    def initialize(self):
        log.info("Initialize AOD2A4")
        self.sg = PyAthena.py_svc("StoreGateSvc")
        self.sum_mc_event_weights = 0.0
        self.number_events = 0

        self.init()
        return PyAthena.StatusCode.Success

    def load_event_info(self, event):
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

        event.event_number = self.event_info.event_ID().event_number()
        event.run_number = self.event_info.event_ID().run_number()
        event.lumi_block = self.event_info.event_ID().lumi_block()
        event.bunch_crossing_id = self.event_info.event_ID().bunch_crossing_id()
        event.error_state_lar = self.event_info.errorState(self.event_info.LAr)

        if not event.run_number in self.runs_encountered:
            self.runs_encountered[event.run_number] = 0
            self.runs_encountered_w[event.run_number] = 0
            self.lbs_encountered[event.run_number] = set()

        if not self.is_mc:
            self.lbs_encountered[event.run_number].add(event.lumi_block)

            s_tags = [t.name() for t in self.event_info.trigger_info().streamTags()]
            if self.possible_streams is None:
                self.possible_streams = set(s_tags)
            elif len(self.possible_streams) > 0:
                self.possible_streams = set(s for s in self.possible_streams if s in s_tags)

            event.stream_tag.extend(
                getattr(Atlas.EventStreamInfo_pb2, name) for name in list(s_tags) 
                if hasattr(Atlas.EventStreamInfo_pb2, name)
            )


        event_weight = 1.0
        if self.is_mc:

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
            event.mc_event_weight = event_weight
            self.sum_mc_event_weights += event_weight

        self.number_events += 1
        self.runs_encountered[event.run_number] += 1
        self.runs_encountered_w[event.run_number] += event_weight

    def finalize(self):
        log.info("Finalizing AOD2A4")
        meta = EventStreamInfo()
        total_events = 0
        for run in sorted(self.runs_encountered.keys()):
            ri = meta.run_info.add()
            ri.run_number = run
            ri.event_count = self.runs_encountered[run]
            ri.sum_mc_weights = self.runs_encountered_w[run]
            ri.lumi_blocks.extend(sorted(self.lbs_encountered[run]))
            if self.possible_streams:
                streams = []
                for s in sorted(self.possible_streams):
                    if hasattr(Atlas.EventStreamInfo_pb2, s):
                        streams.append(getattr(Atlas.EventStreamInfo_pb2, s))
                ri.stream.extend(streams)
            total_events += self.runs_encountered[run]
        meta.total_events = total_events
        meta.simulation = self.is_mc
        self.a4.close(meta)
        return PyAthena.StatusCode.Success
