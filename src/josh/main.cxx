#include <util/args.h>
#include "run_josh.h"
#include "run_cal.h"
#include "run_dog.h"
#include "run_Alice.h"
#include "run_fastjet.h"
#include "run_chargedHadron.h"

int main ( int argc, char *argv[] )
{
	int rv = 0;

    SysUtil::Args args(argc, argv);

    if (args.isSet("--josh"))
    {
    	rv = run_josh(args.asString());
    }

    if (args.isSet("--cal"))
    {
    	rv = run_cal(args.asString());
    }

    if (args.isSet("--dog"))
    {
    	rv = run_dog(args.asString());
    }

    if (args.isSet("--Alice"))
    {
        rv = run_Alice(args.asString());
    }

    if (args.isSet("--fastjet"))
    {
        rv = run_fastjet(args.asString());
    }

        if (args.isSet("--chargedHadron"))
    {
        rv = run_chargedHadron(args.asString());
    }


    return rv;
}
