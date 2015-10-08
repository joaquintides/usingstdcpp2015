/* usingstdcpp2015: matrix sum.
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
#include <boost/multi_array.hpp>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
  std::size_t n0=10000,n1=40000000,dn=2000;
  double      fdn=1.1;    

  std::cout<<"matrix sum:"<<std::endl;
  std::cout<<"n;row_col;col_row"<<std::endl;
    
  for(std::size_t n=n0;n<=n1;n+=dn,dn=(unsigned int)(dn*fdn)){
    std::size_t               m=static_cast<std::size_t>(std::sqrt(n));
    boost::multi_array<int,2> a(boost::extents[m][m]);

    /* fill with some values */
    for(std::size_t i=0;i<m;++i){
      for(std::size_t j=0;j<m;++j){
        a[i][j]=i+j;
      }
    }

    std::cout<<m*m<<";";
    std::cout<<measure(m*m,[&](){
      long int res=0;
      for(std::size_t i=0;i<m;++i){
        for(std::size_t j=0;j<m;++j){
          res+=a[i][j];
        }
      }
      return res;
    })<<";";

    std::cout<<measure(m*m,[&](){
      long int res=0;
      for(std::size_t j=0;j<m;++j){
        for(std::size_t i=0;i<m;++i){
          res+=a[i][j];
        }
      }
      return res;
    })<<"\n";
  }
}
