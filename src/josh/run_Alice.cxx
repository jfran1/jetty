#include "run_Alice.h"
#include <util/pyargs.h>
#include <util/pyutil.h>
#include <util/looputil.h>
#include <Pythia8/Pythia.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>
#include <TVector.h>
#include <string>
#include <iostream>
#include "fastjet/ClusterSequence.hh"
#include <TNtuple.h>
#include <cmath> // std::abs
#include <TMath.h>

using namespace std;

int run_Alice (const std::string &s)
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
        
        //Grab the Alice histogram
        TFile *f1 = TFile::Open("/global/homes/j/jfran/test/output/HEPData-ins1241422-v1-Table1.root");
        f1->cd("Table 1");
        TH1F *skeleton = (TH1F*)gDirectory->Get("Hist1D_y3");
        skeleton->Reset();
        
        TFile *fout = TFile::Open(outfname.c_str(), "RECREATE");
        fout->cd();

        double dEta = 0.8;
        
        TH1F *norm = new TH1F("norm", "", 6, 0, 6);
        TNtuple *jet = new TNtuple("jet", "gamma+jet", "pt:eta:phi:dphi");
        TNtuple *photon = new TNtuple("photon", "gamma", "pt:eta:phi");
        TNtuple *hadron_jet_20_50 = new TNtuple("hadron_jet_20_50", "hadron recoil jet", "pt:eta:phi");
        TNtuple *hadron_jet_8_9 = new TNtuple("hadron_jet_8_9", "hadron recoil jet", "pt:eta:phi");
        TNtuple *hadrons_20_50 = new TNtuple("hadrons_20_50", "hadrons", "pt:eta:phi");
        TNtuple *hadrons_8_9 = new TNtuple("hadrons_8_9", "hadrons", "pt:eta:phi");

        // initialize pythia with a config and command line args
        Pythia8::Pythia *ppythia = PyUtil::make_pythia(args.asString());
        Pythia8::Pythia &pythia  = *ppythia;
        auto &event              = pythia.event;

        // this is where the event loop section starts
        auto nEv = args.getI("Main:numberOfEvents");
        LoopUtil::TPbar pbar(nEv);

        

        for (unsigned int iE = 0; iE < nEv; iE++)//looping over events  (pp collision)
        {
            pbar.Update();
            if (pythia.next() == false) continue;

            vector<fastjet::PseudoJet> input_particles;
            vector<fastjet::PseudoJet> prompt_photons;
            vector<fastjet::PseudoJet> trigger_hadrons_20_50;
            vector<fastjet::PseudoJet> trigger_hadrons_8_9;
            vector<fastjet::PseudoJet> hadron_recoil_20_50;
            vector<fastjet::PseudoJet> hadron_recoil_8_9;

            //loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++)//looping over particles
            {  
                
                if( event[ip].isFinal() && event[ip].isHadron() && event[ip].pT() >= 20 && event[ip].pT() <= 50)
                {
                    if(event[ip].pT() >= 20 && event[ip].pT() <= 50)
                    {
                        trigger_hadrons_20_50.push_back(fastjet::PseudoJet( event[ip].px(), event[ip].py(), event[ip].pz(), event[ip].e()));
                        hadrons_20_50->Fill(event[ip].pT(), event[ip].eta(), event[ip].phi() );
                        for (int i =0; i < event.size(); i++)
                        {
                            if (event[i].isCharged() && event[i].isFinal() )
                            {
                                hadron_recoil_20_50.push_back(fastjet::PseudoJet( event[i].px(), event[i].py(), event[i].pz(), event[i].e()));                            
                            }
                        } // end for
                    } // end if

                    else if (event[ip].pT() >= 8 && event[ip].pT() <= 9)
                    {
                        trigger_hadrons_8_9.push_back(fastjet::PseudoJet( event[ip].px(), event[ip].py(), event[ip].pz(), event[ip].e()));
                        hadrons_8_9->Fill(event[ip].pT(), event[ip].eta(), event[ip].phi() );
                        for (int i =0; i < event.size(); i++)
                        {
                            if (event[i].isCharged() && event[i].isFinal() )
                            {
                                hadron_recoil_20_50.push_back(fastjet::PseudoJet( event[i].px(), event[i].py(), event[i].pz(), event[i].e()));                            
                            }
                        } // end for

                    }
                }




                if(event[ip].isFinal() && event[ip].id()==22 && event[ip].status() != 91 && std::abs(event[ip].eta()) < 1 && event[ip].pT() < 30 && event[ip].pT() > 20)
                {
                    prompt_photons.push_back(fastjet::PseudoJet(event[ip].px(),event[ip].py(),event[ip].pz(),event[ip].e()));
                    photon->Fill(event[ip].pT(), event[ip].eta(), event[ip].phi() );

                    for(int i=0; i < event.size(); i++)
                    {
                        if(event[i].isFinal() )
                        {
                            input_particles.push_back(fastjet::PseudoJet(event[i].px(),event[i].py(),event[i].pz(),event[i].e()));
                        }
                    }
                }
            }

            double R= 0.3;
            double R_alice = 0.5;
            double pT_min = 0.15; //GeV

            fastjet::JetDefinition jet_def(fastjet::antikt_algorithm, R);
            fastjet::JetDefinition charged_def(fastjet::antikt_algorithm, R_alice);

            fastjet::ClusterSequence clust_seq(input_particles, jet_def);
            fastjet::ClusterSequence alice_clust_20_50(hadron_recoil_20_50, charged_def );
            fastjet::ClusterSequence alice_clust_8_9(hadron_recoil_8_9, charged_def );

            vector<fastjet::PseudoJet> inclusive_jets = sorted_by_pt(clust_seq.inclusive_jets());
            vector<fastjet::PseudoJet> recoil_jets_hadron_20_50 = sorted_by_pt(alice_clust_20_50.inclusive_jets(pT_min) );
            vector<fastjet::PseudoJet> recoil_jets_hadron_8_9 = sorted_by_pt(alice_clust_8_9.inclusive_jets(pT_min) );

            // for pt 20-50 on trigger hadrons
            for( int i = 0; i < trigger_hadrons_20_50.size(); i++)
            {
                for(int j =0; j < recoil_jets_hadron_20_50.size(); j++)
                {
                    double dPhi = recoil_jets_hadron_20_50[j].delta_phi_to(trigger_hadrons_20_50[i]); 

                    if (std::abs(recoil_jets_hadron_20_50[j].eta()) < 0.4 && dPhi > (TMath::Pi() - 0.6) )   
                    {
                        hadron_jet_20_50->Fill(recoil_jets_hadron_20_50[j].pt(), recoil_jets_hadron_20_50[j].eta(), recoil_jets_hadron_20_50[j].phi() );
                    }
                }
            }

            // For pt 8-9 on trigger hadrons
            for( int i = 0; i < trigger_hadrons_8_9.size(); i++)
            {
                for(int j =0; j < recoil_jets_hadron_8_9.size(); j++)
                {
                    double dPhi = recoil_jets_hadron_8_9[j].delta_phi_to(trigger_hadrons_8_9[i]); 

                    if (std::abs(recoil_jets_hadron_8_9[j].eta()) < 0.4 && dPhi > (TMath::Pi() - 0.6) )   
                    {
                        hadron_jet_8_9->Fill(recoil_jets_hadron_8_9[j].pt(), recoil_jets_hadron_8_9[j].eta(), recoil_jets_hadron_8_9[j].phi() );
                    }
                }
            }

            for(int i =0; i < prompt_photons.size(); i++)
            {
                for(int j=0; j < inclusive_jets.size() ; j++ )
                {
                    double dPhi = inclusive_jets[j].delta_phi_to(prompt_photons[i]);

                    if(std::abs(inclusive_jets[i].eta()) < 1-R && std::abs(dPhi) > 3*TMath::Pi()/4)
                    {
                        jet->Fill(inclusive_jets[j].pt(), inclusive_jets[j].eta(), inclusive_jets[j].phi(), dPhi);
                    }
                }
            }
        }

        // hadron_jet->SetWeight(pythia.info.sigmaGen());

        pythia.stat();
        cout << "[i] Done." << endl;

        double vSigma = pythia.info.sigmaGen();
        double wSum = pythia.info.weightSum();

        norm->SetBinContent(1, vSigma);
        norm->SetBinContent(2, wSum);
                
        // remember to properly save/update and close the output file
        fout->Write();
        fout->Close();
        delete fout;

        // delete the pythia
        delete ppythia;
        return 0;
}
