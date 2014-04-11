#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cstdlib>
#include <iostream>
#include <cassert>
#include <string>
#include "MersenneTwister.h"

// ====================================================================
// ====================================================================

class ArgParser {

public:

  ArgParser() { DefaultValues(); }

  ArgParser(int argc, char *argv[]) {
    DefaultValues();
    // parse the command line arguments
    for (int i = 1; i < argc; i++) {
      if (argv[i] == std::string("-input")) {
	i++; assert (i < argc); 	
	// we need to separate the filename from the path
	// (we assume the vertex & fragment shaders are in the same directory)
	std::string filename = argv[i];
	// first, locate the last '/' in the filename
        size_t last = std::string::npos;  
        while (1) {
          int next = filename.find('/',last+1);
          if (next == std::string::npos) { break;
          }
          last = next;
        }
        if (last == std::string::npos) {
	  // if there is no directory in the filename
          input_file = filename;
          path = ".";
        } else {
	  // separate filename & path
          input_file = filename.substr(last+1,filename.size()-last-1);
          path = filename.substr(0,last);
        }
      } else if (argv[i] == std::string("-points")) {
	i++; assert (i < argc); 
	points = atoi(argv[i]);
      } else if (argv[i] == std::string("-iters")) {
	i++; assert (i < argc); 
	iters = atoi(argv[i]);
      } else if (argv[i] == std::string("-size")) {
	i++; assert (i < argc); 
	width = height = atoi(argv[i]);
      } else if (argv[i] == std::string("-cubes")) {
        cubes = true;
      } else {
	std::cout << "ERROR: unknown command line argument " 
		  << i << ": '" << argv[i] << "'" << std::endl;
	exit(1);
      }
    }
  }

  void DefaultValues() {
    points = 10000;
    iters = 3;
    width = 400;
    height = 400;
    cubes = 0;
    path = ".";
  }

  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  std::string input_file;
  std::string path;
  int points;
  int iters;
  int width;
  int height;
  int cubes;

  // default initialization
  MTRand mtrand;
};

#endif
