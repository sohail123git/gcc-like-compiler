#include "ast.hpp"
#include "symtab.hpp"
// #include <bits/stdc++.h>
#include <cstdlib>

int Glabel=0;
int Jlabel=0;
bool par_deref=false;
std::vector<std::string> Gstrings;
extern std::map<std::string,abstract_astnode*> ast;
extern std::string CurrFun;
extern LsymTab *CurrSymTab;
extern GlobalSymTab gstfun;
extern GlobalSymTab gststruct;

bool isAddr(exp_astnode* exp){
    if(exp->astnode_type == typeExp::IDENTIFIER){
        return true;
    }
    else if(exp->astnode_type == typeExp::MEMBER_NODE){
        return true;
    }
    else if(exp->astnode_type == typeExp::ARROW_NODE){
        return true;
    }
    else if(exp->astnode_type == typeExp::ARRAY_REF){
        return true;
    }
    else if(exp->astnode_type == typeExp::OP_UNARY_NODE){
        op_unary_astnode *opu = (op_unary_astnode*)exp;
        if(opu->op == UNARY_OP::DEREF){
            return true;
        }
    }
    return false;
}

abstract_astnode::abstract_astnode()
{

}
exp_astnode::exp_astnode()
{
}

statement_astnode::statement_astnode()
{
}

empty_astnode::empty_astnode(){
    astnode_type = typeExp::EMPTY;
}

void empty_astnode::print(int blanks)
{  
}

seq_astnode::seq_astnode(std::vector<statement_astnode*> &stmts)
    :stmts(stmts)
{
    astnode_type = typeExp::SEQ;
}

void seq_astnode::print(int blanks)
{   
    for(int i=0;i<stmts.size();i++){
        stmts[i]->print(0);
    }
}

assignS_astnode::assignS_astnode(exp_astnode* left, exp_astnode* right)
    :left(left), right(right)
{
    astnode_type = typeExp::ASSIGN_S;
}

void assignS_astnode::print(int blanks)
{  
    if(right->type.find("*")!=std::string::npos || right->type.find("[")!=std::string::npos || right->type=="int"){
        right->print(0);
        if(isAddr(right))
            std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
        std::cout<<"\tpushl\t%eax"<<std::endl;    
        left->print(0);
        std::cout<<"\tmovl\t%eax, %edx"<<std::endl;
        std::cout<<"\tpopl\t%eax"<<std::endl;
        std::cout<<"\tmovl\t%eax,(%edx)"<<std::endl;
    }
    else{
        int width = gststruct.entries[right->type].info.width;
        right->print(0);
        std::cout<<"\tpushl\t%eax"<<std::endl;    
        left->print(0);
        std::cout<<"\tmovl\t%eax, %edx"<<std::endl;
        std::cout<<"\tpopl\t%eax"<<std::endl;
        std::cout<<"\tmovl\t%eax, %ecx"<<std::endl;
        std::cout<<"\tmovl\t%ecx, %eax"<<std::endl;
        std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
        std::cout<<"\tmovl\t%eax,(%edx)"<<std::endl;
        for(int i=4;i<width;i+=4){
            std::cout<<"\taddl\t$4, %ecx"<<std::endl;
            std::cout<<"\taddl\t$4, %edx"<<std::endl;
            std::cout<<"\tmovl\t%ecx, %eax"<<std::endl;
            std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            std::cout<<"\tmovl\t%eax,(%edx)"<<std::endl;
        }
    } 
}

return_expnode::return_expnode(exp_astnode* ret_exp)
    :ret_exp(ret_exp)
{
    astnode_type = typeExp::RETURN;
}

void return_expnode::print(int blanks)
{
    ret_exp->print(0);
    if(ret_exp->type!="int" && (ret_exp->type.find("*")==std::string::npos && ret_exp->type.find("]")==std::string::npos)){
     //dont deref   
    }
    else if(isAddr(ret_exp)){
        std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
    }   
    std::cout<<"\tleave\n\tret"<<std::endl;
}

if_astnode::if_astnode(exp_astnode* if_condition, statement_astnode* then_stmt, statement_astnode* else_stmt)
    :if_condition(if_condition), then_stmt(then_stmt), else_stmt(else_stmt)
{
    astnode_type = typeExp::IF;
}

void if_astnode::print(int blanks)
{   
    int Jlabel1 = Jlabel++;
    int Jlabel2 = Jlabel++;
    if_condition->print(0);
    if(isAddr(if_condition)){
        std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
    }
    std::cout<<"\tcmp\t$0, %eax"<<std::endl;
    std::cout<<"\tje .L"<<Jlabel1<<std::endl;
    then_stmt->print(0);
    std::cout<<"\tjmp .L"<<Jlabel2<<std::endl;
    std::cout<<".L"<<Jlabel1<<":"<<std::endl;
    else_stmt->print(0);
    std::cout<<".L"<<Jlabel2<<":"<<std::endl;
}

while_astnode::while_astnode(exp_astnode* condition, statement_astnode* stmts)
    :condition(condition), stmts(stmts)
{
    astnode_type = typeExp::WHILE;
}

void while_astnode::print(int blanks)
{
    int Jlabel1 = Jlabel++;
    int Jlabel2 = Jlabel++;
    std::cout<<".L"<<Jlabel1<<":"<<std::endl;
    condition->print(0);  
    if(isAddr(condition)){
        std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
    } 
    std::cout<<"\tcmp\t$0, %eax"<<std::endl;
    std::cout<<"\tje .L"<<Jlabel2<<std::endl;
    stmts->print(0);
    std::cout<<"\tjmp .L"<<Jlabel1<<std::endl;
    std::cout<<".L"<<Jlabel2<<":"<<std::endl;
}

for_astnode::for_astnode(
    exp_astnode* init,
    exp_astnode* condition,
    exp_astnode* update,
    statement_astnode* stmts
)
    :init(init), condition(condition), update(update), stmts(stmts)
{
    astnode_type = typeExp::FOR;
}

void for_astnode::print(int blanks)
{ 
    init->print(0); 
    int Jlabel1 = Jlabel++;
    int Jlabel2 = Jlabel++;
    std::cout<<".L"<<Jlabel1<<":"<<std::endl;
    condition->print(0);  
    if(isAddr(condition)){
        std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
    } 
    std::cout<<"\tcmp\t$0, %eax"<<std::endl;
    std::cout<<"\tje .L"<<Jlabel2<<std::endl;
    stmts->print(0);
    update->print(0);
    std::cout<<"\tjmp .L"<<Jlabel1<<std::endl;
    std::cout<<".L"<<Jlabel2<<":"<<std::endl; 
}

intconst_astnode::intconst_astnode(std::string &val)
{
    astnode_type = typeExp::INT;
    this->val = std::stoi(val);
}
void intconst_astnode::print(int blanks)
{   
    std::cout<<"\tmovl\t$"<<val<<", %eax"<<std::endl;
}

floatconst_astnode::floatconst_astnode(std::string &val)
{
    astnode_type = typeExp::FLOAT;
    this->val = std::stof(val);
}

void floatconst_astnode::print(int blanks)
{   
}

stringconst_astnode::stringconst_astnode(std::string &val)
{
    astnode_type = typeExp::STRING;
    this->val = val;
    this->label = Glabel++;
    Gstrings.push_back(val);
}

void stringconst_astnode::print(int blanks)
{   
    std::cout<<"\tpushl\t$.LC"<<label<<std::endl;
}

identifier_astnode::identifier_astnode(std::string &val)
    :val(val)
{
    iid = val;
    astnode_type = typeExp::IDENTIFIER;
}

void identifier_astnode::print(int blanks)
{   
    std::cout<<"\tmovl\t%ebp, %eax"<<std::endl;
    std::cout<<"\taddl\t$"<<CurrSymTab->elems[val].offset<<", %eax"<<std::endl;
    if(CurrSymTab->elems[val].scope == "param" && (type.find("*")!=std::string::npos||type.find("[")!=std::string::npos)){
        par_deref=true;
    }
    else{
        par_deref=false;
    }
}

ref_astnode::ref_astnode()
{
}

op_binary_astnode::op_binary_astnode(BINARY_OP op, exp_astnode* left, exp_astnode* right)
    :op(op), left(left), right(right)
{
    astnode_type = typeExp::OP_BINARY_NODE;
}

void op_binary_astnode::print(int blanks){
    switch(op){
        case BINARY_OP::PLUS_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\taddl\t%edx, %eax"<<std::endl;
            break;
        case BINARY_OP::MINUS_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            std::cout<<"\tmovl\t%eax, %edx"<<std::endl;
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%edx), %edx"<<std::endl;
            }
            std::cout<<"\tpopl\t%eax"<<std::endl;
            if(left->type.find("*")!=std::string::npos){
                std::cout<<"\timull\t$4, %edx"<<std::endl;
            }
            std::cout<<"\tsubl\t%edx, %eax"<<std::endl;
            break;
        case BINARY_OP::MULT_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\timull\t%edx, %eax"<<std::endl;
            break;
        case BINARY_OP::DIV_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tmovl\t%eax, %ebx"<<std::endl;
            std::cout<<"\tpopl\t%eax"<<std::endl;
            std::cout<<"\tmovl\t$0, %edx"<<std::endl;
            std::cout<<"\tcltd"<<std::endl;
            std::cout<<"\tidivl\t%ebx"<<std::endl;
            break;
        case BINARY_OP::LT_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\tcmp\t%eax, %edx"<<std::endl;
            std::cout<<"\tjl .L"<<Jlabel++<<std::endl;
            std::cout<<"\tmovl\t$0, %eax"<<std::endl;
            std::cout<<"\tjmp .L"<<Jlabel++<<std::endl;
            std::cout<<".L"<<Jlabel-2<<":"<<std::endl;
            std::cout<<"\tmovl\t$1, %eax"<<std::endl;
            std::cout<<".L"<<Jlabel-1<<":"<<std::endl;
            break;
        case BINARY_OP::LE_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\tcmp\t%eax, %edx"<<std::endl;
            std::cout<<"\tjle .L"<<Jlabel++<<std::endl;
            std::cout<<"\tmovl\t$0, %eax"<<std::endl;
            std::cout<<"\tjmp .L"<<Jlabel++<<std::endl;
            std::cout<<".L"<<Jlabel-2<<":"<<std::endl;
            std::cout<<"\tmovl\t$1, %eax"<<std::endl;
            std::cout<<".L"<<Jlabel-1<<":"<<std::endl;
            break;
        case BINARY_OP::GE_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\tcmp\t%eax, %edx"<<std::endl;
            std::cout<<"\tjge .L"<<Jlabel++<<std::endl;
            std::cout<<"\tmovl\t$0, %eax"<<std::endl;
            std::cout<<"\tjmp .L"<<Jlabel++<<std::endl;
            std::cout<<".L"<<Jlabel-2<<":"<<std::endl;
            std::cout<<"\tmovl\t$1, %eax"<<std::endl;
            std::cout<<".L"<<Jlabel-1<<":"<<std::endl;
            break;
        case BINARY_OP::GT_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\tcmp\t%eax, %edx"<<std::endl;
            std::cout<<"\tjg .L"<<Jlabel++<<std::endl;
            std::cout<<"\tmovl\t$0, %eax"<<std::endl;
            std::cout<<"\tjmp .L"<<Jlabel++<<std::endl;
            std::cout<<".L"<<Jlabel-2<<":"<<std::endl;
            std::cout<<"\tmovl\t$1, %eax"<<std::endl;
            std::cout<<".L"<<Jlabel-1<<":"<<std::endl;
            break;
        case BINARY_OP::EQ_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\tcmp\t%eax, %edx"<<std::endl;
            std::cout<<"\tje .L"<<Jlabel++<<std::endl;
            std::cout<<"\tmovl\t$0, %eax"<<std::endl;
            std::cout<<"\tjmp .L"<<Jlabel++<<std::endl;
            std::cout<<".L"<<Jlabel-2<<":"<<std::endl;
            std::cout<<"\tmovl\t$1, %eax"<<std::endl;
            std::cout<<".L"<<Jlabel-1<<":"<<std::endl;
            break;
        case BINARY_OP::NE_INT:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\tcmp\t%eax, %edx"<<std::endl;
            std::cout<<"\tjne .L"<<Jlabel++<<std::endl;
            std::cout<<"\tmovl\t$0, %eax"<<std::endl;
            std::cout<<"\tjmp .L"<<Jlabel++<<std::endl;
            std::cout<<".L"<<Jlabel-2<<":"<<std::endl;
            std::cout<<"\tmovl\t$1, %eax"<<std::endl;
            std::cout<<".L"<<Jlabel-1<<":"<<std::endl;
            break;
        case BINARY_OP::AND:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\tcmp\t$0, %edx"<<std::endl;
            std::cout<<"\tje .L"<<Jlabel++<<std::endl;
            std::cout<<"\tcmp\t$0, %eax"<<std::endl;
            std::cout<<"\tje .L"<<Jlabel-1<<std::endl;
            std::cout<<"\tmovl\t$1, %eax"<<std::endl;
            std::cout<<"\tjmp .L"<<Jlabel++<<std::endl;
            std::cout<<".L"<<Jlabel-2<<":"<<std::endl;
            std::cout<<"\tmovl\t$0, %eax"<<std::endl;
            std::cout<<".L"<<Jlabel-1<<":"<<std::endl;
            break;
        case BINARY_OP::OR:
            left->print(0);
            if(left->astnode_type==typeExp::INT){
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else if(isAddr(left)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else{
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            right->print(0);
            if(isAddr(right)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tpopl\t%edx"<<std::endl;
            std::cout<<"\tcmp\t$0, %edx"<<std::endl;
            std::cout<<"\tjne .L"<<Jlabel++<<std::endl;
            std::cout<<"\tcmp\t$0, %eax"<<std::endl;
            std::cout<<"\tjne .L"<<Jlabel-1<<std::endl;
            std::cout<<"\tmovl\t$0, %eax"<<std::endl;
            std::cout<<"\tjmp .L"<<Jlabel++<<std::endl;
            std::cout<<".L"<<Jlabel-2<<":"<<std::endl;
            std::cout<<"\tmovl\t$1, %eax"<<std::endl;
            std::cout<<".L"<<Jlabel-1<<":"<<std::endl;
            break;
    }
}

op_unary_astnode::op_unary_astnode(UNARY_OP op, exp_astnode* child)
    :op(op), child_exp(child)
{
    astnode_type = typeExp::OP_UNARY_NODE;

}

void op_unary_astnode::print(int blanks)
{
    switch(op){
        case UNARY_OP::MINUS:
            child_exp->print(0);
            if(isAddr(child_exp)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\timull\t$-1, %eax"<<std::endl;
            break;
        case UNARY_OP::NOT:
            child_exp->print(0);
            if(isAddr(child_exp)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            }
            std::cout<<"\tcmp\t$0, %eax"<<std::endl;
            std::cout<<"\tjne .L"<<Jlabel++<<std::endl;
            std::cout<<"\tmovl\t$1, %eax"<<std::endl;
            std::cout<<"\tjmp .L"<<Jlabel++<<std::endl;
            std::cout<<".L"<<Jlabel-2<<":"<<std::endl;
            std::cout<<"\tmovl\t$0, %eax"<<std::endl;
            std::cout<<".L"<<Jlabel-1<<":"<<std::endl;
            break;
        case UNARY_OP::PP:
            child_exp->print(0);
            std::cout<<"\tmovl\t%eax, %edx"<<std::endl;
            std::cout<<"\tmovl\t(%edx), %eax"<<std::endl;
            std::cout<<"\tmovl\t%eax, %ecx"<<std::endl;
            std::cout<<"\taddl\t$1, %ecx"<<std::endl;
            std::cout<<"\tmovl\t%ecx, (%edx)"<<std::endl;
            break;
        case UNARY_OP::ADDRESS:
            child_exp->print(0);
            break;
        case UNARY_OP::DEREF:
            child_exp->print(0);
            std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            break;
    }
}

assignE_astnode::assignE_astnode(exp_astnode* left, exp_astnode* right)
    :left(left), right(right)
{
    astnode_type = typeExp::ASSIGN_E;
}

void assignE_astnode::print(int blanks)
{
     if(right->type.find("*")!=std::string::npos || right->type.find("[")!=std::string::npos || right->type=="int"){
        right->print(0);
        if(isAddr(right))
            std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
        std::cout<<"\tpushl\t%eax"<<std::endl;    
        left->print(0);
        std::cout<<"\tmovl\t%eax, %edx"<<std::endl;
        std::cout<<"\tpopl\t%eax"<<std::endl;
        std::cout<<"\tmovl\t%eax,(%edx)"<<std::endl;
    }
    else{
        int width = gststruct.entries[right->type].info.width;
        right->print(0);
        std::cout<<"\tpushl\t%eax"<<std::endl;    
        left->print(0);
        std::cout<<"\tmovl\t%eax, %edx"<<std::endl;
        std::cout<<"\tpopl\t%eax"<<std::endl;
        std::cout<<"\tmovl\t%eax, %ecx"<<std::endl;
        std::cout<<"\tmovl\t%ecx, %eax"<<std::endl;
        std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
        std::cout<<"\tmovl\t%eax,(%edx)"<<std::endl;
        for(int i=4;i<width;i+=4){
            std::cout<<"\taddl\t$4, %ecx"<<std::endl;
            std::cout<<"\taddl\t$4, %edx"<<std::endl;
            std::cout<<"\tmovl\t%ecx, %eax"<<std::endl;
            std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
            std::cout<<"\tmovl\t%eax,(%edx)"<<std::endl;
        }
    } 
}

exp_astnode* assignE_astnode::get_left(){
    return left;
}

exp_astnode* assignE_astnode::get_right(){
    return right;
}

funcall_astnode::funcall_astnode(exp_astnode* fname)
    :fname(fname)
{
    astnode_type = typeExp::FUNCALL;
}

funcall_astnode::funcall_astnode(exp_astnode* fname, std::vector<exp_astnode*> &exps)
    :fname(fname), exps(exps)
{
    astnode_type = typeExp::FUNCALL;
}

void funcall_astnode::print(int blanks)
{
    if(((identifier_astnode*)fname)->val=="printf"){
        for(int i=exps.size()-1;i>=0;i--){
            exps[i]->print(0);
            if(exps[i]->astnode_type!=typeExp::STRING){
                if(isAddr(exps[i]) && (exps[i]->type.find("[")==std::string::npos)){
                    // std::cout<<((identifier_astnode*)exps[i])->val<<std::endl;
                    std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                }
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
        }
    }
    else{
        for(int i=0;i<exps.size();i++){
            exps[i]->print(0);
            if(exps[i]->type!="int" && exps[i]->type.find("[")==std::string::npos && exps[i]->type.find("*")==std::string::npos){
                int width = gststruct.entries[exps[i]->type].info.width;
                std::cout<<"\tmovl\t%eax, %ecx"<<std::endl;
                std::cout<<"\taddl\t$"<<width-4<<", %ecx"<<std::endl;
                for(int i=width-4;i>=0;i-=4){
                    std::cout<<"\tmovl\t%ecx, %eax"<<std::endl;
                    std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                    std::cout<<"\tpushl\t%eax"<<std::endl;
                    std::cout<<"\tsubl\t$4, %ecx"<<std::endl;
                }
            }
            else if(isAddr(exps[i]) && (exps[i]->type.find("[")==std::string::npos)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else {
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
        }
    }
    if(((identifier_astnode*)fname)->val!="printf")
        std::cout<<"\tsubl\t$4, %esp"<<std::endl;
    if(fname->astnode_type==typeExp::IDENTIFIER){
        std::cout<<"\tcall\t"<<((identifier_astnode*)fname)->val<<std::endl;
    }
    if(((identifier_astnode*)fname)->val!="printf")
        std::cout<<"\taddl\t$4 ,%esp"<<std::endl;
    for (auto it = gstfun.entries.begin(); it != gstfun.entries.end(); ++it)
    {
        int width = 0;
        if(it->first == ((identifier_astnode*)fname)->val){
            LsymTab* Currproc = it->second.symtab;
            for(auto elem = Currproc->elems.begin();elem!=Currproc->elems.end();elem++){
	    	    if(elem->second.scope == "param"){
                    if(elem->second.datatype.find("*")==std::string::npos && elem->second.datatype.find("[")==std::string::npos)
			            width+=elem->second.width;
                    else{
                        width+=4;
                    }
		        }
	        }
        }
        if(width!=0){
            std::cout<<"\taddl\t$"<<width<<" ,%esp"<<std::endl;
        }
    }
    if(((identifier_astnode*)fname)->val == "printf"){
        std::cout<<"\taddl\t$"<<4*exps.size()<<", %esp"<<std::endl;
    }
}

proccall_astnode::proccall_astnode(exp_astnode* fname)
    :fname(fname)
{
    astnode_type = typeExp::PROCCALL;
}

proccall_astnode::proccall_astnode(exp_astnode* fname, std::vector<exp_astnode*> &exps)
    :fname(fname), exps(exps)
{
    astnode_type = typeExp::PROCCALL;
}

void proccall_astnode::print(int blanks)
{
    if(((identifier_astnode*)fname)->val=="printf"){
        for(int i=exps.size()-1;i>=0;i--){
            exps[i]->print(0);
            if(exps[i]->astnode_type!=typeExp::STRING){
                if(isAddr(exps[i]) && (exps[i]->type.find("[")==std::string::npos)){
                    // std::cout<<((identifier_astnode*)exps[i])->val<<std::endl;
                    std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                }
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
        }
    }
    else{
        for(int i=0;i<exps.size();i++){
            exps[i]->print(0);
            if(exps[i]->type!="int" && exps[i]->type.find("[")==std::string::npos && exps[i]->type.find("*")==std::string::npos){
                int width = gststruct.entries[exps[i]->type].info.width;
                std::cout<<"\tmovl\t%eax, %ecx"<<std::endl;
                std::cout<<"\taddl\t$"<<width-4<<", %ecx"<<std::endl;
                for(int i=width-4;i>=0;i-=4){
                    std::cout<<"\tmovl\t%ecx, %eax"<<std::endl;
                    std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                    std::cout<<"\tpushl\t%eax"<<std::endl;
                    std::cout<<"\tsubl\t$4, %ecx"<<std::endl;
                }
            }
            else if(isAddr(exps[i]) && (exps[i]->type.find("[")==std::string::npos)){
                std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
            else {
                std::cout<<"\tpushl\t%eax"<<std::endl;
            }
        }
    }
    if(((identifier_astnode*)fname)->val!="printf")
        std::cout<<"\tsubl\t$4, %esp"<<std::endl;
    if(fname->astnode_type==typeExp::IDENTIFIER)
    std::cout<<"\tcall\t"<<((identifier_astnode*)fname)->val<<std::endl;
    if(((identifier_astnode*)fname)->val!="printf")
        std::cout<<"\taddl\t$4, %esp"<<std::endl;
    for (auto it = gstfun.entries.begin(); it != gstfun.entries.end(); ++it)
    {
        int width = 0;
        if(it->first == ((identifier_astnode*)fname)->val){
            LsymTab* Currproc = it->second.symtab;
            for(auto elem = Currproc->elems.begin();elem!=Currproc->elems.end();elem++){
	    	    if(elem->second.scope == "param"){
                    if(elem->second.datatype.find("*")==std::string::npos && elem->second.datatype.find("[")==std::string::npos)
			            width+=elem->second.width;
                    else{
                        width+=4;
                    }
		        }
	        }
        }
        if(width!=0){
            std::cout<<"\taddl\t$"<<width<<" ,%esp"<<std::endl;
        }
    }
    if(((identifier_astnode*)fname)->val == "printf"){
        std::cout<<"\taddl\t$"<<4*exps.size()<<", %esp"<<std::endl;
    }
}

arrayref_astnode::arrayref_astnode(exp_astnode* array, exp_astnode* index)
    :array(array), index(index)
{
    astnode_type = typeExp::ARRAY_REF;
}

void arrayref_astnode::print(int blanks)
{ 
    array->print(0);
    std::cout<<"\tpushl\t%eax"<<std::endl;
    index->print(0);
    if(isAddr(index)){
        std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
    }  
    std::cout<<"\tpopl\t%edx"<<std::endl;
    int beg=0,width;
    std::string base_type="";
    for(int i=0;i<=array->type.size();i++){
        if(array->type[i]=='*' || array->type[i]=='['){
            break;
        }
        else{
            base_type = base_type + array->type[i];
        }
    }
    // std::cout<<base_type<<std::endl;
    // std::cout<<CurrSymTab->elems[base_type].scope<<std::endl;
    if(par_deref){
        std::cout<<"\tmovl\t(%edx), %edx"<<std::endl;
        par_deref = false;
    }
    if(base_type == "int"){
        width = 4;
    }
    else{
        for (auto it = gststruct.entries.begin(); it != gststruct.entries.end(); ++it)
        {
            if(it->first == base_type){
                width = it->second.info.width;
            }
        }
    }
    // std::cout<<base_type<<std::endl;
    // std::cout<<width<<std::endl;
    for(int i=0;i<=array->type.size();i++){
        std::string temp = "";
        if(array->type[i]=='[' && beg==0){
            beg=1;
            while(array->type[i]!=']'){
                i++;
            }
        }
        else if((array->type)[i]=='['){
            i++;
            while((array->type[i])!=']'){
                temp = temp+array->type[i];
                i++;
            }
        }
        if(temp!="")
            width = width*stoi(temp);
        // std::cout<<temp<<std::endl;
    }
    std::cout<<"\timull\t$"<<width<<", %eax"<<std::endl;
    std::cout<<"\taddl\t%eax,%edx"<<std::endl;
    std::cout<<"\tmovl\t%edx,%eax"<<std::endl;
}

member_astnode::member_astnode(exp_astnode* expr, identifier_astnode* id)
    :expr(expr),id(id)
{
    astnode_type = typeExp::MEMBER_NODE;
}

void member_astnode::print(int blanks)
{   
    expr->print(0);
    for (auto it = gststruct.entries.begin(); it != gststruct.entries.end(); ++it)
    {
        if(it->first == expr->type){
            LsymTab* Currproc = it->second.symtab;
            for(auto elem = Currproc->elems.begin();elem!=Currproc->elems.end();elem++){
	    	    if(elem->first == id->val){
			        std::cout<<"\taddl\t$"<<elem->second.offset<<", %eax"<<std::endl;
		        }
	        }
        }
    }
}

arrow_astnode::arrow_astnode(exp_astnode* expr, identifier_astnode* id)
:   expr(expr), id(id)
{
    astnode_type = typeExp::ARROW_NODE;
}

void arrow_astnode::print(int blanks)
{   
    expr->print(0);
    for (auto it = gststruct.entries.begin(); it != gststruct.entries.end(); ++it)
    {
        if(it->first+"*" == expr->type){
            LsymTab* Currproc = it->second.symtab;
            for(auto elem = Currproc->elems.begin();elem!=Currproc->elems.end();elem++){
	    	    if(elem->first == id->val){
                    std::cout<<"\tmovl\t(%eax), %eax"<<std::endl;
			        std::cout<<"\taddl\t$"<<elem->second.offset<<", %eax"<<std::endl;
		        }
	        }
        }
    }
}

