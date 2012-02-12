#include <iostream>

#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <TFile.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>

#include <a4/application.h>
#include <a4/histogram.h>
#include <a4/cutflow.h>
#include <a4/results_processor.h>

using namespace std;
using namespace boost::filesystem;
using namespace a4::process;
using namespace a4::hist;
using namespace a4::io;

TDirectory* mkdirs(TFile &f, string file) {

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep("/");

    path rpath(file);

    string dir = "";
    string prevdir = "";

    string path = rpath.parent_path().string();

    tokenizer tokens(path, sep);
    bool first = true;
    BOOST_FOREACH(string tok, tokens) {
        if (first) {
            dir = tok;
            if (!f.Get(dir.c_str())) f.mkdir(dir.c_str());
            prevdir = dir;
            first = false;
        } else {
            dir += "/" + tok;
            if (!f.Get(dir.c_str())) {
                ((TDirectory*)f.Get(prevdir.c_str()))->mkdir(tok.c_str());
            }
            prevdir += "/" + tok;
        }
    }
    const char* pth = dir.c_str();
    if (rpath.parent_path().string().empty()) {
        f.cd();
        return static_cast<TDirectory*>(&f);
    }
    f.cd(pth);
    return static_cast<TDirectory*>(f.Get(pth));
}

class A2RProcessor : public ResultsProcessor<A2RProcessor, a4::io::NoProtoClass, H1, H2, H3, Cutflow> {
  public:
    virtual ~A2RProcessor() {f->Close();};

    void a4_axis_to_root(const Axis& lhs, TAxis* rhs) {
        rhs->SetTitle(lhs.label.c_str());
        if (lhs.variable()) {
            double* edges = new double[lhs.bins()+1];
            for (uint32_t i = 0; i < lhs.bins()+1; i++)
                edges[i] = lhs.bin_edge(i);
            rhs->Set(lhs.bins(), edges);
            delete [] edges;
        } else {
            rhs->Set(lhs.bins(), lhs.min(), lhs.max());
        }
    }

    template<class H>
    void a4_hist_to_root(const std::string& name, H& h, TH1* root_hist) {
        if (weight != 1.0) h.__mul__(weight);
        uint32_t total_bins = 1;
        
        switch (root_hist->GetDimension()) {
            // Fallthrough
            case 3: a4_axis_to_root(h.axis(2), root_hist->GetZaxis()); total_bins *= h.axis(2).bins() + 2;
            case 2: a4_axis_to_root(h.axis(1), root_hist->GetYaxis()); total_bins *= h.axis(1).bins() + 2;
            case 1: a4_axis_to_root(h.axis(0), root_hist->GetXaxis()); total_bins *= h.axis(0).bins() + 2;
                break;
            default:
                FATAL("Unexpect histogram dimension! "
                    "(expect 1-3, got ", root_hist->GetDimension(), "!)");
        }
        
        path rpath(name);
        root_hist->SetName(rpath.leaf().c_str());
        root_hist->SetTitle(h.title.c_str());
        
        root_hist->Rebuild();
        
        for(uint32_t bin = 0; bin < total_bins; ++bin)
            root_hist->SetBinContent(bin, *(h.data().get() + bin));
            
        if (h.weights_squared()) {
            root_hist->Sumw2();
            for(uint32_t bin = 0; bin < total_bins; ++bin)
                root_hist->SetBinError(bin, sqrt(*(h.weights_squared().get() + bin)));
        }
        
        root_hist->SetEntries(h.entries());
        root_hist->SetDirectory(mkdirs(*f, name));
        root_hist->Write();
        delete root_hist;
    }

    void process(std::string name, shared<H1> h) { a4_hist_to_root(name, *h, new TH1D()); }
    void process(std::string name, shared<H2> h) { a4_hist_to_root(name, *h, new TH2D()); }
    void process(std::string name, shared<H3> h) { a4_hist_to_root(name, *h, new TH3D()); }
    
    void process(std::string name, shared<Cutflow> h) {
        if (weight != 1.0) h->__mul__(weight);
        mkdirs(*f, name);
        path rpath(name);

        std::vector<Cutflow::CutNameCount> content = h->content();
        TH1D * cf = new TH1D(rpath.leaf().c_str(), h->title.c_str(), content.size(), 0, content.size());
        cf->Sumw2();
        int i = 0;
        for (vector<Cutflow::CutNameCount>::const_iterator it = content.begin(); it != content.end(); it++) {
            i++;
            cf->SetBinContent(i, it->count);
            cf->SetBinError(i, sqrt(it->weights_squared));
            string * n = new string(it->name);
            cf->GetXaxis()->SetBinLabel(i, n->c_str());
        }
        cf->SetDirectory((TDirectory*)f->Get(rpath.parent_path().string().c_str()));
        cf->Write();
    }

    std::string filename;
    shared<TFile> f;
    double weight;
};

class A2RConfig : public ConfigurationOf<A2RProcessor> {
    public:
        A2RConfig() : processor_count(0) {};

        virtual ~A2RConfig() {
            //foreach(string f, root_files) {}
        };

        void add_options(po::options_description_easy_init opt) {
            opt("weight,w", po::value(&weight)->default_value(1.0), "Multiplicative weight");
            opt("root-file,R", po::value(&filename), "ROOT output file");
            opt("compression-level,C", po::value(&compression_level)->default_value(1), "compression level");
        };

        virtual void setup_processor(A2RProcessor &g) {
            assert(processor_count++ == 0);
            g.filename = filename;
            //g.filename = str_cat(filename, "_", processor_count++);
            root_files.push_back(g.filename);
            g.f.reset(new TFile(filename.c_str(), "RECREATE"));
            g.f->SetCompressionLevel(compression_level);
            g.weight = weight;
        }

        std::string filename;
        int compression_level;
        std::vector<std::string> root_files;
        int processor_count;
        double weight;
};

int main(int argc, const char * argv[]) {
    return a4_main_configuration<A2RConfig>(argc, argv);
};
