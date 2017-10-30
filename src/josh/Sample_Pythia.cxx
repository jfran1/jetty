#include "Sample_Pythia.h"

Sample_Pythia::Sample_Pythia() 
{
    sigma_sample = 1;
    sigma_sample_error = 2;
}


/*
# This function sets the sample sigma and the erro of that sigma
#
# s: A string of prameters of pythia settings, needs to be same exact parm as the event generatio this is called in
# 
# nev: is the number of events you associte with your sample (should make >= 1000 for decent results)
#   
*/
void Sample_Pythia::set_sigma (const std::string &s, int nev)
{
        std::cout << "############################ ETNTERTING SAMPLE RUN ####################" << std::endl; 
        std::cout << "#######################################################################" << std::endl;
        std::cout << "#######################################################################" << std::endl;

        // test(s); return;
        PyUtil::Args args(s);

        // initialize pythia with a config and command line args
		Pythia8::Pythia *ppythia = PyUtil::make_pythia(args.asString());
		Pythia8::Pythia &pythia  = *ppythia;
		auto &event              = pythia.event;

		// this is where the event loop section starts
        // auto nEv = args.getI("Main:numberOfEvents");
        LoopUtil::TPbar pbar(nev);

        for (unsigned int iE = 0; iE < nev; iE++)
        {
        	pbar.Update();
            if (pythia.next() == false) continue;
        }

        // string xsec_outfname = outfname + ".txt";
        // PyUtil::CrossSections(pythia, xsec_outfname.c_str());

        // delete the pythia
        delete ppythia;
        sigma_sample = pythia.info.sigmaGen();
        sigma_sample_error = pythia.info.sigmaErr();

        std::cout << "#######################################################################" << std::endl;
        std::cout << "#######################################################################" << std::endl;
        std::cout << "############################ LEAVING SAMPLE RUN #######################" << std::endl; 

}


void Sample_Pythia::hello()
{
    std::cout << "Hello World!" << std::endl;
}