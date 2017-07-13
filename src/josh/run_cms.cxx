#include "run_cms.h"
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
#include <string>
#include <iostream>
#include <cmath> // std::abs

using namespace std;
using namespace fastjet;

int run_cms (const std::string &s)
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
        TFile *fout = TFile::Open(outfname.c_str(), "RECREATE");
        fout->cd();

        TH1F *gammaPrompt = new TH1F("gammaPrompt", "p_{T} Distribution; #gamma p_{T} [GeV/#it{C}]; counts", 75, 0, 150);
        TH1F *decayGamma = new TH1F("decayGamma", "p_{T} Distribution; #gamma p_{T} [GeV/#it{C}]; counts", 75, 0, 150);
        TH1F *jet = new TH1F("jet", "p_{T} Distribution; jet p_{T} [GeV/#it{C}]; counts", 75, 0, 150);
        TH1F *norm = new TH1F("norm", " ", 3, 0,3);

        // initialize pythia with a config and command line args
        Pythia8::Pythia *ppythia = PyUtil::make_pythia(args.asString());
        Pythia8::Pythia &pythia  = *ppythia;
        auto &event              = pythia.event;

        // this is where the event loop section starts
        auto nEv = args.getI("Main:numberOfEvents");
        LoopUtil::TPbar pbar(nEv);

        

        for (unsigned int iE = 0; iE < nEv; iE++)//loopin over events  (pp collision)
        {
            pbar.Update();
            if (pythia.next() == false) continue;

            vector<fastjet::PseudoJet> input_particles;

            // loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++) 
            {
                
                
                
                if(event[ip].isFinal() && event[ip].id() == 22 && std::abs(event[ip].eta()) < 1.37 )
                {
                    if(event[ip].isFinal())
                    {
                        // This vecotr will only make sense if run with prompt photon on or else will collect all types of jets
                        input_particles.push_back(fastjet::PseudoJet(event[ip].px(),event[ip].py(),event[ip].pz(),event[ip].e()));
                    }
                    if(event[ip].status() != 91)
                    {
                        gammaPrompt->Fill(event[ip].pT());
                    }
                    if(event[ip].status() == 91)
                    {
                        decayGamma->Fill(event[ip].pT()); // This only makes sense if ran with hardAll = on
                    }
                }
            } // end partilce loop

            double R = 0.4;
            fastjet::JetDefinition jet_def(fastjet::antikt_algorithm, R);

            fastjet::ClusterSequence clust_seq(input_particles, jet_def);
            vector<fastjet::PseudoJet> inclusive_jets = sorted_by_pt(clust_seq.inclusive_jets(.15));

            for(int i =0; i < inclusive_jets.size(); i++ )
            {
                jet->Fill(inclusive_jets[i].pt());
            }

        } // end event loop

        norm->SetBinContent(1, pythia.info.sigmaGen());
        norm->SetBinContent(2, pythia.info.weightSum());

        event.list();

        pythia.stat();
        cout << "[i] Generation done." << endl;

        // remember to properly save/update and close the output file
        fout->Write();
        fout->Close();
        delete fout;

        string xsec_outfname = outfname + ".txt";
        PyUtil::CrossSections(pythia, xsec_outfname.c_str());

        // delete the pythia
        delete ppythia;
        return 0;
}
