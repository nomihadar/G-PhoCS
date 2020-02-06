G-PhoCS
=======

G-PhoCS is a software package for inferring ancestral population sizes, population divergence times, and migration rates from individual genome sequences. G-PhoCS accepts as input a set of multiple sequence alignments from separate neutrally evolving loci along the genome. Parameter inference is done in a Bayesian manner, using a Markov Chain Monte Carlo (MCMC) to jointly sample model parameters and genealogies at the input loci. 

Currently I am a research assistance of Dr. Ilan Gronau in IDC Herzliya.
In this project I develop his existing software, G-PhoCS, which is under the domain of population genetics. 

Files I wrote (rest were not written by me):

AllLoci.cpp/.h[](src/AllLoci.cpp)
DbgErrMsgIntervals.h
GenealogyStats.h
LocusEmbeddedGenealogy.cpp/.h
LocusGenealogy.cpp/.h
LocusPopIntervals.cpp/.h
PopInterval.cpp/.h
TreeNode.cpp/.h

UML diagram of classes I created:



<p align="center">
  <img width="800" height="600" src="UML diagram.png">
</p>

U
---------------



