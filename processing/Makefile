GRL_XML   = $(wildcard grl/*.xml)
GRL_GRL   = $(patsubst %.xml,%.grl,$(GRL_XML))
GRL_EGAM  = $(patsubst grl/%.xml,datasets/data/%.physics_Egamma.datasets,$(GRL_XML))
GRL_MUON  = $(patsubst grl/%.xml,datasets/data/%.physics_Muons.datasets,$(GRL_XML))

MC_IDS    = $(wildcard datasets/mc/*.ids)
MC_DSS    = $(patsubst %.ids,%.datasets,$(MC_IDS))

MC10b = r2302_r2300
MC_TAG = $(MC10b)

all: $(GRL_DSS) $(GRL_EGAM) $(GRL_MUON) datasets/data.datasets datasets/mc.datasets

%.grl: %.xml
	xml2grl $^ $@

datasets/data.datasets: $(GRL_EGAM) $(GRL_MUON)
	cat $^ > $@

datasets/mc.datasets: $(MC_DSS)
	cat $^ > $@

%.datasets: %.ids
	ids2datasets $^ $(MC_TAG) | tee $@

datasets/data/%.physics_Egamma.datasets: grl/%.grl
	grl2datasets $^ physics_Egamma | tee $@

datasets/data/%.physics_Muons.datasets: grl/%.grl
	grl2datasets $^ physics_Muons | tee $@



