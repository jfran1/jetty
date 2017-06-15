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
        //TH1F *hpT = new TH1F("hpT", "pT;p_{T} (GeV/#it{c});counts", 300, 0, 300);

        TH2F *pT2pTHat = new TH2F("pT2pTHat", "", 300, 0, 300, 300, 0, 300);
        pT2pTHat->SetXTitle("pT (both)");
        pT2pTHat->SetYTitle("pT hat");

        TH2F *h5pT = new TH2F("h5pT", "", 300,0, 300,300,0, 300);
        h5pT->SetXTitle("pT of #5");
        h5pT->SetYTitle("pT hat");
     
        TH2F *h6pT = new TH2F("h6pT", "", 300,0, 300,300,0, 300);
        h6pT->SetXTitle("pT of #6");
        h6pT->SetYTitle("pT hat");

        TH1F *difpT = new TH1F("difpT", "", 200,0, 100);
        difpT->SetXTitle("pT difference #5 & #6");
        difpT->SetYTitle("counts");

        TH2F *sumpT = new TH2F("sumpT", "", 300,0, 300, 300, 0, 300);
        sumpT->SetXTitle("pT sum #5 & #6");
        sumpT->SetYTitle("pT hat");

        TH2F *avgpT = new TH2F("avgpT", "", 300,0, 300, 300, 0, 300);
        avgpT->SetXTitle("pT avg #5 & #6");
        avgpT->SetYTitle("pT hat");

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

            float v5pT = event[5].pT();
            float v6pT = event[6].pT();
            float sum = v5pT + v6pT;
            float dif = v5pT- v6pT;
            float avg = std::abs((sum)/2);
           
            pT2pTHat->Fill(event[5].pT(), pythia.info.pTHat());
            pT2pTHat->Fill(event[6].pT(), pythia.info.pTHat());
            h5pT->Fill(v5pT, pythia.info.pTHat());
            h6pT->Fill(v6pT, pythia.info.pTHat());
            difpT->Fill(dif);
            sumpT->Fill(sum,pythia.info.pTHat());
            avgpT->Fill(avg, pythia.info.pTHat());


            //loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++)//looping over particles
            {  

                etaHighpT->Fill(event[ip].eta());
                //Filter for |eta| < 3 and final state particles
                if ((std::abs(event[ip].eta()) < 3) && (event[ip].status() > 0))  
                {
                    //we can keep these particles
                    eta2pT->Fill(event[ip].pT(), event[ip].eta());
                    if (std::abs(event[ip].pT()) <= 30 && std::abs(event[ip].pT()) >= 20) etaMedpT->Fill(event[ip].eta());
                 }

                  /*  
                    else if (std::abs(event[ip].pT()) <= 30 && std::abs(event[ip].pT()) >= 20) etaMedpT->Fill(event[ip].eta());
                    else if (std::abs(event[ip].pT()) <= 60 && std::abs(event[ip].pT()) >= 50) etaHighpT->Fill(event[ip].eta());
                  */  

                



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
