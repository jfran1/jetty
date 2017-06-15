#include <util/args.h>
#include "run_josh.h"
#include "run_cal.h"

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

    return rv;
}
