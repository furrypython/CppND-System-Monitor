#ifndef PROCESSPARSER_H
#define PROCESSPARSER_H

#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>

#include "constants.h"
#include "util.h"

class ProcessParser{
public:
    static std::string getCmd(std::string pid);
    static std::vector<std::string> getPidList();
    static std::string getVmSize(std::string pid);
    static std::string getCpuPercent(std::string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(std::string pid);
    static std::string getProcUser(std::string pid);
    static std::vector<std::string> getSysCpuPercent(std::string coreNumber = "");
    static float getSysRamPercent();
    static std::string getSysKernelVersion();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfCores();
    static int getNumberOfRunningProcesses();
    static std::string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
};


std::string ProcessParser::getVmSize(std::string pid) {
    std::string line;
    //Declaring search attribute for file
    std::string name = "VmData";
    //std::string value;
    float result;
    // Opening stream for specific file
    std::ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
    // Get each line from the stream
    while(std::getline(stream, line)){
        // Searching line by line
        // Compares 6 characters from index number 0 of line with name (= VmData)
        if (line.compare(0, name.size(), name) == 0) {
            // slicing string line on white pace for values using sstream
            // Form an input string stream buffer from that line
            std::istringstream buf(line);
            // Declare begin and end iterators for the string stream
            std::istream_iterator<std::string> beg(buf), end;
            std::vector<std::string> values(beg, end);
            //conversion kB -> GB
            result = (stof(values[1])/float(1024));
            break;
        }
    }
    return std::to_string(result);
}

std::string ProcessParser::getCpuPercent(std::string pid) {
    std::string line;
    float result;
    std::ifstream stream = Util::getStream((Path::basePath() + pid + "/" + Path::statPath()));
    std::getline(stream, line);
    std::string str = line;
    std::istringstream buf(str);
    std::istream_iterator<std::string> beg(buf), end;
    std::vector<std::string> values(beg, end);

    // acquiring relevant times for calculation of active occupation of CPU for selected process
    float utime = std::stof(ProcessParser::getProcUpTime(pid));
    float stime = std::stof(values[14]);
    float cutime = std::stof(values[15]);
    float cstime = std::stof(values[16]);
    float starttime = std::stof(values[21]);
    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    result = 100.0*((total_time/freq)/seconds);
    return std::to_string(result);
}

std::string ProcessParser::getProcUpTime(std::string pid) {
  std::string line;
  float result;
  std::ifstream stream = Util::getStream((Path::basePath() + pid + "/" + Path::statPath()));
  std::getline(stream, line);
  std::istringstream buf(line);
  std::istream_iterator<std::string> beg(buf), end;
  std::vector<std::string> values(beg, end);
  // Using sysconf to get clock ticks of the host machine
  result = float(stof(values[13])/sysconf(_SC_CLK_TCK));

  return std::to_string(result);
}

long int ProcessParser::getSysUpTime() {
    std::string line;
    long int result;
    std::ifstream stream = Util::getStream((Path::basePath() + Path::upTimePath()));
    std::getline(stream, line);
    std::istringstream buf(line);
    std::istream_iterator<std::string> beg(buf), end;
    std::vector<std::string> values(beg, end);
    // stoi converts a string to an int.
    result = stoi(values[0]);
    return result;
}

std::string ProcessParser::getProcUser(std::string pid) {
  std::string line;
  //Declaring search attribute for file
  std::string name = "Uid:";
  std::string result = "";

  std::ifstream stream =  Util::getStream(Path::basePath() + pid + Path::statusPath());
  while(std::getline(stream, line)) {
    if(line.compare(0, name.size(), name) == 0) {
      std::istringstream buf(line);
      std::istream_iterator<std::string> beg(buf), end;
      std::vector<std::string> values(beg, end);
      result = values[1];
      break;
    }
  }

  stream = Util::getStream("/etc/passwd");
  name =("x:" + result);
  while(std::getline(stream, line)) {
      if(line.find(name) != std::string::npos) {
        result = line.substr(0, line.find(":"));
        return result;
      }
  }
  return "";
}

std::vector<std::string> ProcessParser::getPidList() {
    DIR* dir;
    // Scan /proc dir for all directories with numbers as their names
    // If we get valid check we store dir names in vector as list of machine pids
    std::vector<std::string> container;
    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));

    while (dirent* dirp = readdir(dir)) {
    // is this a directory?
        if(dirp->d_type != DT_DIR)
            continue;

        // Is every character of the name a digit?
        // This statement is skipped each time dirp->d_type != DT_DIR
        if (std::all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })) {
            container.push_back(dirp->d_name);
        }
    }

    //Validating process of directory closing
    if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));

    return container;
}

// Retrieve the command that executed the process.
std::string ProcessParser::getCmd(std::string pid) {
    std::string line;
    std::ifstream stream = Util::getStream((Path::basePath() + pid + Path::cmdPath()));
    std::getline(stream, line);
    return line;
}

// Retrieve the number of CPU cores on the host.
int ProcessParser::getNumberOfCores() {
    // Get the number of host cpu cores
    std::string line;
    std::string name = "cpu cores";
    std::ifstream stream = Util::getStream((Path::basePath() + "cpuinfo"));
    while(std::getline(stream, line)) {
      if(line.compare(0, name.size(), name) == 0) {
          std::istringstream buf(line);
          std::istream_iterator<std::string> beg(buf), end;
          std::vector<std::string> values(beg, end);
          return std::stoi(values[3]);
      }
    }
    return 0;
}

std::vector<std::string> ProcessParser::getSysCpuPercent(std::string coreNumber) {
    // It is possible to use this method for selection of data for overall cpu or every core.
    // when nothing is passed "cpu" line is read
    // when, for example "0" is passed  -> "cpu0" -> data for first core is read
  std::string line;
  std::string name = "cpu" + coreNumber;
  std::string value;
  int result;
  std::ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
  while (std::getline(stream, line)) {
    if (line.compare(0, name.size(), name) == 0) {
      std::istringstream buf(line);
      std::istream_iterator<std::string> beg(buf), end;
      std::vector<std::string> values(beg, end);
      // set of cpu data active and idle times;
      return values;
    }
  }
  return std::vector<std::string>();
}

float getSysActiveCpuTime(std::vector<std::string> values) {
    return (std::stof(values[S_USER]) +
            std::stof(values[S_NICE]) +
            std::stof(values[S_SYSTEM]) +
            std::stof(values[S_IRQ]) +
            std::stof(values[S_SOFTIRQ]) +
            std::stof(values[S_STEAL]) +
            std::stof(values[S_GUEST]) +
            std::stof(values[S_GUEST_NICE]));
}

float getSysIdleCpuTime(std::vector<std::string> values) {
    return (std::stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string> values2) {
  /*
    Because CPU stats can be calculated only if you take measures in two different time,
    this function has two paramaters: two vectors of relevant values.
    We use a formula to calculate overall activity of processor.
  */
  float active_time = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
  float idle_time = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
  float total_time = active_time + idle_time;
  float result = 100.0 * (active_time / total_time);
  return std::to_string(result);
}

float ProcessParser::getSysRamPercent() {
  std::string line;
  std::string name1 = "MemAvailable:";
  std::string name2 = "MemFree:";
  std::string name3 = "Buffers:";

  std::ifstream stream = Util::getStream((Path::basePath() + Path::memInfoPath()));
  float total_mem = 0;
  float free_mem = 0;
  float buffers = 0;
  while(std::getline(stream, line)) {
    if(total_mem != 0 && free_mem != 0)
      break;

    if(line.compare(0, name1.size(), name1) == 0) {
      std::istringstream buf(line);
      std::istream_iterator<std::string> beg(buf), end;
      std::vector<std::string> values(beg, end);
      total_mem = stof(values[1]);
    }

    if(line.compare(0, name2.size(), name2) == 0) {
      std::istringstream buf(line);
      std::istream_iterator<std::string> beg(buf), end;
      std::vector<std::string> values(beg, end);
      free_mem = stof(values[1]);
    }

    if(line.compare(0, name3.size(), name3) == 0) {
      std::istringstream buf(line);
      std::istream_iterator<std::string> beg(buf), end;
      std::vector<std::string> values(beg, end);
      buffers = stof(values[1]);
    }
  }
  //calculating usage:
  return float(100.0 * (1 - (free_mem / (total_mem - buffers))));
}

std::string ProcessParser::getSysKernelVersion() {
  std::string line;
  std::string name = "Linux version ";
  std::ifstream stream = Util::getStream((Path::basePath() + Path::versionPath()));
  while(std::getline(stream, line)) {
    if(line.compare(0, name.size(), name) == 0) {
      std::istringstream buf(line);
      std::istream_iterator<std::string>  beg(buf), end;
      std::vector<std::string> values(beg, end);
      return values[2];
    }
  }
  return "";
}

std::string ProcessParser::getOSName() {
  std::string line;
  std::string name = "PRETTY_NAME";

  std::ifstream stream = Util::getStream(("/etc/os-release"));

  while(std::getline(stream, line)) {
    if(line.compare(0, name.size(), name) == 0) {
      std::size_t found = line.find("=");
      found++;
      std::string result = line.substr(found);
      result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
      return result;
    }
  }
  return "";
}

// Calculate the total thread count
int ProcessParser::getTotalThreads() {
  std::string line;
  int result = 0;
  std::string name = "Threads:";
  std::vector<std::string>_list = ProcessParser::getPidList();
  for (int i = 0 ; i < _list.size(); i++) {
    std::string pid = _list[i];
    //getting every process and reading their number of their threads
    std::ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
    while(std::getline(stream, line)) {
      if(line.compare(0, name.size(), name) == 0) {
        std::istringstream buf(line);
        std::istream_iterator<std::string> beg(buf), end;
        std::vector<std::string> values(beg, end);
        result += std::stoi(values[1]);
        break;
      }
    }
  }
  return result;
}

int ProcessParser::getTotalNumberOfProcesses() {
  std::string line;
  int result = 0;
  std::string name = "processes";
  std::ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
  while(std::getline(stream, line)) {
    if(line.compare(0, name.size(), name) == 0) {
      std::istringstream buf(line);
      std::istream_iterator<std::string> beg(buf), end;
      std::vector<std::string> values(beg, end);
      result += std::stoi(values[1]);
      break;
    }
  }
  return result;
}

int ProcessParser::getNumberOfRunningProcesses() {
  std::string line;
  int result = 0;
  std::string name = "procs_running";
  std::ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
  while(std::getline(stream, line)) {
    if(line.compare(0, name.size(), name) == 0) {
      std::istringstream buf(line);
      std::istream_iterator<std::string> beg(buf), end;
      std::vector<std::string> values(beg, end);
      result += std::stoi(values[1]);
      break;
    }
  }
  return result;
}

#endif // PROCESSPARSER_H
