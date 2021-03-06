#ifndef __SYSUTIL__HH
#define __SYSUTIL__HH

#include <string>
#include <sys/stat.h>

namespace SysUtil
{
	bool file_exists (const std::string& name);
}

#endif
