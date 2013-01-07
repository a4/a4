
from logging import getLogger; log = getLogger("a4")

from a4.atlas.Physics_pb2 import MissingEnergy
from a4.atlas.EventMetaData_pb2 import EventMetaData
from a4.atlas.EventStreamInfo_pb2 import EventStreamInfo, RunInfo
from a4.atlas import EventStreamInfo_pb2

from AthenaPython import PyAthena

data10_7TeV = {
"A1" : [152166, 152214, 152220, 152221, 152345, 152409, 152441, 152508, 152777,
  152844, 152845, 152878, 152933, 152994, 153030, 153134, 153136, 153159, 153200],
"B1" : [153565, 153599, 154810, 154813, 154815, 154817],
"B2" : [154822, 155073, 155112, 155116, 155118, 155160],
"C1" : [155228, 155280, 155569, 155634, 155669, 155678, 155697],
"C2" : [156682],
"D1" : [158045, 158116, 158269, 158299, 158392],
"D2" : [158443, 158466, 158545, 158548, 158549, 158582],
"D3" : [158632, 158801, 158975],
"D4" : [159041, 159086],
"D5" : [159113],
"D6" : [159179, 159202, 159203, 159224],
"E1" : [160387, 160472, 160479],
"E2" : [160530],
"E3" : [160613, 160736, 160800, 160801, 160879],
"E4" : [160899, 160953, 160954, 160958, 160963, 160975, 160980],
"E5" : [161118, 161379],
"E6" : [161407, 161520],
"E7" : [161562, 161948],
"F1" : [162347, 162526, 162576, 162577],
"F2" : [162620, 162623, 162690, 162764, 162843, 162882],
"G1" : [165591, 165632],
"G2" : [165703, 165732],
"G3" : [165767, 165815],
"G4" : [165817, 165818],
"G5" : [165821, 165954, 165956, 166094, 166097, 166142, 166143],
"G6" : [166198, 166305, 166383],
"H1" : [166466, 166658, 166786, 166850],
"H2" : [166856, 166924, 166925, 166927, 166964],
"I1" : [167575, 167576, 167607, 167661, 167680],
"I2" : [167776, 167844],
}

data11_7TeV = {
"A1" : [177531, 177539, 177540, 177593, 177682],
"A2" : [177904, 177911, 177912, 177924, 177929, 177960, 177965],
"B1" : [177986, 178020, 178021, 178026],
"B2" : [178044, 178047, 178109],
"D1" : [179710, 179725, 179739],
"D2" : [179771, 179804],
"D3" : [179938, 179939, 179940, 180122, 180124, 180139, 180144],
"D4" : [180149, 180153, 180164, 180212],
"D5" : [180225, 180241, 180242],
"D6" : [180309, 180400, 180448],
"D7" : [180481],
"E1" : [180614, 180636, 180664, 180710, 180776],
"F1" : [182013, 182032, 182034],
"F2" : [182161, 182284, 182346, 182372, 182424, 182449, 182450, 182454, 182455,
  182456, 182486],
"F3" : [182516, 182518, 182519],
"G1" : [182726],
"G2" : [182747, 182766, 182787, 182796, 182879, 182886],
"G3" : [182997, 183003, 183021],
"G4" : [183038, 183045, 183054, 183078, 183079, 183081, 183127, 183129, 183130],
"G5" : [183216, 183272, 183286, 183347],
"G6" : [183391, 183407, 183412, 183426, 183462],
"H1" : [183544, 183580, 183581, 183602],
"H2" : [183780],
"H3" : [183963, 184022, 184066, 184072],
"H4" : [184074, 184088, 184130, 184169],
"I1" : [185353, 185518, 185536, 185644, 185649],
"I2" : [185731, 185747, 185761, 185823, 185856, 185976],
"I3" : [185998, 186049, 186156, 186169, 186178, 186179, 186180, 186182],
"I4" : [186216, 186217, 186275, 186361, 186396, 186399, 186456, 186493],
"J1" : [186516, 186532, 186533, 186669, 186673],
"J2" : [186721, 186729, 186753, 186755],
"K1" : [186873, 186877, 186878, 186923, 186933, 186934],
"K2" : [186965, 187014, 187196, 187219],
"K3" : [187453, 187457, 187501, 187543, 187552],
"K4" : [187763],
"K5" : [187811, 187812],
"K6" : [187815],
"L1" : [188902, 188903, 188904, 188908, 188909, 188910],
"L2" : [188921, 188949, 188951, 189011, 189027, 189028, 189049, 189079, 189090],
"L3" : [189184, 189205, 189207, 189242, 189280, 189288, 189366, 189372, 189421,
  189425],
"L4" : [189481, 189483, 189530, 189536, 189561, 189598, 189602, 189610],
"L5" : [189639, 189655, 189660, 189692, 189693, 189719, 189751, 189774, 189781,
  189813],
"L6" : [189822, 189836, 189845, 189875, 189963, 189965, 190046, 190116, 190119,
  190120],
"L7" : [190236, 190256, 190295, 190297, 190300, 190343],
"M1" : [190503, 190504, 190505],
"M10" : [191635, 191676, 191715, 191920, 191933],
"M2" : [190608, 190611, 190617, 190618, 190643, 190644, 190661, 190689],
"M3" : [190728],
"M4" : [190872, 190878, 190933, 190934, 190975, 191138, 191139],
"M5" : [191149, 191150, 191190],
"M6" : [191217, 191218, 191235, 191239],
"M7" : [191272, 191334, 191341, 191342, 191343, 191355, 191358, 191373, 191376,
  191381],
"M8" : [191425, 191426, 191428, 191513, 191514, 191517],
"M9" : [191628],
}

data11_7TeV_period = {}

for p, rns in data11_7TeV.iteritems():
    for r in rns:
        data11_7TeV_period[r] = p


data12_8TeV = {
"A6"  : [201383, 201351],
"A1"  : [200804],
"A3"  : [200863, 200842, 200913],
"A4"  : [201006, 201191, 201190, 201052, 201138, 200967, 200965,
         200987, 201120, 200982, 200926, 201113],
"A5"  : [201269, 201289, 201280, 201257],
"A6"  : [201351, 201383],
"A7"  : [201494, 201489],
"A8"  : [201555, 201556],

'B1'  : [202712, 202798, 202668, 202740, 202660],
'B2'  : [203027, 202991, 202987, 202965],
'B3'  : [203191, 203195, 203169],
'B4'  : [203456, 203524, 203523, 203335, 203258, 203454, 203228, 
         203432, 203336, 203353, 203277, 203256],
'B5'  : [203602, 203680, 203636, 203605],
'B6'  : [203760, 203745, 203739, 203719, 203792, 203779],
'B7'  : [203875, 203876],
'B8'  : [204071, 204026, 203934, 204073, 204025],
'B9'  : [204153, 204158, 204134],
'B10' : [204265, 204240, 204442, 204416],
'B11' : [204668, 204633, 204564, 204474],
'B12' : [204932, 204910, 204726, 204707, 205016, 205017, 205010,
         204796, 204954, 204955, 204857, 204976, 204769, 204853,
         204772, 204763],
'B13' : [205055, 205071],
'B14' : [205113, 205112],
'C2'  : [206299, 206367, 206368, 206369, 206409, 206497],
'C3'  : [206564, 206573, 206614],
'C6'  : [206955, 206962, 206971, 207044, 207046],
'C7'  : [207113],
'C8'  : [207221, 207262, 207304, 207306, 207332],
'C9'  : [207397],
'D1'  : [207447, 207490, 207528, 207531, 207532],
'D2'  : [207582, 207589, 207620, 207664, 207696, 207749, 207772, 207800, 207809, 207845, 207864, 207865, 207929, 207931],
'D3'  : [207934, 207975, 207982, 208123, 208126],
'D4'  : [208179, 208184, 208189],
'D5'  : [208258, 208261, 208354],
'D6'  : [208484, 208485],
'D7'  : [208631, 208642, 208662, 208705, 208717, 208720],
'D8'  : [208780, 208781, 208811, 208870, 208930, 208931, 208970, 208982, 209025],
'E1'  : [209074, 209084, 209109],
'E2'  : [209161, 209183, 209214, 209254, 209265, 209269, 209353, 209381],
'E3'  : [209550, 209580, 209608, 209628, 209629, 209736, 209776, 209787, 209812, 209864, 209866, 209899],
'E4'  : [209909, 209980, 209994, 209995],
'E5'  : [210302, 210308],
'G1'  : [211522],
'G2'  : [211620],
'G3'  : [211670],
'G4'  : [211697, 211772, 211787, 211867, 211902, 211937, 212000, 212034, 212103, 212142],
'G5'  : [212144, 212172, 212199, 212272],
'H'   : [212619, 212663, 212687, 212721, 212742, 212809, 212815, 212858, 212967,
         212993, 213039, 213079, 213092, 213130, 213155, 213157, 213204, 213250, 213268, 213314, 213359],
'I1'  : [213431],
'I2'  : [213479, 213480, 213486, 213539, 213627, 213640, 213684, 213695, 213702],
'I3'  : [213754, 213796, 213816, 213819],
'J1'  : [213900, 213951, 213964, 213968, 214021, 214086, 214160],
'J2'  : [214176, 214216],
'J3'  : [214388, 214390, 214494, 214523, 214544, 214553],
'J4'  : [214618, 214651, 214680, 214714, 214721, 214758, 214777],
'J6'  : [215027, 215061],
'J7'  : [215063],
'J8'  : [215091],
'L1'  : [215414, 215433, 215456, 215464, 215473, 215541],
'L2'  : [215559],
'L3'  : [215571, 215589, 215643],

}

data12_8TeV_period = {}

for p, rns in data12_8TeV.iteritems():
    for r in rns:
        data12_8TeV_period[r] = p


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

    # hopefully vor STVF MET
    #from METRefGetter_newplup import *
    #from QcdD3PDMaker.QcdD3PDMakerFlags import QcdD3PDMakerFlags

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
        #self.stvf_algo = make_METRefAlg(_suffix='_STVF')
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

        eid = self.event_info.event_ID()
        event.event_number = eid.event_number()
        event.run_number = eid.run_number()
        event.lumi_block = eid.lumi_block()
        event.bunch_crossing_id = eid.bunch_crossing_id()
        event.error_state_lar = self.event_info.errorState(self.event_info.LAr)
        event.error_state_tile = self.event_info.errorState(self.event_info.Tile)
        event.error_state_coreflag = self.event_info.errorState(self.event_info.Core)

        effective_run = event.run_number
        event_weight = 1.0
        event.average_interactions_per_crossing = self.event_info.averageInteractionsPerCrossing()
        if self.is_mc:
            event.actual_interactions_per_crossing = self.event_info.actualInteractionsPerCrossing()
            et = self.event_info.event_type()
            event.mc_channel_number = et.mc_channel_number()
            effective_run = event.mc_channel_number
            event_weight = et.mc_event_weight()
            event.mc_event_weight = event_weight
            self.sum_mc_event_weights += event_weight

        if not effective_run in self.runs_encountered:
            self.runs_encountered[effective_run] = 0
            self.runs_encountered_w[effective_run] = 0
            self.lbs_encountered[effective_run] = set()

        if not self.is_mc:
            self.lbs_encountered[event.run_number].add(event.lumi_block)

            s_tags = [t.name() for t in self.event_info.trigger_info().streamTags()]
            if self.possible_streams is None:
                self.possible_streams = set(s_tags)
            elif len(self.possible_streams) > 0:
                self.possible_streams = set(s for s in self.possible_streams if s in s_tags)

            event.stream_tag.extend(
                getattr(EventStreamInfo_pb2, name) for name in list(s_tags) 
                if hasattr(EventStreamInfo_pb2, name)
            )

        self.number_events += 1
        self.runs_encountered[effective_run] += 1
        self.runs_encountered_w[effective_run] += event_weight

    def finalize(self):
        log.info("Finalizing AOD2A4")
        meta = EventMetaData()
        meta.simulation = self.is_mc
        meta.run.extend(sorted(self.runs_encountered.keys()))

        total_events = 0
        sum_mc_weights = 0
        streams = set()
        for run in sorted(self.runs_encountered.keys()):
            sum_mc_weights += self.runs_encountered_w[run]
            for lb in sorted(self.lbs_encountered[run]):
                rlb = meta.lumiblock.add()
                rlb.run = run
                rlb.lumiblock = lb
            if self.possible_streams:
                for s in sorted(self.possible_streams):
                    if hasattr(EventStreamInfo_pb2, s):
                        streams.add(getattr(EventStreamInfo_pb2, s))
            total_events += self.runs_encountered[run]

        meta.stream.extend(streams)
        if not self.is_mc and self.year==2011:
            meta.period.extend(sorted(set(data11_7TeV_period[r] for r in sorted(self.runs_encountered.keys())))) 
        if not self.is_mc and self.year==2012:
            meta.period.extend(sorted(set(data12_8TeV_period[r] for r in sorted(self.runs_encountered.keys()))))
        meta.event_count = total_events
        meta.sum_mc_weights = sum_mc_weights

        self.a4.metadata(meta)
        self.a4.close()
        return PyAthena.StatusCode.Success
