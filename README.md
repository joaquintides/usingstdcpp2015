using std::cpp 2015
-------------------

Presentation from Joaquín Mª López Muñoz at [using std::cpp 2015](http://usingstdcpp.org/using-stdcpp-2015/) and associated material.
* [Mind the cache](https://github.com/joaquintides/usingstdcpp2015/raw/master/Mind%20the%20cache.pdf) ([video](https://www.youtube.com/watch?v=TipTVUGBFtY) in Spanish): Classical big-O algorithmic complexity analysis proves insufficient to estimate program performance for modern computer architectures: current processors are equipped with several low-level components (hierarchical cache structures, pipelining, branch prediction) that greatly favor certain code and data layout patterns not taken into account by naïve computation models. In this talk we see some examples of the impact these factors have and provide suggestions for performance improvement based on data locality and regularity in code execution.
