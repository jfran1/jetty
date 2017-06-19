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

        

        TH1F *partNorm = new TH1F("partNorm", "Outgoing Partons; p_{T} [GeV]; d#sigma/dp_{T} [mb] ", 500,0,500);
        TH1F *final = new TH1F("final", "Finals State Particles; p_{T} [GeV]; d#sigma/dp_{T} [mb] ", 500,0,500);

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

            if (std::abs(event[5].eta()) < 3) partNorm->Fill(event[5].pT());
            else if (std::abs(event[6].eta()) < 3) partNorm->Fill(event[6].pT());

            //loop over particles in the event
            for (unsigned int ip = 0; ip < event.size(); ip++)//looping over particles
            {  

                //Filter for |eta| < 3 and final state particles
                if ((std::abs(event[ip].eta()) < 3) && (event[ip].isFinal()));  
                {
                    final->Fill(event[ip].pT());
                }


            }

        }


        pythia.stat();
        cout << "[i] Done." << endl;

        partNorm->Sumw2();
        final->Sumw2();
        partNorm->Scale(pythia.info.sigmaGen()/(partNorm->GetBinWidth(1)*pythia.info.weightSum()));
        final->Scale(pythia.info.sigmaGen()/(final->GetBinWidth(1)*pythia.info.weightSum()));

        // remember to properly save/update and close the output file
        fout->Write();
        fout->Close();
        delete fout;

        // delete the pythia
        delete ppythia;
        return 0;
}
