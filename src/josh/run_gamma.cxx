#include "run_gamma.h"
#include "run_fastjet.h"
#include "fastjet/ClusterSequence.hh"

#include <util/pyargs.h>
#include <util/pyutil.h>
#include <util/looputil.h>
#include <Pythia8/Pythia.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>
#include <TNtuple.h>
#include <string>
#include <iostream>
#include <cmath> // std::abs
#include "Sample_Pythia.h"

using namespace std;
using namespace fastjet;

int run_gamma (const std::string &s)
{
        // test(s); return;
        PyUtil::Args args(s);
        cout << args.asString("[pythia_run_wrapper:status]") << endl;
        if (args.isSet("--dry")) return 0;

        // create the output root file
        string outfname = args.get("--output");
        if (outfname.size() < 1)
        {
            outfname = "default_output.root";
        }
        
        TFile *f1 = TFile::Open("/global/homes/j/jfran/test/output/atlas_data.root");
        f1->cd("Table 1");
        TH1F *gamma_prompt = (TH1F*)gDirectory->Get("Hist1D_y1");
        gamma_prompt->Reset();
        gamma_prompt->ResetStats();
        gamma_prompt->Sumw2();

        TFile *fout = TFile::Open(outfname.c_str(), "RECREATE");
        fout->cd();

        TH1F *norm = new TH1F("norm", " ", 3, 0,3);
        TNtuple *photon = new TNtuple("photon", "photon", "pt:eta:phi");
        TNtuple *weighted_photon = new TNtuple("weighted_photon", "weighted_photon", "pt:eta:phi");

        // initialize pythia with a config and command line args
        Pythia8::Pythia *ppythia = PyUtil::make_pythia(args.asString());
        Pythia8::Pythia &pythia  = *ppythia;
        auto &event              = pythia.event;

        // this is where the event loop section starts
        auto nEv = args.getI("Main:numberOfEvents");
        LoopUtil::TPbar pbar(nEv);

        // Sample_Pythia sampling;
        // sampling.set_sigma(s, 1000);
        // double sigma = sampling.get_sigma();

        for (unsigned int iE = 0; iE < nEv; iE++)//loopin over events  (pp collision)
        {
            pbar.Update();
            if (pythia.next() == false) continue;

            // loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++) 
            {
                if(event[ip].isFinal() && event[ip].id() == 22 && std::abs(event[ip].eta()) < .47 )
                {
                    if(std::abs(event[ip].status()) != 91)
                    {
                        photon->Fill(event[ip].pT(), event[ip].eta(), event[ip].phi());
                        weighted_photon->Fill(event[ip].pT(), event[ip].eta(), event[ip].phi() );
                    }
                }

                if(event[ip].isFinal() && event[ip].id() == 22 && std::abs(event[ip].eta()) < 1.37 && event[ip].status() != 91)
                {
                        gamma_prompt->Fill(event[ip].pT());
                }

            }
        }

        gamma_prompt->Sumw2();
        weighted_photon->SetWeight(pythia.info.sigmaGen());
        norm->SetBinContent(1, pythia.info.sigmaGen());
        norm->SetBinContent(2, pythia.info.weightSum());

        pythia.stat();
        cout << "[i] Generation done." << endl;

        // remember to properly save/update and close the output file
        fout->Write();
        gamma_prompt->Write("gamma_prompt");
        fout->Close();
        delete fout;

        string xsec_outfname = outfname + ".txt";
        PyUtil::CrossSections(pythia, xsec_outfname.c_str());

        // delete the pythia
        delete ppythia;
        return 0;
}
