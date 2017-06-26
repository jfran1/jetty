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


    vector<fastjet::PseudoJet> input_particles;
    TH1F *hpT = new TH1F("hpT", "p_{T} of Jets; p_{T} [GeV]; Counts", 500,0,1000);
    TH1F *rap = new TH1F("rap", "Rapidity of Jets; y; counts", 50, -10, 10);
    TH1F *phi = new TH1F("phi", "Azmuth of Jets; phi [rad]; coutns", 100, 0, 10);
    TH1F *hE = new TH1F("hE","Energy of Jets; E [GeV]; counts", 25000, 0, 25000);
    TH1F *data= new TH1F("data", "data for pT", 4, 0, 4);

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

        // loop over particles in the event
        for (unsigned int ip = 0; ip < event.size(); ip++)
            {
                if (event[ip].isFinal() && std::abs(event[ip].eta()) < 0.9 && event[ip].isCharged())
                {
                    input_particles.push_back(fastjet::PseudoJet(event[ip].px(),event[ip].py(),event[ip].pz(),event[ip].e()));
                }
            }
        }


    pythia.stat();
    cout << "[i] Generation done." << endl;

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
    vector<fastjet::PseudoJet> inclusive_jets = sorted_by_pt(clust_seq.inclusive_jets());

    // tell the user what was done
    //  - the description of the algorithm used
    //  - extract the inclusive jets with pt > 5 GeV
    //    show the output as 
    //      {index, rap, phi, pt}
    //----------------------------------------------------------
    cout << "Ran " << jet_def.description() << endl;

    // label the columns
    printf("%5s %15s %15s %15s %15s\n","jet #", "Energy", "rapidity", "phi", "pt");
 
    // print out the details for each jet
    for (unsigned int i = 0; i < inclusive_jets.size(); i++)
    {
        printf("%5u %15.4f %15.8f %15.8f %15.8f %15.8f\n",
        i,inclusive_jets[i].e() ,inclusive_jets[i].rap(), inclusive_jets[i].phi(),
        inclusive_jets[i].perp());
        if ( std::abs(inclusive_jets[i].eta()) < 0.5)
        { 
            hpT->Fill(inclusive_jets[i].pt());
            rap->Fill(inclusive_jets[i].rap());
            phi->Fill(inclusive_jets[i].phi());
            hE->Fill(inclusive_jets[i].e());
        }
    }

    data->SetBinContent(1, pythia.info.sigmaGen());
    data->SetBinContent(2, pythia.info.weightSum());



    // remember to properly save/update and close the output file
    fout->Write();
    fout->Close();
    delete fout;

    // delete the pythia 
    delete ppythia;
    return 0;
}
