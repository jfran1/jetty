#include "pyutil.h"
#include "strutil.h"
#include "sysutil.h"

#include <Pythia8/Pythia.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <string>
#include <cstdlib>

#include <TMath.h>

using namespace std;

namespace PyUtil
{
	double sqrts(double _eA, double _eB, double mA, double mB)
		{
				double eA  = TMath::Abs(eA);
				double eB  = TMath::Abs(eB);
				double pA  = TMath::Sqrt(eA * eA - mA * mA);
				double pB  = TMath::Sqrt(eB * eB - mB * mB);
				double eCM = TMath::Sqrt( TMath::Power(eA + eB, 2.) - TMath::Power(pA + (-1. * pB), 2.) );
				return eCM;
		}

	Pythia8::Pythia *make_pythia(const SysUtil::Args &args)
	{
		string cfgfile;
		if (args.isSet("--config"))
		{
			cfgfile = args.get("--config");
			if (SysUtil::file_exists(cfgfile) == false)
			{
				cerr << "[w] config file does not exists... - tried: " << cfgfile << endl;
				cerr << "    since you asked for a specific config to load we stop here... not creating pythia." << endl;
				return 0x0;
			}
		}
		// create pythia generator with some config
		Pythia8::Pythia *ppythia = new Pythia8::Pythia();
		// Read in commands from external file.
		// nope - no need to... this is pre-loaded with pyargs::_cook
		// ppythia->readFile(cfgfile.c_str());
		// cout << "[i] Setup standard pythia... using:" << cfgfile << endl;

		auto pairs = args.pairs();
		for (unsigned int i = 0; i < pairs.size(); i++)
		{
			if (pairs[i].second.size() < 1) continue;
			cout << "[make_pythia] paired arg: #" << i << " " << pairs[i].first << " " << pairs[i].second << endl;
			string spypar = pairs[i].first + " = " + pairs[i].second;
			ppythia->readString(spypar.c_str());
		}

		ppythia->init();
		cout << "[i] ppythia is at " << ppythia << endl;

		return ppythia;
	}

	void PrintParticle(const Pythia8::Particle &p)
	{
		std::cout << " | id: "; std::cout.flush();
		std::cout << std::setfill(' ') << std::setw(10) << std::right << std::showpos;
		std::cout << p.id(); std::cout.flush();
		std::cout << " | name: "; std::cout.flush();
		std::cout << std::setfill(' ') << std::setw(10) << std::right << std::noshowpos;
		std::cout << p.name(); std::cout.flush();
		std::cout << " | pT: "; std::cout.flush();
		std::cout << std::setfill(' ') << std::setw(10) << std::right << std::noshowpos;
		std::cout << p.pT(); std::cout.flush();
		std::cout << " | pz: "; std::cout.flush();
		std::cout << std::setfill(' ') << std::setw(10) << std::right << std::noshowpos;
		std::cout << p.pz(); std::cout.flush();
		std::cout << " | y: "; std::cout.flush();
		std::cout << std::setfill(' ') << std::setw(10) << std::right << std::noshowpos;
		std::cout << p.y(); std::cout.flush();
		std::cout << std::endl;
	}

	void PrintParticle(const Pythia8::Event &event, int idx)
	{
		Pythia8::Particle p = event[idx];

		std::cout << "    p index: "; std::cout.flush();
		std::cout << std::setfill(' ') << std::setw(7) << std::right << std::noshowpos;
		std::cout << idx; std::cout.flush();
		PrintParticle(event[idx]);
	}

	void PrintParticle(const Pythia8::Pythia &pythia, int idx)
	{
		Pythia8::Event event = pythia.event;
		PrintParticle(event, idx);
	}

	void PrintEventInfo(const Pythia8::Pythia &pythia)
	{
		Pythia8::Event event = pythia.event;
		event.list();
		// Also  junctions.
		//	event.listJunctions();
	}

	std::vector<int> GetDaughters(const Pythia8::Event &event, int idx, int minID, int maxID, bool quiet)
	{
		int pDaughter1 = event[idx].daughter1();
		int pDaughter2 = event[idx].daughter2();
		std::vector<int> retval;
		for (int idxd = pDaughter1; idxd <= pDaughter2; idxd++)
		{
			int id = event[idxd].id();
			if (quiet == false)
				PrintParticle(idxd);
			if (abs(id) >= minID && abs(id) <= maxID)
			{
				if (quiet == false)
				{
					std::cout << "    -> selected daughter" << std::endl;
				}
				retval.push_back(idxd);
			}
		}
		return retval;
	}

	std::vector<int> FollowDaughters(const Pythia8::Event &event, int idx, int minID, int maxID, bool quiet)
	{
		std::vector<int> retval;
		std::vector<int> daughters  = GetDaughters(event, idx, minID, maxID, quiet);
		for (unsigned int i = 0; i < daughters.size(); i++)
		{
			if (quiet == false)
				PrintParticle(event, daughters[i]);
			retval.push_back(daughters[i]);
			std::vector<int> subds = FollowDaughters(event, daughters[i], minID, maxID, quiet);
			for (unsigned int id = 0; id < subds.size(); id++)
			{
				retval.push_back(subds[id]);
			}
		}
		return retval;
	}

	std::vector<std::pair <double, double>> make_pThat_bins(std::vector<double> ptbins)
	{
		std::vector<std::pair<double, double>> retv;
		for (unsigned int i = 0; i < ptbins.size(); i++)
		{
			double ptlow = ptbins[i];
			double pthigh = -1.0;
			if (i + 1 < ptbins.size())
			{
				pthigh = ptbins[i + 1];
			}
			auto pair = std::make_pair(ptlow, pthigh);
			retv.push_back(pair);
		}
		return retv;
	}

	std::vector<double> hard_bins_from_string(const std::string &s) //const SysUtil::Args &args)
	{
		SysUtil::Args args(s);
		std::string sbins = args.get("--pTHatBins");
		auto vsbins = SysUtil::Args::breakup(sbins.c_str(), ',');
		std::vector<double> v;
		for (auto s : vsbins)
		{
			if (s.size() < 1) continue;
			double sd = StrUtil::str_to_double(s.c_str(), -1);
			v.push_back(sd);
		}
		return v;
	}

	bad_mothers_exception not_so_bad_moms;
	double delta_pz_mother(const int i, const Pythia8::Event &event, const int maxmothers)
	{
		auto p          = event[i];
		double delta_pz = p.pz();
		auto mothers    = p.motherList();
		std::cout << "mothers of " << i << " " << p.id() << " " << p.name() << " " << p.status() << std::endl;
		if (mothers.size() > maxmothers)
		{
			throw not_so_bad_moms;
		}
		for (auto &im : mothers)
		{
			auto &m = event[im];
			std::cout << " - " << im << " " << m.id() << " " << m.name() << " " << m.status() << std::endl;
			delta_pz -= m.pz();
		}
		return delta_pz;
	}

	std::string part_to_str(const Pythia8::Particle &p)
	{
		std::ostringstream ss;
		ss << p.id() << " " << p.name() << " pt=" << p.pT() << " pz=" << p.pz();
		return ss.str();
	}

	int has_beam_mother(const Pythia8::Particle &p)
	{
		int im_sum   = 0;
		auto mothers = p.motherList();
		for (auto &im : mothers)
		{
			if (im == 1 || im == 2)
				im_sum += im;
		}
		return im_sum;
	}

};
