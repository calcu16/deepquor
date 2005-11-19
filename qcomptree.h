/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

#ifndef INCLUDE_comptree_h
#define INCLUDE_comptree_h


#include "qtypes.h"
#include "qposinfo.h"
#include "parameters.h"

IDSTR("$Id: qcomptree.h,v 1.1 2005/11/19 08:22:33 bmiller Exp $");

/* The idea behind a computation tree is to store whatever state helps
 * speed up the process of evaluating new positions.  This state is not
 * generally useful unless computing the score of new positions, so
 * we do not want to keep it stored indefinitely along with the scores
 * of positions.  It is kept for the life of a single call to iSearch,
 * then recycled from scratch for whatever position iSearch is next
 * called on.
 */
typedef TREETYPE<class qComputationNode> qComputationTree;

// Note: by using eval pointers instead of copies, any time we free memory
// from the positionHash we will be forced to discard the current state
// of the computation tree and rebuild it.

class qComputationNode {
  // Info regarding the edge that leads to this node:
  struct edge {
    qMove                mv;    // what move led to the position in this node
    qPositionEvaluation *eval;  // the score returned for that move
  };

  /* Need these???
     qPosition pos;
     qPlayer   playerMoving;
  */
  guint16                  parentNodeIdx;
  vector<guint16>          childNodes;
  guint16                  childWithBestEvalScore;

  // And now the saved state used to accelerate things...
  qPositionInfo           *posInfo;

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
  typedef qComputationTreeNodeId guint16;
  enum { qComputationTreeNode_invalid = 0 };

  // Sets all nodes to uninitialized
  initializeTree();

  // Returns the root node
  getRootNode();

  // addChildNode: 
  // Adds an edge to the current node, leading to a new child node
  // Returns new child node's id.
  // Note: we store the mv's eval pointer instead of copying, so any time we
  // free memory from the positionHash we will be forced to discard the
  // current state of the computation tree and rebuild it.
  qComputationTreeNodeId addNodeChild(qComputationTreeNodeId node,
				      qMove mv,
				      qPositionEvaluation *eval);

  // Returns 0 if no such child;
  // Children are ordered according to eval.score + eval.complexity
  qComputationTreeNodeId getNthChild(qComputationTreeNodeId node, guint8 n);
  qComputationTreeNodeId getTopScoringChild(qComputationTreeNodeId node);

  qComputationTreeNodeId getNodeParent(qComputationTreeNodeId node);

  qPositionInfo *getNodePosInfo(qComputationTreeNodeId node);
  setNodePosInfo(qComputationTreeNodeId node, qPositionInfo *posInfo);

  // Note: using setEval alters the order of the parent node's child list 
  setNodeEval(qComputationTreeNodeId node, qPositionEval const *eval);
  qPositionEval const *getNodeEval(qComputationTreeNodeId node);

  qMove getNodePrecedingMove(qComputationTreeNodeId node);

 private:
  // Maybe replace these with simple arrays, if we're sure we won't need to
  // grow them (or if we can get by without the graph)
  vector<qComputationNode> nodeHeap;
  guint16 nodeNum; // lowest free node
  guint16 maxNode; // highest existing node; time to reserve more in heap
};

#endif // INCLUDE_comptree_h
