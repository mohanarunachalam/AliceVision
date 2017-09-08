// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#ifndef ALICEVISION_GRAPH_CONNECTED_COMPONENT_H_
#define ALICEVISION_GRAPH_CONNECTED_COMPONENT_H_

#include <aliceVision/types.hpp>
#include <aliceVision/system/Logger.hpp>
#include <aliceVision/graph/graph.hpp>
#include <set>

namespace aliceVision {
namespace graph {

/// Export node of each CC (Connected Component) in a map
template <typename GraphT, typename IndexT>
std::map<IndexT, std::set<lemon::ListGraph::Node> >  exportGraphToMapSubgraphs(
  const GraphT & g)
{
  typedef lemon::ListGraph::NodeMap<IndexT> IndexMap;
  IndexMap connectedNodeMap(g);
  lemon::connectedComponents(g, connectedNodeMap);

  std::map<IndexT, std::set<lemon::ListGraph::Node> > map_subgraphs;

  // Create subgraphs' map
  typedef lemon::ListGraph::NodeIt NodeIterator;
  NodeIterator itNode(g);
  for (typename IndexMap::MapIt it(connectedNodeMap);
    it != lemon::INVALID; ++it, ++itNode) {
    map_subgraphs[*it].insert(itNode);
  }
  return map_subgraphs;
}

/// Return imageIds that belongs to the largest bi-edge connected component
template<typename EdgesInterface_T, typename IndexT>
std::set<IndexT> CleanGraph_KeepLargestBiEdge_Nodes(
  const EdgesInterface_T & edges,
  const std::string & _sOutDirectory = "")
{
  std::set<IndexT> largestBiEdgeCC;

  // Create a graph from pairwise correspondences:
  // - remove not biedge connected component,
  // - keep the largest connected component.

  typedef lemon::ListGraph Graph;
  graph::indexedGraph putativeGraph(edges);
/*
  if (!_sOutDirectory.empty())
  {
    // Save the graph before cleaning:
    imageGraph::exportToGraphvizData(
    stlplus::create_filespec(_sOutDirectory, "initialGraph"),
    putativeGraph.g);
  }
  */

  // Remove not bi-edge connected edges
  typedef Graph::EdgeMap<bool> EdgeMapAlias;
  EdgeMapAlias cutMap(putativeGraph.g);

  if (lemon::biEdgeConnectedCutEdges(putativeGraph.g, cutMap) > 0)
  {
    // Some edges must be removed because they don't follow the biEdge condition.
    typedef Graph::EdgeIt EdgeIterator;
    EdgeIterator itEdge(putativeGraph.g);
    for (EdgeMapAlias::MapIt it(cutMap); it!=lemon::INVALID; ++it, ++itEdge)
    {
      if (*it)
        putativeGraph.g.erase(itEdge); // remove the not bi-edge element
    }
  }

  // Graph is bi-edge connected, but still many connected components can exist
  // Keep only the largest one
  const int connectedComponentCount = lemon::countConnectedComponents(putativeGraph.g);
  ALICEVISION_LOG_DEBUG("CleanGraph_KeepLargestBiEdge_Nodes():: => connected Component: "
    << connectedComponentCount);
  if (connectedComponentCount >= 1)
  {
    // Keep only the largest connected component
    // - list all CC size
    // - if the largest one is meet, keep all the edges that belong to this node

    const std::map<IndexT, std::set<Graph::Node> > map_subgraphs = exportGraphToMapSubgraphs<Graph,IndexT>(putativeGraph.g);
    size_t count = std::numeric_limits<size_t>::min();
    typename std::map<IndexT, std::set<Graph::Node> >::const_iterator iterLargestCC = map_subgraphs.end();
    for(typename std::map<IndexT, std::set<Graph::Node> >::const_iterator iter = map_subgraphs.begin();
        iter != map_subgraphs.end(); ++iter)
    {
      if (iter->second.size() > count)  {
        count = iter->second.size();
        iterLargestCC = iter;
      }
      ALICEVISION_LOG_DEBUG("Connected component of size: " << iter->second.size());
    }

    //-- Keep only the nodes that are in the largest CC
    for(typename std::map<IndexT, std::set<lemon::ListGraph::Node> >::const_iterator iter = map_subgraphs.begin();
        iter != map_subgraphs.end(); ++iter)
    {
      if (iter == iterLargestCC)
      {
        // list all nodes that belong to the current CC and update the Node Ids list
        const std::set<lemon::ListGraph::Node> & ccSet = iter->second;
        for (std::set<lemon::ListGraph::Node>::const_iterator iter2 = ccSet.begin();
          iter2 != ccSet.end(); ++iter2)
        {
          const IndexT Id = (*putativeGraph.map_nodeMapIndex)[*iter2];
          largestBiEdgeCC.insert(Id);
        }
      }
      else
      {
        // remove the edges from the graph
        const std::set<lemon::ListGraph::Node> & ccSet = iter->second;
        for (std::set<lemon::ListGraph::Node>::const_iterator iter2 = ccSet.begin();
          iter2 != ccSet.end(); ++iter2)
        {
          typedef Graph::OutArcIt OutArcIt;
          for (OutArcIt e(putativeGraph.g, *iter2); e!=lemon::INVALID; ++e)
          {
            putativeGraph.g.erase(e);
          }
        }
      }
    }
  }
/*
  if (!_sOutDirectory.empty())
  {
    // Save the graph after cleaning:
    imageGraph::exportToGraphvizData(
      stlplus::create_filespec(_sOutDirectory, "cleanedGraph"),
      putativeGraph.g);
  }*/

  ALICEVISION_LOG_DEBUG(
    "Cardinal of nodes: " << lemon::countNodes(putativeGraph.g) << "\n" <<
    "Cardinal of edges: " << lemon::countEdges(putativeGraph.g)
    );

  return largestBiEdgeCC;
}

} // namespace graph
} // namespace aliceVision

#endif // ALICEVISION_GRAPH_CONNECTED_COMPONENT_H_
