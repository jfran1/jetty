#include "run_josh.h"

#include <util/pyargs.h>
#include <util/pyutil.h>
#include <util/looputil.h>

#include <Pythia8/Pythia.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>

#include <string>
#include <iostream>

#include <cmath> // std::abs

using namespace std;

int run_josh (const std::string &s)
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

        TH2F *eta2pT = new TH2F("eta2pT", "eta vs. pT;pT (Final State); eta", 100, 0, 25, 800, -3, 3);

        TH1F *etaLowpT = new TH1F("etaLowpT", "; eta;counts", 100,-3,3);
        TH1F *etaMedpT = new TH1F("etaMedpT", " ; eta;counts", 100,-3,3);
        TH1F *etaHighpT = new TH1F("etaHighpT", " ; eta;counts", 100,-3,3);

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

            //loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++)//looping over particles
            {  

                //Filter for |eta| < 3 and final state particles
                if ((std::abs(event[ip].eta()) < 3) && (event[ip].status() > 0))  
                {
                    eta2pT->Fill(event[ip].pT(), event[ip].eta());

                    if (std::abs(event[ip].pT()) <= 5 && std::abs(event[ip].pT()) >= 0) etaLowpT->Fill(event[ip].eta());
                    else if (std::abs(event[ip].pT()) <= 15 && std::abs(event[ip].pT()) >= 10) etaMedpT->Fill(event[ip].eta());
                    else if (std::abs(event[ip].pT()) <= 25 && std::abs(event[ip].pT()) >= 20) etaHighpT->Fill(event[ip].eta());
                 }


            }

        }


        pythia.stat();
        cout << "[i] Done." << endl;

        // remember to properly save/update and close the output file
        fout->Write();
        fout->Close();
        delete fout;

        // delete the pythia
        delete ppythia;
        return 0;
}
