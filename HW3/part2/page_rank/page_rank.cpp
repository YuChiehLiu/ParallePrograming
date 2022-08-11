#include "page_rank.h"

#include <stdlib.h>
#include <cmath>
#include <omp.h>
#include <utility>

#include "../common/CycleTimer.h"
#include "../common/graph.h"

#include <stdio.h>

// pageRank --
//
// g:           graph to process (see common/graph.h)
// solution:    array of per-vertex vertex scores (length of array is num_nodes(g))
// damping:     page-rank algorithm's damping parameter
// convergence: page-rank algorithm's convergence threshold
//
void pageRank(Graph g, double *solution, double damping, double convergence)
{

  // initialize vertex weights to uniform probability. Double
  // precision scores are used to avoid underflow for large graphs

  int numNodes = num_nodes(g);
  double equal_prob = 1.0 / numNodes;
  bool converged = false;
  double* score_old = (double*)malloc(sizeof(double)*numNodes);
  double* score_new = (double*)malloc(sizeof(double)*numNodes);

  for (int i=0 ; i <numNodes ; ++i)
  {
    solution[i] = equal_prob;
    score_old[i] = solution[i];
  }

  //initialization

  //while(!converged){}


  while(!converged)
  {
    /* 1st step : compute score_new[vi] for all nodes vi:
       score_new[vi] = sum over all nodes vj reachable from incoming edges
                          { score_old[vj] / number of edges leaving vj  } */


    #pragma omp parallel for
    for ( int i=0 ; i<numNodes ; i++ ) 
    {
      score_new[i] = 0.0;
      const Vertex *start = incoming_begin(g, i);
      int incoming_nums = incoming_size(g, i);
      for ( int j=0 ; j<incoming_nums ; j++)
      {
        int incoming_vertex = start[j];
        score_new[i] += score_old[incoming_vertex] / outgoing_size(g,incoming_vertex);
      }

    /* 2nd step :
       score_new[vi] = (damping * score_new[vi]) + (1.0-damping) / numNodes; */
      score_new[i] = (damping * score_new[i]) + (1.0-damping) / numNodes;
    }

    /* 3rd step : 
       score_new[vi] += sum over all nodes v in graph with no outgoing edges
                          { damping * score_old[v] / numNodes } */
    
    double nooutsum = 0.0;
    #pragma omp parallel for reduction(+:nooutsum)
    for ( int i=0 ; i<numNodes ; i++ )
    {
      if( outgoing_size(g,i) == 0 )
      {
        nooutsum += damping * score_old[i] / numNodes;
      }
    }

    #pragma omp parallel for
    for ( int i=0 ; i<numNodes ; i++ )
    {
      score_new[i] += nooutsum;
    }
        
    /* 4th step : compute how much per-node scores have changed
       global_diff = sum over all nodes vi { abs(score_new[vi] - score_old[vi]) } */
    
    double global_diff = 0.0;
    #pragma omp parallel for reduction(+:global_diff)
    for ( int i=0 ; i<numNodes ; i++)
    {    
      global_diff += fabs(score_new[i] - score_old[i]);
      score_old[i] = score_new[i];
    }

    /* 5th step : quit once algorithm has converged
       converged = (global_diff < convergence) */
    if( global_diff < convergence )
      converged = true;

  }

  for ( int i=0 ; i<numNodes ; i++)
  {
    solution[i] = score_new[i];
  }

  free(score_new);
  free(score_old);
}