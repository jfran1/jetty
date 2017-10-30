
#include "Sample_Pythia.h"

Sample_Pythia::Sample_Pythia() 
{
    sigma_sample = 0;
    sigma_sample_error = 0;
}


double set_sigma (const std::string &s, int nev=1000)
{
        // test(s); return;
        PyUtil::Args args(s);

        // initialize pythia with a config and command line args
		Pythia8::Pythia *ppythia = PyUtil::make_pythia(args.asString());
		Pythia8::Pythia &pythia  = *ppythia;
		auto &event              = pythia.event;

		// this is where the event loop section starts
        // auto nEv = args.getI("Main:numberOfEvents");
        LoopUtil::TPbar pbar(nev);

        for (unsigned int iE = 0; iE < nEv; iE++)
        {
        	pbar.Update();
            if (pythia.next() == false) continue;
        }

        // string xsec_outfname = outfname + ".txt";
        // PyUtil::CrossSections(pythia, xsec_outfname.c_str());

        // delete the pythia
        delete ppythia;
        return pythia.info.sigmaGen();
}


void hello()
{
    std::cout << "Hello World!" << std::endl;
}