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
#include <string>
#include <iostream>
#include <cmath> // std::abs

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
        TFile *fout = TFile::Open(outfname.c_str(), "RECREATE");
        fout->cd();

        TH1F *gammaPrompt = new TH1F("gammaPrompt", "p_{T} Distribution; #gamma p_{T} [GeV/#it{C}]; counts", 75, 0, 150); 
        TH1F *gammaSoft = new TH1F("gammaSoft", "p_{T} Distribution; #gamma p_{T} [GeV/#it{C}]; counts", 75, 0, 150);
        TH1F *gammaJet_Prompt = new TH1F("gammaJet_Prompt", "p_{T} Distribution; #gamma p_{T} [GeV/#it{C}]; counts", 75, 0, 150);
        TH1F *gammaJet_Soft = new TH1F("gammaJet_Soft", "p_{T} Distribution; #gamma p_{T} [GeV/#it{C}]; counts", 75, 0, 150);
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

            vector<fastjet::PseudoJet> input_particles_Prompt; 
            vector<fastjet::PseudoJet> input_particles_Soft; 
 

            // loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++) 
            {
                if(event[ip].isFinal() && event[ip].id() == 22 && std::abs(event[ip].eta()) < 2 )
                {
                    if(std::abs(event[ip].status()) != 91)
                    {
                        gammaPrompt->Fill(event[ip].pT());
                        input_particles_Prompt.push_back(fastjet::PseudoJet(event[ip].px(), event[ip].py(), event[ip].pz(), event[ip].e()));
                    }

                    if(std::abs(event[ip].status()) == 91)
                    {
                        gammaSoft->Fill(event[ip].pT() );
                        input_particles_Soft.push_back(fastjet::PseudoJet(event[ip].px(), event[ip].py(), event[ip].pz(), event[ip].e()));   
                    }
                }

            }
         

            // run Jet Algorith per event
            double R = 0.4;
            fastjet::JetDefinition jet_def(fastjet::antikt_algorithm, R); // define jet

            fastjet::ClusterSequence clust_seq_Prompt(input_particles_Prompt, jet_def); // run cluster with jet definition on, hadron filter
            vector<fastjet::PseudoJet> inclusive_jets_Prompt = sorted_by_pt(clust_seq_Prompt.inclusive_jets()); // sort jets

            fastjet::ClusterSequence clust_seq_Soft(input_particles_Soft, jet_def); // run cluster with jet definition on, cuts                        
            vector<fastjet::PseudoJet> inclusive_jets_Soft = sorted_by_pt(clust_seq_Soft.inclusive_jets()); // sort jets


            for(unsigned int i = 0; i < inclusive_jets_Prompt.size(); i++)
            {
                if(std::abs(inclusive_jets_Prompt[i].eta() < 1.5) )
                {    
                    gammaJet_Prompt->Fill(inclusive_jets_Prompt[i].pt());
                }
            }

            for(unsigned int i = 0; i < inclusive_jets_Soft.size(); i++)
            {
                if(std::abs(inclusive_jets_Soft[i].eta() < 1.5) )
                {    
                    gammaJet_Soft->Fill(inclusive_jets_Soft[i].pt());
                }
            }

        }

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
