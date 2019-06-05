
#include "LocusEmbeddedGenealogy.h"
#include "DbgErrMsgIntervals.h"
#include "PopulationTree.h"

#include <iostream>
#include <iomanip>
#include <cmath>

/*
	Constructor
*/
LocusEmbeddedGenealogy::LocusEmbeddedGenealogy(
        int locusID,
        int numIntervals,
        DATA_SETUP *pDataSetup,
        DATA_STATE *pDataState,
        GENETREE_MIGS *pGenetreeMigs)

        : genealogy_(pDataSetup->numSamples), //construct genealogy
          intervals_(locusID, numIntervals),  //construct intervals

          locusID_(locusID),
          pDataSetup_(pDataSetup),
          pPopTree_(pDataSetup->popTree),
          pDataState_(pDataState),
          pGenetreeMigs_(pGenetreeMigs) {

    //create map between leaf to its pop
    // and map between pop to its leaves
    for (int node = 0; node < pDataSetup_->numSamples; node++) {
        int pop = nodePops[locusID_][node];
        leafToPop_[node] = pop;
        popToLeaves_[pop].emplace_back(node); //todo reserve?
    }

    //fill queue with pops, sorted by post order
    populationPostOrder(pPopTree_->rootPop, popQueue_);

}


/*
   @return: a pointer to locus data of current locus
*/
LocusData *LocusEmbeddedGenealogy::getLocusData() {
    return pDataState_->lociData[locusID_];
}


/*
	Constructs genealogy and intervals
	Genealogy: construct branches and link to corresponding intervals,
                add mig nodes to tree
    Intervals: reset intervals, ling them to each other
                initialize with start and end intervals,
                create samples start intervals,
                create coalescent and migration intervals,
                link intervals to corresponding nodes.

	Typically used only for initial genetrees or for testing.
	Records number of lineages only for first events in leaf populations.
	The rest are recorded by computeGenetreeStats
*/
int LocusEmbeddedGenealogy::construct_genealogy_and_intervals() {

    //reset genealogy
    genealogy_.reset();

    //construct genealogy branches (edges between tree nodes)
    genealogy_.constructBranches(this->getLocusData());

    //reset intervals
    intervals_.reset();

    //link intervals to each other
    intervals_.linkIntervals();

    //add start and end intervals
    intervals_.createStartEndIntervals();

    //create samples start intervals (for ancient samples)
    for (int pop = 0; pop < pPopTree_->numCurPops; pop++) {

        //create interval
        double age = pPopTree_->pops[pop]->sampleAge;
        PopInterval *pInterval = intervals_.createInterval(pop, age,
                                                           IntervalType::SAMPLES_START);
        if (!pInterval) {
            INTERVALS_FATAL_0024
        }
    }

    //create coalescent intervals
    //and link intervals to genealogy and vice versa
    int nSamples = pDataSetup_->numSamples;
    for (int node = 0; node < 2 * nSamples - 1; node++) {

        //get population and age of node
        int pop = nodePops[locusID_][node];
        double age = getNodeAge(getLocusData(), node);

        // if node is a leaf - link it to its sampleStart interval
        if (genealogy_.isLeaf(node)) {

            //get samples start interval of pop
            PopInterval *pInterval = intervals_.getSamplesStart(pop);

            //get leaf node by current node id
            LeafNode *pNode = genealogy_.getLeafNode(node);

            //leaf node points to samplesStart interval (but samplesStart
            // interval points to null since there are several leaves)
            pNode->setSamplesInterval(pInterval);


        } else { //if node is not a leaf create a coal interval and link to node

            //create a coalescent interval
            PopInterval *pInterval =
                    intervals_.createInterval(pop, age, IntervalType::COAL);

            if (!pInterval) {
                INTERVALS_FATAL_0025
            }

            //get coal node by current node id
            CoalNode *pNode = genealogy_.getCoalNode(node);

            //coal interval points to coal node
            pInterval->setTreeNode(pNode);

            //coal node points to coal interval
            pNode->setCoalInterval(pInterval);
        }
    }

    //create migration intervals, add mig nodes to genealogy tree
    //and link between them
    for (int node = 0; node < 2 * nSamples - 1; node++) {

        //get tree node by current node id
        TreeNode *pTreeNode = genealogy_.getTreeNodeByID(node);

        //find migration above current node and after specified time
        int mig = findFirstMig(locusID_, node,
                               getNodeAge(getLocusData(), node));

        //while there are migration events on the edge above current node
        while (mig != -1) {

            //create migration events (source and target)

            //get age of migration, and target and source populations
            double age = pGenetreeMigs_[locusID_].mignodes[mig].age;
            int target_pop = pGenetreeMigs_[locusID_].mignodes[mig].target_pop;
            int source_pop = pGenetreeMigs_[locusID_].mignodes[mig].source_pop;

            //create an incoming migration interval
            PopInterval *pMigIn =
                    intervals_.createInterval(target_pop, age,
                                              IntervalType::IN_MIG);
            if (!pMigIn) {
                INTERVALS_FATAL_0022
            }

            //create an outgoing migration interval
            PopInterval *pMigOut =
                    intervals_.createInterval(source_pop, age,
                                              IntervalType::OUT_MIG);
            if (!pMigOut) {
                INTERVALS_FATAL_0023
            }

            //add a migration node to genealogy
            MigNode *pMigNode = genealogy_.addMigNode(pTreeNode);

            //set age
            pMigNode->setAge(age);

            //mig intervals points to mig node
            pMigIn->setTreeNode(pMigNode);
            pMigOut->setTreeNode(pMigNode);

            //mig node points to incoming and outgoing intervals
            pMigNode->setInMigInterval(pMigIn);
            pMigNode->setOutMigInterval(pMigOut);

            //update tree node
            pTreeNode = pMigNode;

            //find next migration (after time of current migration)
            mig = findFirstMig(locusID_, node, age);
        }

    }

    return 0;
}




/*	computeGenetreeStats
	Computes the statistics of a given locus.
    Assumes intervals chains are built, but number of lineages is ONLY
    set for first intervals in the leaf populations.
	Sets number of lineages for each non-leaf event by traversing the
	population tree post-order. In parallel, also records the statistics.
*/

int
LocusEmbeddedGenealogy::computeGenetreeStats(GenealogyStats& genStatsTotal) {

    // go over all intervals and compute num_lineages per each interval
    // also update genetree statistics
    for (int i = 0; i < pDataSetup_->popTree->numPops; i++) {

        //get current pop
        int pop = popQueue_[i];

        // if not leaf population get number of in-lineages from end-intervals
        // of son populations
        if (pop >= pPopTree_->numCurPops) {

            //get num lineages of first interval of the left pop son
            int lSon = pPopTree_->pops[pop]->sons[0]->id;
            int n1 = intervals_.getPopEnd(lSon)->getNumLineages();

            //get num lineages of first interval of the right pop son
            int rSon = pPopTree_->pops[pop]->sons[1]->id;
            int n2 = intervals_.getPopEnd(rSon)->getNumLineages();

            //set num lineages of first interval of current pop to the sum
            //of sons' num lineages
            intervals_.getFirstInterval(pop)->setNumLineages(n1 + n2);

        } else {
            //set num lineages of first interval of current pop to 0
            intervals_.getFirstInterval(pop)->setNumLineages(0);
        }

        this->recalcStats(pop, genStatsTotal);

    }

    return 0;
}


double
LocusEmbeddedGenealogy::recalcStats(int pop, GenealogyStats& genStatsTotal) {


    double popAge = pPopTree_->pops[pop]->age;

    GenealogyStats& stats = this->genealogyStats_;
    GenealogyStats& statsCheck = this->genealogyStatsCheck_;


    //get first interval
    PopInterval* pInterval = intervals_.getFirstInterval(pop);

    TimeMigBands* timeBand = getLiveMigBands(dataSetup.popTree, pop, pInterval->getAge());


    //get num lineages of first interval
    int n = pInterval->getNumLineages();

    // follow intervals chain and set number of lineages per interval according
    // to previous interval also update statistics
    while (true) {

        std::string s =pInterval->typeToStr();

        //set num lineages
        pInterval->setNumLineages(n);

        double t, delta;
        if (timeBand) {


            if (timeBand->endTime > pInterval->getAge()) {
                t = pInterval->getElapsedTime();
            } else {
                delta = pInterval->getAge() - timeBand->endTime;
                t = pInterval->getElapsedTime() - delta;
            }
        } else
        {
            t = pInterval->getElapsedTime();
        }



        //update coal statistics
        statsCheck.incrementCoalStats(pop, n * (n - 1) * t);

        if (timeBand) {
            //for each live mig band update mig statistics
            for (auto pMigBand : timeBand->migBands) {
                statsCheck.incrementMigsStats(pMigBand->id, n * t);
            }
        }



        //switch by interval type
        switch (pInterval->getType()) {

            case (IntervalType::SAMPLES_START):
                n += dataSetup.numSamplesPerPop[pop];
                break;

            case (IntervalType::COAL):
                statsCheck.incrementNumCoal(pop, 1);
                n--;
                break;

            case (IntervalType::IN_MIG):


                // getMigBandByPops()

                // figure out migration band and update its statistics
                //mig_band = genetree_migs[locusID_].mignodes[id].migration_band;
                //locus_data[locusID_].genetree_stats_check.num_migs[mig_band]++;
                //statsCheck.incrementNumMigs(,1);//todo: get mig ID

                n--;
                break;

            case (IntervalType::OUT_MIG):
                n++;
                break;


                /*case (MIG_BAND_END):
                    // compare and copy stats for mig band
                    delta_lnLd -= (locus_data[locusID_].genetree_stats_check.mig_stats[id] - genetree_stats[locusID_].mig_stats[id])
                    * dataSetup.popTree->migBands[id].migRate;

                    #ifdef ENABLE_OMP_THREADS
                    #pragma omp atomic
                    #endif
                    genetree_stats_total.mig_stats[id] +=
                            locus_data[locusID_].genetree_stats_check.mig_stats[id] -
                            genetree_stats[locusID_].mig_stats[id];
    #ifdef ENABLE_OMP_THREADS
    #pragma omp atomic
    #endif
                    genetree_stats_total.num_migs[id] +=
                            locus_data[locusID_].genetree_stats_check.num_migs[id] -
                            genetree_stats[locusID_].num_migs[id];
                    genetree_stats[locusID_].mig_stats[id] = locus_data[locusID_].genetree_stats_check.mig_stats[id];
                    genetree_stats[locusID_].num_migs[id] = locus_data[locusID_].genetree_stats_check.num_migs[id];
                    // remove mig band from living list
                    for (mig_band = 0; mig_band < num_live_mig_bands; mig_band++) {
                        if (live_mig_bands[mig_band] == id)
                            break;
                    }
                    if (mig_band == num_live_mig_bands) {
                        if (debug) {
                            fprintf(stderr,
                                    "\nError: recalcStats: migration band %d not alive in population %d, gen %d.\n",
                                    event_chains[locusID_].events[event].getId(), pop, locusID_);
                            fprintf(stderr, "Live migration bands are:");
                            for (mig_band = 0; mig_band < num_live_mig_bands; mig_band++) {
                                fprintf(stderr, " %d", live_mig_bands[mig_band]);
                            }
                            fprintf(stderr, "\n");
                        } else {
                            fprintf(stderr, "Fatal Error 0025.\n");
                        }
                        printGenealogyAndExit(locusID_, -1);
                    }
                    live_mig_bands[mig_band] = live_mig_bands[--num_live_mig_bands];
                    break;
                    */
            case (IntervalType::DUMMY):
            case (IntervalType::POP_END):
                break;

            /*default:
                if (debug) {
                    std::cout
                            << endl
                            << "Error: recalcStats: event of unknown type "
                            << pInterval->typeToStr() << " in population "
                            << pop
                            << " gen " << locusID_ << endl;

                } else {
                    std::cout << "Fatal Error 0026." << endl;
                }
                printGenealogyAndExit(locusID_, -1);*/
        }// end of switch

        if (pInterval->isType(IntervalType::POP_END))
            break;


        pInterval = pInterval->getNext();
        double a = pInterval->getAge();
        //get live mig bands
        //double age = pInterval->getAge();//todo: check
        timeBand = getLiveMigBands(dataSetup.popTree, pop, pInterval->getAge());

    }// end of while

    double delta_lnLd = 0.0;
    double heredity_factor = 1;


    delta_lnLd -= (locus_data[locusID_].genetree_stats_check.coal_stats[pop] -
                   genetree_stats[locusID_].coal_stats[pop]) /
                  (dataSetup.popTree->pops[pop]->theta * heredity_factor);



#ifdef ENABLE_OMP_THREADS
#pragma omp atomic
#endif
    genetree_stats_total.coal_stats[pop] +=
            (locus_data[locusID_].genetree_stats_check.coal_stats[pop] -
             genetree_stats[locusID_].coal_stats[pop]) / heredity_factor;
#ifdef ENABLE_OMP_THREADS
#pragma omp atomic
#endif
    genetree_stats_total.num_coals[pop] +=
            locus_data[locusID_].genetree_stats_check.num_coals[pop] -
            genetree_stats[locusID_].num_coals[pop];
    genetree_stats[locusID_].coal_stats[pop] = locus_data[locusID_].genetree_stats_check.coal_stats[pop];
    genetree_stats[locusID_].num_coals[pop] = locus_data[locusID_].genetree_stats_check.num_coals[pop];


    return delta_lnLd;
}



/*
	print pop to leaves map
*/
void LocusEmbeddedGenealogy::printPopToLeaves() {

    std::cout << "Pop to leaves: " << std::endl;
    for (auto &x : popToLeaves_) {
        std::cout << "pop " << x.first << ", leaves: ";
        for (int leaf : x.second) {
            cout << leaf << ", ";
        }
        std::cout << std::endl;
    }
}

/*
	print leaf to map
*/
void LocusEmbeddedGenealogy::printLeafToPop() {
    std::cout << "Leaf to pop: " << std::endl;
    for (auto &x : leafToPop_) {
        std::cout << "leaf " << x.first << ", "
                  << "pop: " << x.second << std::endl;
    }
}

/*
	print population tree, genealogy and intervals
*/
void LocusEmbeddedGenealogy::printEmbeddedGenealogy() {

    //print population tree
    printPopulationTree(this->pDataSetup_->popTree, stderr, 1);

    //print pop to leaves
    std::cout << "------------------------------------------------------"
              << std::endl;
    this->printPopToLeaves();

    //print leaf to pop
    std::cout << "------------------------------------------------------"
              << std::endl;
    this->printLeafToPop();

    //print genealogy
    std::cout << "------------------------------------------------------"
              << std::endl;
    genealogy_.printGenealogy();

    //print intervals
    std::cout << "------------------------------------------------------"
              << std::endl;
    intervals_.printIntervals();

}

int LocusEmbeddedGenealogy::getLocusID() {
    return locusID_;
}


std::vector<int> &LocusEmbeddedGenealogy::getPopLeaves(int pop) {
    return popToLeaves_[pop];
}

int LocusEmbeddedGenealogy::getLeafPop(int leafId) {
    return leafToPop_[leafId];
}

void LocusEmbeddedGenealogy::testLocusGenealogy() {

    //define precision for double comparision
    double EPSILON = 0.0000000001;

    //get locus id and locus data
    LocusData* pLocusData = this->getLocusData();
    int numSamples = pDataSetup_->numSamples;

    //get all migs into a map: <node,[migsAges]>
    std::map<int, std::vector<double>> migsMap;
    for (int node = 0; node < 2 * numSamples - 1; node++) {
        int mig = findFirstMig(locusID_, node, getNodeAge(pLocusData, node));
        while (mig != -1) {
            double age = pGenetreeMigs_[locusID_].mignodes[mig].age;
            migsMap[node].emplace_back(age);
            mig = findFirstMig(locusID_, node, age);
        }
    }

    //iterate by node id
    for (int node = 0; node < 2 * numSamples - 1; node++) {

        //get coal or leaf node
        TreeNode* pNode = genealogy_.getTreeNodeByID(node);

        //get ages
        double age = getNodeAge(pLocusData, node);
        double ageNew = pNode->getAge();

        //compare ages
        assert(fabs(age - ageNew) < EPSILON);

        TreeNode* parentNew = pNode;

        //if there are migrations above
        for (double migAge : migsMap[node]) {

            parentNew = parentNew->getParent();
            double migAgeNew = parentNew->getAge();

            //compare ages
            assert(fabs(migAge - migAgeNew) < EPSILON);
        }

        //compare parents ages
        int parent = getNodeFather(pLocusData, node);
        parentNew = parentNew->getParent();
        if (parent != -1) {
            double age = getNodeAge(pLocusData, parent);
            double ageNew = parentNew->getAge();
            //compare ages
            assert(fabs(age - ageNew) < EPSILON);
        }

        //compare sons ages
        for (int son = 0; son < 2; son++) {
            int lSon = getNodeSon(pLocusData, node, son);
            TreeNode *pSonNew = pNode;

            //reverse vector
            vector<double> rev(migsMap[lSon].rbegin(), migsMap[lSon].rend());
            for (double migAge : rev) {

                pSonNew = son ? pSonNew->getRightSon() : pSonNew->getLeftSon();
                double migAgeNew = pSonNew->getAge();

                //compare ages
                assert(fabs(migAge - migAgeNew) < EPSILON);
            }

            pSonNew = son ? pSonNew->getRightSon() : pSonNew->getLeftSon();
            if (lSon != -1) {
                double age = getNodeAge(pLocusData, lSon);
                double ageNew = pSonNew->getAge();
                //compare ages
                assert(fabs(age - ageNew) < EPSILON);
            }
        }

    }

}


//test if the new events data structure is consistent with the original
//(terminology: old: events, new: intervals)
void LocusEmbeddedGenealogy::testPopIntervals() {

    //define epsilon for double comparision
    double EPSILON = 0.0000000001;

    //for each pop
    for (int pop = 0; pop < pPopTree_->numPops; pop++) {

        //get first event in old structure
        int event = event_chains[locusID_].first_event[pop];

        //get pop-start interval
        PopInterval* pInterval = intervals_.getPopStart(pop);

        //get pop age
        double eventAge = pPopTree_->pops[pop]->age;

        //assert ages equal
        assert(fabs(eventAge - pInterval->getAge()) < EPSILON);

        //vector of live mig bands (original data structure)
        std::vector<int> liveMigsOri;

        //get next interval
        pInterval = pInterval->getNext();

        //iterate both old and new structures (events VS intervals)
        for (event; event >= 0;
                    event = event_chains[locusID_].events[event].getNextIdx()) {

            EventType eventType = event_chains[locusID_].events[event].getType();
            double elapsedTime = event_chains[locusID_].events[event].getElapsedTime();

            //num lineages
            //verify that nums lineages are equal
            int eventLin = event_chains[locusID_].events[event].getNumLineages();
            int intervalLin = pInterval->getNumLineages();
            assert(eventLin == intervalLin);

            //mig bands
            //if elapsed time of event is greater than epsilon, compare live mig bands
            if (elapsedTime > 2 * EPSILON) {

                double age = eventAge + elapsedTime/2;
                TimeMigBands* liveMigsNew = getLiveMigBands(pPopTree_, pop, age);

                //assert live migs band is not null
                assert(liveMigsNew != nullptr);

                //compare num of live mig bands
                assert(liveMigsOri.size() == liveMigsNew->migBands.size());

                //for each live mig band
                for (int id : liveMigsOri) {
                    //get pointer to mig band by its id
                    MigrationBand* migBand = getMigBandByID(pPopTree_, id);
                    //verify that current mig is found in the new data structure
                    assert(std::find(liveMigsNew->migBands.begin(),
                                     liveMigsNew->migBands.end(), migBand) != liveMigsNew->migBands.end());
                }

                //verify if time band contains old event
                assert(liveMigsNew->startTime <= eventAge + EPSILON);
                assert(eventAge + elapsedTime <= liveMigsNew->endTime + EPSILON);
            }

            //age
            //add elapsed time to event age
            eventAge += elapsedTime;

            //event type
            //get event type and verify that interval is of same type
            // or, in case of mig band start/end, add/remove a mig band

            switch (eventType) {
                case SAMPLES_START: {
                    assert(pInterval->isType(IntervalType::SAMPLES_START)); break;
                }
                case COAL: {
                    assert(pInterval->isType(IntervalType::COAL)); break;
                }
                case IN_MIG: {
                    assert(pInterval->isType(IntervalType::IN_MIG)); break;
                }
                case OUT_MIG: {
                    assert(pInterval->isType(IntervalType::OUT_MIG)); break;
                }
                case END_CHAIN: {
                    assert(pInterval->isType(IntervalType::POP_END)); break;
                }

                //mig bands (don't exist in the new data structure)
                case MIG_BAND_START: {
                    //add id of mig band to live mig bands
                    int event_id = event_chains[locusID_].events[event].getId();
                    liveMigsOri.push_back(event_id);
                    continue;
                }
                case MIG_BAND_END: {
                    //remove id of mig band from live mig bands
                    int event_id = event_chains[locusID_].events[event].getId();
                    liveMigsOri.erase(
                            std::remove(liveMigsOri.begin(), liveMigsOri.end(),
                                        event_id), liveMigsOri.end());
                    continue;
                }
            }

            //age
            //verify that ages are equal
            double intervalAge = pInterval->getAge();
            assert(fabs(eventAge - intervalAge) < EPSILON);

            //get next interval
            pInterval = pInterval->getNext();
        }

    }//end of pop loop

}

