#pragma once
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

struct VertexData
{
    Boost_Point_2 p;
};


typedef boost::adjacency_list<boost::setS, boost::setS,
    boost::undirectedS,
    VertexData,
    boost::no_property
> BoostPolygonizerGraph;

typedef boost::graph_traits<BoostPolygonizerGraph>::vertex_iterator vertex_iterator;
typedef boost::graph_traits<BoostPolygonizerGraph>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<BoostPolygonizerGraph>::edge_descriptor edge_descriptor;
typedef boost::graph_traits<BoostPolygonizerGraph>::adjacency_iterator adjacency_iterator;
typedef boost::graph_traits<BoostPolygonizerGraph>::out_edge_iterator out_edge_iterator;
