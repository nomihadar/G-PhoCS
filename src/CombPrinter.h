#include <stdio.h>


#ifndef SRC_COMBSTATSHEADER_H_
#define SRC_COMBSTATSHEADER_H_


void printCombStats(int iteration, FILE *file);

void printCombStatsHeader(FILE *file);

void printOneCombHeader(int comb, FILE *file);

void printCombPopHeaders(int comb, char *combName, FILE *file);

void printCombMigHeaders(int comb, char *combName, FILE *file);


void printOneCombStats(int clade, FILE *file);

void printCombCoalStats(int comb, FILE *file);

void printCombMigStats(int comb, FILE *file);

#endif

