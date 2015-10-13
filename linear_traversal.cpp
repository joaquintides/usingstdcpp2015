/* usingstdcpp2015: linear traversal performance for two std containers.
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
#include <numeric>
#include <list>
#include <random>
#include <vector>

int main()
{
  std::size_t n0=10000,n1=40000000,dn=2000;
  double      fdn=1.1;    

  std::cout<<"linear traversal:"<<std::endl;
  std::cout<<"n;vector;list;shuffled list"<<std::endl;
    
  for(std::size_t n=n0;n<=n1;n+=dn,dn=(unsigned int)(dn*fdn)){
    std::cout<<n<<";";
    {
      std::vector<int> v(n);
      std::iota(v.begin(),v.end(),0);
      std::cout<<measure(n,[&](){
        return std::accumulate(v.begin(),v.end(),0);
      })<<";";
    }
    {
      std::list<int> l(n);
      std::iota(l.begin(),l.end(),0);
      std::cout<<measure(n,[&](){
        return std::accumulate(l.begin(),l.end(),0);
      })<<";";
    }
    {
      std::mt19937                    gen;
      std::uniform_int_distribution<> rnd(0,n-1);
      std::list<int>                  l;
      for(std::size_t i=0;i<n;++i)l.push_back(rnd(gen));
      l.sort();
      std::iota(l.begin(),l.end(),0);
      std::cout<<measure(n,[&](){
        return std::accumulate(l.begin(),l.end(),0);
      })<<"\n";
    }
  }
}
