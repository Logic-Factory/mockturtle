#pragma once

#include "common.hpp"
#include "detail/tokenizer.hpp"
#include "detail/utils.hpp"
#include "diagnostics.hpp"
#include "verilog_regex.hpp"
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>

namespace lorina
{

/*! \brief A reader visitor for a simplistic GTECH VERILOG format.
 *
 * Callbacks for the GTECH VERILOG format.
 *
 *  buffer、not、and、nand、or、nor、xor、xnor
 */
class gtech_reader
{
public:
  /*! \brief Callback method for parsed module.
   *
   * \param module_name Name of the module
   * \param inouts Container for input and output names
   */
  virtual void on_module_header( const std::string& module_name, const std::vector<std::string>& inouts ) const
  {
    (void)module_name;
    (void)inouts;
  }

  /*! \brief Callback method for parsed inputs.
   *
   * \param inputs Input names
   * \param size Size modifier
   */
  virtual void on_inputs( const std::vector<std::string>& inputs, std::string const& size = "" ) const
  {
    (void)inputs;
    (void)size;
  }

  /*! \brief Callback method for parsed outputs.
   *
   * \param outputs Output names
   * \param size Size modifier
   */
  virtual void on_outputs( const std::vector<std::string>& outputs, std::string const& size = "" ) const
  {
    (void)outputs;
    (void)size;
  }

  /*! \brief Callback method for parsed wires.
   *
   * \param wires Wire names
   * \param size Size modifier
   */
  virtual void on_wires( const std::vector<std::string>& wires, std::string const& size = "" ) const
  {
    (void)wires;
    (void)size;
  }

  /*! \brief Callback method for parsed parameter definition of form ` parameter M = 10;`.
   *
   * \param name Name of the parameter
   * \param value Value of the parameter
   */
  virtual void on_parameter( const std::string& name, const std::string& value ) const
  {
    (void)name;
    (void)value;
  }

  /*! \brief Callback method for parsed immediate assignment of form `LHS = RHS ;`.
   *
   * \param lhs Left-hand side of assignment
   * \param rhs Right-hand side of assignment
   */
  virtual void on_assign( const std::string& lhs, const std::pair<std::string, bool>& rhs ) const
  {
    (void)lhs;
    (void)rhs;
  }

  /*! \brief Callback method for parsed module instantiation of form `NAME #(P1,P2) NAME(.SIGNAL(SIGANL), ..., .SIGNAL(SIGNAL));`
   *
   * \param module_name Name of the module
   * \param params List of parameters
   * \param inst_name Name of the instantiation
   * \param args List (a_1,b_1), ..., (a_n,b_n) of name pairs, where
   *             a_i is a name of a signals in module_name and b_i is a name of a
   *             signal in inst_name.
   */
  virtual void on_module_instantiation( std::string const& module_name, std::vector<std::string> const& params, std::string const& inst_name,
                                        std::vector<std::pair<std::string, std::string>> const& args ) const
  {
    (void)module_name;
    (void)params;
    (void)inst_name;
    (void)args;
  }

  /*! \brief Callback method for parsed BUF-gate with 1 operands `LHS = OP1;`.
   *
   * \param lhs Left-hand side of assignment
   * \param op1 operand1 of assignment
   */
  virtual void on_buf( const std::string& lhs, const std::pair<std::string, bool>& op1 ) const
  {
    (void)lhs;
    (void)op1;
  }

  /*! \brief Callback method for parsed NOT-gate with 1 operands `LHS = !OP1;`.
   *
   * \param lhs Left-hand side of assignment
   * \param op1 operand1 of assignment
   */
  virtual void on_not( const std::string& lhs, const std::pair<std::string, bool>& op1 ) const
  {
    (void)lhs;
    (void)op1;
  }

  /*! \brief Callback method for parsed AND-gate with 2 operands `LHS = OP1 & OP2 ;`.
   *
   * \param lhs Left-hand side of assignment
   * \param op1 operand1 of assignment
   * \param op2 operand2 of assignment
   */
  virtual void on_and( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const
  {
    (void)lhs;
    (void)op1;
    (void)op2;
  }

  /*! \brief Callback method for parsed NAND-gate with 2 operands `LHS = ~(OP1 & OP2) ;`.
   *
   * \param lhs Left-hand side of assignment
   * \param op1 operand1 of assignment
   * \param op2 operand2 of assignment
   */
  virtual void on_nand( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const
  {
    (void)lhs;
    (void)op1;
    (void)op2;
  }

  /*! \brief Callback method for parsed OR-gate with 2 operands `LHS = OP1 | OP2 ;`.
   *
   * \param lhs Left-hand side of assignment
   * \param op1 operand1 of assignment
   * \param op2 operand2 of assignment
   */
  virtual void on_or( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const
  {
    (void)lhs;
    (void)op1;
    (void)op2;
  }

  /*! \brief Callback method for parsed NOR-gate with 2 operands `LHS = ~(OP1 | OP2) ;`.
   *
   * \param lhs Left-hand side of assignment
   * \param op1 operand1 of assignment
   * \param op2 operand2 of assignment
   */
  virtual void on_nor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const
  {
    (void)lhs;
    (void)op1;
    (void)op2;
  }

  /*! \brief Callback method for parsed XOR-gate with 2 operands `LHS = OP1 ^ OP2 ;`.
   *
   * \param lhs Left-hand side of assignment
   * \param op1 operand1 of assignment
   * \param op2 operand2 of assignment
   */
  virtual void on_xor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const
  {
    (void)lhs;
    (void)op1;
    (void)op2;
  }

  /*! \brief Callback method for parsed XOR-gate with 2 operands `LHS = ~(OP1 ^ OP2) ;`.
   *
   * \param lhs Left-hand side of assignment
   * \param op1 operand1 of assignment
   * \param op2 operand2 of assignment
   */
  virtual void on_xnor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const
  {
    (void)lhs;
    (void)op1;
    (void)op2;
  }

  /*! \brief Callback method for parsed comments `// comment string`.
   *
   * \param comment Comment string
   */
  virtual void on_comment( std::string const& comment ) const
  {
    (void)comment;
  }

  /*! \brief Callback method for parsed endmodule.
   *
   */
  virtual void on_endmodule() const {}

}; /* gtech_reader */

/*! \brief A GTECH VERILOG reader for prettyprinting a simplistic GTECH VERILOG format.
 *
 * Callbacks for prettyprinting of BLIF.
 *
 */
class gtech_pretty_printer : public gtech_reader
{
public:
  /*! \brief Constructor of the GTECH VERILOG pretty printer.
   *
   * \param os Output stream
   */
  gtech_pretty_printer( std::ostream& os = std::cout )
      : _os( os )
  {
  }

  void on_module_header( const std::string& module_name, const std::vector<std::string>& inouts ) const override
  {
    std::string params;
    if ( inouts.size() > 0 )
    {
      params = inouts[0];
      for ( auto i = 1u; i < inouts.size(); ++i )
      {
        params += " , ";
        params += inouts[i];
      }
    }
    _os << fmt::format( "module {}( {} ) ;\n", module_name, params );
  }

  void on_inputs( const std::vector<std::string>& inputs, std::string const& size = "" ) const override
  {
    if ( inputs.size() == 0 )
      return;
    _os << "input ";
    if ( size != "" )
      _os << "[" << size << "] ";

    _os << inputs[0];
    for ( auto i = 1u; i < inputs.size(); ++i )
    {
      _os << " , ";
      _os << inputs[i];
    }
    _os << " ;\n";
  }

  void on_outputs( const std::vector<std::string>& outputs, std::string const& size = "" ) const override
  {
    if ( outputs.size() == 0 )
      return;
    _os << "output ";
    if ( size != "" )
      _os << "[" << size << "] ";

    _os << outputs[0];
    for ( auto i = 1u; i < outputs.size(); ++i )
    {
      _os << " , ";
      _os << outputs[i];
    }
    _os << " ;\n";
  }

  void on_wires( const std::vector<std::string>& wires, std::string const& size = "" ) const override
  {
    if ( wires.size() == 0 )
      return;
    _os << "wire ";
    if ( size != "" )
      _os << "[" << size << "] ";

    _os << wires[0];
    for ( auto i = 1u; i < wires.size(); ++i )
    {
      _os << " , ";
      _os << wires[i];
    }
    _os << " ;\n";
  }

  void on_parameter( const std::string& name, const std::string& value ) const override
  {
    _os << "parameter " << name << " = " << value << ";\n";
  }

  void on_assign( const std::string& lhs, const std::pair<std::string, bool>& rhs ) const override
  {
    const std::string param = rhs.second ? fmt::format( "~{}", rhs.first ) : rhs.first;
    _os << fmt::format( "assign {} = {} ;\n", lhs, param );
  }

  virtual void on_module_instantiation( std::string const& module_name, std::vector<std::string> const& params, std::string const& inst_name,
                                        std::vector<std::pair<std::string, std::string>> const& args ) const override
  {
    _os << module_name << " ";
    if ( params.size() > 0u )
    {
      _os << "#(";
      for ( auto i = 0u; i < params.size(); ++i )
      {
        _os << params.at( i );
        if ( i + 1 < params.size() )
          _os << ", ";
      }
      _os << ")";
    }

    _os << " " << inst_name << "(";
    for ( auto i = 0u; i < args.size(); ++i )
    {
      _os << args.at( i ).first << "(" << args.at( i ).second << ")";
      if ( i + 1 < args.size() )
        _os << ", ";
    }
    _os << ")";

    _os << ";\n";
  }

  void on_not( const std::string& lhs, const std::pair<std::string, bool>& op1 ) const override
  {
    const std::string p1 = op1.second ? fmt::format( "~{}", op1.first ) : op1.first;
    _os << fmt::format( "assign {} = ! {};\n", lhs, p1 );
  }

  void on_buf( const std::string& lhs, const std::pair<std::string, bool>& op1 ) const override
  {
    const std::string p1 = op1.second ? fmt::format( "~{}", op1.first ) : op1.first;
    _os << fmt::format( "assign {} = {};\n", lhs, p1 );
  }

  void on_and( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    const std::string p1 = op1.second ? fmt::format( "~{}", op1.first ) : op1.first;
    const std::string p2 = op2.second ? fmt::format( "~{}", op2.first ) : op2.first;
    _os << fmt::format( "assign {} = {} & {} ;\n", lhs, p1, p2 );
  }

  void on_nand( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    const std::string p1 = op1.second ? fmt::format( "~{}", op1.first ) : op1.first;
    const std::string p2 = op2.second ? fmt::format( "~{}", op2.first ) : op2.first;
    _os << fmt::format( "assign {} = ~({} & {}) ;\n", lhs, p1, p2 );
  }

  void on_or( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    const std::string p1 = op1.second ? fmt::format( "~{}", op1.first ) : op1.first;
    const std::string p2 = op2.second ? fmt::format( "~{}", op2.first ) : op2.first;
    _os << fmt::format( "assign {} = {} | {} ;\n", lhs, p1, p2 );
  }

  void on_nor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    const std::string p1 = op1.second ? fmt::format( "~{}", op1.first ) : op1.first;
    const std::string p2 = op2.second ? fmt::format( "~{}", op2.first ) : op2.first;
    _os << fmt::format( "assign {} = ~({} | {}) ;\n", lhs, p1, p2 );
  }

  void on_xor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    const std::string p1 = op1.second ? fmt::format( "~{}", op1.first ) : op1.first;
    const std::string p2 = op2.second ? fmt::format( "~{}", op2.first ) : op2.first;
    _os << fmt::format( "assign {} = {} ^ {} ;\n", lhs, p1, p2 );
  }

  void on_xnor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    const std::string p1 = op1.second ? fmt::format( "~{}", op1.first ) : op1.first;
    const std::string p2 = op2.second ? fmt::format( "~{}", op2.first ) : op2.first;
    _os << fmt::format( "assign {} = ~({} ^ {}) ;\n", lhs, p1, p2 );
  }

  void on_endmodule() const override
  {
    _os << "endmodule\n"
        << std::endl;
  }

  void on_comment( const std::string& comment ) const override
  {
    _os << "// " << comment << std::endl;
  }

  std::ostream& _os; /*!< Output stream */
}; /* gtech_pretty_printer */

/*! \brief A writer for a simplistic GTECH VERILOG format.
 *
 * Callbacks for writing the GTECH VERILOG format.
 *
 */
class gtech_writer
{
public:
  /*! \brief Constructs a GTECH VERILOG writer.
   *
   * \param os Output stream
   */
  explicit gtech_writer( std::ostream& os )
      : _os( os )
  {
  }

  /*! \brief Callback method for writing begin of a module declaration.
   *
   * \param name Module name
   * \param xs List of module inputs
   * \param ys List of module outputs
   */
  virtual void on_module_begin( std::string const& name, std::vector<std::string> const& xs, std::vector<std::string> const& ys ) const
  {
    std::vector<std::string> names = xs;
    std::copy( ys.begin(), ys.end(), std::back_inserter( names ) );

    _os << fmt::format( "module {}( {} );\n", name, fmt::join( names, " , " ) );
  }

  /*! \brief Callback method for writing single 1-bit input.
   *
   * \param name Input name
   */
  virtual void on_input( std::string const& name ) const
  {
    _os << fmt::format( "  input {} ;\n", name );
  }

  /*! \brief Callback method for writing input register.
   *
   * \param width Register size
   * \param name Input name
   */
  virtual void on_input( uint32_t width, std::string const& name ) const
  {
    _os << fmt::format( "  input [{}:0] {} ;\n", width - 1, name );
  }

  /*! \brief Callback method for writing multiple single 1-bit input.
   *
   * \param names Input names
   */
  virtual void on_input( std::vector<std::string> const& names ) const
  {
    _os << fmt::format( "  input {} ;\n", fmt::join( names, " , " ) );
  }

  /*! \brief Callback method for writing multiple input registers.
   *
   * \param width Register size
   * \param names Input names
   */
  virtual void on_input( uint32_t width, std::vector<std::string> const& names ) const
  {
    _os << fmt::format( "  input [{}:0] {} ;\n", width - 1, fmt::join( names, " , " ) );
  }

  /*! \brief Callback method for writing single 1-bit output.
   *
   * \param name Output name
   */
  virtual void on_output( std::string const& name ) const
  {
    _os << fmt::format( "  output {} ;\n", name );
  }

  /*! \brief Callback method for writing output register.
   *
   * \param width Register size
   * \param name Output name
   */
  virtual void on_output( uint32_t width, std::string const& name ) const
  {
    _os << fmt::format( "  output [{}:0] {} ;\n", width - 1, name );
  }

  /*! \brief Callback method for writing multiple single 1-bit output.
   *
   * \param names Output names
   */
  virtual void on_output( std::vector<std::string> const& names ) const
  {
    _os << fmt::format( "  output {} ;\n", fmt::join( names, " , " ) );
  }

  /*! \brief Callback method for writing multiple output registers.
   *
   * \param width Register size
   * \param names Output names
   */
  virtual void on_output( uint32_t width, std::vector<std::string> const& names ) const
  {
    _os << fmt::format( "  output [{}:0] {} ;\n", width - 1, fmt::join( names, " , " ) );
  }

  /*! \brief Callback method for writing single 1-bit wire.
   *
   * \param name Wire name
   */
  virtual void on_wire( std::string const& name ) const
  {
    _os << fmt::format( "  wire {} ;\n", name );
  }

  /*! \brief Callback method for writing wire register.
   *
   * \param width Register size
   * \param name Wire name
   */
  virtual void on_wire( uint32_t width, std::string const& name ) const
  {
    _os << fmt::format( "  wire [{}:0] {} ;\n", width - 1, name );
  }

  /*! \brief Callback method for writing multiple single 1-bit wire.
   *
   * \param names Wire names
   */
  virtual void on_wire( std::vector<std::string> const& names ) const
  {
    _os << fmt::format( "  wire {} ;\n", fmt::join( names, " , " ) );
  }

  /*! \brief Callback method for writing multiple wire registers.
   *
   * \param width Register size
   * \param names Wire names
   */
  virtual void on_wire( uint32_t width, std::vector<std::string> const& names ) const
  {
    _os << fmt::format( "  wire [{}:0] {} ;\n", width - 1, fmt::join( names, " , " ) );
  }

  /*! \brief Callback method for writing end of a module declaration. */
  virtual void on_module_end() const
  {
    _os << "endmodule" << std::endl;
  }

  /*! \brief Callback method for writing a module instantiation.
   *
   * \param module_name Module name
   * \param params List of parameters
   * \param inst_name Instance name
   * \param args List of arguments (first: I/O pin name, second: wire name)
   */
  virtual void on_module_instantiation( std::string const& module_name, std::vector<std::string> const& params, std::string const& inst_name,
                                        std::vector<std::pair<std::string, std::string>> const& args ) const
  {
    _os << fmt::format( "  {} ", module_name );
    if ( params.size() > 0u )
    {
      _os << "#(";
      for ( auto i = 0u; i < params.size(); ++i )
      {
        _os << params.at( i );
        if ( i + 1 < params.size() )
          _os << ", ";
      }
      _os << ") ";
    }

    _os << fmt::format( "{}( ", inst_name );
    for ( auto i = 0u; i < args.size(); ++i )
    {
      _os << fmt::format( ".{} ({})", args.at( i ).first, args.at( i ).second );
      if ( i + 1 < args.size() )
        _os << ", ";
    }
    _os << " );\n";
  }

  /*! \brief Callback method for writing an assignment statement.
   *
   * \param out Output signal
   * \param ins List of input signals
   * \param op Operator
   * \param no_neg whether need to negate the output
   */
  virtual void on_assign( std::string const& out, std::vector<std::pair<bool, std::string>> const& ins, std::string const& op, bool no_neg = true ) const
  {
    std::string args;

    /* assemble arguments */
    for ( auto i = 0u; i < ins.size(); ++i )
    {
      args.append( fmt::format( "{}{}", ins.at( i ).first ? "~" : "", ins.at( i ).second ) );
      if ( i != ins.size() - 1 )
        args.append( fmt::format( " {} ", op ) );
    }

    if ( no_neg )
    {
      _os << fmt::format( "  assign {} = {} ;\n", out, args );
    }
    else
    {
      _os << fmt::format( "  assign {} = ~( {} ) ;\n", out, args );
    }
  }

  void on_not( const std::string& lhs, std::vector<std::pair<bool, std::string>> const& ins ) const
  {
    assert( ins.size() == 1 );
    const std::string p1 = ins.at( 0 ).first ? fmt::format( "~{}", ins.at( 0 ).second ) : ins.at( 0 ).second;
    _os << fmt::format( "  assign {} = ~{};\n", lhs, p1 );
  }

  void on_buf( const std::string& lhs, std::vector<std::pair<bool, std::string>> const& ins ) const
  {
    assert( ins.size() == 1 );
    const std::string p1 = ins.at( 0 ).first ? fmt::format( "~{}", ins.at( 0 ).second ) : ins.at( 0 ).second;
    _os << fmt::format( "  assign {} = {};\n", lhs, p1 );
  }

  void on_and( const std::string& lhs, std::vector<std::pair<bool, std::string>> const& ins ) const
  {
    on_assign( lhs, ins, "&" );
  }

  void on_nand( const std::string& lhs, std::vector<std::pair<bool, std::string>> const& ins ) const
  {
    on_assign( lhs, ins, "&", false );
  }

  void on_or( const std::string& lhs, std::vector<std::pair<bool, std::string>> const& ins ) const
  {
    on_assign( lhs, ins, "|" );
  }

  void on_nor( const std::string& lhs, std::vector<std::pair<bool, std::string>> const& ins ) const
  {
    on_assign( lhs, ins, "|", false );
  }

  void on_xor( const std::string& lhs, std::vector<std::pair<bool, std::string>> const& ins ) const
  {
    on_assign( lhs, ins, "^" );
  }

  void on_xnor( const std::string& lhs, std::vector<std::pair<bool, std::string>> const& ins ) const
  {
    on_assign( lhs, ins, "^", false );
  }

  /*! \brief Callback method for writing an assignment statement with unknown operator.
   *
   * \param out Output signal
   */
  virtual void on_assign_unknown_gate( std::string const& out ) const
  {
    _os << fmt::format( "  assign {} = unknown gate;\n", out );
  }

  /*! \brief Callback method for writing a maj3 assignment statement.
   *
   * \param out Output signal
   * \param in An input signal
   */
  virtual void on_assign_po( std::string const& out, std::pair<bool, std::string> const& in ) const
  {
    _os << fmt::format( "  assign {} = {}{} ;\n",
                        out,
                        in.first ? "~" : "", in.second );
  }

protected:
  std::ostream& _os; /*!< Output stream */
}; /* gtech_writer */

/*! \brief Simple parser for GTECH VERILOG format.
 *
 * Simplistic grammar-oriented parser for a structural GTECH VERILOG format.
 *
 */
class gtech_parser
{
public:
  struct module_info
  {
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
  };

public:
  /*! \brief Construct a GTECH VERILOG parser
   *
   * \param in Input stream
   * \param reader A verilog reader
   * \param diag A diagnostic engine
   */
  gtech_parser( std::istream& in,
                const gtech_reader& reader,
                diagnostic_engine* diag = nullptr )
      : tok( in ), reader( reader ), diag( diag ), on_action( PackedFns( GateFn( [&]( const std::vector<std::pair<std::string, bool>>& inputs,
                                                                                      const std::string output,
                                                                                      const std::string type ) {
                                                                           if ( type == "assign" )
                                                                           {
                                                                             assert( inputs.size() == 1u );
                                                                             reader.on_assign( output, inputs[0] );
                                                                           }
                                                                           else if ( type == "not" )
                                                                           {
                                                                             assert( inputs.size() == 1u );
                                                                             reader.on_not( output, inputs[0] );
                                                                           }
                                                                           else if ( type == "buf" )
                                                                           {
                                                                             assert( inputs.size() == 1u );
                                                                             reader.on_buf( output, inputs[0] );
                                                                           }
                                                                           else if ( type == "and" )
                                                                           {
                                                                             assert( inputs.size() == 2u );
                                                                             reader.on_and( output, inputs[0], inputs[1] );
                                                                           }
                                                                           else if ( type == "nand" )
                                                                           {
                                                                             assert( inputs.size() == 2u );
                                                                             reader.on_nand( output, inputs[0], inputs[1] );
                                                                           }
                                                                           else if ( type == "or" )
                                                                           {
                                                                             assert( inputs.size() == 2u );
                                                                             reader.on_or( output, inputs[0], inputs[1] );
                                                                           }
                                                                           else if ( type == "nor" )
                                                                           {
                                                                             assert( inputs.size() == 2u );
                                                                             reader.on_nor( output, inputs[0], inputs[1] );
                                                                           }
                                                                           else if ( type == "xor" )
                                                                           {
                                                                             assert( inputs.size() == 2u );
                                                                             reader.on_xor( output, inputs[0], inputs[1] );
                                                                           }
                                                                           else if ( type == "xnor" )
                                                                           {
                                                                             assert( inputs.size() == 2u );
                                                                             reader.on_xnor( output, inputs[0], inputs[1] );
                                                                           }
                                                                           else
                                                                           {
                                                                             assert( false && "unknown gate function" );
                                                                             std::cerr << "unknown gate function" << std::endl;
                                                                             std::abort();
                                                                           }
                                                                         } ),
                                                                         ModuleInstFn( [&]( const std::string module_name,
                                                                                            const std::vector<std::string>& params,
                                                                                            const std::string instance_name,
                                                                                            const std::vector<std::pair<std::string, std::string>>& pin_to_pin ) {
                                                                           reader.on_module_instantiation( module_name, params, instance_name, pin_to_pin );
                                                                         } ) ) )
  {
    on_action.declare_known( "0" );
    on_action.declare_known( "1" );
    on_action.declare_known( "1'b0" );
    on_action.declare_known( "1'b1" );
    set_all_wires.insert( "0" );
    set_all_wires.insert( "1" );
    set_all_wires.insert( "1'b0" );
    set_all_wires.insert( "1'b1" );
  }

  bool get_token( std::string& token )
  {
    detail::tokenizer_return_code result;
    do
    {
      if ( tokens.empty() )
      {
        result = tok.get_token_internal( token );
        detail::trim( token );
      }
      else
      {
        token = tokens.front();
        tokens.pop();
        return true;
      }

      /* switch to comment mode */
      if ( token == "//" && result == detail::tokenizer_return_code::valid )
      {
        tok.set_comment_mode();
      }
      else if ( result == detail::tokenizer_return_code::comment )
      {
        reader.on_comment( token );
      }
      /* keep parsing if token is empty or if in the middle or at the end of a comment */
    } while ( ( token == "" && result == detail::tokenizer_return_code::valid ) ||
              tok.get_comment_mode() ||
              result == detail::tokenizer_return_code::comment );

    return ( result == detail::tokenizer_return_code::valid );
  }

  void push_token( std::string const& token )
  {
    tokens.push( token );
  }

  bool parse_signal_name()
  {
    valid = get_token( token ); // name
    if ( !valid || token == "[" )
      return false;
    auto const name = token;

    valid = get_token( token );
    if ( token == "[" )
    {
      valid = get_token( token ); // size
      if ( !valid )
        return false;
      auto const size = token;

      valid = get_token( token ); // size
      if ( !valid && token != "]" )
        return false;
      token = name + "[" + size + "]";
      std::cout << token << std::endl;
      return true;
    }

    push_token( token );

    token = name;
    return true;
  }

  bool skip_comments()
  {
    while ( valid )
    {
      if ( token.substr( 0, 2 ) == "//" )
      {
        reader.on_comment( token );
        valid = get_token( token ); // Skip the entire line
        continue;
      }
      else if ( token == "/*" )
      {
        while ( valid && token != "*/" )
        {
          valid = get_token( token ); // Skip block comments
        }
        valid = get_token( token ); // Skip the closing "*/"
        continue;
      }
      break;
    }
    return valid;
  }

  bool parse_modules()
  {
    //! nlwmode: skip the start comments
    valid = get_token( token );
    if ( !valid )
      return false;

    skip_comments();

    // start at module
    if ( token != "module" )
    {
      return false;
    }

    if ( !parse_module() )
    {
      return false;
    }
    return true;
  }

  bool parse_module()
  {
    //! nlwmode: parser header first
    bool success = parse_module_header();
    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_MODULE_HEADER );
      }
      return false;
    }

    do
    {
      valid = get_token( token );
      if ( !valid )
        return false;

      if ( token == "input" )
      {
        success = parse_inputs();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_INPUT_DECLARATION );
          }
          return false;
        }
      }
      else if ( token == "output" )
      {
        success = parse_outputs();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_OUTPUT_DECLARATION );
          }
          return false;
        }
      }
      else if ( token == "wire" )
      {
        success = parse_wires();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_WIRE_DECLARATION );
          }
          return false;
        }
      }
      else if ( token == "parameter" )
      {
        success = parse_parameter();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_WIRE_DECLARATION );
          }
          return false;
        }
      }
      else if ( token == "not" )
      {
        success = parse_not();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_GATE_NOT );
          }
          return false;
        }
      }
      else if ( token == "inv" )
      {
        success = parse_not();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_GATE_NOT );
          }
          return false;
        }
      }
      else if ( token == "buf" )
      {
        success = parse_buf();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_GATE_BUF );
          }
          return false;
        }
      }
      else if ( token == "and" )
      {
        success = parse_and();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_GATE_AND );
          }
          return false;
        }
      }
      else if ( token == "nand" )
      {
        success = parse_nand();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_GATE_NAND );
          }
          return false;
        }
      }
      else if ( token == "or" )
      {
        success = parse_or();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_GATE_OR );
          }
          return false;
        }
      }
      else if ( token == "nor" )
      {
        success = parse_nor();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_GATE_NOR );
          }
          return false;
        }
      }
      else if ( token == "xor" )
      {
        success = parse_xor();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_GATE_XOR );
          }
          return false;
        }
      }
      else if ( token == "xnor" )
      {
        success = parse_xnor();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_GATE_XNOR );
          }
          return false;
        }
      }
      else
      {
        break;
      }
    } while ( token != "assign" && token != "endmodule" );

    while ( token != "endmodule" )
    {
      if ( token == "assign" )
      {
        success = parse_assign();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_ASSIGNMENT );
          }
          return false;
        }

        valid = get_token( token );
        if ( !valid )
          return false;
      }
      else
      {
        success = parse_module_instantiation();
        if ( !success )
        {
          if ( diag )
          {
            diag->report( diag_id::ERR_GTECH_MODULE_INSTANTIATION_STATEMENT );
          }
          return false;
        }

        valid = get_token( token );
        if ( !valid )
          return false;
      }
    }

    /* check dangling objects */
    bool result = true;
    const auto& deps = on_action.unresolved_dependencies();
    if ( deps.size() > 0 )
      result = false;

    for ( const auto& r : deps )
    {
      if ( diag )
      {
        diag->report( diag_id::WRN_UNRESOLVED_DEPENDENCY )
            .add_argument( r.first )
            .add_argument( r.second );
      }
    }

    if ( !result )
      return false;

    if ( token == "endmodule" )
    {
      /* callback */
      reader.on_endmodule();

      return true;
    }
    else
    {
      return false;
    }
  }

  bool parse_module_header()
  {
    if ( token != "module" )
      return false;

    valid = get_token( token );
    if ( !valid )
      return false;

    module_name = token;

    valid = get_token( token );
    if ( !valid || token != "(" )
      return false;

    std::vector<std::string> inouts;
    do
    {
      if ( !parse_signal_name() )
        return false;
      inouts.emplace_back( token );

      valid = get_token( token ); // , or )
      if ( !valid || ( token != "," && token != ")" ) )
        return false;
    } while ( valid && token != ")" );

    valid = get_token( token );
    if ( !valid || token != ";" )
      return false;

    /* callback */
    reader.on_module_header( module_name, inouts );

    return true;
  }

  bool parse_inputs()
  {
    std::vector<std::string> inputs;
    if ( token != "input" )
      return false;

    std::string size = "";
    if ( !parse_signal_name() && token == "[" )
    {
      do
      {
        valid = get_token( token );
        if ( !valid )
          return false;

        if ( token != "]" )
          size += token;
      } while ( valid && token != "]" );

      if ( !parse_signal_name() )
        return false;
    }
    inputs.emplace_back( token );

    while ( true )
    {
      valid = get_token( token );

      if ( !valid || ( token != "," && token != ";" ) )
        return false;

      if ( token == ";" )
        break;

      if ( !parse_signal_name() )
        return false;

      inputs.emplace_back( token );
    }

    // store all inputs
    if ( size == "" )
    {
      for ( auto input : inputs )
      {
        set_wires_input.insert( input );
        set_all_wires.insert( input );
      }
    }
    else
    {
      for ( auto input : inputs )
      {
        detail::trim( size );
        auto pos = size.find( ':' );
        int length = std::atoi( size.substr( 0, pos ).c_str() );
        for ( int i = 0; i <= length; ++i )
        {
          std::string tmp_input = input + "[" + std::to_string( i ) + "]";
          set_wires_input.insert( tmp_input );
          set_all_wires.insert( tmp_input );
        }
      }
    }

    /* callback */
    reader.on_inputs( inputs, size );
    modules[module_name].inputs = inputs;

    for ( const auto& i : inputs )
    {
      on_action.declare_known( i );
    }

    if ( std::smatch m; std::regex_match( size, m, verilog_regex::const_size_range ) )
    {
      const auto a = std::stoul( m[1].str() );
      const auto b = std::stoul( m[2].str() );
      for ( auto j = std::min( a, b ); j <= std::max( a, b ); ++j )
      {
        for ( const auto& i : inputs )
        {
          on_action.declare_known( fmt::format( "{}[{}]", i, j ) );
        }
      }
    }

    return true;
  }

  bool parse_outputs()
  {
    std::vector<std::string> outputs;
    if ( token != "output" )
      return false;

    std::string size = "";
    if ( !parse_signal_name() && token == "[" )
    {
      do
      {
        valid = get_token( token );
        if ( !valid )
          return false;

        if ( token != "]" )
          size += token;
      } while ( valid && token != "]" );

      if ( !parse_signal_name() )
        return false;
    }
    outputs.emplace_back( token );

    while ( true )
    {
      valid = get_token( token );

      if ( !valid || ( token != "," && token != ";" ) )
        return false;

      if ( token == ";" )
        break;

      if ( !parse_signal_name() )
        return false;

      outputs.emplace_back( token );
    }

    // store all outputs
    if ( size == "" )
    {
      for ( auto output : outputs )
      {
        set_wires_output.insert( output );
        set_all_wires.insert( output );
      }
    }
    else
    {
      for ( auto output : outputs )
      {
        detail::trim( size );
        auto pos = size.find( ':' );
        int length = std::atoi( size.substr( 0, pos ).c_str() );
        for ( int i = 0; i <= length; ++i )
        {
          std::string tmp_output = output + "[" + std::to_string( i ) + "]";
          set_wires_output.insert( tmp_output );
          set_all_wires.insert( tmp_output );
        }
      }
    }

    /* callback */
    reader.on_outputs( outputs, size );
    modules[module_name].outputs = outputs;

    return true;
  }

  bool parse_wires()
  {
    std::vector<std::string> wires;
    if ( token != "wire" )
      return false;

    std::string size = ""; // left:right
    if ( !parse_signal_name() && token == "[" )
    {
      do
      {
        valid = get_token( token );
        if ( !valid )
          return false;

        if ( token != "]" )
          size += token;
      } while ( valid && token != "]" );

      if ( !parse_signal_name() )
        return false;
    }
    wires.emplace_back( token );

    while ( true )
    {
      valid = get_token( token );

      if ( !valid || ( token != "," && token != ";" ) )
        return false;

      if ( token == ";" )
        break;

      if ( !parse_signal_name() )
        return false;

      wires.emplace_back( token );
    }

    // store all wires, for topo checker
    if ( size == "" )
    {
      for ( auto wire : wires )
        set_all_wires.insert( wire );
    }
    else
    {
      for ( auto wire : wires )
      {
        detail::trim( size );
        auto pos = size.find( ':' );
        int length = std::atoi( size.substr( 0, pos ).c_str() );
        for ( int i = 0; i <= length; ++i )
        {
          std::string tmp_wire = wire + "[" + std::to_string( i ) + "]";
          set_all_wires.insert( tmp_wire );
        }
      }
    }
    // wires is the objects of this statement line , size is bus_wire ? "left:right" : ""
    // 但是对于实际的电路转换来说，知道 input和output的 port name 就足够了
    reader.on_wires( wires, size );
    return true;
  }

  bool parse_parameter()
  {
    if ( token != "parameter" )
      return false;

    valid = get_token( token );
    if ( !valid )
      return false;
    auto const name = token;

    valid = get_token( token );
    if ( !valid || ( token != "=" ) )
      return false;

    valid = get_token( token );
    if ( !valid )
      return false;
    auto const value = token;

    valid = get_token( token );
    if ( !valid || ( token != ";" ) )
      return false;

    /* callback */
    reader.on_parameter( name, value );

    return true;
  }

  bool parse_not()
  {
    if ( token != "not" )
      return false;
    std::string lhs;
    std::pair<std::string, bool> op1;

    bool success = parse_general_unate_expression( lhs, op1 );

    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_GATE_NOT )
            .add_argument( lhs );
      }
      return false;
    }

    std::vector<std::pair<std::string, bool>> args{ op1 };
    on_action.call_deferred<GATE_FN>( /* dependencies */ { op1.first }, { lhs },
                                      /* gate-function params */ std::make_tuple( args, lhs, "not" ) );
    return true;
  }

  bool parse_buf()
  {
    if ( token != "buf" )
      return false;
    std::string lhs;
    std::pair<std::string, bool> op1;

    bool success = parse_general_unate_expression( lhs, op1 );

    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_GATE_BUF )
            .add_argument( lhs );
      }
      return false;
    }

    std::vector<std::pair<std::string, bool>> args{ op1 };
    on_action.call_deferred<GATE_FN>( /* dependencies */ { op1.first }, { lhs },
                                      /* gate-function params */ std::make_tuple( args, lhs, "buf" ) );
    return true;
  }
  bool parse_and()
  {
    if ( token != "and" )
      return false;
    std::string lhs;
    std::pair<std::string, bool> op1;
    std::pair<std::string, bool> op2;

    bool success = parse_general_binate_expression( lhs, op1, op2 );

    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_GATE_AND )
            .add_argument( lhs );
      }
      return false;
    }

    std::vector<std::pair<std::string, bool>> args{ op1, op2 };
    on_action.call_deferred<GATE_FN>( /* dependencies */ { op1.first, op2.first }, { lhs },
                                      /* gate-function params */ std::make_tuple( args, lhs, "and" ) );
    return true;
  }
  bool parse_nand()
  {
    if ( token != "nand" )
      return false;
    std::string lhs;
    std::pair<std::string, bool> op1;
    std::pair<std::string, bool> op2;

    bool success = parse_general_binate_expression( lhs, op1, op2 );

    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_GATE_NAND )
            .add_argument( lhs );
      }
      return false;
    }

    std::vector<std::pair<std::string, bool>> args{ op1, op2 };
    on_action.call_deferred<GATE_FN>( /* dependencies */ { op1.first, op2.first }, { lhs },
                                      /* gate-function params */ std::make_tuple( args, lhs, "nand" ) );
    return true;
  }
  bool parse_or()
  {
    if ( token != "or" )
      return false;
    std::string lhs;
    std::pair<std::string, bool> op1;
    std::pair<std::string, bool> op2;

    bool success = parse_general_binate_expression( lhs, op1, op2 );

    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_GATE_OR )
            .add_argument( lhs );
      }
      return false;
    }

    std::vector<std::pair<std::string, bool>> args{ op1, op2 };
    on_action.call_deferred<GATE_FN>( /* dependencies */ { op1.first, op2.first }, { lhs },
                                      /* gate-function params */ std::make_tuple( args, lhs, "or" ) );
    return true;
  }
  bool parse_nor()
  {
    if ( token != "nor" )
      return false;
    std::string lhs;
    std::pair<std::string, bool> op1;
    std::pair<std::string, bool> op2;

    bool success = parse_general_binate_expression( lhs, op1, op2 );

    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_GATE_NOR )
            .add_argument( lhs );
      }
      return false;
    }

    std::vector<std::pair<std::string, bool>> args{ op1, op2 };
    on_action.call_deferred<GATE_FN>( /* dependencies */ { op1.first, op2.first }, { lhs },
                                      /* gate-function params */ std::make_tuple( args, lhs, "nor" ) );
    return true;
  }
  bool parse_xor()
  {
    if ( token != "xor" )
      return false;

    std::string lhs;
    std::pair<std::string, bool> op1;
    std::pair<std::string, bool> op2;

    bool success = parse_general_binate_expression( lhs, op1, op2 );

    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_GATE_XOR )
            .add_argument( lhs );
      }
      return false;
    }

    std::vector<std::pair<std::string, bool>> args{ op1, op2 };
    on_action.call_deferred<GATE_FN>( /* dependencies */ { op1.first, op2.first }, { lhs },
                                      /* gate-function params */ std::make_tuple( args, lhs, "xor" ) );
    return true;
  }
  bool parse_xnor()
  {
    if ( token != "xnor" )
      return false;

    std::string lhs;
    std::pair<std::string, bool> op1;
    std::pair<std::string, bool> op2;

    bool success = parse_general_binate_expression( lhs, op1, op2 );

    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_GATE_XNOR )
            .add_argument( lhs );
      }
      return false;
    }

    std::vector<std::pair<std::string, bool>> args{ op1, op2 };
    on_action.call_deferred<GATE_FN>( /* dependencies */ { op1.first, op2.first }, { lhs },
                                      /* gate-function params */ std::make_tuple( args, lhs, "xnor" ) );
    return true;
  }

  std::string trim_str( const std::string& str )
  {
    std::string cur = str;
    cur.erase( 0, cur.find_first_not_of( " \n\r\t" ) );
    cur.erase( cur.find_last_not_of( " \n\r\t" ) + 1 );
    return cur;
  };

  std::string parse_port( const std::string& port )
  {
    auto pos = port.find( '(' );
    auto pos_end = port.find( ')' );
    if ( pos != std::string::npos && pos_end != std::string::npos )
    {
      std::string sub_port = port.substr( pos + 1, pos_end - pos - 1 );
      return trim_str( sub_port );
    }
    return trim_str( port );
  };

  bool parse_general_unate_expression( std::string& lhs, std::pair<std::string, bool>& op1 )
  {
    // Parse the gate name
    valid = get_token( token );
    if ( !valid )
      return false;

    // Check whether this gate has been processed
    if ( set_gates_been_processed.find( token ) != set_gates_been_processed.end() )
      return true;

    set_gates_been_processed.insert( token );

    valid = get_token( token );
    if ( !valid || token != "(" )
      return false;

    // Parse the line within parentheses
    std::string line;
    std::stack<std::string> paren_stk;
    paren_stk.push( "(" );

    while ( valid && !paren_stk.empty() && token != ";" )
    {
      valid = get_token( token );

      if ( !valid )
        return false;

      if ( token == "(" )
      {
        paren_stk.push( "(" );
      }
      else if ( token == ")" )
      {
        paren_stk.pop();
        if ( paren_stk.empty() )
          break;
      }

      line += token + " ";
    }

    if ( token != ")" )
      return false;

    // Remove trailing spaces
    line.erase( line.find_last_not_of( " \n\r\t" ) + 1 );

    // Split line by commas and dots
    std::istringstream iss( line );
    std::vector<std::string> words;
    std::string word;
    while ( std::getline( iss, word, ',' ) )
    {
      // Remove leading and trailing spaces
      std::string port = parse_port( word );
      words.push_back( port );
    }

    // Ensure that there are exactly two elements
    if ( words.size() != 2 )
      return false;

    // Parse the output (lhs) and input (op1)
    lhs = words[0];
    if ( set_all_wires.find( lhs ) == set_all_wires.end() )
      return false;

    std::string input = parse_port( words[1] );
    if ( input[0] == '~' )
    {
      op1.first = input.substr( 1 );
      op1.second = true;
    }
    else
    {
      op1.first = input;
      op1.second = false;
    }

    if ( set_all_wires.find( op1.first ) == set_all_wires.end() )
      return false;

    valid = get_token( token );
    return ( valid && token == ";" );
  }

  bool parse_general_binate_expression( std::string& lhs, std::pair<std::string, bool>& op1, std::pair<std::string, bool>& op2 )
  {
    // Parse the gate name
    valid = get_token( token );
    if ( !valid )
      return false;

    // Check whether this gate has been processed
    if ( set_gates_been_processed.find( token ) != set_gates_been_processed.end() )
      return true;

    set_gates_been_processed.insert( token );

    valid = get_token( token );
    if ( !valid || token != "(" )
      return false;

    // Parse the line within parentheses
    std::string line;
    std::stack<std::string> paren_stk;
    paren_stk.push( "(" );

    while ( valid && !paren_stk.empty() && token != ";" )
    {
      valid = get_token( token );

      if ( !valid )
        return false;

      if ( token == "(" )
      {
        paren_stk.push( "(" );
      }
      else if ( token == ")" )
      {
        paren_stk.pop();
        if ( paren_stk.empty() )
          break;
      }

      line += token + " ";
    }

    if ( token != ")" )
      return false;

    // Remove trailing spaces
    line.erase( line.find_last_not_of( " \n\r\t" ) + 1 );

    // Split line by commas and dots
    std::istringstream iss( line );
    std::vector<std::string> words;
    std::string word;
    while ( std::getline( iss, word, ',' ) )
    {
      std::string port = parse_port( word );
      words.push_back( port );
    }

    // Ensure that there are exactly two elements
    if ( words.size() != 3 )
      return false;

    // Parse the output (lhs) and input (op1, op2)
    lhs = words[0];
    assert( set_all_wires.find( lhs ) != set_all_wires.end() );
    if ( words[1][0] == '~' )
    {
      assert( set_all_wires.find( words[1].substr( 1 ) ) != set_all_wires.end() );
      op1.first = words[1].substr( 1 );
      op1.second = true;
    }
    else
    {
      assert( set_all_wires.find( words[1] ) != set_all_wires.end() );
      op1.first = words[1];
    }

    if ( words[2][0] == '~' )
    {
      assert( set_all_wires.find( words[2].substr( 1 ) ) != set_all_wires.end() );
      op2.first = words[2].substr( 1 );
      op2.second = true;
    }
    else
    {
      assert( set_all_wires.find( words[2] ) != set_all_wires.end() );
      op2.first = words[2];
    }
    valid = get_token( token );
    if ( !valid || token != ";" )
      return false;
    return true;
  }

  bool parse_assign()
  {
    if ( token != "assign" )
      return false;

    if ( !parse_signal_name() )
      return false;

    const std::string lhs = token;
    valid = get_token( token );
    if ( !valid || token != "=" )
      return false;

    /* expression */
    bool success = parse_rhs_expression( lhs );
    if ( !success )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_ASSIGNMENT_RHS )
            .add_argument( lhs );
      }
      return false;
    }

    if ( token != ";" )
      return false;
    return true;
  }

  bool parse_module_instantiation()
  {
    bool success = true;
    std::string const module_name{ token }; // name of module

    auto const it = modules.find( module_name );
    if ( it == std::end( modules ) )
    {
      if ( diag )
      {
        diag->report( diag_id::ERR_GTECH_MODULE_INSTANTIATION_UNDECLARED_MODULE )
            .add_argument( module_name );
      }
      return false;
    }

    /* get module info */
    auto const& info = modules[module_name];

    valid = get_token( token );
    if ( !valid )
      return false;

    std::vector<std::string> params;
    if ( token == "#" )
    {
      valid = get_token( token ); // (
      if ( !valid || token != "(" )
        return false;

      do
      {
        valid = get_token( token ); // param
        if ( !valid )
          return false;
        params.emplace_back( token );

        valid = get_token( token ); // ,
        if ( !valid )
          return false;
      } while ( valid && token == "," );

      if ( !valid || token != ")" )
        return false;

      valid = get_token( token );
      if ( !valid )
        return false;
    }

    std::string const inst_name = token; // name of instantiation

    valid = get_token( token );
    if ( !valid || token != "(" )
      return false;

    std::vector<std::pair<std::string, std::string>> args;
    do
    {
      valid = get_token( token );

      if ( !valid )
        return false; // refers to signal
      std::string const arg0{ token };

      /* check if a signal with this name exists in the module declaration */
      if ( ( std::find( std::begin( info.inputs ), std::end( info.inputs ), arg0.substr( 1, arg0.size() ) ) == std::end( info.inputs ) ) &&
           ( std::find( std::begin( info.outputs ), std::end( info.outputs ), arg0.substr( 1, arg0.size() ) ) == std::end( info.outputs ) ) )
      {
        if ( diag )
        {
          diag->report( diag_id::ERR_GTECH_MODULE_INSTANTIATION_UNDECLARED_PIN )
              .add_argument( arg0.substr( 1, arg0.size() ) )
              .add_argument( module_name );
        }

        success = false;
      }

      valid = get_token( token );
      if ( !valid || token != "(" )
        return false; // (

      valid = get_token( token );
      if ( !valid )
        return false; // signal name
      auto const arg1 = token;

      valid = get_token( token );
      if ( !valid || token != ")" )
        return false; // )

      valid = get_token( token );
      if ( !valid )
        return false;

      args.emplace_back( std::make_pair( arg0, arg1 ) );
    } while ( token == "," );

    if ( !valid || token != ")" )
      return false;

    valid = get_token( token );
    if ( !valid || token != ";" )
      return false;

    std::vector<std::string> inputs;
    for ( const auto& input : modules[module_name].inputs )
    {
      for ( const auto& a : args )
      {
        if ( a.first.substr( 1, a.first.length() - 1 ) == input )
        {
          inputs.emplace_back( a.second );
        }
      }
    }

    std::vector<std::string> outputs;
    for ( const auto& output : modules[module_name].outputs )
    {
      for ( const auto& a : args )
      {
        if ( a.first.substr( 1, a.first.length() - 1 ) == output )
        {
          outputs.emplace_back( a.second );
        }
      }
    }

    /* callback */
    on_action.call_deferred<MODULE_INST_FN>( inputs, outputs,
                                             std::make_tuple( module_name, params, inst_name, args ) );

    return success;
  }

  bool parse_rhs_expression( const std::string& lhs )
  {
    std::string s;
    do
    {
      valid = get_token( token );
      if ( !valid )
        return false;

      if ( token == ";" || token == "assign" || token == "endmodule" )
        break;
      s.append( token );
    } while ( token != ";" && token != "assign" && token != "endmodule" );

    std::smatch sm;
    if ( std::regex_match( s, sm, verilog_regex::immediate_assign ) )
    {
      assert( sm.size() == 3u );
      std::vector<std::pair<std::string, bool>> args{ { sm[2], sm[1] == "~" } };

      on_action.call_deferred<GATE_FN>( /* dependencies */ { sm[2] }, { lhs },
                                        /* gate-function params */ std::make_tuple( args, lhs, "assign" ) );
    }
    else if ( std::regex_match( s, sm, verilog_regex::binary_expression ) )
    {
      assert( sm.size() == 6u );
      std::pair<std::string, bool> arg0 = { sm[2], sm[1] == "~" };
      std::pair<std::string, bool> arg1 = { sm[5], sm[4] == "~" };
      std::vector<std::pair<std::string, bool>> args{ arg0, arg1 };

      auto op = sm[3];

      if ( op == "&" )
      {
        on_action.call_deferred<GATE_FN>( /* dependencies */ { arg0.first, arg1.first }, { lhs },
                                          /* gate-function params */ std::make_tuple( args, lhs, "and" ) );
      }
      else if ( op == "|" )
      {
        on_action.call_deferred<GATE_FN>( /* dependencies */ { arg0.first, arg1.first }, { lhs },
                                          /* gate-function params */ std::make_tuple( args, lhs, "or" ) );
      }
      else if ( op == "^" )
      {
        on_action.call_deferred<GATE_FN>( /* dependencies */ { arg0.first, arg1.first }, { lhs },
                                          /* gate-function params */ std::make_tuple( args, lhs, "xor" ) );
      }
      else
      {
        return false;
      }
    }
    else if ( std::regex_match( s, sm, verilog_regex::negated_binary_expression ) )
    {
      assert( sm.size() == 6u );
      std::pair<std::string, bool> arg0 = { sm[2], sm[1] == "~" };
      std::pair<std::string, bool> arg1 = { sm[5], sm[4] == "~" };
      std::vector<std::pair<std::string, bool>> args{ arg0, arg1 };

      auto op = sm[3];
      if ( op == "&" )
      {
        on_action.call_deferred<GATE_FN>( /* dependencies */ { arg0.first, arg1.first }, { lhs },
                                          /* gate-function params */ std::make_tuple( args, lhs, "nand" ) );
      }
      else if ( op == "|" )
      {
        on_action.call_deferred<GATE_FN>( /* dependencies */ { arg0.first, arg1.first }, { lhs },
                                          /* gate-function params */ std::make_tuple( args, lhs, "nor" ) );
      }
      else if ( op == "^" )
      {
        on_action.call_deferred<GATE_FN>( /* dependencies */ { arg0.first, arg1.first }, { lhs },
                                          /* gate-function params */ std::make_tuple( args, lhs, "xnor" ) );
      }
      else
      {
        return false;
      }
    }
    else if ( std::regex_match( s, sm, verilog_regex::ternary_expression ) )
    {
      assert( sm.size() == 9u );
      std::pair<std::string, bool> arg0 = { sm[2], sm[1] == "~" };
      std::pair<std::string, bool> arg1 = { sm[5], sm[4] == "~" };
      std::pair<std::string, bool> arg2 = { sm[8], sm[7] == "~" };
      std::vector<std::pair<std::string, bool>> args{ arg0, arg1, arg2 };

      auto op = sm[3];
      if ( sm[6] != op )
      {
        return false;
      }

      if ( op == "&" )
      {
        on_action.call_deferred<GATE_FN>( /* dependencies */ { arg0.first, arg1.first, arg2.first }, { lhs },
                                          /* gate-function params */ std::make_tuple( args, lhs, "and3" ) );
      }
      else if ( op == "|" )
      {
        on_action.call_deferred<GATE_FN>( /* dependencies */ { arg0.first, arg1.first, arg2.first }, { lhs },
                                          /* gate-function params */ std::make_tuple( args, lhs, "or3" ) );
      }
      else if ( op == "^" )
      {
        on_action.call_deferred<GATE_FN>( /* dependencies */ { arg0.first, arg1.first, arg2.first }, { lhs },
                                          /* gate-function params */ std::make_tuple( args, lhs, "xor3" ) );
      }
      else
      {
        return false;
      }
    }
    else if ( std::regex_match( s, sm, verilog_regex::maj3_expression ) )
    {
      assert( sm.size() == 13u );
      std::pair<std::string, bool> a0 = { sm[2], sm[1] == "~" };
      std::pair<std::string, bool> b0 = { sm[4], sm[3] == "~" };
      std::pair<std::string, bool> a1 = { sm[6], sm[5] == "~" };
      std::pair<std::string, bool> c0 = { sm[8], sm[7] == "~" };
      std::pair<std::string, bool> b1 = { sm[10], sm[9] == "~" };
      std::pair<std::string, bool> c1 = { sm[12], sm[11] == "~" };

      if ( a0 != a1 || b0 != b1 || c0 != c1 )
        return false;

      std::vector<std::pair<std::string, bool>> args;
      args.push_back( a0 );
      args.push_back( b0 );
      args.push_back( c0 );

      on_action.call_deferred<GATE_FN>( /* dependencies */ { a0.first, b0.first, c0.first }, { lhs },
                                        /* gate-function params */ std::make_tuple( args, lhs, "maj3" ) );
    }
    else
    {
      return false;
    }

    return true;
  }

private:
  /* Function signatures */
  using GateFn = detail::Func<
      std::vector<std::pair<std::string, bool>>,
      std::string,
      std::string>;
  using ModuleInstFn = detail::Func<
      std::string,
      std::vector<std::string>,
      std::string,
      std::vector<std::pair<std::string, std::string>>>;

  /* Parameter maps */
  using GateParamMap = detail::ParamPackMap<
      /* Key */
      std::string,
      /* Params */
      std::vector<std::pair<std::string, bool>>,
      std::string,
      std::string>;
  using ModuleInstParamMap = detail::ParamPackMap<
      /* Key */
      std::string,
      /* Param */
      std::string,
      std::vector<std::string>,
      std::string,
      std::vector<std::pair<std::string, std::string>>>;

  constexpr static const int GATE_FN{ 0 };
  constexpr static const int MODULE_INST_FN{ 1 };

  using ParamMaps = detail::ParamPackMapN<GateParamMap, ModuleInstParamMap>;
  using PackedFns = detail::FuncPackN<GateFn, ModuleInstFn>;

private:
  detail::tokenizer tok;
  const gtech_reader& reader;
  diagnostic_engine* diag;

  std::string token;
  std::queue<std::string> tokens; /* lookahead */
  std::string module_name;

  bool valid = false;

  detail::call_in_topological_order<PackedFns, ParamMaps> on_action;
  std::unordered_map<std::string, module_info> modules;

  // handle the condition that the gate order is not topo order
  std::unordered_set<std::string> set_all_wires;
  std::unordered_set<std::string> set_wires_been_implemented; // make sure the gate generated is according to topologic order
  std::unordered_set<std::string> set_wires_input;            // all input wires
  std::unordered_set<std::string> set_wires_output;           // all output wires
  std::unordered_set<std::string> set_gates_been_processed;   // flag the processed gates

}; /* gtech_parser */

/*! \brief Reader function for GTECH VERILOG format.
 *
 * Reads a simplistic GTECH VERILOG format from a stream and invokes a callback
 * method for each parsed primitive and each detected parse error.
 *
 * \param in Input stream
 * \param reader A GTECH VERILOG reader with callback methods invoked for parsed primitives
 * \param diag An optional diagnostic engine with callback methods for parse errors
 * \return Success if parsing has been successful, or parse error if parsing has failed
 */
[[nodiscard]] inline return_code read_gtech( std::istream& in, const gtech_reader& reader, diagnostic_engine* diag = nullptr )
{
  gtech_parser parser( in, reader, diag );
  auto result = parser.parse_modules();
  if ( !result )
  {
    return return_code::parse_error;
  }
  else
  {
    return return_code::success;
  }
}

/*! \brief Reader function for GTECH VERILOG format.
 *
 * Reads a simplistic GTECH VERILOG format from a file and invokes a callback
 * method for each parsed primitive and each detected parse error.
 *
 * \param filename Name of the file
 * \param reader A GTECH VERILOG reader with callback methods invoked for parsed primitives
 * \param diag An optional diagnostic engine with callback methods for parse errors
 * \return Success if parsing has been successful, or parse error if parsing has failed
 */
[[nodiscard]] inline return_code read_gtech( const std::string& filename, const gtech_reader& reader, diagnostic_engine* diag = nullptr )
{
  std::ifstream in( detail::word_exp_filename( filename ), std::ifstream::in );
  if ( !in.is_open() )
  {
    if ( diag )
    {
      diag->report( diag_id::ERR_FILE_OPEN ).add_argument( filename );
    }
    return return_code::parse_error;
  }
  else
  {
    auto const ret = read_gtech( in, reader, diag );
    in.close();
    return ret;
  }
}

} // namespace lorina
