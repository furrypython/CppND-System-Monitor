#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

enum CPUStates{
	S_USER = 1,
	S_NICE,
	S_SYSTEM,
	S_IDLE,
	S_IOWAIT,
	S_IRQ,
	S_SOFTIRQ,
	S_STEAL,
	S_GUEST,
	S_GUEST_NICE
};

class Path{

public:
    static std::string basePath() {
        return "/proc/";
    }
    static std::string cmdPath(){
        return "/cmdline";
    }
    static std::string statusPath(){
        return "/status";
    }
    static std::string statPath(){
        return "stat";
    }
    static std::string upTimePath(){
        return "uptime";
    }
    static std::string memInfoPath(){
        return "meminfo";
    }
    static std::string versionPath(){
        return "version";
    }
};

#endif
