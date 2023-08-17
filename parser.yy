%skeleton "lalr1.cc"
%require  "3.0.1"

%defines 
%define api.namespace {IPL}
%define api.parser.class {Parser}

%define parse.trace

%code requires{
  #include <bits/stdc++.h>
  namespace IPL {
    class Scanner;
  }
  class abstract_astnode;
  class exp_astnode;
  class assignE_astnode;
  class statement_astnode;
  class Info;
  class GlobalSymTab;
  class Entry;
  #include "symtab.hpp"
  #include "location.hh"
}

%printer { std::cerr << $$; } IDENTIFIER
%printer { std::cerr << $$; } FLOAT_CONSTANT
%printer { std::cerr << $$; } INT_CONSTANT
%printer { std::cerr << $$; } STRUCT
%printer { std::cerr << $$; } VOID  
%printer { std::cerr << $$; } INT
%printer { std::cerr << $$; } FLOAT
%printer { std::cerr << $$; } RETURN
%printer { std::cerr << $$; } OR_OP
%printer { std::cerr << $$; } EQ_OP
%printer { std::cerr << $$; } AND_OP
%printer { std::cerr << $$; } LE_OP
%printer { std::cerr << $$; } GE_OP
%printer { std::cerr << $$; } NE_OP
%printer { std::cerr << $$; } IF
%printer { std::cerr << $$; } ELSE 
%printer { std::cerr << $$; } PTR_OP
%printer { std::cerr << $$; } INC_OP
%printer { std::cerr << $$; } WHILE
%printer { std::cerr << $$; } FOR

%parse-param { Scanner  &scanner  }
%locations
%code {
#include <iostream>
#include <cstdlib>
// #include <map>
#include <fstream>
#include "ast.hpp"
#include "scanner.hh"
#include "utils.hpp"

std::map<std::string,int> width_map;
std::map<std::string,abstract_astnode*> ast;
extern GlobalSymTab gst;
extern std::vector<std::string> Gstrings;
LocalVarHandler local_var_handler(gst);


#undef yylex
#define yylex IPL::Parser::scanner.yylex

}



%define api.location.type {IPL::location}
%define api.value.type variant
%define parse.assert
/* %start translation_unit */
%start translation_unit


%token <std::string> IDENTIFIER 
%token <std::string> INT_CONSTANT 
%token <std::string> FLOAT_CONSTANT 
%token <std::string> STRUCT
%token <std::string> VOID  
%token <std::string> INT
%token <std::string> FLOAT
%token <std::string> RETURN
%token <std::string> OR_OP
%token <std::string> EQ_OP
%token <std::string> AND_OP
%token <std::string> LE_OP
%token <std::string> GE_OP
%token <std::string> NE_OP
%token <std::string> IF
%token <std::string> ELSE 
%token <std::string> PTR_OP
%token <std::string> INC_OP
%token <std::string> WHILE
%token <std::string> FOR
%token <std::string> STRING_LITERAL
%token  OTHERS
%token '(' ')'  '+' '-' '/' '*' '!' '<' '>' '&' ',' ';' ']' '[' '{' '}'

%nterm <int> translation_unit
%nterm <Struct> struct_specifier
%nterm <Function> function_definition
%nterm <std::string> type_specifier

%nterm <std::pair<std::string,ParamDeclarationList*>> fun_declarator
%nterm <ParamDeclarationList*> parameter_list
%nterm <Declaration*> declarator_arr declarator parameter_declaration

%nterm <std::pair<DeclarationList*, statement_astnode*>> compound_statement
%nterm <std::vector<statement_astnode*>> statement_list
%nterm <statement_astnode*> statement
%nterm <statement_astnode*> assignment_statement
%nterm <statement_astnode*> procedure_call
%nterm <exp_astnode*> expression
%nterm <assignE_astnode*> assignment_expression
%nterm <exp_astnode*> logical_and_expression
%nterm <exp_astnode*> equality_expression
%nterm <exp_astnode*> relational_expression
%nterm <exp_astnode*> additive_expression
%nterm <exp_astnode*> unary_expression
%nterm <exp_astnode*> multiplicative_expression
%nterm <exp_astnode*> postfix_expression
%nterm <exp_astnode*> primary_expression
%nterm <std::vector<exp_astnode*>> expression_list
%nterm <char> unary_operator
%nterm <statement_astnode*> selection_statement
%nterm <statement_astnode*> iteration_statement
%nterm <DeclarationList*> declaration_list
%nterm <std::vector<Declaration*>> declarator_list declaration
%%

translation_unit: 
  struct_specifier
  {
    local_var_handler.reset();
    gst.addStruct($1);
    width_map[$1.name]=$1.membervars->getTotalWidth();
  }
  | function_definition
  {
    local_var_handler.reset();
    gst.addFunction($1);
    ast[$1.name]=$1.ast;
  }
  | translation_unit struct_specifier
  {
    local_var_handler.reset();
    gst.addStruct($2);
    width_map[$2.name]=$2.membervars->getTotalWidth();
  }
  | translation_unit function_definition
  {
    local_var_handler.reset();
    gst.addFunction($2);
    ast[$2.name]=$2.ast;
  }
;
struct_specifier:
  STRUCT IDENTIFIER 
  {
    std::string temp = $1+" "+$2; local_var_handler.setCurrentFuncName(temp);
  } 
  '{' declaration_list '}' ';'
  {   
    std::string temp = $1+" "+$2;
    if(gst.hasStruct(temp))
    {
      error(@$, "\""+temp+"\" has a previous definition");
    }
    $$ = Struct(temp, $5);
  }
;
function_definition: 
  type_specifier {local_var_handler.setReturnType($1);} fun_declarator compound_statement{
    if(gst.hasFunc($3.first))
    {
      error(@$, "The function \""+$3.first+"\" has a previous definition");
    }
    $$=Function($1, $3.first, $3.second, $4.first, $4.second);
  }
;


type_specifier: 
  VOID
  {
    $$ = $1;
  }
  | INT
  {
    $$ = $1;
  }
  | FLOAT
  {
    $$ = $1;
  }
  | STRUCT IDENTIFIER
  {
    $$ = $1+ " " +$2;
    if(!gst.hasStruct($$) && local_var_handler.getCurrentFuncName()!=$$)
    {
      error(@$, "\""+$1+ " " +$2+"\" is not defined");
    }
  }
;
fun_declarator: 
  IDENTIFIER '(' parameter_list ')'
  {
    $$ = std::make_pair($1, $3);
    local_var_handler.setCurrentFuncName($1);
  }
   | IDENTIFIER '(' ')'
  {
    $$ = std::make_pair($1,nullptr);
    local_var_handler.setCurrentFuncName($1);
  }
   ;
parameter_list: 
  parameter_declaration
  {
    $$ = new ParamDeclarationList();
    local_var_handler.setParamDeclList($$);

    if(!(local_var_handler.addParamDecl($1)))
      error(@$, "\""+$1->id+"\" has a previous declaration");
  }
  | parameter_list ',' parameter_declaration
  {
    $$=$1;
    if(!(local_var_handler.addParamDecl($3)))
      error(@$, "\""+$3->id+"\" has a previous declaration");
  }
  ;
parameter_declaration: 
  type_specifier declarator
  {
    $$=$2;
    $$->type = $1 + $$->type; 
    if($$->type=="void")
    {
      error(@$,"Cannot declare the type of a parameter as  \""+$$->type+"\"");
    }
    if($$->type.find("*")!=std::string::npos)
      $$->width *= 4; 
    else if($1 == "int")
      $$->width *= 4; 
    else if($1 == "float")
      $$->width *= 4;
    else 
      $$->width *= width_map[$1]; 
  }
;

declarator_arr: 
  IDENTIFIER
  {
    $$ = new Declaration($1);
  }
  | declarator_arr '[' INT_CONSTANT ']'
  {
    $$ = $1;
    $$->type+=("["+$3+"]");
    $$->width*=stoi($3);
  }
;
declarator: 
  declarator_arr
  {
    $$=$1;
  }
  | '*' declarator
  {
    $$ = $2;
    $$->type=("*"+$$->type);
  }
;

compound_statement: 
     '{' '}'{
      seq_astnode* temp2 = new seq_astnode();
      $$ = std::make_pair(nullptr, temp2);
   }
   | '{' statement_list '}'{
      seq_astnode* temp2 = new seq_astnode($2);
      $$ = std::make_pair(nullptr, temp2);
   }
   | '{' declaration_list '}'{
      seq_astnode* temp2 = new seq_astnode();
      $$ = std::make_pair($2, temp2);
   }
   | '{' declaration_list statement_list '}'{
      seq_astnode* temp2 = new seq_astnode($3);
      $$ = std::make_pair($2, temp2);
   }
   ;
statement_list: 
     statement{
      std::vector<statement_astnode*> vec;
      vec.push_back($1);
      $$ = vec;
   }
   | statement_list statement{
      $$ = $1;
      $$.push_back($2);
   }
   ;
statement: 
     ';'{
      $$ = new empty_astnode();
   }
   | '{' statement_list '}'{
      $$ = new seq_astnode($2);
   }
   | selection_statement{
      $$ = $1;
   }
   | iteration_statement{
      $$ = $1;
   }
   | assignment_statement{
      $$ = $1;
   }
   | procedure_call{
      $$ = $1;
   }
   | RETURN expression ';'{
    if($2->type=="int"&& local_var_handler.getReturnType()=="int")
    {
      $$ = new return_expnode($2);
    }
    else if($2->type=="int"&& local_var_handler.getReturnType()=="float")
    {
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$2);
      $$ = new return_expnode(temp);
    }
    else if($2->type=="float"&& local_var_handler.getReturnType()=="int")
    {
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_INT,$2);
      $$ = new return_expnode(temp);
    }
    else if($2->type=="float"&& local_var_handler.getReturnType()=="float")
    {
      $$ = new return_expnode($2);
    }
    else if($2->type==local_var_handler.getReturnType()){
      $$ = new return_expnode($2);
    }
   }
   ;
assignment_expression: 
  unary_expression '=' expression{
    if($1->isLval==false){
      error(@$,"Left operand of assignment should have an lvalue");
    }
    else if(convert($1->type)==convert($3->type) && $1->type.find("[")==std::string::npos){
      $$ = new assignE_astnode($1,$3);
      $$->type = "int";
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_INT,$3);
      $$ = new assignE_astnode($1,temp);
      $$->type = "int";
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new assignE_astnode($1,temp);
      $$->type = "int";
    }
    else if($1->type=="void*" && ($3->type.find("*")!=std::string::npos || $3->type.find("[")!=std::string::npos)){
      $$ = new assignE_astnode($1,$3);
      $$->type = "int";
    }
    else if($3->type=="void*" && ($1->type.find("*")!=std::string::npos && $1->type.find("[")==std::string::npos)){
      $$ = new assignE_astnode($1,$3);
      $$->type = "int";
    }
    else if($1->type.find("[")==std::string::npos&&$3->type=="int"&&$3->astnode_type==typeExp::INT&&((intconst_astnode*)$3)->getVal()==0)
    {
      $$ = new assignE_astnode($1,$3);
      $$->type = "int";
    }
    else{
      error(@$,"Incompatible assignment when assigning to type \""+$1->type+"\" from type \""+$3->type+"\"");
    }
  }
;
assignment_statement: 
  assignment_expression ';'{
    $$ = new assignS_astnode($1->get_left(),$1->get_right());
  }
;
procedure_call: 
  IDENTIFIER '(' ')' ';'{
    std::string er_msg;
    auto temp = std::vector<exp_astnode*>();
    if(!checkFuncCall($1, temp, er_msg))
      error(@$, er_msg);
    $$ = new proccall_astnode(new identifier_astnode($1));
  }
  | IDENTIFIER '(' expression_list ')' ';'{
    std::string er_msg;
    if(!checkFuncCall($1, $3, er_msg))
      error(@$, er_msg);
    $$ = new proccall_astnode(new identifier_astnode($1), $3);
  }
;
expression: 
  logical_and_expression{
    $$ = $1;
  }
  | expression OR_OP logical_and_expression{

    $$ = new op_binary_astnode(BINARY_OP::OR,$1,$3);
    $$->type = "int";
    $$->isLval = false;
  }
;
logical_and_expression: 
  equality_expression
  {
    $$ = $1;
  }
  | logical_and_expression AND_OP equality_expression
  {
    $$ = new op_binary_astnode(BINARY_OP::AND,$1,$3);
    $$->type = "int";
    $$->isLval = false;
  }
;
equality_expression: 
  relational_expression
  {
    $$ = $1;
  }
  | equality_expression EQ_OP relational_expression
  {
    if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::EQ_FLOAT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if(convert($1->type)==convert($3->type)){
      $$ = new op_binary_astnode(BINARY_OP::EQ_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::EQ_FLOAT,temp,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::EQ_FLOAT,$1,temp);
      $$->type = "int";
      $$->isLval = false;
    }
    else{
      $$ = new op_binary_astnode(BINARY_OP::EQ_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
  }
  | equality_expression NE_OP relational_expression
  {
    if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::NE_FLOAT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    if(convert($1->type)==convert($3->type)){
      $$ = new op_binary_astnode(BINARY_OP::NE_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::NE_FLOAT,temp,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::NE_FLOAT,$1,temp);
      $$->type = "int";
      $$->isLval = false;
    }
    else{
      $$ = new op_binary_astnode(BINARY_OP::NE_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
  }
;
relational_expression: 
  additive_expression
  {
    $$ = $1;
  }
  | relational_expression '<' additive_expression
  {
    if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::LT_FLOAT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::LT_FLOAT,temp,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::LT_FLOAT,$1,temp);
      $$->type = "int";
      $$->isLval = false;
    }
    else if(convert($1->type)==convert($3->type)){
      $$ = new op_binary_astnode(BINARY_OP::LT_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else{
      error(@$,"Invalid operands types for binary <, \""+$1->type+"\" and \""+$3->type+"\"");
    }
  }
  | relational_expression '>' additive_expression{
    if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::GT_FLOAT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::GT_FLOAT,temp,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::GT_FLOAT,$1,temp);
      $$->type = "int";
      $$->isLval = false;
    }
    else if(convert($1->type)==convert($3->type)){
      $$ = new op_binary_astnode(BINARY_OP::GT_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else{
      error(@$,"Invalid operands types for binary >, \""+$1->type+"\" and \""+$3->type+"\"");
    }
  }
  | relational_expression LE_OP additive_expression{
    if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::LE_FLOAT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::LE_FLOAT,temp,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::LE_FLOAT,$1,temp);
      $$->type = "int";
      $$->isLval = false;
    }
    if(convert($1->type)==convert($3->type)){
      $$ = new op_binary_astnode(BINARY_OP::LE_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else{
      error(@$,"Invalid operands types for binary <=, \""+$1->type+"\" and \""+$3->type+"\"");
    }
  }
  | relational_expression GE_OP additive_expression{
    if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::GE_FLOAT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::GE_FLOAT,temp,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::GE_FLOAT,$1,temp);
      $$->type = "int";
      $$->isLval = false;
    }
    else if(convert($1->type)==convert($3->type)){
      $$ = new op_binary_astnode(BINARY_OP::GE_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else{
      error(@$,"Invalid operands types for binary >=, \""+$1->type+"\" and \""+$3->type+"\"");
    }
  }
;
additive_expression:
  multiplicative_expression
  {
    $$ = $1;
  }
  | additive_expression '+' multiplicative_expression
  {
    
    if($1->type=="int" && $3->type=="int"){
      $$ = new op_binary_astnode(BINARY_OP::PLUS_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::PLUS_FLOAT,temp,$3);
      $$->type = "float";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::PLUS_FLOAT,$1,temp);
      $$->type = "float";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::PLUS_FLOAT,$1,$3);
      $$->type = "float";
      $$->isLval = false;
    }
    else if($1->type=="int"){
      $$ = new op_binary_astnode(BINARY_OP::PLUS_INT,$1,$3);
      $$->type = $3->type;
      $$->isLval = false;
    }
    else if($3->type=="int"){
      $$ = new op_binary_astnode(BINARY_OP::PLUS_INT,$1,$3);
      $$->type = $1->type;
      $$->isLval = false;
    }
    else{
      $$ = new op_binary_astnode(BINARY_OP::PLUS_INT,$1,$3);
      $$->type = $1->type;
      $$->isLval = false;
    }
  }
  | additive_expression '-' multiplicative_expression
  {
    if($1->type=="int" && $3->type=="int"){
      $$ = new op_binary_astnode(BINARY_OP::MINUS_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::MINUS_FLOAT,temp,$3);
      $$->type = "float";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::MINUS_FLOAT,$1,temp);
      $$->type = "float";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::MINUS_FLOAT,$1,$3);
      $$->type = "float";
      $$->isLval = false;
    }
    else if(convert($1->type)==convert($3->type)){
      $$ = new op_binary_astnode(BINARY_OP::MINUS_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else{
      $$ = new op_binary_astnode(BINARY_OP::MINUS_INT,$1,$3);
      $$->type = $1->type;
      $$->isLval = false;
    }
  }
;
unary_expression: 
  postfix_expression
  {
    $$ = $1;
  }
  | unary_operator unary_expression
  {
    switch($1){
      case '-':
        if($2->type!="int" && $2->type!="float"){
          error (@$,"Operand of unary - should be an int or float");
        }
        $$ = new op_unary_astnode(UNARY_OP::MINUS,$2);
        $$->type = $2->type;
        $$->isLval = false;
        break;
      case '!':
        $$ = new op_unary_astnode(UNARY_OP::NOT,$2);
        $$->type = "int";
        $$->isLval = false;
        break;
      case '*':
        if($2->type == "void*"){
          error (@$,"Invalid operand type \""+$2->type+"\" of unary *");
        }
        if($2->type.find("*")==std::string::npos && $2->type.find("[")==std::string::npos){
          error (@$,"Invalid operand type \""+$2->type+"\" of unary *");
        }
        $$ = new op_unary_astnode(UNARY_OP::DEREF,$2);
        if($2->type.find("(")!=std::string::npos){ 
          $$->type = $2->type.substr(0,$2->type.find("("))+$2->type.substr($2->type.find(")")+1);
        }
        else if($2->type.find("[")!=std::string::npos){
          $$->type = $2->type.substr(0,$2->type.find("["))+$2->type.substr($2->type.find("]")+1);
        }
        else if($2->type.find("*")!=std::string::npos){
          $$->type = $2->type.substr(0,$2->type.find("*"))+$2->type.substr($2->type.find("*")+1);
        }
        $$->isLval = true; 
        break;
      case '&':
        if($2->isLval==false){
          error (@$,"Operand of & should have lvalue");
        }
        $$ = new op_unary_astnode(UNARY_OP::ADDRESS,$2);
        if($2->type.find("(")!=std::string::npos){ 
          $$->type = $2->type.substr(0,$2->type.find(")")+1)+"*"+$2->type.substr($2->type.find(")")+1);
        }
        else if($2->type.find("[")!=std::string::npos){
          $$->type = $2->type.substr(0,$2->type.find("["))+"(*)"+$2->type.substr($2->type.find("["));
        }
        else if($2->type.find("*")!=std::string::npos){
          $$->type = $2->type+"*";
        }
        else{
          $$->type = $2->type+"*";
        }
        $$->isLval = false;
        break;
    }
  }
;
multiplicative_expression: 
  unary_expression
  {
    $$ = $1; 
  }
  | multiplicative_expression '*' unary_expression
  {
    if($1->type=="int" && $3->type=="int"){
      $$ = new op_binary_astnode(BINARY_OP::MULT_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::MULT_FLOAT,temp,$3);
      $$->type = "float";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::MULT_FLOAT,$1,temp);
      $$->type = "float";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::MULT_FLOAT,$1,$3);
      $$->type = "float";
      $$->isLval = false;
    }
    else{
      error(@$,"Invalid operands types for binary * , \""+$1->type+"\" and \""+$3->type+"\"");
    }
  }
  | multiplicative_expression '/' unary_expression
  {
    if($1->type=="int" && $3->type=="int"){
      $$ = new op_binary_astnode(BINARY_OP::DIV_INT,$1,$3);
      $$->type = "int";
      $$->isLval = false;
    }
    else if($1->type=="int" && $3->type=="float"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$1);
      $$ = new op_binary_astnode(BINARY_OP::DIV_FLOAT,temp,$3);
      $$->type = "float";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="int"){
      op_unary_astnode* temp = new op_unary_astnode(UNARY_OP::TO_FLOAT,$3);
      $$ = new op_binary_astnode(BINARY_OP::DIV_FLOAT,$1,temp);
      $$->type = "float";
      $$->isLval = false;
    }
    else if($1->type=="float" && $3->type=="float"){
      $$ = new op_binary_astnode(BINARY_OP::DIV_FLOAT,$1,$3);
      $$->type = "float";
      $$->isLval = false;
    }
    else{
      error(@$,"Invalid operands types for binary + , \""+$1->type+"\" and \""+$3->type+"\"");
    }
  }
;
postfix_expression: 
  primary_expression
  {
    $$ = $1;   
  }
  | postfix_expression '[' expression ']'
  {
    if($3->type!="int"){
      error(@$,"Array subscript is not an integer");
    }
    if($1->type.find("*")==std::string::npos && $1->type.find("[")==std::string::npos){
      error (@$,"Subscripted value is neither array nor pointer");
    }
    $$ = new arrayref_astnode($1,$3);
    if($1->type.find("(")!=std::string::npos){ 
      $$->type = $1->type.substr(0,$1->type.find("("))+$1->type.substr($1->type.find(")")+1);
    }
    else if($1->type.find("[")!=std::string::npos){
      $$->type = $1->type.substr(0,$1->type.find("["))+$1->type.substr($1->type.find("]")+1);
    }
    else if($1->type.find("*")!=std::string::npos){
      $$->type = $1->type.substr(0,$1->type.find("*"))+$1->type.substr($1->type.find("*")+1);
    }
    $$->isLval = true;
  }
  | IDENTIFIER '(' ')'
  {
    std::string type_or_error;
    auto temp = std::vector<exp_astnode*>();
    if(!checkFuncCall($1, temp, type_or_error))
      error(@$, type_or_error);

    $$ = new funcall_astnode(new identifier_astnode($1));
    $$->type = type_or_error;
    $$->isLval = true;
  }
  | IDENTIFIER '(' expression_list ')'
  {
    std::string type_or_error;
    if(!checkFuncCall($1, $3, type_or_error))
      error(@$, type_or_error);

    $$ = new funcall_astnode(new identifier_astnode($1), $3);
    $$->type = type_or_error;
    $$->isLval = true;
  }
  | postfix_expression '.' IDENTIFIER
  {
    if(gst.entries.count($1->type)&&gst.entries[$1->type].type!=0){
      error(@$,"Left operand of \".\"  is not a  structure");
    }
    identifier_astnode* id_node = new identifier_astnode($3);
    $$ = new member_astnode($1,id_node);
    $$->type = "Not Found";
    $$->isLval = true;
    for(auto &locals:gst.entries[$1->type].symtab->elems){
      if(locals.first == $3){
        $$->type = locals.second.datatype;
        break;
      }
    }
    if($$->type == "Not Found"){
      error(@$,"\""+$1->type+"\" has no member named \""+$3+"\"");
    }
  }
  | postfix_expression PTR_OP IDENTIFIER
  {
    std::string temp;
    temp = convert($1->type).substr(0,convert($1->type).length());
    if(temp.find("*")==std::string::npos){
      error(@$,"Left operand of \"->\"  is not a pointer to structure");
    }
    if(gst.entries.count(convert($1->type).substr(0,convert($1->type).length()-1))==0){
      error(@$,"Left operand of \"->\"  is not a pointer to structure");
    }
    if(gst.entries[convert($1->type).substr(0,convert($1->type).length()-1)].type!=0){
      error(@$,"Left operand of \"->\"  is not a pointer to structure");
    }
    temp = convert($1->type).substr(0,convert($1->type).length()-1);
    identifier_astnode* id_node = new identifier_astnode($3);
    $$ = new arrow_astnode($1,id_node);
    $$->type = "Not Found";
    $$->isLval = true;
    for(auto &locals:gst.entries[temp].symtab->elems){
      if(locals.first == $3){
        $$->type = locals.second.datatype;
        break;
      }
    }
    if($$->type == "Not Found"){
      error(@$,"\""+$1->type+"\" has no member named \""+$3+"\"");
    }
  }
  | postfix_expression INC_OP{
    if($1->isLval==false){
      error (@$,"Operand of \"++\" should have lvalue");
    }
    if($1->type.find("[")!=std::string::npos || ($1->type.find("*")==std::string::npos && $1->type!="int" && $1->type!="float")){
      error (@$,"Operand of \"++\" should be a int, float or pointer");
    }
    $$ = new op_unary_astnode(UNARY_OP::PP,$1);
    $$->type = $1->type;
    $$->isLval = false;
  }
;
primary_expression: 
  IDENTIFIER
  {
    $$ = new identifier_astnode($1);
    std::string type;
    if(local_var_handler.getType($1, type))
    {
      $$->type = type;
      $$->isLval = true;
    }
    else
    {
      error (@$,"Variable \""+$1+"\" not declared");
    }
  }
  | INT_CONSTANT
  {
    $$ = new intconst_astnode($1);
    $$->type = "int";
    $$->isLval = false;
  }
  | FLOAT_CONSTANT
  {
    $$ = new floatconst_astnode($1);
    $$->type = "float";
    $$->isLval = false;
  }
  | STRING_LITERAL
  {
    $$ = new stringconst_astnode($1);
    $$->type = "string";
    $$->isLval = false;
  }
  | '(' expression ')'
  {
      $$ = $2;
  }
;
expression_list: 
     expression{
      std::vector<exp_astnode*> vec;
      vec.push_back($1);
      $$ = vec;
   }
   | expression_list ',' expression{
      $$ = $1;
      $$.push_back($3);
   }
   ;
unary_operator: 
     '-'{
      $$ = '-';
   }
   | '!'{
      $$ = '!';
   }
   | '&'{
      $$ = '&';
   }
   | '*'{
      $$ = '*';
   }
   ;
selection_statement: 
     IF '(' expression ')' statement ELSE statement{
      $$ = new if_astnode($3,$5,$7);
   }
   ;
iteration_statement: 
     WHILE '(' expression ')' statement{
      $$ = new while_astnode($3,$5);
   }
   | FOR '(' assignment_expression ';' expression ';' assignment_expression ')' statement{
      $$ = new for_astnode((exp_astnode*)$3,$5,(exp_astnode*)$7,$9);
   }
   ;
declaration_list: 
  declaration
  {
    $$ = new DeclarationList();
    local_var_handler.setLocalVarDeclList($$);
    for(const auto decl: $1)
    {
      if(!(local_var_handler.addLocalVarDecl(decl)))
        error(@$, "\""+decl->id+"\" has a previous declaration");
    }
  }
  | declaration_list declaration
  {
    $$=$1;
    for(const auto decl: $2)
    {
      if(!(local_var_handler.addLocalVarDecl(decl)))
        error(@$, "\""+decl->id+"\" has a previous declaration");
    }
  }
;
declaration: 
  type_specifier declarator_list ';'
  {
    $$=$2;
    for(auto& decl:$$)
    {
      decl->type = $1 + decl->type;
      if(decl->type=="void")
      {
        error(@$,"Cannot declare variable of type \""+decl->type+"\"");
      }

      if(decl->type.find("*")!=std::string::npos)
        decl->width *= 4; 
      else if($1 == "int")
        decl->width *= 4;
      else if($1 == "float")
        decl->width *= 4;
      else
      {
        if(!width_map.count($1))
          error(@$, "\""+$1+"\" is not defined");
        decl->width *= width_map[$1];
      }
    }
  }
;
declarator_list: 
  declarator
  {
    $$.push_back($1);
  }
  | declarator_list ',' declarator
  {
    $$=$1;
    $$.push_back($3);
  }
;


%%
void 
IPL::Parser::error( const location_type &l, const std::string &err_message )
{
      std::cout << "Error at line " << l.begin.line << ": " << err_message << "\n";
    /* std::cout << "Error at location " << l << ": " << err_message << "\n";  */
   exit(1);

}





