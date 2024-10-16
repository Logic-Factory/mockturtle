/* mockturtle: C++ logic network library
 * Copyright (C) 2018-2022  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*!
  \file write_dot.hpp
  \brief Write graphical representation of networks to DOT format

  \author Heinz Riener
  \author Mathias Soeken
  \author Marcel Walter
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "../traits.hpp"
#include "../views/depth_view.hpp"

namespace mockturtle
{
template<class Ntk>
class default_dot_drawer
{
public:
  virtual ~default_dot_drawer()
  {
  }

public: /* callbacks */
  virtual std::string node_label( Ntk const& ntk, node<Ntk> const& n ) const
  {
    return std::to_string( ntk.node_to_index( n ) );
  }

  virtual std::string node_shape( Ntk const& ntk, node<Ntk> const& n ) const
  {
    if ( ntk.is_constant( n ) )
    {
      return "box";
    }
    else if ( ntk.is_ci( n ) )
    {
      return "house";
    }
    else
    {
      if constexpr ( has_is_buf_v<Ntk> )
      {
        if ( ntk.is_buf( n ) )
        {
          return "box";
        }
      }
      return "ellipse";
    }
  }

  virtual uint32_t node_level( Ntk const& ntk, node<Ntk> const& n ) const
  {
    if ( !_depth_ntk )
    {
      _depth_ntk = std::make_shared<depth_view<Ntk>>( ntk );
    }

    return _depth_ntk->level( n );
  }

  virtual std::string po_shape( Ntk const& ntk, uint32_t i ) const
  {
    (void)ntk;
    (void)i;
    return "invhouse";
  }

  virtual std::string node_fillcolor( Ntk const& ntk, node<Ntk> const& n ) const
  {
    if constexpr ( has_is_buf_v<Ntk> )
    {
      if ( ntk.is_buf( n ) )
      {
        if ( ntk.fanout_size( n ) > 1 )
          return "lightcoral";
        else
          return "lightskyblue";
      }
    }
    return ( ntk.is_constant( n ) || ntk.is_ci( n ) ) ? "snow2" : "white";
  }

  virtual std::string po_fillcolor( Ntk const& ntk, uint32_t i ) const
  {
    (void)ntk;
    (void)i;
    return "snow2";
  }

  virtual bool draw_signal( Ntk const& ntk, node<Ntk> const& n, signal<Ntk> const& f ) const
  {
    (void)ntk;
    (void)n;
    (void)f;
    if constexpr ( is_buffered_network_type_v<Ntk> )
    {
      if ( ntk.is_constant( ntk.get_node( f ) ) )
        return false;
    }
    return true;
  }

  virtual std::string signal_style( Ntk const& ntk, signal<Ntk> const& f ) const
  {
    return ntk.is_complemented( f ) ? "dashed" : "solid";
  }

private:
  mutable std::shared_ptr<depth_view<Ntk>> _depth_ntk;
};

template<class Ntk>
class gate_dot_drawer : public default_dot_drawer<Ntk>
{
public:
  virtual std::string node_label( Ntk const& ntk, node<Ntk> const& n ) const override
  {
    if constexpr ( has_has_binding_v<Ntk> )
    {
      if ( ntk.has_binding( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-" + ntk.get_binding( n ).name;
      }
    }

    if constexpr ( has_is_buf_v<Ntk> )
    {
      if ( ntk.is_buf( n ) && !ntk.is_ci( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-buf";
      }
    }

    if constexpr ( has_is_not_v<Ntk> )
    {
      if ( ntk.is_not( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-not";
      }
    }

    if constexpr ( has_is_and_v<Ntk> )
    {
      if ( ntk.is_and( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-and2";
      }
    }

    if constexpr ( has_is_nand_v<Ntk> )
    {
      if ( ntk.is_nand( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-nand2";
      }
    }

    if constexpr ( has_is_or_v<Ntk> )
    {
      if ( ntk.is_or( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-or2";
      }
    }

    if constexpr ( has_is_nor_v<Ntk> )
    {
      if ( ntk.is_nor( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-nor2";
      }
    }

    if constexpr ( has_is_xor_v<Ntk> )
    {
      if ( ntk.is_xor( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-xor2";
      }
    }

    if constexpr ( has_is_xnor_v<Ntk> )
    {
      if ( ntk.is_xnor( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-xnor2";
      }
    }

    if constexpr ( has_is_maj_v<Ntk> )
    {
      if ( ntk.is_maj( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-maj3";
      }
    }

    if constexpr ( has_is_xor3_v<Ntk> )
    {
      if ( ntk.is_xor3( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-xor3";
      }
    }

    if constexpr ( has_is_mux21_v<Ntk> )
    {
      if ( ntk.is_mux21( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-mux21";
      }
    }

    if constexpr ( has_is_nmux21_v<Ntk> )
    {
      if ( ntk.is_nmux21( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-nmux21";
      }
    }

    if constexpr ( has_is_nand3_v<Ntk> )
    {
      if ( ntk.is_nand3( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-nand3";
      }
    }

    if constexpr ( has_is_nor3_v<Ntk> )
    {
      if ( ntk.is_nor3( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-nor3";
      }
    }

    if constexpr ( has_is_aoi21_v<Ntk> )
    {
      if ( ntk.is_aoi21( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-aoi21";
      }
    }

    if constexpr ( has_is_oai21_v<Ntk> )
    {
      if ( ntk.is_oai21( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-oai21";
      }
    }

    if constexpr ( has_is_axi21_v<Ntk> )
    {
      if ( ntk.is_axi21( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-axi21";
      }
    }

    if constexpr ( has_is_xai21_v<Ntk> )
    {
      if ( ntk.is_xai21( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-xai21";
      }
    }

    if constexpr ( has_is_oxi21_v<Ntk> )
    {
      if ( ntk.is_oxi21( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-oxi21";
      }
    }

    if constexpr ( has_is_xoi21_v<Ntk> )
    {
      if ( ntk.is_xoi21( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-xoi21";
      }
    }

    if constexpr ( has_is_nary_and_v<Ntk> )
    {
      if ( ntk.is_nary_and( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-and_n";
      }
    }

    if constexpr ( has_is_nary_or_v<Ntk> )
    {
      if ( ntk.is_nary_or( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-or_n";
      }
    }

    if constexpr ( has_is_nary_xor_v<Ntk> )
    {
      if ( ntk.is_nary_xor( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-xor_n";
      }
    }

    if constexpr ( has_is_crossing_v<Ntk> )
    {
      if ( ntk.is_crossing( n ) )
      {
        return std::to_string( ntk.node_to_index( n ) ) + "-cross";
      }
    }

    return default_dot_drawer<Ntk>::node_label( ntk, n );
  }

  virtual std::string node_fillcolor( Ntk const& ntk, node<Ntk> const& n ) const override
  {

    if constexpr ( has_is_buf_v<Ntk> )
    {
      if ( ntk.is_buf( n ) && !ntk.is_ci( n ) )
      {
        return "palegoldenrod";
      }
    }

    if constexpr ( has_is_not_v<Ntk> )
    {
      if ( ntk.is_not( n ) )
      {
        return "white";
      }
    }

    if constexpr ( has_is_and_v<Ntk> )
    {
      if ( ntk.is_and( n ) )
      {
        return "lightcoral";
      }
    }

    if constexpr ( has_is_nand_v<Ntk> )
    {
      if ( ntk.is_nand( n ) )
      {
        return "lightyellow";
      }
    }

    if constexpr ( has_is_or_v<Ntk> )
    {
      if ( ntk.is_or( n ) )
      {
        return "palegreen2";
      }
    }

    if constexpr ( has_is_nor_v<Ntk> )
    {
      if ( ntk.is_nor( n ) )
      {
        return "lightpink";
      }
    }

    if constexpr ( has_is_xor_v<Ntk> )
    {
      if ( ntk.is_xor( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_xnor_v<Ntk> )
    {
      if ( ntk.is_xnor( n ) )
      {
        return "lightsalmon";
      }
    }

    if constexpr ( has_is_maj_v<Ntk> )
    {
      if ( ntk.is_maj( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_xor3_v<Ntk> )
    {
      if ( ntk.is_xor3( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_mux21_v<Ntk> )
    {
      if ( ntk.is_mux21( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_nmux21_v<Ntk> )
    {
      if ( ntk.is_nmux21( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_nand3_v<Ntk> )
    {
      if ( ntk.is_nand3( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_nor3_v<Ntk> )
    {
      if ( ntk.is_nor3( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_aoi21_v<Ntk> )
    {
      if ( ntk.is_aoi21( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_oai21_v<Ntk> )
    {
      if ( ntk.is_oai21( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_axi21_v<Ntk> )
    {
      if ( ntk.is_axi21( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_xai21_v<Ntk> )
    {
      if ( ntk.is_xai21( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_oxi21_v<Ntk> )
    {
      if ( ntk.is_oxi21( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_xoi21_v<Ntk> )
    {
      if ( ntk.is_xoi21( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_nary_and_v<Ntk> )
    {
      if ( ntk.is_nary_and( n ) )
      {
        return "lightcoral";
      }
    }

    if constexpr ( has_is_nary_or_v<Ntk> )
    {
      if ( ntk.is_nary_or( n ) )
      {
        return "palegreen2";
      }
    }

    if constexpr ( has_is_nary_xor_v<Ntk> )
    {
      if ( ntk.is_nary_xor( n ) )
      {
        return "lightskyblue";
      }
    }

    if constexpr ( has_is_crossing_v<Ntk> )
    {
      if ( ntk.is_crossing( n ) )
      {
        return "palegoldenrod";
      }
    }

    return default_dot_drawer<Ntk>::node_fillcolor( ntk, n );
  }

  virtual bool draw_signal( Ntk const& ntk, node<Ntk> const& n, signal<Ntk> const& f ) const override
  {
    if constexpr ( has_is_maj_v<Ntk> )
    {
      if ( ntk.is_maj( n ) )
      {
        return !ntk.is_constant( ntk.get_node( f ) );
      }
    }

    return default_dot_drawer<Ntk>::draw_signal( ntk, n, f );
  }
};

/*! \brief Writes network in DOT format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required network functions:**
 * - is_constant
 * - is_ci
 * - foreach_node
 * - foreach_fanin
 * - foreach_po
 *
 * \param ntk Network
 * \param os Output stream
 */
template<class Ntk, class Drawer = default_dot_drawer<Ntk>>
void write_dot( Ntk const& ntk, std::ostream& os, Drawer const& drawer = {} )
{
  static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
  static_assert( has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
  static_assert( has_is_ci_v<Ntk>, "Ntk does not implement the is_ci method" );
  static_assert( has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );
  static_assert( has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method" );
  static_assert( has_foreach_po_v<Ntk>, "Ntk does not implement the foreach_po method" );

  std::stringstream nodes, edges, levels;

  std::vector<std::vector<uint32_t>> level_to_node_indexes;

  ntk.foreach_node( [&]( auto const& n ) {
    std::string node = fmt::format( "{} [label=\"{}\",shape={},style=filled,fillcolor={}]\n",
                                    ntk.node_to_index( n ),
                                    drawer.node_label( ntk, n ),
                                    drawer.node_shape( ntk, n ),
                                    drawer.node_fillcolor( ntk, n ) );
    nodes << node;
    if ( !ntk.is_constant( n ) && !ntk.is_ci( n ) )
    {
      ntk.foreach_fanin( n, [&]( auto const& f ) {
        // if ( !drawer.draw_signal( ntk, n, f ) )
        //   return true;
        std::string edge = fmt::format( "{} -> {} [style={}]\n",
                                        ntk.node_to_index( ntk.get_node( f ) ),
                                        ntk.node_to_index( n ),
                                        drawer.signal_style( ntk, f ) );
        edges << edge;
        return true;
      } );
    }

    const auto lvl = drawer.node_level( ntk, n );
    if ( level_to_node_indexes.size() <= lvl )
    {
      level_to_node_indexes.resize( lvl + 1 );
    }
    level_to_node_indexes[lvl].push_back( ntk.node_to_index( n ) );
  } );

  for ( auto const& indexes : level_to_node_indexes )
  {
    levels << "{rank = same; ";
    std::copy( indexes.begin(), indexes.end(), std::ostream_iterator<uint32_t>( levels, "; " ) );
    levels << "}\n";
  }

  levels << "{rank = same; ";
  ntk.foreach_po( [&]( auto const& f, auto i ) {
    nodes << fmt::format( "po{} [shape={},style=filled,fillcolor={}]\n", i, drawer.po_shape( ntk, i ), drawer.po_fillcolor( ntk, i ) );
    edges << fmt::format( "{} -> po{} [style={}]\n",
                          ntk.node_to_index( ntk.get_node( f ) ),
                          i,
                          drawer.signal_style( ntk, f ) );
    levels << fmt::format( "po{}; ", i );
  } );
  levels << "}\n";

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t( now );
  std::tm* tm = std::localtime( &t );
  char buffer[80];
  std::strftime( buffer, sizeof( buffer ), "%Y-%m-%d %H:%M:%S", tm );
  std::string curr_time = std::string( buffer );

  std::string legned = "powerd by LogicFactory \n (" + curr_time + ")\n";
  os << "digraph {\n"
     << "rankdir=BT;\n"
     << "label=\"" << legned << "\";"
     << "labelloc = \"b\";\n"
     << nodes.str() << edges.str() << levels.str() << "}\n";
}

/*! \brief Writes network in DOT format into a file
 *
 * **Required network functions:**
 * - is_constant
 * - is_ci
 * - foreach_node
 * - foreach_fanin
 * - foreach_po
 *
 * \param ntk Network
 * \param filename Filename
 */
template<class Ntk, class Drawer = gate_dot_drawer<Ntk>>
void write_dot( Ntk const& ntk, std::string const& filename, Drawer const& drawer = {} )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_dot( ntk, os, drawer );
  os.close();
}

} /* namespace mockturtle */