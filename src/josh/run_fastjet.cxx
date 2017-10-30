#include "run_fastjet.h"
#include <util/pyargs.h>
#include <util/pyutil.h>
#include <util/looputil.h>
#include <Pythia8/Pythia.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <string>
#include <iostream>
#include <cmath> 
#include "fastjet/ClusterSequence.hh"
#include <cstdio>
#include <TNtuple.h>
using namespace fastjet;
using namespace std;

int run_fastjet (const std::string &s)
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


    TH1F *jet_pt = new TH1F("jet_pt" , "charged jet; p_{T} GeV; counts", 50, 15,100);
    TH1F *norm = new TH1F("norm", "things to normalize", 3, 0, 3);
    TNtuple *jet = new TNtuple("jet", "charged Jet", "pt:eta:phi");

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
                if (event[ip].isFinal() && std::abs(event[ip].eta()) < 0.9 && event[ip].isCharged())
                {
                    input_particles.push_back(fastjet::PseudoJet(event[ip].px(),event[ip].py(),event[ip].pz(),event[ip].e()));
                }
            }

        // create a jet definition: 
        // a jet algorithm with a given radius parameter
        //----------------------------------------------------------
        double R = 0.4;
        fastjet::JetDefinition jet_def(fastjet::antikt_algorithm, R);

        // run the jet clustering with the above jet definition
        //----------------------------------------------------------
        fastjet::ClusterSequence clust_seq(input_particles, jet_def);

        // get the resulting jets ordered in pt
        //----------------------------------------------------------
        double ptmin = 0.15;
        vector<fastjet::PseudoJet> inclusive_jets = sorted_by_pt(clust_seq.inclusive_jets(ptmin));

        for (unsigned int i = 0; i < inclusive_jets.size(); i++)
        {
            if ( std::abs(inclusive_jets[i].eta()) < 0.5)
            { 
                jet_pt->Fill(inclusive_jets[i].pt());
                jet->Fill(inclusive_jets[i].pt(), inclusive_jets[i].eta(), inclusive_jets[i].phi());
            }
        }
    }

    pythia.stat();
    cout << "[i] Generation done." << endl;

    jet->SetWeight(pythia.info.sigmaGen());
    norm->SetBinContent(1, pythia.info.sigmaGen());
    norm->SetBinContent(2, pythia.info.weightSum());

    // remember to properly save/update and close the output file
    fout->Write();
    fout->Close();
    delete fout;

    // delete the pythia 
    delete ppythia;
    return 0;
}
