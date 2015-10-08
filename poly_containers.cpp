/* usingstdcpp2015: processing polymorphic containers.
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
#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <typeindex>
#include <type_traits>
#include <vector>

template<class Base>
class poly_collection_segment_base
{
public:
  virtual ~poly_collection_segment_base(){};

  void insert(const Base& x)
  {
    this->insert_(x);
  }

  template<typename F>
  void for_each(F& f)
  {
    std::size_t s=this->element_size_();
    for(auto it=this->begin_(),end=it+this->size_()*s;it!=end;it+=s){
      f(*reinterpret_cast<Base*>(it));
    }
  }
  
  template<typename F>
  void for_each(F& f)const
  {
    std::size_t s=this->element_size_();
    for(auto it=this->begin_(),end=it+this->size_()*s;it!=end;it+=s){
      f(*reinterpret_cast<const Base*>(it));
    }
  }

private:  
  virtual void insert_(const Base& x)=0;
  virtual char* begin_()=0;
  virtual const char* begin_()const=0;
  virtual std::size_t size_()const=0;
  virtual std::size_t element_size_()const=0;
};

template<class Derived,class Base>
class poly_collection_segment:
  public poly_collection_segment_base<Base>
{
private:
  virtual void insert_(const Base& x)
  {
    store.push_back(static_cast<const Derived&>(x));
  }

  virtual char* begin_()
  {
    return reinterpret_cast<char*>(
      static_cast<Base*>(const_cast<Derived*>(store.data())));
  }

  virtual const char* begin_()const
  {
    return reinterpret_cast<const char*>(
      static_cast<const Base*>(store.data()));
  }

  virtual std::size_t size_()const{return store.size();}
  virtual std::size_t element_size_()const{return sizeof(Derived);}

  std::vector<Derived> store;
};

template<class Base>
class poly_collection
{
public:
  template<class Derived>
  void insert(
    const Derived& x,
    typename std::enable_if<std::is_base_of<Base,Derived>::value>::type* =0)
  {
    auto& pchunk=chunks[typeid(x)];
    if(!pchunk)pchunk.reset(new poly_collection_segment<Derived,Base>());
    pchunk->insert(x);
  }
 
  template<typename F>
  F for_each(F f)
  {
    for(const auto& p:chunks)p.second->for_each(f);
    return std::move(f);
  }

  template<typename F>
  F for_each(F f)const
  {
    for(const auto& p:chunks)
      const_cast<const segment&>(*p.second).for_each(f);
    return std::move(f);
  }

private:
  typedef poly_collection_segment_base<Base> segment;
  typedef std::unique_ptr<segment>           pointer;

  std::map<std::type_index,pointer> chunks;
};

struct base
{
  virtual int f()const=0;
  virtual ~base(){}
};

struct derived1:base
{
  virtual int f()const{return 1;};  
};

struct derived2:base
{
  virtual int f()const{return 2;};  
};

struct derived3:base
{
  virtual int f()const{return 3;};  
};

int main()
{
  std::size_t n0=10000,n1=40000000,dn=2000;
  double      fdn=1.1;    

  std::cout<<"polymorphic containers:"<<std::endl;
  std::cout<<"n;unsorted;sorted;poly_collection"<<std::endl;
    
  for(std::size_t n=n0;n<=n1;n+=dn,dn=(unsigned int)(dn*fdn)){
    std::cout<<n<<";";
    {
      using pointer=std::shared_ptr<base>;
      std::vector<pointer>            v;
      std::mt19937                    gen;
      std::uniform_int_distribution<> rnd(1,3);
      v.reserve(n);
      for(std::size_t i=0;i<n;++i){
        switch(rnd(gen)){
          case 1:  v.push_back(std::make_shared<derived1>());break;
          case 2:  v.push_back(std::make_shared<derived2>());break;
          case 3: 
          default: v.push_back(std::make_shared<derived3>());break;
        }
      }
      std::shuffle(v.begin(),v.end(),gen);

      auto f=[&](){
        long int res=0;
        for(const auto& p:v)res+=p->f();
        return res;
      };

      std::cout<<measure(n,f)<<";";
      std::sort(v.begin(),v.end(),[](const pointer& p,const pointer& q){
        return std::type_index(typeid(*p))<std::type_index(typeid(*q));
      });
      std::cout<<measure(n,f)<<";";
    }
    {
      poly_collection<base>           v;
      std::mt19937                    gen;
      std::uniform_int_distribution<> rnd(1,3);
      for(std::size_t i=0;i<n;++i){
        switch(rnd(gen)){
          case 1:  v.insert(derived1());break;
          case 2:  v.insert(derived2());break;
          case 3: 
          default: v.insert(derived3());break;
        }
      }

      std::cout<<measure(n,[&](){
        long int res=0;
        v.for_each([&](const base& x){res+=x.f();});
        return res;
      })<<"\n";
    }
  }
}
