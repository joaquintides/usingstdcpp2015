/* usingstdcpp2015: threads acting on shared memory.
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

#include <iostream>
#include <random>
#include <thread>
#include <vector>

int main()
{
  std::size_t n0=10000,n1=40000000,dn=2000;
  double      fdn=1.1;    

  std::cout<<"parallel count:"<<std::endl;
  std::cout<<"n;near;far"<<std::endl;
    
  for(std::size_t n=n0;n<=n1;n+=dn,dn=(unsigned int)(dn*fdn)){
    std::mt19937                    gen;
    std::uniform_int_distribution<> rnd(0,255);
    std::vector<int> v;
    v.reserve(n);
    
    /* fill with some values */
    for(std::size_t i=0;i<n;++i)v.push_back(rnd(gen));
    
    int res[49];

    auto f=[&](int* px,int* py,int* pz,int* pw){
      auto th=[](int* p,int* first,int* last){
        *p=0;
        while(first!=last){
          int x=*first++;
          *p+=x%2;
        }
      };

      std::thread t1(th,px,v.data(),v.data()+n/4);
      std::thread t2(th,py,v.data()+n/4,v.data()+n/2);
      std::thread t3(th,pz,v.data()+n/2,v.data()+n*3/4);
      std::thread t4(th,pw,v.data()+n*3/4,v.data()+n);
      t1.join();
      t2.join();
      t3.join();
      t4.join();
      return *px+*py+*pz+*pw;
    };

    std::cout<<n<<";";
    std::cout<<measure(n,[&](){
      return f(&res[0],&res[1],&res[2],&res[3]);
    })<<";";
    std::cout<<measure(n,[&](){
      return f(&res[0],&res[16],&res[32],&res[48]);
    })<<"\n";
  }
}
