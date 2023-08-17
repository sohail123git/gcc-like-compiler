#include <cstdlib>
#include <string>
#include <vector>
#include "ast.hpp"
#include "symtab.hpp"

std::string convert(std::string s);

bool checkFuncCall(
 const std::string& funcname,
 std::vector<exp_astnode*>& params,
 std::string& type_or_error
);

bool typeCompatible(exp_astnode*& given_param, const std::string& req_type);
