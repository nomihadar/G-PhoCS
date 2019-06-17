
#ifndef G_PHOCS_TREENODE_H
#define G_PHOCS_TREENODE_H

#include "PopInterval.h"
#include <iostream>

/*=============================================================================
 *
 * TreeNode class
 *
 * TreeNode is a single node in genealogy tree.
 * Tree node is an abstract class, and is inherited by three class:
 * LeafNode, CoalNode and MigNode.
 *
 * TreeNode Contains:
 * 1. Type of node (leaf, migration or coal).
 * 2. Id of tree node (analogous to node id of old structure.)
 * 3. Age of tree node.
 * 4. Pointers ("edges") to parent and sons.
 *
 * The derived classes also contain:
 * 5. Pointer(s) to corresponding interval(s).
 *===========================================================================*/

enum class TreeNodeType; //forward declaration

class TreeNode {

protected:

    TreeNodeType  type_;    //type of node

    int nodeID_; //id of tree node

    double age_; //tree node age

    TreeNode*  pParent_;    //pointer to parent in genealogy
    TreeNode*  pLeftSon_;   //pointer to left son in genealogy
    TreeNode*  pRightSon_;  //pointer to right son in genealogy

public:

    //constructor
    TreeNode();

    //copy constructor
    TreeNode(const TreeNode& treeNode2);

    //get pop id. A pure virtual method.
    virtual int getPopId() = 0;

    //print tree node
    virtual void printTreeNode();

    //get type as string
    virtual std::string typeToStr() = 0;

    //getters
    TreeNodeType getType() const;
    int getNodeId() const;
    double getAge() const;
    TreeNode* getParent() const;
    TreeNode* getLeftSon() const;
    TreeNode* getRightSon() const;

    //setters
    void setNodeId(int nodeId);
    void setAge(double age);
    void setParent(TreeNode* pParent);
    void setLeftSon(TreeNode* pLeftSon);
    void setRightSon(TreeNode* pRightSon);

};

////////////////////////////////////////////////////////////////////////////////
class LeafNode : public TreeNode {

private:
    PopInterval* pSamplesStart_; //pointer to samplesStart interval

public:
    LeafNode(); //constructor

    LeafNode(const LeafNode& leafNode2); //copy-constructor

    int getPopId() override;

    void printTreeNode() override;

    std::string typeToStr() override;

    //getters and setters
    PopInterval* getSamplesStart() const;
    void setSamplesInterval(PopInterval* pSamplesStart);

};

////////////////////////////////////////////////////////////////////////////////
class CoalNode : public TreeNode {

private:
    PopInterval* pCoal_; //pointer to coalescent interval

public:
    CoalNode(); //constructor

    CoalNode(const CoalNode& coalNode2); //copy-constructor

    int getPopId() override;

    void printTreeNode() override;

    std::string typeToStr() override;

    //getters and setters
    PopInterval* getCoalInterval() const;
    void setCoalInterval(PopInterval *pCoal);

};

////////////////////////////////////////////////////////////////////////////////
class MigNode : public TreeNode {

private:
    PopInterval* pOutMig_;   //pointer to outgoing migration interval
    PopInterval* pInMig_;   //pointer to incoming migration interval
    int migBandId_;         //mig band ID

public:
    explicit MigNode(int migBandID); //constructor

    MigNode(const MigNode& migNode2); //copy-constructor

    int getPopId() override;

    void printTreeNode() override;

    std::string typeToStr() override;

    //getters and setters
    PopInterval* getOutMigInterval() const;
    PopInterval* getInMigInterval() const;
    int getMigBandId() const;
    void setOutMigInterval(PopInterval* pOutMig);
    void setInMigInterval(PopInterval* pInMig);

};


/*
    Types of tree nodes
*/
enum class TreeNodeType {
    LEAF,   //leaf
    COAL,   //coalescence
    MIG,    //migration
};


#endif //G_PHOCS_TREENODE_H
