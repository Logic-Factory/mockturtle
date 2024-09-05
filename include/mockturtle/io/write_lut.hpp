#pragma once

#include "../traits.hpp"
#include "mockturtle/io/write_verilog.hpp"
#include "mockturtle/networks/klut.hpp"

namespace mockturtle
{
/**
 * @brief A structure os LUT
 * definition:
 *    LUT4 nj (                     //  type / name
 *        .o(_w2_),                 // o      , fanout , sould be set to wire
 *        .a(\b[0]),                // a -> o , fanin
 *        .b(\a[0]),                // b -> o , fanin
 *        .c(\b[1]),                // c -> o , fanin
 *        .d(\a[1]) );              // d -> o , fanin
 *    defparam nj.INIT = 16'h8778;  // .INTI(XXX) : function
 *
 * strcuture:
 *                o
 *                ^
 *                |
 *          --------------
 *          |    nj      |
 *          | (16'h8778) |
 *          --------------
 *            ^  ^  ^  ^
 *            |  |  |  |
 *            a  b  c  d
 */
struct LUT
{
  std::string name;
  kitty::dynamic_truth_table lut_function;
  std::string fanout;
  std::vector<std::string> fanins;
};

/*! \brief Writes network in LUT format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required network functions:**
 * - `is_constant`
 * - `is_pi`
 * - `is_complemented`
 * - `get_node`
 * - `num_pos`
 * - `node_to_index`
 * - `node_function`
 *
 * \param ntk Network
 * \param os Output stream
 */
template<class Ntk = klut_network>
void write_lut( Ntk const& ntk, std::ostream& os, write_verilog_params const& ps )
{
  std::vector<std::string> vec_pis;
  std::map<uint32_t, std::string> pi_map;
  uint32_t i = 0u;
  if ( ps.input_names.empty() )
  {
    ntk.foreach_pi( [&]( auto const& n ) {
      vec_pis.emplace_back( fmt::format( "_i{}_", ntk.node_to_index( n ) ) );
    } );
  }
  else
  {
    for ( auto const& [name, width] : ps.input_names )
    {
      (void)width;
      vec_pis.emplace_back( name ); // scalar only, TODO
    }
  }
  ntk.foreach_pi( [&]( auto const& n, auto i ) {
    pi_map[ntk.node_to_index( n )] = vec_pis[i];
  } );

  std::vector<std::string> vec_pos;
  if ( ps.output_names.empty() )
  {
    for ( auto i = 0u; i < ntk.num_pos(); ++i )
    {
      vec_pos.emplace_back( fmt::format( "_o{}_", i ) );
    }
  }
  else
  {
    for ( auto const& [name, width] : ps.output_names )
    {
      (void)width;
      vec_pos.emplace_back( name ); // scalar only, TODO
    }
  }

  // extract the LUTs
  std::vector<LUT> vec_LUTs;
  std::unordered_set<uint32_t> uset_wires; // using the set to flag the already exsit node.
  ntk.foreach_node( [&]( auto const& n ) {
    if ( ntk.is_constant( n ) || ntk.is_pi( n ) )
      return; /* continue */
    uset_wires.insert( ntk.node_to_index( n ) );
    LUT tmp_lut{};
    tmp_lut.lut_function = ntk.node_function( n );
    ntk.foreach_fanin( n, [&]( auto const& c, auto i ) {
      if ( ntk.is_complemented( c ) )
      {
        kitty::flip_inplace( tmp_lut.lut_function, i );
      }
      std::string tmp_fanin;
      if ( uset_wires.count( ntk.node_to_index( ntk.get_node( c ) ) ) )
        tmp_fanin = fmt::format( "_w{}_", ntk.node_to_index( ntk.get_node( c ) ) );
      else if ( pi_map.count( ntk.node_to_index( ntk.get_node( c ) ) ) )
        tmp_fanin = fmt::format( "{}", pi_map[ntk.node_to_index( ntk.get_node( c ) )] );
      else
        assert( 0 );
      tmp_lut.fanins.emplace_back( tmp_fanin );
    } );
    tmp_lut.name = fmt::format( "_g{}_", vec_LUTs.size() );
    tmp_lut.fanout = fmt::format( "_w{}_", ntk.node_to_index( n ) ); // should be node , not output
    vec_LUTs.emplace_back( tmp_lut );
  } );

  // fill all the lut verilog
  if ( ps.module_name.has_value() )
  {
    os << fmt::format( "module {} (", ps.module_name.value() );
  }
  else
  {
    os << "module top (";
  }
  bool first = true;
  for ( auto pi : vec_pis )
  {
    if ( first )
    {
      first = false;
      os << pi;
    }
    else
      os << fmt::format( ", {}", pi );
  }
  for ( auto po : vec_pos )
  {
    os << fmt::format( ", {}", po );
  }
  os << ");\n";
  for ( auto pi : vec_pis )
  {
    os << fmt::format( "\tinput {} ;\n", pi );
  }
  for ( auto po : vec_pos )
  {
    os << fmt::format( "\toutput {} ;\n", po );
  }
  for ( auto wire : uset_wires )
  {
    os << fmt::format( "\twire _w{}_ ;\n", wire );
  }
  for ( auto lut : vec_LUTs )
  {
    os << fmt::format( "\tLUT{} {} (\n", lut.fanins.size(), lut.name );
    os << fmt::format( "\t\t{},\n", lut.fanout );
    for ( i = 0u; i < lut.fanins.size() - 1; i++ )
    {
      os << fmt::format( "\t\t{},\n", lut.fanins[i] );
    }
    os << fmt::format( "\t\t{}\n", lut.fanins[i] );
    os << "\t);\n";
    os << fmt::format( "\tdefparam {}.INIT = {}'h{};\n\n", lut.name, 1 << lut.fanins.size(), kitty::to_hex( lut.lut_function ) );
  }

  ntk.foreach_po( [&]( auto const& s, auto i ) {
    if ( ntk.is_constant( ntk.get_node( s ) ) )
    {
      os << fmt::format( "\tassign {} = 1'b{};\n", vec_pos[i], ntk.node_to_index( ntk.get_node( s ) ) );
    }
    else if ( ntk.is_pi( ntk.get_node( s ) ) )
    {
      os << fmt::format( "\tassign {} = {};\n", vec_pos[i], pi_map[ntk.node_to_index( ntk.get_node( s ) )] );
    }
    else
    {
      os << fmt::format( "\tassign {} = _w{}_ ;\n", vec_pos[i], ntk.node_to_index( ntk.get_node( s ) ) );
    }
  } );
  os << "endmodule";
  os << std::flush;
}

/*! \brief Writes network in LUT format into a file
 *
 * **Required network functions:**
 * - `is_constant`
 * - `is_pi`
 * - `is_complemented`
 * - `get_node`
 * - `num_pos`
 * - `node_to_index`
 * - `node_function`
 *
 * \param ntk Network
 * \param filename Filename
 */
template<class Ntk>
void write_lut( Ntk const& ntk, std::string const& filename, write_verilog_params const& portnames = {} )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_lut( ntk, os, portnames );
  os.close();
}
} // namespace mockturtle