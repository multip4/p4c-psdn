/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef _MULTIP4_GRAPHS_H_
#define _MULTIP4_GRAPHS_H_

#include "config.h"

// Shouldn't happen as cmake will not try to build this backend if the boost
// graph headers couldn't be found.
#ifndef HAVE_LIBBOOST_GRAPH
#error "This backend requires the boost graph headers, which could not be found"
#endif

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include <boost/optional.hpp>

#include <map>
#include <utility>  // std::pair
#include <vector>

#include "ir/ir.h"
#include "ir/visitor.h"
#include "frontends/p4/parserCallGraph.h"

namespace PSDN {

class Graphs {
 public:
    enum class VertexType {
        TABLE,
        CONDITION,
        SWITCH,
        STATEMENTS,
        CONTROL,
        OTHER
    };
    enum class EdgeType {
      TABLE,
      ACTION
    };
    struct Vertex {
        cstring name;
        VertexType type;
    };
    struct Edge {
      cstring name;
      EdgeType type;
    };
    
    // The boost graph support for graphviz subgraphs is not very intuitive. In
    // particular the write_graphviz code assumes the existence of a lot of
    // properties. See
    // https://stackoverflow.com/questions/29312444/how-to-write-graphviz-subgraphs-with-boostwrite-graphviz
    // for more information.
    using GraphvizAttributes = std::map<cstring, cstring>;
    using vertexProperties =
        boost::property<boost::vertex_attribute_t, GraphvizAttributes,
        Vertex>;
    using edgeProperties =
        boost::property<boost::edge_attribute_t, GraphvizAttributes,
        boost::property<boost::edge_name_t, cstring,
        boost::property<boost::edge_index_t, int, 
        Edge> > >;
    using graphProperties =
        boost::property<boost::graph_name_t, cstring,
        boost::property<boost::graph_graph_attribute_t, GraphvizAttributes,
        boost::property<boost::graph_vertex_attribute_t, GraphvizAttributes,
        boost::property<boost::graph_edge_attribute_t, GraphvizAttributes> > > >;
    using Graph_ = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                         vertexProperties, edgeProperties,
                                         graphProperties>;
    using Graph = boost::subgraph<Graph_>;
    using vertex_t = boost::graph_traits<Graph>::vertex_descriptor;
    using edge_t = boost::graph_traits<Graph>::edge_descriptor;


    vertex_t add_vertex(const cstring &name, VertexType type);
    void add_edge(const vertex_t &from, const vertex_t &to, const cstring &name, EdgeType type);
    void writeGraphToFile(const cstring &name);
    bool isTableIndependent(const vertex_t &v1, const vertex_t &v2);
    bool isActionIndependent(const vertex_t &v1, const vertex_t &v2);
    bool isCondition(const vertex_t &v);
    void deleteActionEdge();

    class GraphAttributeSetter {
     public:
        void operator()(Graph &g) const {
            auto vertices = boost::vertices(g);
            for (auto vit = vertices.first; vit != vertices.second; ++vit) {
                const auto &vinfo = g[*vit];
                auto attrs = boost::get(boost::vertex_attribute, g);
                attrs[*vit]["label"] = vinfo.name;
                attrs[*vit]["style"] = vertexTypeGetStyle(vinfo.type);
                attrs[*vit]["shape"] = vertexTypeGetShape(vinfo.type);
                attrs[*vit]["margin"] = vertexTypeGetMargin(vinfo.type);
            }
            auto edges = boost::edges(g);
            for (auto eit = edges.first; eit != edges.second; ++eit) {
                const auto &einfo = g[*eit];
                auto attrs = boost::get(boost::edge_attribute, g);
                attrs[*eit]["label"] = einfo.name;
                attrs[*eit]["style"] = edgeTypeGetStyle(einfo.type);
            }
        }

     private:
        static cstring vertexTypeGetShape(VertexType type) {
            switch (type) {
            case VertexType::TABLE:
                return "ellipse";
            default:
                return "rectangle";
            }
            BUG("unreachable");
            return "";
        }

        static cstring vertexTypeGetStyle(VertexType type) {
            switch (type) {
            case VertexType::CONTROL:
                return "dashed";
            default:
                return "solid";
            }
            BUG("unreachable");
            return "";
        }

        static cstring vertexTypeGetMargin(VertexType type) {
            switch (type) {
            default:
                return "";
            }
        }

        static cstring edgeTypeGetStyle(EdgeType type) {
          switch (type) {
            case EdgeType::TABLE:
              return "solid";
            default:
              return "dashed";
          }
          BUG("unreachable");
          return "";
        }
    };  // end class GraphAttributeSetter

 protected:
    Graph g;
};

}  // namespace multip4

#endif  // _MULTIP4_GRAPHS_H_
