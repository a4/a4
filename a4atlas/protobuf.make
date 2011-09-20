protodir=${localstatedir}/a4/proto/$(A4PACK)
protoincludedir=${includedir}/a4/proto/$(A4PACK)
protopythondir=${pythondir}/a4/proto/$(A4PACK)

PYDIR=python/a4/proto/$(A4PACK)
CPPDIR=src/a4/proto/$(A4PACK)

PROTOBUF_PY=$(PYDIR)/Histograms_pb2.py $(PYDIR)/Photon_pb2.py $(PYDIR)/Cutflow_pb2.py $(PYDIR)/Event_pb2.py $(PYDIR)/Jet_pb2.py $(PYDIR)/Electron_pb2.py $(PYDIR)/Physics_pb2.py $(PYDIR)/Muon_pb2.py $(PYDIR)/Results_pb2.py $(PYDIR)/Atlas/Isolation_pb2.py $(PYDIR)/Atlas/TrackHits_pb2.py $(PYDIR)/Atlas/DataQuality_pb2.py $(PYDIR)/Atlas/Trigger_pb2.py $(PYDIR)/Atlas/ShowerShape_pb2.py $(PYDIR)/Atlas/EventStreamInfo_pb2.py
PROTOBUF_H=$(CPPDIR)/Histograms.pb.h $(CPPDIR)/Photon.pb.h $(CPPDIR)/Cutflow.pb.h $(CPPDIR)/Event.pb.h $(CPPDIR)/Jet.pb.h $(CPPDIR)/Electron.pb.h $(CPPDIR)/Physics.pb.h $(CPPDIR)/Muon.pb.h $(CPPDIR)/Results.pb.h $(CPPDIR)/Atlas/Isolation.pb.h $(CPPDIR)/Atlas/TrackHits.pb.h $(CPPDIR)/Atlas/DataQuality.pb.h $(CPPDIR)/Atlas/Trigger.pb.h $(CPPDIR)/Atlas/ShowerShape.pb.h $(CPPDIR)/Atlas/EventStreamInfo.pb.h
PROTOBUF_CC=$(CPPDIR)/Histograms.pb.cc $(CPPDIR)/Photon.pb.cc $(CPPDIR)/Cutflow.pb.cc $(CPPDIR)/Event.pb.cc $(CPPDIR)/Jet.pb.cc $(CPPDIR)/Electron.pb.cc $(CPPDIR)/Physics.pb.cc $(CPPDIR)/Muon.pb.cc $(CPPDIR)/Results.pb.cc $(CPPDIR)/Atlas/Isolation.pb.cc $(CPPDIR)/Atlas/TrackHits.pb.cc $(CPPDIR)/Atlas/DataQuality.pb.cc $(CPPDIR)/Atlas/Trigger.pb.cc $(CPPDIR)/Atlas/ShowerShape.pb.cc $(CPPDIR)/Atlas/EventStreamInfo.pb.cc
PROTOBUF_PROTO=proto/Histograms.proto proto/Photon.proto proto/Cutflow.proto proto/Event.proto proto/Jet.proto proto/Electron.proto proto/Physics.proto proto/Muon.proto proto/Results.proto proto/Atlas/Isolation.proto proto/Atlas/TrackHits.proto proto/Atlas/DataQuality.proto proto/Atlas/Trigger.proto proto/Atlas/ShowerShape.proto proto/Atlas/EventStreamInfo.proto

protoAtlasdir=${protodir}/Atlas
protoincludeAtlasdir=${protoincludedir}/Atlas
protopythonAtlasdir=${protopythondir}/Atlas

dist_proto_DATA=proto/Muon.proto proto/Jet.proto proto/Histograms.proto proto/Photon.proto proto/Results.proto proto/Cutflow.proto proto/Electron.proto proto/Physics.proto proto/Event.proto
protoinclude_HEADERS=$(CPPDIR)/Muon.pb.h $(CPPDIR)/Jet.pb.h $(CPPDIR)/Histograms.pb.h $(CPPDIR)/Photon.pb.h $(CPPDIR)/Results.pb.h $(CPPDIR)/Cutflow.pb.h $(CPPDIR)/Electron.pb.h $(CPPDIR)/Physics.pb.h $(CPPDIR)/Event.pb.h
protopython_PYTHON=$(PYDIR)/Muon_pb2.py $(PYDIR)/Jet_pb2.py $(PYDIR)/Histograms_pb2.py $(PYDIR)/Photon_pb2.py $(PYDIR)/Results_pb2.py $(PYDIR)/Cutflow_pb2.py $(PYDIR)/Electron_pb2.py $(PYDIR)/Physics_pb2.py $(PYDIR)/Event_pb2.py $(PYDIR)//__init__.py

dist_protoAtlas_DATA=proto/Atlas/Isolation.proto proto/Atlas/ShowerShape.proto proto/Atlas/TrackHits.proto proto/Atlas/Trigger.proto proto/Atlas/EventStreamInfo.proto proto/Atlas/DataQuality.proto
protoincludeAtlas_HEADERS=$(CPPDIR)/Atlas/Isolation.pb.h $(CPPDIR)/Atlas/ShowerShape.pb.h $(CPPDIR)/Atlas/TrackHits.pb.h $(CPPDIR)/Atlas/Trigger.pb.h $(CPPDIR)/Atlas/EventStreamInfo.pb.h $(CPPDIR)/Atlas/DataQuality.pb.h
protopythonAtlas_PYTHON=$(PYDIR)/Atlas/Isolation_pb2.py $(PYDIR)/Atlas/ShowerShape_pb2.py $(PYDIR)/Atlas/TrackHits_pb2.py $(PYDIR)/Atlas/Trigger_pb2.py $(PYDIR)/Atlas/EventStreamInfo_pb2.py $(PYDIR)/Atlas/DataQuality_pb2.py $(PYDIR)/Atlas/__init__.py


# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: ${top_srcdir}/proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOBUF_PROTOC} -I=${top_srcdir}/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho 'class [A-Za-z0-9]*' $^ | sed 's/.py:class/ import/' | sed "s/python\/a4\/proto\/$(A4PACK)\//from ./" | sed 's/\//./g' > $@

# make sure all protobuf are generated before they are built!
$(PROTOBUF_CPP): $(PROTOBUF_H)


