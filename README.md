# fastsearch

A simple and multithreaded file search utility written in C, fastsearch is designed for user and script usage in a Unix environment.
It creates the maximum possible number of system threads that it can use for efficiency.
It returns one found location of the filename per line, beginning its search from the current working directory.
Filename comparison is exact-match only.
