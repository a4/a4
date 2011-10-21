#include <iostream>

#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <TFile.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2D.h>

#include <a4/application.h>
#include <a4/histogram.h>
#include <a4/cutflow.h>
#include <a4/results_processor.h>

using namespace std;
using namespace boost::filesystem;
using namespace a4::process;
using namespace a4::hist;
using namespace a4::io;

void mkdirs(TFile &f, string file) {

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
    f.cd(rpath.parent_path().string().c_str());
}

class A2RProcessor : public ResultsProcessor<A2RProcessor, a4::io::NoProtoClass, H1, H2, Cutflow> {
  public:
    virtual ~A2RProcessor() {f->Close();};

    void process(std::string name, H1 & h) {
        mkdirs(*f, name);
        path rpath(name);

        TH1D * h1 = new TH1D(name.c_str(), h.title.c_str(), h.x().bins(), h.x().min(), h.x().max());
        h1->GetXaxis()->SetTitle(h.x().label.c_str());
        for(uint32_t bin = 0, bins = h.x().bins() + 2; bins > bin; ++bin)
            h1->SetBinContent(bin, *(h.data().get() + bin));
        if (h.weights_squared()) {
            h1->Sumw2();
            for(uint32_t bin = 0, bins = h.x().bins() + 2; bins > bin; ++bin)
                h1->SetBinError(bin, sqrt(*(h.weights_squared().get() + bin)));
        }
        h1->SetEntries(h.entries());
        h1->SetDirectory((TDirectory*)f->Get(rpath.parent_path().string().c_str()));
        h1->Write();
    }
    void process(std::string name, H2 & h) {
        mkdirs(*f, name);
        path rpath(name);

        TH2D * h2 = new TH2D(name.c_str(), h.title.c_str(), h.x().bins(), h.x().min(), h.x().max(),  h.y().bins(), h.y().min(), h.y().max());
        h2->GetXaxis()->SetTitle(h.x().label.c_str());
        h2->GetYaxis()->SetTitle(h.y().label.c_str());
        const int skip = h.x().bins() + 2;
        for(uint32_t bin = 0, bins = h.x().bins() + 2; bins > bin; ++bin)
            for(uint32_t ybin = 0, ybins = h.y().bins() + 2; ybins > ybin; ++ybin)
                h2->SetBinContent(bin, ybin, *(h.data().get() + ybin*skip + bin));
        if (h.weights_squared()) {
            h2->Sumw2();
            for(uint32_t bin = 0, bins = h.x().bins() + 2; bins > bin; ++bin)
                for(uint32_t ybin = 0, ybins = h.y().bins() + 2; ybins > ybin; ++ybin)
                    h2->SetBinError(bin, ybin, sqrt(*(h.weights_squared().get() + ybin*skip + bin)));
        }
        h2->SetEntries(h.entries());
        h2->SetDirectory((TDirectory*)f->Get(rpath.parent_path().string().c_str()));
        h2->Write();

    }
    void process(std::string name, Cutflow & h) {
        mkdirs(*f, name);
        path rpath(name);

        std::vector<Cutflow::CutNameCount> content = h.content();
        TH1D * cf = new TH1D(name.c_str(), h.title.c_str(), content.size(), 0, content.size());
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
};

class A2RConfig : public ConfigurationOf<A2RProcessor> {
    public:
        A2RConfig() : processor_count(0) {};

        virtual ~A2RConfig() {
            //foreach(string f, root_files) {}
        };

        virtual po::options_description get_options() { 
            po::options_description opt; 
            opt.add_options()("root-file,R", po::value<string>(&filename), "ROOT output file");
            return opt;
        };

        virtual void setup_processor(A2RProcessor &g) {
            assert(processor_count++ == 0);
            g.filename = filename;
            //g.filename = str_cat(filename, "_", processor_count++);
            root_files.push_back(g.filename);
            g.f.reset(new TFile(filename.c_str(), "RECREATE"));
        }

        std::string filename;
        std::vector<std::string> root_files;
        int processor_count;
};

int main(int argc, const char * argv[]) {
    return a4_main_configuration<A2RConfig>(argc, argv);
};
