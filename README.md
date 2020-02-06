G-PhoCS
=======

[G-PhoCS](https://github.com/gphocs-dev/G-PhoCS) iis a software package for inferring ancestral population sizes, population divergence times, and migration rates from individual genome sequences, developed by Dr. Ilan Gronau from the IDC Herzliya.
As a research assistance of Dr. Ilan Gronau, I developed the next version of G-PhoCS, v140 (not released yet).

The main updates in version v140 include of redesigning fundamental data structures. New design is OOP oriented.

Below is an UML diagram shows the high level picture of the relationships between the new classes. In general, new data structure consists of two main components: genealogy and intervals chains (i.e., chains of events).


UML diagram of classes I created:

<p align="center">
  <img width="800" height="600" src="UML diagram.png">
</p>


New files in version v140:
---------------

AllLoci [.h](src/AllLoci.h)/[.cpp](src/AllLoci.cpp) <br>
GenealogyStats [.h](src/GenealogyStats.h) <br>
LocusEmbeddedGenealogy [.h](src/LocusEmbeddedGenealogy.h)/[.cpp](src/LocusEmbeddedGenealogy.cpp) <br>
LocusGenealogy [.h](src/LocusGenealogy.h)/[.cpp](src/LocusGenealogy.cpp) <br>
LocusPopIntervals [.h](src/LocusPopIntervals.h)/[.cpp](src/LocusPopIntervals.cpp) <br>
PopInterval [.h](src/PopInterval.h)/[.cpp](src/PopInterval.cpp) <br>
TreeNode [.h](src/TreeNode.h)/[.cpp](src/TreeNode.cpp) <br>


