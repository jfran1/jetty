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
        TFile *fout = TFile::Open(outfname.c_str(), "RECREATE");
        fout->cd();

        // set pi 
        double dEta = 0.8;


        TH1F *hpT = new TH1F("hpT", ";p_{T} [GeV]; 1/(2#pip_{T})(d^{2}#sigma)/(d#etadp_{T})(mb Gev^{-2} c^{2})", 50, 0, 100);
        TH2F *eta2pT = new TH2F("eta2pT", " ;p_{T}; #eta", 50,0,100,50,-.8,.8);
        TH1F *data = new TH1F("data", "", 5, 0, 5);
         TH1F *hpTRaw = new TH1F("hpTRaw", ";p_{T}; Counts", 50, 0, 100);

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


                //Filter for |eta|, final state particles, and pT > 0.15 GeV
                if ((std::abs(event[ip].eta()) < dEta) && (event[ip].isFinal()) && (event[ip].isCharged()) && (event[ip].pT()>0.15));  
                {
                    hpT->Fill(event[ip].pT());
                    eta2pT->Fill(event[ip].pT(),event[ip].eta());
                    hpTRaw->Fill(event[ip].pT());
                }


            }

        }


        pythia.stat();
        cout << "[i] Done." << endl;

        double vSigma = pythia.info.sigmaGen();
        double wSum = pythia.info.weightSum();
        double binWidth = hpT->GetBinWidth(1);

        data->SetBinContent(1, vSigma);
        data->SetBinContent(2, wSum);
        data->SetBinContent(3, binWidth);

        cout << "The cross section used is: " << vSigma << endl;
        cout << "The weightSum used is: " << wSum << endl;
        cout << "The binWidth used is: " << binWidth << endl;

        hpT->Sumw2();
        hpT->Scale( vSigma/
                    (binWidth*2*dEta*wSum*2*TMath::Pi()) );
                     

        TF1 *fun = new TF1("fun","[0]+[1]*x",0,100);
        fun->SetParameter(0, 0);
        fun->SetParameter(1,1);
        hpT->Divide(fun);

        // remember to properly save/update and close the output file
        fout->Write();
        fout->Close();
        delete fout;

        // delete the pythia
        delete ppythia;
        return 0;
}
