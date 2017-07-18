#include "run_jetGamma.h"
#include "run_fastjet.h"
#include "fastjet/ClusterSequence.hh"

#include <util/pyargs.h>
#include <util/pyutil.h>
#include <util/looputil.h>
#include <Pythia8/Pythia.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TNtuple.h>
#include <TMath.h>
#include <string>
#include <iostream>
#include <cmath> // std::abs

using namespace std;
using namespace fastjet;

int run_jetGamma (const std::string &s)
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

        TH1F *gammaPrompt = new TH1F("gammaPrompt", "p_{T} Distribution; #gamma p_{T} [GeV/#it{C}]; counts", 50, 0, 400);
        TH1F *jet = new TH1F("jet", "p_{T} Distribution; jet p_{T} [GeV/#it{C}]; counts", 50, 0, 400);
        TH1F *gamma_decay = new TH1F("gamma_decay", "p_{T} Distribution; #gamma p_{T} [GeV/#it{C}]; counts", 50, 0, 400);
        TH1F *delta_phi = new TH1F("delta_phi", "#Delta #phi; #Delta #phi (Rad); counts", 100, -TMath::Pi(), TMath::Pi());
        TH1F *delta_phi2 = new TH1F("delta_phi2", "#Delta #phi; #Delta #phi (Rad); counts", 100, -TMath::Pi(), TMath::Pi());
        TH2F *dphi_pT = new TH2F("dphi_pT", ";#Delta #phi; p_{T}", 100,-TMath::Pi(), TMath::Pi(), 50, 0, 100);
        TH2F *dJet_pT = new TH2F("dJet_pT", "; #gamma p_{T} GeV; #Delta p_{T}", 75,0, 150, 500, -100, 100);
        TH2F *dJet_pT_max = new TH2F("dJet_pT_max", "; #gamma p_{T} GeV; #Delta p_{T}", 75,0, 150, 500, -100, 100);
        TH1F *norm = new TH1F("norm", " ", 3, 0,3);
        TNtuple *jet_space_pT = new TNtuple("jet_space_pT", "Jet","dphi:delta_pt:gamma_pt:jet_phi");

        TH1F *test_hist = new TH1F("test_hist", "Test; #Delta p_{T}; counts", 500, -100, 100);

        // initialize pythia with a config and command line args
        Pythia8::Pythia *ppythia = PyUtil::make_pythia(args.asString());
        Pythia8::Pythia &pythia  = *ppythia;
        auto &event              = pythia.event;

        // this is where the event loop section starts
        auto nEv = args.getI("Main:numberOfEvents");
        LoopUtil::TPbar pbar(nEv);

        double pTHatmin = args.getD("PhaseSpace:pTHatMin");

        for (unsigned int iE = 0; iE < nEv; iE++)//loopin over events  (pp collision)
        {
            pbar.Update();
            if (pythia.next() == false) continue;

            vector<fastjet::PseudoJet> input_particles;
            vector<fastjet::PseudoJet> input_particles_gamma;        

            // loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++) 
            {
                
                // This vecotr will only make sense if run with prompt photon on
                if(event[ip].isFinal())
                {
                    input_particles.push_back(fastjet::PseudoJet(event[ip].px(),event[ip].py(),event[ip].pz(),event[ip].e()));
                }
                if(event[ip].isFinal() && event[ip].id() == 22 && std::abs(event[ip].eta()) < 2.0 )
                {
                    if(event[ip].status() != 91)
                    {
                        gammaPrompt->Fill(event[ip].pT());
                        input_particles_gamma.push_back(fastjet::PseudoJet(event[ip].px(),event[ip].py(),event[ip].pz(),event[ip].e()));
                    }
                    if(event[ip].status() == 91)
                    {
                        gamma_decay->Fill(event[ip].pT());
                    }
                }
            } // end partilce loop

            double R = 0.6;
            fastjet::JetDefinition jet_def(fastjet::antikt_algorithm, R);

            fastjet::ClusterSequence clust_seq(input_particles, jet_def);
            vector<fastjet::PseudoJet> inclusive_jets = sorted_by_pt(clust_seq.inclusive_jets(pTHatmin/2));

            int location = 0;
            double max_pt = 0.;
            
            for (int i = 0; i < input_particles_gamma.size() ; i++)
            {
                
                for(int j = 0; j < inclusive_jets.size(); j++)
                {
                    double dphi = inclusive_jets[j].delta_phi_to(input_particles_gamma[i]);
                    delta_phi->Fill(dphi);
                    
                    if(std::abs(dphi) > 2*TMath::Pi()/3)                                                                        // should cut out the prompt photons
                    {
                        delta_phi2->Fill(dphi);                                                                                 // delta phi with delta phi cut
                        dphi_pT->Fill(dphi, inclusive_jets[j].pt());                                                            // 2D hist delata phi vs. pT with delta phi cut
                        dJet_pT->Fill(input_particles_gamma[i].pt(), inclusive_jets[j].pt() - input_particles_gamma[i].pt());   // 2D hist gamma pT vs delta pT 
                        jet->Fill(inclusive_jets[i].pt());                                                                      // fill with jets delta phi cut 
                        if (inclusive_jets[i].pt() > max_pt)                                                                    // Find Higest pT get in delta phi cut
                        {
                            location = j;
                            max_pt = inclusive_jets[j].pt();
                        }
                    }   
                }
            }

            
            for(int i=0; i<input_particles_gamma.size(); i++)
            {
                double dphi = inclusive_jets[location].delta_phi_to(input_particles_gamma[i]);
                double dpT = inclusive_jets[location].pt()- input_particles_gamma[i].pt();
                double gamma_pt = input_particles_gamma[i].pt();
                double jet_phi = inclusive_jets[location].phi();

                if(inclusive_jets[i].eta() < (2.0 - R) )
                {
                    dJet_pT_max->Fill(gamma_pt, dpT);                    // Fill 2D histogram with Delta pT Max and Gamma pT
                    jet_space_pT->Fill( dphi, dpT, gamma_pt, jet_phi);   // Fill tuple

                    if(gamma_pt > 20 && gamma_pt < 40)
                    {
                        test_hist -> Fill(dpT);
                    }
                }
            }


        } // end event loop

        norm->SetBinContent(1, pythia.info.sigmaGen());
        norm->SetBinContent(2, pythia.info.weightSum());

        // pythia.stat();
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
