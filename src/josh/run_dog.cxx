#include "run_dog.h"
#include <util/pyargs.h>
#include <util/pyutil.h>
#include <util/looputil.h>
#include <Pythia8/Pythia.h>
#include </global/homes/j/jfran/test/pythia8226/include/Pythia8/SigmaTotal.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <string>
#include <iostream>
#include <cmath>
#include "TNtuple.h"
#include "Sample_Pythia.h"

using namespace std;

int run_dog (const std::string &s)
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

        TH1F *hpT = new TH1F("hpT", "p_{T} Final state;p_{T} [GeV] ; Counts", 50, 0, 220);
        TH1F *norm1 = new TH1F("norm1", "norm pT; p_{T}; d#sigma/dp_{T} )", 30, 0, 60);
        TNtuple *raw_entries = new TNtuple("raw_entries", "raw data", "pT;phi;eta");
        TNtuple *scaled_entries = new TNtuple("scaled_entries", "scaled data", "pT;phi;eta");
    
        // initialize pythia with a config and command line args
        Pythia8::Pythia *ppythia = PyUtil::make_pythia(args.asString());
        Pythia8::Pythia &pythia  = *ppythia;
        auto &event              = pythia.event;

        // this is where the event loop section starts
        auto nEv = args.getI("Main:numberOfEvents");
        LoopUtil::TPbar pbar(nEv);


        Sample_Pythia sample;

        std::cout << "[i] Sigma right now is: " << sample.get_sigma() << std::endl;
        std::cout << "[i] Sigma error       :" << sample.get_error() << std::endl;

        sample.set_sigma(s , nEv);
        std::cout << "[i] Now sigma is: " << sample.get_sigma() << std::endl; 

        for (unsigned int iE = 0; iE < nEv; iE++)//loopin over events  (pp collision)
        {
            pbar.Update();
            if (pythia.next() == false) continue;

            //loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++)//looping over particles
            {  
                //cout << "Cross section: " << pythia.info.sigmaGen() << endl;
                //Filter for |eta| < 3 and outgoing partons
                if ((std::abs(event[ip].eta()) < 3) && event[ip].isFinal() && event[ip].isHadron())  
                {
                    raw_entries->Fill(event[ip].pT(), event[ip].phi(), event[ip].eta());   
                }

            
            }
        }


        pythia.stat();
        cout << "[i] Done." << endl;

        norm1->Scale( pythia.info.sigmaGen() /  (norm1->GetBinWidth(1)*pythia.info.weightSum()));
        // norm2->Scale( pythia.info.sigmaGen() /  pythia.info.weightSum());

        // TH1F *divide = (TH1F*)norm1->Clone("divide");
        // divide->Divide(norm2);

        // remember to properly save/update and close the output file
        fout->Write();
        fout->Close();
        delete fout;

        // delete the pythia
        delete ppythia;
        return 0;
    }
