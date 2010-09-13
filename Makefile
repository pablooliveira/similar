CC=gcc -O2
LIBS=-lboost_graph -lboost_system -lboost_filesystem -lxapian
INCLUDE=-I/usr/include/xapian
similar: similar.cc
	$(CC) -o similar similar.cc $(INCLUDE) $(LIBS)
