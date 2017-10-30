
#include <string>
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


class  Sample_Pythia
{
private:

	double sigma_sample;
	double sigma_sample_error;

public:

	void set_sigma(const std::string &s, unsigned int nev=1000);
	void hello();

	double get_sigma() ;
	double get_error() ;
};

