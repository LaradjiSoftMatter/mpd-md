#include "include.h"

int main(int argc, char **argv)
{
	if(argc<6)
	{
		std::cout << "Usage: " << argv[0] << " name seed nLipids arealDensity lengthRadiusRatio" << std::endl;
		return 0;
	}

	char *name=argv[1];
	int nLipids=atoi(argv[3]);
	double arealDensity=atof(argv[4]);
	double lengthRadiusRatio=atof(argv[5]);
	
	///the variables for the simulation
	Blob<double> System;
	
	System.setGamma(1.0);
	System.setNTypes(6);
	System.setSeed(atoi(argv[2]));
	
	threeVector<bool> wrap;//periodic boundaries flag, be forwarned, this doesn't work yet, it is always on
	wrap.x=true;
	wrap.y=true;
	wrap.z=true;
	System.setPeriodic(wrap);
	System.setCutoff(CUTOFF);
	
	System.setInitialTime(0);
	System.setFinalTime(50000);
	System.setDeltaT(0.02);
	System.setStoreInterval(10);
	System.setMeasureInterval(10);
	//System.setDeltaLXY(0.01);//if you don't set it, it doesn't activate
	System.setInitialTemp(3.0);
	System.setFinalTemp(3.0);
	
	///initialize constants (force and potential constants)
	//generate force constants
	std::vector<double> Umax(System.readNTypes()*System.readNTypes());
	std::vector<double> Umin(System.readNTypes()*System.readNTypes());
	
	//could be one loop, but it's good for an example
	//of how to set constants
	for(int i=0;i<System.readNTypes();i++)
	{
		for(int j=0;j<System.readNTypes();j++)
		{
			//i=first type, j=second type, indexed grid
			int k=i+j*System.readNTypes();
			Umin[k]=0;
			Umax[k]=100;
		}
	}
	
	//umin and umax exceptions, tail types
	Umin[TAIL+TAIL*System.readNTypes()]=-6;
	Umax[TAIL+TAIL*System.readNTypes()]=200;
	
	//two body constants
	std::vector<double> twoBodyFconst=mpd::laradjiRevaleeFC(Umax,Umin,CUTOFF,RMIN);
	for(auto &v:twoBodyFconst)
		System.addTwoBodyFconst(v);
	
	std::vector<double> twoBodyUconst=mpd::laradjiRevaleePC(Umax,Umin,CUTOFF,RMIN);
	for(auto &v:twoBodyUconst)
		System.addTwoBodyUconst(v);
	
	//Make it zero
	threeVector<double> size;
	size.x=0;
	size.y=0;
	size.z=0;
	System.setSize(size);
	
	//just place it far away, in a corner
	threeVector<double> pos;
	pos.x=0;
	pos.y=0;
	pos.z=0;
	
	double lipidConstants[4];
	lipidConstants[0]=0.7;
	lipidConstants[1]=100;
	lipidConstants[2]=1.0;
	lipidConstants[3]=100;
	
	//Add tube
	double radius=tube(System, nLipids, 3, pos, 0.7, arealDensity, lipidConstants, 4, lengthRadiusRatio);
	
	///Write configuration
	Script<double,Blob <double> > output(name,std::ios::out,&System);
	output.write();
	
	return 0;
}
