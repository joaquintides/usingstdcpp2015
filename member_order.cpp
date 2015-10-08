/* usingstdcpp2015: effect of member order on performance.
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
#include <random>
#include <set>

struct value1
{
  value1(std::size_t id,int value):id(id),value(value),other_data(""){}
  std::size_t id;
  int         value;
  char        other_data[8];
};

bool operator<(const value1&x, const value1& y){return x.id<y.id;}

struct value2
{
  value2(std::size_t id,int value):other_data(""),id(id),value(value){}
  char        other_data[128];
  std::size_t id;
  int         value;
};

bool operator<(const value2&x, const value2& y){return x.id<y.id;}

template<typename Value>
std::set<Value> create_set(std::size_t n)
{
  std::set<Value> res;
  for(std::size_t i=0;i<n;++i)res.emplace(i,2*i);
  return res;
}

int main()
{
  std::size_t n0=1000000,n1=40000000,dn=2000;
  double      fdn=1.1;    

  std::cout<<"member_order:"<<std::endl;
  std::cout<<"n;value1;value2"<<std::endl;
    
  for(std::size_t n=n0;n<=n1;n+=dn,dn=(unsigned int)(dn*fdn)){
    std::cout<<n<<";";
    {
      auto s=create_set<value1>(n);
      std::cout<<measure(n,[&](){
        std::mt19937                   gen;
        std::uniform_int_distribution<
          std::size_t>                 rnd(0,n-1);
        long int                       res=0;
        for(std::size_t i=0;i<n;++i)res+=s.lower_bound({rnd(gen),0})->value;
        return res;
      })<<";";
    }
    {
      auto s=create_set<value2>(n);
      std::cout<<measure(n,[&](){
        std::mt19937                   gen;
        std::uniform_int_distribution<
          std::size_t>                 rnd(0,n-1);
        long int                       res=0;
        for(std::size_t i=0;i<n;++i)res+=s.lower_bound({rnd(gen),0})->value;
        return res;
      })<<"\n";
    }
  }
}
