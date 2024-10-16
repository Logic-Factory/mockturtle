#pragma once

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include "fmt/format.h"
#include "lorina/gtech.hpp"
#include "mockturtle/generators/modular_arithmetic.hpp"
#include "mockturtle/traits.hpp"

namespace mockturtle
{

struct read_verilog_params
{
  std::optional<std::string> module_name{ std::nullopt };
  std::vector<std::pair<std::string, uint32_t>> input_names;
  std::vector<std::pair<std::string, uint32_t>> output_names;
};

/*! \brief Lorina reader callback for VERILOG files.
 *
 * **Required network functions:**
 * - `create_pi`
 * - `create_po`
 * - `get_constant`
 * - `create_not`
 * - `create_and`
 * - `create_nand`
 * - `create_or`
 * - `create_nor`
 * - `create_xor`
 * - `create_xnor`
 * - `create_nand3`
 * - `create_nor3`
 * - `create_mux21`
 * - `create_nmux21`
 * - `create_aoi21`
 * - `create_oai21`
 * - `create_axi21`
 * - `create_xai21`
 * - `create_oxi21`
 * - `create_xoi21`
 *
 *
   \verbatim embed:rst

   Example

   .. code-block:: c++

      gtech_network gtg;
      lorina::read_verilog( "file.v", gtech_reader( gtg ) );
   \endverbatim
 */
template<typename Ntk>
class gtech_reader : public lorina::gtech_reader
{
public:
  explicit gtech_reader( Ntk& ntk, read_verilog_params& port_infors, std::string const& top_module_name = "top" )
      : ntk_( ntk ),
        port_infors_( port_infors ),
        top_module_name_( top_module_name )
  {
    static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( has_create_pi_v<Ntk>, "Ntk does not implement the create_pi function" );
    static_assert( has_create_po_v<Ntk>, "Ntk does not implement the create_po function" );
    static_assert( has_get_constant_v<Ntk>, "Ntk does not implement the get_constant function" );
    static_assert( has_create_not_v<Ntk>, "Ntk does not implement the create_not function" );
    static_assert( has_create_and_v<Ntk>, "Ntk does not implement the create_and function" );
    static_assert( has_create_nand_v<Ntk>, "Ntk does not implement the create_nand function" );
    static_assert( has_create_or_v<Ntk>, "Ntk does not implement the create_or function" );
    static_assert( has_create_nor_v<Ntk>, "Ntk does not implement the create_nor function" );
    static_assert( has_create_xor_v<Ntk>, "Ntk does not implement the create_xor function" );
    static_assert( has_create_xnor_v<Ntk>, "Ntk does not implement the create_xnor function" );
    static_assert( has_create_nand3_v<Ntk>, "Ntk does not implement the create_nand3 function" );
    static_assert( has_create_nor3_v<Ntk>, "Ntk does not implement the create_nor3 function" );
    static_assert( has_create_mux21_v<Ntk>, "Ntk does not implement the create_mux21 function" );
    static_assert( has_create_nmux21_v<Ntk>, "Ntk does not implement the create_nmux21 function" );
    static_assert( has_create_aoi21_v<Ntk>, "Ntk does not implement the create_aoi21 function" );
    static_assert( has_create_oai21_v<Ntk>, "Ntk does not implement the create_oai21 function" );
    static_assert( has_create_axi21_v<Ntk>, "Ntk does not implement the create_axi21 function" );
    static_assert( has_create_xai21_v<Ntk>, "Ntk does not implement the create_xai21 function" );
    static_assert( has_create_oxi21_v<Ntk>, "Ntk does not implement the create_oxi21 function" );
    static_assert( has_create_xoi21_v<Ntk>, "Ntk does not implement the create_xoi21 function" );

    signals_["0"] = ntk_.get_constant( false );
    signals_["1"] = ntk_.get_constant( true );
    signals_["1'b0"] = ntk_.get_constant( false );
    signals_["1'b1"] = ntk_.get_constant( true );
    signals_["1'h0"] = ntk_.get_constant( false );
    signals_["1'h1"] = ntk_.get_constant( true );
  }

  void on_module_header( const std::string& module_name, const std::vector<std::string>& inouts ) const override
  {
    (void)inouts;
    if constexpr ( has_set_network_name_v<Ntk> )
    {
      ntk_.set_network_name( module_name );
    }

    name_ = module_name;
    port_infors_.module_name = module_name;
    if ( name_ != top_module_name_ )
    {
      std::cout << "warning:parser: current module name is not matching the given module name!" << std::endl;
    }
  }

  void on_inputs( const std::vector<std::string>& names, std::string const& size = "" ) const override
  {
    (void)size;
    for ( const auto& name : names )
    {
      if ( size.empty() )
      {
        signals_[name] = ntk_.create_pi();
        input_names_.emplace_back( name, 1u );
        port_infors_.input_names.emplace_back( name, 1u );
        if constexpr ( has_set_name_v<Ntk> )
        {
          ntk_.set_name( signals_[name], name );
        }
      }
      else
      {
        std::vector<signal<Ntk>> word;
        const auto length = parse_size( size );
        for ( auto i = 0u; i < length; ++i )
        {
          const auto sname = fmt::format( "{}[{}]", name, i );
          word.push_back( ntk_.create_pi() );
          signals_[sname] = word.back();
          if constexpr ( has_set_name_v<Ntk> )
          {
            ntk_.set_name( signals_[sname], sname );
          }
        }
        registers_[name] = word;
        input_names_.emplace_back( name, length );
        port_infors_.input_names.emplace_back( name, length );
      }
    }
  }

  void on_outputs( const std::vector<std::string>& names, std::string const& size = "" ) const override
  {
    (void)size;

    for ( const auto& name : names )
    {
      if ( size.empty() )
      {
        outputs_.emplace_back( name );
        output_names_.emplace_back( name, 1u );
        port_infors_.output_names.emplace_back( name, 1u );
      }
      else
      {
        const auto length = parse_size( size );
        for ( auto i = 0u; i < length; ++i )
        {
          outputs_.emplace_back( fmt::format( "{}[{}]", name, i ) );
        }
        output_names_.emplace_back( name, length );
        port_infors_.output_names.emplace_back( name, length );
      }
    }
  }

  /**
   * @brief on latch
   * @param lhs: Q
   * @param op1: D
   * @param init
   */
  void on_latch( const std::string& lhs, const std::pair<std::string, bool>& op1, latch_init_value init ) const override
  {
    latches_.push_back( std::make_tuple( lhs, op1.first, init ) );
  }

  void on_latch_output( const std::string& lhs ) const override
  {
    if constexpr ( has_create_ri_v<Ntk> && has_create_ro_v<Ntk> )
    {
      signals_[lhs] = ntk_.create_ro();
    }
  }

  void on_latch_input( const std::string& lhs ) const override
  {
    if constexpr ( has_create_ri_v<Ntk> && has_create_ro_v<Ntk> )
    {
    }
  }

  void on_wires( const std::vector<std::string>& wires, std::string const& size = "" ) const override
  {
    (void)size;

    for ( const auto& wire : wires )
    {
      if ( size.empty() )
      {
        wires_.insert( wire );
      }
      else
      {
        const auto length = parse_size( size );
        for ( auto i = 0u; i < length; ++i )
        {
          wires_.insert( fmt::format( "{}[{}]", wire, i ) );
        }
      }
    }
  }

  void on_assign( const std::string& lhs, const std::pair<std::string, bool>& rhs ) const override
  {
    if ( signals_.find( rhs.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", rhs.first );

    auto r = signals_[rhs.first];
    signals_[lhs] = rhs.second ? ntk_.create_not( r ) : r;
  }

  void on_zero( const std::string& lhs ) const override
  {
    signals_[lhs] = ntk_.get_constant( false );
  }

  void on_one( const std::string& lhs ) const override
  {
    signals_[lhs] = ntk_.get_constant( true );
  }

  void on_buf( const std::string& lhs, const std::pair<std::string, bool>& op1 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    auto a = signals_[op1.first];
    signals_[lhs] = ntk_.create_buf( a );
  }

  void on_not( const std::string& lhs, const std::pair<std::string, bool>& op1 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    auto a = signals_[op1.first];
    signals_[lhs] = ntk_.create_not( a );
  }

  void on_inv( const std::string& lhs, const std::pair<std::string, bool>& op1 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    auto a = signals_[op1.first];
    signals_[lhs] = ntk_.create_not( a );
  }

  void on_and( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    signals_[lhs] = ntk_.create_and( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b );
  }

  void on_nand( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    signals_[lhs] = ntk_.create_nand( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b );
  }

  void on_or( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    signals_[lhs] = ntk_.create_or( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b );
  }

  void on_nor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    signals_[lhs] = ntk_.create_nor( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b );
  }

  void on_xor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    signals_[lhs] = ntk_.create_xor( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b );
  }

  void on_xnor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    signals_[lhs] = ntk_.create_xnor( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b );
  }

  void on_maj( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_maj( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_ite( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_ite( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_xor3( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_xor3( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_nand3( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_nand3( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_nor3( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_nor3( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_mux21( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_mux21( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_nmux21( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_nmux21( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_aoi21( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_aoi21( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_oai21( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_oai21( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_axi21( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_axi21( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_xai21( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_xai21( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_oxi21( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_oxi21( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_xoi21( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    if ( signals_.find( op1.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op1.first );
    if ( signals_.find( op2.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op2.first );
    if ( signals_.find( op3.first ) == signals_.end() )
      fmt::print( stderr, "[w] undefined signal {} assigned 0\n", op3.first );

    auto a = signals_[op1.first];
    auto b = signals_[op2.first];
    auto c = signals_[op3.first];
    signals_[lhs] = ntk_.create_xoi21( op1.second ? ntk_.create_not( a ) : a, op2.second ? ntk_.create_not( b ) : b, op3.second ? ntk_.create_not( c ) : c );
  }

  void on_module_instantiation( std::string const& module_name, std::vector<std::string> const& params, std::string const& inst_name,
                                std::vector<std::pair<std::string, std::string>> const& args ) const override
  {
    (void)inst_name;

    /* check routines */
    const auto num_args_equals = [&]( uint32_t expected_count ) {
      if ( args.size() != expected_count )
      {
        fmt::print( stderr, "[e] {} module expects {} arguments\n", module_name, expected_count );
        return false;
      }
      return true;
    };

    const auto num_params_equals = [&]( uint32_t expected_count ) {
      if ( params.size() != expected_count )
      {
        fmt::print( stderr, "[e] {} module expects {} parameters\n", module_name, expected_count );
        return false;
      }
      return true;
    };

    const auto register_exists = [&]( std::string const& name ) {
      if ( registers_.find( name ) == registers_.end() )
      {
        fmt::print( stderr, "[e] register {} does not exist\n", name );
        return false;
      }
      return true;
    };

    const auto register_has_size = [&]( std::string const& name, uint32_t size ) {
      if ( !register_exists( name ) || registers_[name].size() != size )
      {
        fmt::print( stderr, "[e] register {} must have size {}\n", name, size );
        return false;
      }
      return true;
    };

    const auto add_register = [&]( std::string const& name, std::vector<signal<Ntk>> const& fs ) {
      for ( auto i = 0u; i < fs.size(); ++i )
      {
        signals_[fmt::format( "{}[{}]", name, i )] = fs[i];
      }
      registers_[name] = fs;
    };

    fmt::print( stderr, "[e] unknown module name {}\n", module_name );
  }

  void on_endmodule() const override
  {

    for ( auto const& o : outputs_ )
    {
      ntk_.create_po( signals_[o] );
    }

    for ( auto const& l : latches_ )
    {
      if constexpr ( has_create_ri_v<Ntk> && has_create_ro_v<Ntk> )
      {
        auto Q = std::get<0>( l );
        auto D = std::get<1>( l );
        if ( signals_.find( D ) == signals_.end() )
          fmt::print( stderr, "[w] undefined signal {} assigned 0\n", D );
        auto a = signals_[D];
        ntk_.create_ri( a );
      }
    }

    if constexpr ( has_set_output_name_v<Ntk> )
    {
      uint32_t ctr{ 0u };
      for ( auto const& output_name : output_names_ )
      {
        if ( output_name.second == 1u )
        {
          ntk_.set_output_name( ctr++, output_name.first );
        }
        else
        {
          for ( auto i = 0u; i < output_name.second; ++i )
          {
            ntk_.set_output_name( ctr++, fmt::format( "{}[{}]", output_name.first, i ) );
          }
        }
      }
      assert( ctr == ntk_.num_pos() );
    }
  }

  const std::string& name() const
  {
    return name_;
  }

  const std::vector<std::pair<std::string, uint32_t>> input_names()
  {
    return input_names_;
  }

  const std::vector<std::pair<std::string, uint32_t>> output_names()
  {
    return output_names_;
  }

private:
  std::vector<bool> parse_value( const std::string& value ) const
  {
    std::smatch match;

    if ( std::all_of( value.begin(), value.end(), isdigit ) )
    {
      std::vector<bool> res( 64u );
      bool_vector_from_dec( res, static_cast<uint64_t>( std::stoul( value ) ) );
      return res;
    }
    else if ( std::regex_match( value, match, hex_string ) )
    {
      std::vector<bool> res( static_cast<uint64_t>( std::stoul( match.str( 1 ) ) ) );
      bool_vector_from_hex( res, match.str( 2 ) );
      return res;
    }
    else
    {
      fmt::print( stderr, "[e] cannot parse number '{}'\n", value );
    }
    assert( false );
    return {};
  }

  uint64_t parse_small_value( const std::string& value ) const
  {
    return bool_vector_to_long( parse_value( value ) );
  }

  uint32_t parse_size( const std::string& size ) const
  {
    if ( size.empty() )
    {
      return 1u;
    }

    if ( auto const l = size.size(); l > 2 && size[l - 2] == ':' && size[l - 1] == '0' )
    {
      return static_cast<uint32_t>( parse_small_value( size.substr( 0u, l - 2 ) ) + 1u );
    }

    assert( false );
    return 0u;
  }

private:
  Ntk& ntk_;

  std::string const top_module_name_;
  mutable std::string name_;

  mutable std::map<std::string, signal<Ntk>> signals_;
  mutable std::map<std::string, std::vector<signal<Ntk>>> registers_;
  mutable std::vector<std::string> outputs_;
  mutable std::set<std::string> wires_;
  mutable std::vector<std::pair<std::string, uint32_t>> input_names_;
  mutable std::vector<std::pair<std::string, uint32_t>> output_names_;
  mutable std::vector<std::tuple<std::string, std::string, latch_init_value>> latches_;

  read_verilog_params& port_infors_;

  std::regex hex_string{ "(\\d+)'h([0-9a-fA-F]+)" };
};

} /* namespace mockturtle */