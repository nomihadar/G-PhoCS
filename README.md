G-PhoCS
=======

[G-PhoCS](https://github.com/gphocs-dev/G-PhoCS) is a software package for inferring ancestral population sizes, population divergence times, and migration rates from individual genome sequences, developed by **Dr. Ilan Gronau** from the IDC Herzliya. <br>
As a research assistance of Dr. Gronau, I developed the next version of G-PhoCS - v140 (not released yet).

The main updates in version v140 include redesigning fundamental data structures. New design is OOP oriented. <br>
Here is a [document file](https://github.com/nomihadar/G-PhoCS/blob/v140/G_PhoCS%201.40V%20Doc%20November%202019.pdf) I wrote.

### New files in version v140:

AllLoci [.h](src/AllLoci.h)/[.cpp](src/AllLoci.cpp) <br>
GenealogyStats [.h](src/GenealogyStats.h) <br>
LocusEmbeddedGenealogy [.h](src/LocusEmbeddedGenealogy.h)/[.cpp](src/LocusEmbeddedGenealogy.cpp) <br>
LocusGenealogy [.h](src/LocusGenealogy.h)/[.cpp](src/LocusGenealogy.cpp) <br>
LocusPopIntervals [.h](src/LocusPopIntervals.h)/[.cpp](src/LocusPopIntervals.cpp) <br>
PopInterval [.h](src/PopInterval.h)/[.cpp](src/PopInterval.cpp) <br>
TreeNode [.h](src/TreeNode.h)/[.cpp](src/TreeNode.cpp) <br> <br>

Below is an **UML diagram** shows the high level picture of the relationships between the new classes. <br><br>

<p align="center">
  <img width="800" height="600" src="UML diagram.png">
</p>
