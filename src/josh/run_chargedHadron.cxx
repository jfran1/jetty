#include "run_chargedHadron.h"
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

int run_chargedHadron (const std::string &s)
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

        TH1F *hpT = new TH1F("hpT", "p_{T} Distribution;p_{T} (GeV/#it{c});counts", 50, 0, 100);
        TH1F *hpTFilter = new TH1F("hpTFilter", "p_{T} Distribution;p_{T} (GeV/#it{c});counts", 50, 0, 100);
        TH1F *norm = new TH1F("norm", " ", 2, 0,2);
        TH1F *jetCuts = new TH1F("jetpT", "p_{T} Distribution; p_{T} [GeV/#it{C}]; counts", 75, 0, 150);
        TH1F *jetNoFilter = new TH1F("jetNoFilter", "p_{T} Distribution; p_{T} [GeV/#it{C}]; counts", 75, 0, 150);
        TH1F *jetFilter = new TH1F("jetFilter", "p_{T} Distribution; p_{T} [GeV/#it{C}]; counts", 75, 0, 150);


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

            vector<fastjet::PseudoJet> input_particles1; // no filter
            vector<fastjet::PseudoJet> input_particles2; // charged hardons           
            vector<fastjet::PseudoJet> input_particles3; // all particles

            // loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++)
            {

                input_particles3.push_back(fastjet::PseudoJet(event[ip].px(), event[ip].py(), event[ip].pz(), event[ip].e()));

                if (event[ip].isFinal() && event[ip].isHadron() && event[ip].isCharged() )
                {
                    hpT->Fill(event[ip].pT());
                    input_particles1.push_back(fastjet::PseudoJet(event[ip].px(),event[ip].py(),event[ip].pz(),event[ip].e()));

                    if (event[ip].pT() > 20 && std::abs(event[ip].eta()) < 2 )
                    {
                        hpTFilter->Fill(event[ip].pT());
                        input_particles2.push_back(fastjet::PseudoJet(event[ip].px(),event[ip].py(),event[ip].pz(),event[ip].e()));
                    }
                }
            }

            double R = 0.4;
            fastjet::JetDefinition jet_def(fastjet::antikt_algorithm, R); // define jet

            fastjet::ClusterSequence clust_seq1(input_particles1, jet_def); // run cluster with jet definition on, hadron filter
            vector<fastjet::PseudoJet> inclusive_jets1 = sorted_by_pt(clust_seq1.inclusive_jets()); // sort jets

            fastjet::ClusterSequence clust_seq2(input_particles2, jet_def); // run cluster with jet definition on, cuts                        
            vector<fastjet::PseudoJet> inclusive_jets2 = sorted_by_pt(clust_seq2.inclusive_jets()); // sort jets

            fastjet::ClusterSequence clust_seq3(input_particles3, jet_def); // run cluster with jet definition on, no filter                        
            vector<fastjet::PseudoJet> inclusive_jets3 = sorted_by_pt(clust_seq3.inclusive_jets()); // sort jets

            for(unsigned int i = 0; i < inclusive_jets3.size(); i++)
            {

                jetNoFilter->Fill(inclusive_jets3[i].pt());
            }

            for(unsigned int i = 0; i < inclusive_jets1.size(); i++)
            {
                jetFilter->Fill(inclusive_jets1[i].pt());
            }

            for (unsigned int i = 0; i < inclusive_jets2.size(); i++)
            {
                // eta cut on jets
                if ( std::abs(inclusive_jets2[i].eta()) < 1.5)
                { 
                    jetCuts->Fill(inclusive_jets2[i].pt());
                }
            }

        }

        norm->SetBinContent(1, pythia.info.sigmaGen());
        norm->SetBinContent(2, pythia.info.weightSum());

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
