#include <iostream>
#include <fstream>

#include <a4/application.h>
#include <a4/histogram.h>
#include <a4/cutflow.h>
#include <a4/results_processor.h>
#include <a4/object_store.h>
#include <a4/atlas/Event.pb.h>

using namespace std;
using namespace boost::filesystem;
using namespace a4::process;
using namespace a4::io;
using namespace a4::hist;
using namespace a4::atlas;

class A4ReweightConfiguration;

class A4ReweightProcessor : public ResultsProcessor<A4ReweightProcessor, EventMetaData, H1, H2, H3, Cutflow> {
  public:
    void process(std::string name, shared<Storable> s) {
        if (S.get_slow<Storable>(name)) {
            throw a4::Fatal("Encountered the same histogram twice! Merge files before reweighting them!");
        } else {
            *s *= weight;
            S.set_slow(name, s); 
        }
    }
    void process_new_metadata();
    double weight;
};

class A4ReweightConfiguration : public ConfigurationOf<A4ReweightProcessor> {
  public:
    void setup_processor(A4ReweightProcessor &g) {
        g.weight = 0.0;
    }
    void add_options(po::options_description_easy_init opt) {
        opt("lumi,l", po::value(&lumi), "Luminosity in pb^-1");
        opt("xs,x", po::value(&xs_file), "Cross-Section file (Lines of the form <MC-ID> <XS [pb]>)");
    }
    void read_arguments(po::variables_map &arguments) {
        if (arguments.count("lumi") == 0) {
            throw a4::Fatal("No Luminosity specified!");
        }
        if (arguments.count("xs") == 0) {
            throw a4::Fatal("No cross-section file specified!");
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
                std::cerr << "Failed to read line: " << ln.str() << std::endl;
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
};

void A4ReweightProcessor::process_new_metadata() {
    auto config = my<A4ReweightConfiguration>();
    if (metadata().run_size() != 1) {
        throw a4::Fatal("Cannot reweight if runs have been merged!");
    }
    int run = metadata().run(0);
    if (!metadata().has_sum_mc_weights()) {
        throw a4::Fatal("Cannot reweight without sum of MC weights!");
    }
    if (metadata().reweight_lumi() != 0) {
        throw a4::Fatal("This set of histograms has already been reweighted!");
    }
    double sum_mcw = metadata().sum_mc_weights();
    double xs = 0;
    if (config->xs.find(run) ==  config->xs.end()) {
        std::cerr << "ERROR: run " << run << " not found in XS file - reweighting with 0!" << std::endl;
    } else {
        xs = config->xs.find(run)->second;
    }
    weight = xs * config->lumi / sum_mcw;
    metadata().set_sum_mc_weights(xs * config->lumi);
    metadata().set_reweight_lumi(config->lumi);
}

int main(int argc, const char * argv[]) {
    return a4_main_configuration<A4ReweightConfiguration>(argc, argv);
};
