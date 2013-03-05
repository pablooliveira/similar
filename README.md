# Similar

Similar allows to find which files are similar inside a directory.

# Usage

Similar uses Xapian to index the files in ``[path]``.  It then computes the
relevance between the terms of every pair of files. A directed similarity graph
is built with a vertex for each indexed file. An edge ``A->B`` exists iff relevance
of B for the terms in A is higher than ``[threshold]``. similar returns the strong
connected components of the similarity graph.

~~~
usage: similar [path] [threshold]
 [path]      : the directory that contains the files to compare.
 [threshold] : cut-off percentage that decides if two files
               are similar (at 100 %, files must be really close to 
               be considered similar)
~~~ 

# Install Similar

## Dependencies

To build Similar you need to install the following libraries:

    * libxapian-dev 

    * libboost-dev

    * libboost-filesystem-dev 

    * libboost-graph-dev

    * libboost-system-dev

On Debian, you can type:

~~~
$ sudo apt-get install libxapian-dev libboost-dev libboost-filesystem-dev libboost-graph-dev libboost-system-dev 
~~~

## Building

After installing dependencies, just type

~~~
$ make
~~~
