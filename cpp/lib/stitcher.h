#pragma once

#include <vector>

#include "similarity.h"


namespace CamHDMotionTracking {


  struct Node {
    Node( int i )
    : id(i)
    {;}

    int id;
  };

  struct Edge {
    Edge( int f, int t, const CalculatedSimilarity &s  )
    : from(f), to(t), sim(s)
    {;}

    int from, to;
    CalculatedSimilarity sim;
  };


  struct Graph {
    Graph()
    {;}


    void addEdge( int f, int t, const CalculatedSimilarity &s )
    {
      // First check if either edge exists
      findOrAdd( f );
      findOrAdd( t );

      if( !findEdge( f, t ) ) {
        edges.emplace_back( f, t, s );
      }
    }

    void findOrAdd( int id )
    {
      for( auto node : nodes ) {
        if( node.id == id ) return;
      }

      nodes.emplace_back( id );
    }

    bool findEdge( int f, int t ) {
      for( auto edge : edges ) {
        if( ( edge.from == f && edge.to == t ) ||
            ( edge.from == t && edge.to == f ) ) return true;
      }

      return false;
    }


    std::vector< Node > nodes;
    std::vector< Edge > edges;

  };

}
