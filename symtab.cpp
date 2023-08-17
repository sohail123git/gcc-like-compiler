#include "symtab.hpp"
// #include <bits/stdc++.h>

void GlobalSymTab::printgst(){
}

void LsymTab::print(){
}

void Info::print(const std::string &name){
}

void DeclarationList::add_item(Declaration *decln)
{
    vars.push_back(decln);
}

void ParamDeclarationList::add_item(Declaration *decln)
{
    params.push_back(decln);
    for(auto& offset: offsets)
        offset+=decln->width;
    offsets.push_back(12);
}

void GlobalSymTab::addStruct(const Struct &strct)
{
    Entry new_entry;
    new_entry.info.type = "struct";
    new_entry.info.scope = "global";
    new_entry.info.width = strct.membervars->getTotalWidth();
    new_entry.info.offset = 0;
    new_entry.info.datatype = "-";

    new_entry.type = Entry::EntryType::Struct;
    new_entry.symtab = new LsymTab();

    new_entry.symtab->addStructDeclarationList(strct.membervars);

    entries[strct.name]=new_entry;
}

void GlobalSymTab::addFunction(const Function &func)
{
    Entry new_entry;
    new_entry.info.type = "fun";
    new_entry.info.scope = "global";
    new_entry.info.width = 0;
    new_entry.info.offset = 0;
    new_entry.info.datatype = func.returntype;

    new_entry.type = Entry::EntryType::Func;
    new_entry.symtab = new LsymTab();

    new_entry.symtab->addFunctionDeclarationList(func.localvars);
    new_entry.symtab->addParamDeclarationList(func.params);

    entries[func.name]=new_entry;
}

void LsymTab::addStructDeclarationList(DeclarationList* vars)
{
    if(vars==nullptr)
        return;    
    for(int i=0;i<vars->getNumOfVars();i++)
    {
        Info info;
        info.type = "var";
        info.scope = "local";
        info.width = vars->getDeclaration(i)->width;
        info.offset = vars->getStructOffset(i);
        info.datatype = vars->getDeclaration(i)->type;
        elems[vars->getDeclaration(i)->id] = info;
    }
}

void LsymTab::addFunctionDeclarationList(DeclarationList* vars)
{
    if(vars==nullptr)
        return;
    for(int i=0;i<vars->getNumOfVars();i++)
    {
        Info info;
        info.type = "var";
        info.scope = "local";
        info.width = vars->getDeclaration(i)->width;
        info.offset = vars->getFuncOffset(i);
        info.datatype = vars->getDeclaration(i)->type;
        elems[vars->getDeclaration(i)->id] = info;
    }
}

void LsymTab::addParamDeclarationList(ParamDeclarationList* params)
{
    if(params == nullptr)
        return;
    for(int i=0;i<params->getNumOfVars();i++)
    {
        Info info;
        info.type = "var";
        info.scope = "param";
        info.width = params->getDeclaration(i)->width;
        info.offset = params->getOffset(i);
        info.datatype = params->getDeclaration(i)->type;
        elems[params->getDeclaration(i)->id] = info;
        this->params.push_back(params->getDeclaration(i)->id);
    }
}

int DeclarationList::getFuncOffset(int index)
{
    int result=0;
    for(int i=0;i<=index;i++)
    {
        result-=vars[i]->width;
    }
    return result;
}

int DeclarationList::getStructOffset(int index)
{
    int result=0;
    for(int i=0;i<index;i++)
    {
        result+=vars[i]->width;
    }
    return result;
}

int DeclarationList::getTotalWidth()
{
    int result=0;
    for(auto var:vars)
    {
        result+=var->width;
    }
    return result;
}

int DeclarationList::getIndex(const std::string &var_name)
{
    for(size_t i =0;i<vars.size(); i++)
    {
        if(var_name==vars[i]->id)
            return i;
    } 
    return -1;
}

int ParamDeclarationList::getIndex(const std::string &var_name)
{
    for(size_t i =0;i<params.size(); i++)
    {
        if(var_name==params[i]->id)
            return i;
    } 
    return -1;
}


bool LocalVarHandler::getType(const std::string &var_name, std::string &type)
{
    if(m_local_vars!=nullptr)
    {
        int index = m_local_vars->getIndex(var_name);
        if(index!=-1)
        {
            type=m_local_vars->getDeclaration(index)->type;
            return true;
        }
    }
    
    if(m_func_params!=nullptr)
    {
        int index = m_func_params->getIndex(var_name);
        if(index!=-1)
        {
            type=m_func_params->getDeclaration(index)->type;
            return true;
        }
    }
    return false;
}
