/* usingstdcpp2015: AOS vs SOA.
 *
 * Copyright 2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
 
#include <algorithm>
#include <array>
#include <chrono>
#include <numeric> 
    
std::chrono::high_resolution_clock::time_point measure_start,measure_pause;
        
template<typename F>
double measure(F f)
{
  using namespace std::chrono;
        
  static const int              num_trials=10;
  static const milliseconds     min_time_per_trial(200);
  std::array<double,num_trials> trials;
  volatile decltype(f())        res; /* to avoid optimizing f() away */
        
  for(int i=0;i<num_trials;++i){
    int                               runs=0;
    high_resolution_clock::time_point t2;
        
    measure_start=high_resolution_clock::now();
    do{
      res=f();
      ++runs;
      t2=high_resolution_clock::now();
    }while(t2-measure_start<min_time_per_trial);
    trials[i]=duration_cast<duration<double>>(t2-measure_start).count()/runs;
  }
  (void)(res); /* var not used warn */
        
  std::sort(trials.begin(),trials.end());
  return std::accumulate(
    trials.begin()+2,trials.end()-2,0.0)/(trials.size()-4)*1E6;
}
 
template<typename Size,typename F>
double measure(Size n,F f)
{
  return measure(f)/n;
}

void pause_timing()
{
  measure_pause=std::chrono::high_resolution_clock::now();
}
        
void resume_timing()
{
  measure_start+=std::chrono::high_resolution_clock::now()-measure_pause;
}

#include <algorithm>
#include <iostream>
#include <vector>

struct particle
{
  int x,y,z;
  int dx,dy,dz;
};

using particle_aos=std::vector<particle>;

particle_aos create_particle_aos(int n)
{
  particle_aos res;
  res.reserve(n);
  for(int i=0;i<n;++i)res.push_back({i,i+1,i+2,i+3,i+4,i+5});
  return res;
}

struct particle_soa
{
  std::vector<int> x,y,z;
  std::vector<int> dx,dy,dz;
};

particle_soa create_particle_soa(int n)
{
  particle_soa res;
  res.x.reserve(n);
  res.y.reserve(n);
  res.z.reserve(n);
  res.dx.reserve(n);
  res.dy.reserve(n);
  res.dz.reserve(n);
  for(int i=0;i<n;++i){
    res.x.push_back(i);
    res.y.push_back(i+1);
    res.z.push_back(i+2);
    res.dx.push_back(i+3);
    res.dy.push_back(i+4);
    res.dz.push_back(i+5);
  }
  return res;
}

int main()
{
  std::size_t n0=10000,n1=40000000,dn=2000;
  double      fdn=1.1;    

  std::cout<<"aos vs soa:"<<std::endl;
  std::cout<<"n;aos;soa"<<std::endl;
    
  for(std::size_t n=n0;n<=n1;n+=dn,dn=(unsigned int)(dn*fdn)){
    std::cout<<n<<";";
    {
      auto ps=create_particle_aos(n);
      std::cout<<measure(n,[&](){
        long int res=0;
        for(std::size_t i=0;i<n;++i)res+=ps[i].x+ps[i].y+ps[i].z;
        return res;
      })<<";";
    }
    {
      auto ps=create_particle_soa(n);
      std::cout<<measure(n,[&](){
        long int res=0;
        for(std::size_t i=0;i<n;++i)res+=ps.x[i]+ps.y[i]+ps.z[i];
        return res;
      })<<"\n";
    }
  }
}
