//Loading an MD simulation "using systemMD.h".
//Everything needed to do a simulation is already included in systemMD.h.
//Even standard libraries like iostream and fstream are inlcuded.
//But, if any extra libraries are needed you can include them here or in systemMD.h.
//Permenent libraries (included with standard library or with this package) should
// go in systemMD.h, and anything you need that is external should go in this file.

//For molecular dynamics forces and potentials
#include "../include/MD.h"

//For the molecular dynamics variables
#include "../include/system.h"

int main(int argc, char **argv)
{
	if(argc!=5)
	{
		std::cout << argv[0] << " name centerType sliceSize dAngle\n";
		return 0;
	}

	std::stringstream cmdArg;
	cmdArg << argv[2] << ' ' << argv[3] << ' ' << argv[4];
	int centerType;
	double sliceSize,dAngle;
	cmdArg >> centerType >> sliceSize >> dAngle;

	//the variables for the simulation, call with empty constructor
	Blob<double> System;
	
	//file input-output script engine
	//name part (argv[1] in this case) is used with no extension (*.mpd)
	Script<double, Blob <double> > fileIO(argv[1],std::ios::in,&System);
	
	//read the variables in
	fileIO.read();
	
	//close the file that it is reading from
	fileIO.close();
	
	std::vector< std::map<int,double> > profile;
	for(int i=0;i<System.readNTypes();i++)
		profile.push_back(std::map<int,double>());
	std::vector< std::map<int,double> > count;
	for(int i=0;i<System.readNTypes();i++)
		count.push_back(std::map<int,double>());
	
	threeVector<double> size=System.readSize();
	position<double> *p=System.getPositions();
	int nParticles=System.readNParticles();
	threeVector<double> com=0;
	position<double> anch=p[0];
	std::vector<position<double> > sp;
	for(int i=0;i<nParticles;i++)
	{
		threeVector<double> d;
		position<double> pt=p[i];
		d.x=pt.x-anch.x;
		d.y=pt.y-anch.y;
		d.z=pt.z-anch.z;
		if(d.x>size.x) pt.x-=size.x;
		if(d.y>size.y) pt.y-=size.y;
		if(d.z>size.z) pt.z-=size.z;
		if(d.x<-size.x) pt.x+=size.x;
		if(d.y<-size.y) pt.y+=size.y;
		if(d.z<-size.z) pt.z+=size.z;
		com.x+=pt.x;
		com.y+=pt.y;
		com.z+=pt.z;
		sp.push_back(pt);
	}
	
	com.x/=double(nParticles);
	com.y/=double(nParticles);
	com.z/=double(nParticles);
	
	threeVector<double> max=0;
	threeVector<double> min=sqrt(size.x*size.x+size.y*size.y+size.z*size.z);
	for(int i=0;i<nParticles;i++)
	{
		sp[i].x-=com.x;
		sp[i].y-=com.y;
		sp[i].z-=com.z;
		position<double> temp=sp[i];
		temp.x=sqrt(sp[i].x*sp[i].x+sp[i].y*sp[i].y+sp[i].z*sp[i].z); //rho
        	temp.y=acos(sp[i].z/temp.x); //theta
	        temp.z=atan2(sp[i].y,sp[i].x); //phi
		if(temp.x<min.x) min.x=temp.x;
		if(temp.y<min.y) min.y=temp.y;
		if(temp.z<min.z) min.z=temp.z;
		if(temp.x>max.x) max.x=temp.x;
		if(temp.y>max.x) max.y=temp.y;
		if(temp.z>max.x) max.z=temp.z;
		sp[i]=temp;
	}
	
	std::vector< std::vector< position<double> > > cellList;
	std::vector<double > avgR;
	threeVector<int> n;
	n.x=floor(max.x/sliceSize);//rho
	n.y=floor(M_PI/dAngle);//theta
	n.z=floor((2.0*M_PI)/dAngle);//phi
	
	threeVector<double> r;
	r.x=max.x/static_cast<double>(n.x);//rho
	r.y=M_PI/static_cast<double>(n.y);//theta
	r.z=(2.0*M_PI)/static_cast<double>(n.z);//phi
	
	for(int i=0;i<nParticles;i++)
	{
		threeVector<int> c;
		c.x=floor(sp[i].x/r.x);//rho
		c.y=floor(sp[i].y/r.y);//theta
		c.z=floor((sp[i].z+M_PI)/r.z);//phi
		int hash=c.y+c.z*n.y;//only theta and phi
		while(cellList.size()<=hash)
			cellList.push_back(std::vector< position<double> >());
		while(avgR.size()<=hash)
			avgR.push_back(0);
		cellList[hash].push_back(sp[i]);
	}
	
	for(int i=0;i<cellList.size();i++)
	{
		for(int j=0;j<cellList[i].size();j++)
			avgR[i]+=cellList[i][j].x;
		avgR[i]/=static_cast<double>(cellList[i].size());
	}
		
	
	for(int i=0;i<cellList.size();i++)
	{
		for(int j=0;j<cellList[i].size();j++)
		{
			int pIndex=floor((cellList[i][j].x-avgR[i])/sliceSize);
			auto pFind=profile[cellList[i][j].type].find(pIndex);
			if(pFind==profile[cellList[i][j].type].end())
			{
				profile[cellList[i][j].type][pIndex]=0;
				count[cellList[i][j].type][pIndex]=0;
			}
			count[cellList[i][j].type][pIndex]+=1.0;
			profile[cellList[i][j].type][pIndex]+=1.0/(1.33333333*M_PI*(pow(cellList[i][j].x+sliceSize,3.0)-pow(cellList[i][j].x,3.0)));
		}
	}

	for(int i=0;i<profile.size();i++)
	{
		for(auto j:profile[i])
		{
			//if(count[i][j.first]!=0)
				std::cout << (static_cast<double>(j.first)*sliceSize) << '\t' << j.second << '\n';
			//else
			//	std::cout << (static_cast<double>(j)*sliceSize) << '\t' << 0.0 << '\n';
		}		
		std::cout << '\n';
	}

	return 0;
}
