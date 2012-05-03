#include <iostream>
#include <fstream>

#include <a4/application.h>
#include <a4/histogram.h>
#include <a4/cutflow.h>
#include <a4/results_processor.h>
#include <a4/object_store.h>

#include <a4/atlas/Event.pb.h>
#include <a4/atlas/EventMetaData.pb.h>

using namespace std;
using namespace a4::process;
using namespace a4::io;
using namespace a4::hist;
using namespace a4::atlas;

class A4ReweightConfiguration;

class A4ReweightProcessor : public ResultsProcessor<A4ReweightProcessor, EventMetaData, H1, H2, H3, Cutflow> {
  public:
    void process(std::string name, shared<Storable> s) {
        if (S.get_slow<Storable>(name)) {
            FATAL("Encountered the same histogram twice! Merge files before reweighting them!");
        } else {
            *s *= weight;
            S.set_slow(name, s); 
        }
    }
    shared<A4Message> process_new_metadata();
    double weight;
};

class A4ReweightConfiguration : public ConfigurationOf<A4ReweightProcessor> {
  public:
    void setup_processor(A4ReweightProcessor& g) {
        g.weight = 0.0;
    }
    void add_options(po::options_description_easy_init opt) {
        opt("lumi,l", po::value(&lumi), "Luminosity in pb^-1");
        opt("xs,x", po::value(&xs_file), "Cross-Section file (Lines of the form <MC-ID> <XS [pb]>)");
        opt("force,F", po::bool_switch(&force)->default_value(false), "Reweight even if metadata().simulation() is not set");
    }
    void read_arguments(po::variables_map& arguments) {
        if (arguments.count("lumi") == 0) {
            FATAL("No Luminosity specified!");
        }
        if (arguments.count("xs") == 0) {
            FATAL("No cross-section file specified!");
        }
        std::ifstream xsf(xs_file);
        while(!xsf.eof()) {
            std::string _ln;
            std::getline(xsf, _ln);
            if (_ln.size() < 2) continue;
            if (_ln[0] == '#') continue;
            std::stringstream ln(_ln);
            int run; double w, k, filt;
            ln >> std::skipws;
            ln >> run >> w;
            if (ln.fail()) {
                WARNING("Failed to read line: ", ln.str());
                continue;
            }
            ln >> k >> filt;
            if (!ln.fail()) w *= k*filt;
            xs[run] = w;
        }
    }
    std::string xs_file;
    std::map<int, double> xs;
    double lumi;
    bool force;
};

shared<A4Message> A4ReweightProcessor::process_new_metadata() {
    auto config = my<A4ReweightConfiguration>();
    if (not metadata().simulation() && not config->force) {
        weight = 1;
        return shared<A4Message>();
    }
    if (metadata().mc_channel_size() != 1) {
        FATAL("Cannot reweight if mc_channels have been merged!");
    }
    int run = metadata().mc_channel(0);
    if (metadata().reweight_lumi() != 0) {
        FATAL("This set of histograms has already been reweighted!");
    }
    double sum_mcw = 0;
    if (!metadata().has_sum_mc_weights()) {
        if (!metadata().has_event_count()) {
            FATAL("Cannot reweight without sum of MC weights or event count!");
        } else {
            VERBOSE("sum_mc_weights not set, falling back to event_count");
            sum_mcw = metadata().event_count();
        }
    } else {
        sum_mcw = metadata().sum_mc_weights();
    }
    double xs = 0;
    if (config->xs.find(run) ==  config->xs.end()) {
        ERROR("run ", run, " not found in XS file - reweighting with 0!");
    } else {
        xs = config->xs.find(run)->second;
    }
    weight = xs * config->lumi / sum_mcw;
    auto md = metadata();
    md.set_sum_mc_weights(xs * config->lumi);
    md.set_reweight_lumi(config->lumi);
    return shared<A4Message>(new A4Message(md));
}

int main(int argc, const char* argv[]) {
    return a4_main_configuration<A4ReweightConfiguration>(argc, argv);
}
