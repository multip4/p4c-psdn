/*
 * Written by Seungbin Song
 * 
 * Modify p4c-graph to a draw data dependence graph.
*/

#include <boost/graph/graphviz.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/breadth_first_search.hpp>

#include "lib/log.h"
#include "lib/error.h"
#include "lib/exceptions.h"
#include "lib/gc.h"
#include "lib/crash.h"
#include "lib/nullstream.h"

#include "graphs.h"

namespace PSDN {

class edge_predicate_c {
  public:
    edge_predicate_c() : g(0) {}
    edge_predicate_c(Graphs::Graph &_g) : g(&_g) {}
    bool operator()(const Graphs::edge_t& edge) const {
      const auto &einfo = (*g)[edge];
      Graphs::EdgeType type = einfo.type;
      return (type == Graphs::EdgeType::TABLE);
    }
  private:
    Graphs::Graph *g;
};


class bfs_visitor : public boost::default_bfs_visitor {

 public:
    void discover_vertex(const Graphs::vertex_t &v, 
        const Graphs::Graph &g __attribute__((unused))) {
      if(v == to) 
        (*result) = 1;
    }
    
    void discover_vertex(const Graphs::vertex_t &v,
        const boost::filtered_graph<Graphs::Graph, edge_predicate_c> &g __attribute__((unused))) {
      if(v == to)
        (*result) = 1;
    }

    bfs_visitor(const Graphs::vertex_t &v, int *r):to(v), result(r){ }

  private:
    Graphs::vertex_t to;
    int *result;
};

bool Graphs::isActionIndependent(const vertex_t &v1, const vertex_t &v2) {
  boost::filtered_graph<Graph, edge_predicate_c> fg(g, edge_predicate_c(g));

  int r1 = 0;
  bfs_visitor vis1(v2, &r1);
  breadth_first_search(fg, v1, boost::visitor(vis1));
  if (r1 == 1)
    return false;
  
  int r2 = 0;
  bfs_visitor vis2(v1, &r2);
  breadth_first_search(fg, v2, boost::visitor(vis2));
  if (r2 == 1)
    return false;

  return true;
}


bool Graphs::isTableIndependent(const vertex_t &v1, const vertex_t &v2) {
  int r1 = 0;
  bfs_visitor vis1(v2, &r1);
  breadth_first_search(g, v1, boost::visitor(vis1));
  if (r1 == 1)
    return false;
  
  int r2 = 0;
  bfs_visitor vis2(v1, &r2);
  breadth_first_search(g, v2, boost::visitor(vis2));
  if (r2 == 1)
    return false;
  
  return true;
}

bool Graphs::isCondition(const vertex_t &v) {
  const auto &vinfo = g[v];
  return (vinfo.type == VertexType::CONDITION);
}

Graphs::vertex_t Graphs::add_vertex(const cstring &name, VertexType type) {
    auto v = boost::add_vertex(g);
    boost::put(&Vertex::name, g, v, name);
    boost::put(&Vertex::type, g, v, type);
    return g.local_to_global(v);
}

void Graphs::add_edge(const vertex_t &from, const vertex_t &to, const cstring &name, EdgeType type) {
    auto ep = boost::add_edge(from, to, g);
    boost::put(&Edge::name, g, ep.first, name);
    boost::put(&Edge::type, g, ep.first, type);
}

void Graphs::writeGraphToFile(const cstring &name) {
  GraphAttributeSetter()(g);
  auto path = name + ".dot";
  auto out = openFile(path, false);
  if (out == nullptr) {
    ::error("Failed to open file %1%", path);
    return;
  }
  boost::write_graphviz(*out, g);
}


}  // namespace multip4
