/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qcomptree.h,v 1.9 2006/07/31 06:25:50 bmiller Exp $

#ifndef INCLUDE_comptree_h
#define INCLUDE_comptree_h 1


#include <vector>
#include <list>
#include "qtypes.h"
#include "qposinfo.h"
#include "parameters.h"



/* The idea behind a computation tree is to store whatever state helps
 * speed up the process of evaluating new positions.  This state is not
 * generally useful unless computing the score of new positions, so
 * we do not want to keep it stored indefinitely along with the scores
 * of positions.  It is kept for the life of a single call to iSearch,
 * then recycled from scratch for whatever position iSearch is next
 * called on.
 */

typedef guint32 qComputationTreeNodeId;
const qComputationTreeNodeId qComputationTreeNode_invalid = 0;
const qComputationTreeNodeId qComputationTreeNode_max = G_MAXUINT32;

typedef std::list<qComputationTreeNodeId> qComputationTreeNodeList;
typedef std::list<qComputationTreeNodeId>::iterator qComputationTreeNodeListIterator;

/* PRIVATE LOCAL CLASS
 * qComputationNode class
 * Used privately by qComputationTree class.
 * !!! Relocate this declaration to inside the implementation file.
 */
class qComputationNode {
public:
  // Info regarding the edge that leads to this node:
  qMove                      mv;    // what move led to the position in this node
  const qPositionEvaluation *eval;  // the score returned for that move

  // Note: by using eval pointers instead of copies, any time we free memory
  // from the positionHash we will be forced to discard the current state
  // of the computation tree and rebuild it.


  /* Need these???
     qPosition pos;
     qPlayer   playerMoving;
  */
  qComputationTreeNodeId              parentNodeIdx;
  std::list<qComputationTreeNodeId>   childNodes;

  // Note that the following is actually the lowest opponent eval
  qComputationTreeNodeId              childWithBestEval;

  // And now the saved state used to accelerate things...
  qPositionInfo           *posInfo;


  qComputationNode::qComputationNode()
  :parentNodeIdx(qComputationTreeNode_invalid),
   childNodes(0),
   childWithBestEval(0),
   posInfo(NULL)
    {
       this->mv = moveNull;
       this->eval = NULL;
    };
  qComputationNode::~qComputationNode() {;};

#if 0
  /* The idea here was to have the evaluation routine's dijkstra algorithm
   * use our storage during computation of a position's score.  Then
   * if we have the graph generated by one invocation on a position; we
   * can use that graph to accelerate evaluation of adjacent positions.
   *
   * Unfortunately, I cannot figure out a practical way to make this work.
   * I wanted to define a set of frontier points adjacent to whatever
   * changed in the graph, and then ripple those points through, stopping
   * any frontier points that did not change their neighbors, but it was
   * too hard to detect which neighbors should and shouldn't be adjusted.
   *
   * It seems faster to just re-evaluate dijkstra than to try to correctly
   * locate and carry forward frontier points modifying an existing graph.
   *
   * An example of my thinkgin:
   * In the new position(s), call all squares adjacent to a wall move the
   * set of "affected nodes."
   * Find the affected node(s) with minimum score.  Without changing their
   * scores, move them to the "frontier set."
   * For all other affected nodes, NULL out their scores.
   * For all nodes in the frontier set:
   *   Remove the square from the frontier set.
   *   If an adjacent square has no existing score, give it current score+1,
   *     make it a frontier point, and remove it from the affected node set.
   *   If it has a higher score, give it current score+1 and make it a
   *     frontier point.
   *   What if it has a lower score???
   */
   
  qDijkstraGraph dijkstraGraph;
#endif
};

class qComputationTree {
 public:
  qComputationTree::qComputationTree();
  qComputationTree::~qComputationTree();

  // Sets all nodes to uninitialized
  void initializeTree();

  // Returns the root node
  qComputationTreeNodeId getRootNode() const;

  // addNodeChild: 
  // Adds an edge to the current node, leading to a new child node
  // Returns new child node's id, or qComputationTreeNode_invalid on failure.
  // Note: we store the mv's eval pointer instead of copying, so any time we
  // free memory from the positionHash we will be forced to discard the
  // current state of the computation tree and rebuild it.
  qComputationTreeNodeId addNodeChild(qComputationTreeNodeId node,
				      qMove mv,
				      const qPositionEvaluation *eval);

  // Because we usually want to find the move yielding the worst possible
  // eval for our opponent, this list is reverse sorted by
  // eval.score + eval.complexity
  bool nodeHasChildList(qComputationTreeNodeId node);
  qComputationTreeNodeList *getNodeChildList(qComputationTreeNodeId node);

  // Returns the childNode with the lowest eval.score
  qComputationTreeNodeId getBestScoringChild(qComputationTreeNodeId node) const;

  qComputationTreeNodeId getNodeParent(qComputationTreeNodeId node) const;

  qPositionInfo *getNodePosInfo(qComputationTreeNodeId node) const;
  void setNodePosInfo(qComputationTreeNodeId node, qPositionInfo *posInfo);

  // Note: using setEval alters the order of the parent node's child list,
  // and can cause the parent node's BestScoringChild to change.
  void setNodeEval(qComputationTreeNodeId     node,
		   const qPositionEvaluation *eval);
  const qPositionEvaluation *getNodeEval(qComputationTreeNodeId node) const;

  qMove getNodePrecedingMove(qComputationTreeNodeId node) const;

 private:

  // Maybe replace these with simple arrays, if we're sure we won't need to
  // grow them (or if we can get by without the graph)
  std::vector<qComputationNode> nodeHeap;

  qComputationTreeNodeId nodeNum; // lowest free node
  qComputationTreeNodeId maxNode; // highest existing node; alloc more when used

  inline bool growNodeHeap()
    { 
      if (nodeHeap.size() > qComputationTreeNode_max - COMPTREE_GROW_SIZE)
        return FALSE;
      nodeHeap.resize(COMPTREE_GROW_SIZE + nodeHeap.size());
      maxNode = nodeHeap.size() - 1;
      return TRUE;
    };
  void qComputationTree::resetBestChild(qComputationNode &n);
};

extern const qComputationNode emptyNode;
#endif // INCLUDE_comptree_h
