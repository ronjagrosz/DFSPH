// SPH.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"


#include <iostream>

#include "util/uVect.h"
#include "render/ogl.h"
#include "particle/sp.h"

using namespace std;

int main(int argc, char* argv[])
{
	ogl sph;

	sph.Start(argc, argv);

	return 0;
}

