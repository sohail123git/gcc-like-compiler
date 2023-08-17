#include "utils.hpp"

extern GlobalSymTab gst;
extern LocalVarHandler local_var_handler;

std::string convert(std::string s){
 if(s.find("(")!=std::string::npos){
   return s;
 }
 else if(s.find("[")!=std::string::npos && s.find("]")==s.length()-1){
    return  s.substr(0,s.find("["))+"*";
 }
 else if(s.find("[")!=std::string::npos){
    return  s.substr(0,s.find("["))+"(*)"+s.substr(s.find("]")+1);
 }
 return s;
}


bool checkFuncCall(
 const std::string& funcname,
 std::vector<exp_astnode*>& params,
 std::string& type_or_error
)
{
 if(gst.hasFunc(funcname))
 {
  if(gst.getNumParams(funcname)<params.size())
  {
   type_or_error="Function \""+funcname+"\"  called with too many arguments";
   return false;
  }
  else if(gst.getNumParams(funcname)>params.size())
  {
   type_or_error="Function \""+funcname+"\"  called with too few arguments";
   return false;
  }
  else
  {
   for(size_t i=0;i<params.size();i++)
   {
    const std::string& req_type = gst.entries[funcname].symtab->getParamType(i);
    if(!typeCompatible(params[i], req_type))
    {
     type_or_error = "Expected \""+req_type+"\" but argument is of type \""+params[i]->type+"\"";
     return false;
    }
   }

   type_or_error = gst.getFuncReturnType(funcname);
   return true;
  }
 }
 else if(local_var_handler.getCurrentFuncName()==funcname)
 {
  if(local_var_handler.getNumParams()<params.size())
  {
   type_or_error="Function \""+funcname+"\"  called with too many arguments";
   return false;
  }
  else if(local_var_handler.getNumParams()>params.size())
  {
   type_or_error="Function \""+funcname+"\"  called with too few arguments";
   return false;
  }
  else
  {
   for(size_t i=0;i<params.size();i++)
   {
    std::string req_type = local_var_handler.getParamType(i);
    if(!typeCompatible(params[i], req_type))
    {
     type_or_error = "Expected \""+req_type+"\" but argument is of type \""+params[i]->type+"\"";
     return false;
    }
   }
   type_or_error = local_var_handler.getReturnType();
   return true;
  }
 }
 else if(funcname=="scanf"||funcname=="printf")
 {
  type_or_error = "void";
  return true;
 }
 else if(funcname=="mod")
 {
  type_or_error = "int";
  return true;
 }
  
 type_or_error="Function \""+funcname+"\"  not declared";
 return false;
}

bool typeCompatible(exp_astnode*& given_param, const std::string& req_type)
{
 if(given_param->type=="int" && req_type=="int"){
  return true;
 }
 else if(given_param->type=="int" && req_type=="float"){
  given_param = new op_unary_astnode(UNARY_OP::TO_FLOAT,given_param);
  given_param->type = "float";
  given_param->isLval = false;
  return true;
 }
 else if(given_param->type=="float" && req_type=="int"){
  given_param = new op_unary_astnode(UNARY_OP::TO_INT,given_param);
  given_param->type = "int";
  given_param->isLval = false;
  return true;
 }
 else if(given_param->type=="float" && req_type=="float"){
  return true;
 }
 else if(convert(given_param->type)==convert(req_type)){
  return true;
 }
 else if(given_param->type=="void*" && (req_type.find("*")!=std::string::npos || req_type.find("[")!=std::string::npos)){
  return true;
 }
 else if(req_type=="void*" && (given_param->type.find("*")!=std::string::npos || given_param->type.find("[")!=std::string::npos)){
  return true;
 }
 
 return false;
}
