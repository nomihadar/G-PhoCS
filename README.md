G-PhoCS
=======


Currently I am a research assistance of Dr. Ilan Gronau in IDC Herzliya.
In this project I develop his existing software, G-PhoCS, which is under the domain of population genetics. 

G-PhoCS is a software package for inferring ancestral population sizes, population divergence times, and migration rates from individual genome sequences. G-PhoCS accepts as input a set of multiple sequence alignments from separate neutrally evolving loci along the genome. Parameter inference is done in a Bayesian manner, using a Markov Chain Monte Carlo (MCMC) to jointly sample model parameters and genealogies at the input loci. 


Files I wrote (rest were not written by me):

[AllLoci.h](src/AllLoci.h)/[.cpp](src/AllLoci.cpp) <br>
[GenealogyStats.h](src/GenealogyStats.h) <br>
[LocusEmbeddedGenealogy.h](src/LocusEmbeddedGenealogy.h) / [LocusEmbeddedGenealogy.cpp](src/LocusEmbeddedGenealogy.cpp) <br>
[LocusGenealogy.h](src/LocusGenealogy.h) / [LocusGenealogy.cpp](src/LocusGenealogy.cpp) <br>
[LocusPopIntervals.h](src/LocusPopIntervals.h) / [LocusPopIntervals.cpp](src/LocusPopIntervals.cpp) <br>
[PopInterval.h](src/PopInterval.h) / [PopInterval.cpp](src/PopInterval.cpp) <br>
[TreeNode.h](src/TreeNode.h) / [TreeNode.cpp](src/TreeNode.cpp) <br>


UML diagram of classes I created:



<p align="center">
  <img width="800" height="600" src="UML diagram.png">
</p>

U
---------------



