#pragma once

#include <bits/stdc++.h>
#include <cstdlib>
#include "ast.hpp"
//    #include <map>

//TODO check identifier reuse
class Function;
class Struct;
class DeclarationList;
class ParamDeclarationList;


class Info{
public:
    std::string type;
    std::string scope;
    int width;
    int offset;
    std::string datatype;
    void print(const std::string &name);
};

class LsymTab{
public:
    std::map<std::string, Info> elems;
    std::vector<std::string> params;

    void print();
    void addStructDeclarationList(DeclarationList* vars);
    void addFunctionDeclarationList(DeclarationList* vars);
    void addParamDeclarationList(ParamDeclarationList* params);
    const std::string& getParamType(int index)
    {
        return elems[params[index]].datatype;
    };
}; 

class Entry{
public:
    enum EntryType
    {
        Struct,
        Func
    };
    Info info;
    EntryType type;
    LsymTab* symtab;
};




class GlobalSymTab{
public:
    std::map<std::string,Entry> entries;
    void printgst();
    void addStruct(const Struct &strct);
    void addFunction(const Function &func);

    inline bool hasStruct(const std::string& id)
    {
        if(entries.count(id))
            return (entries[id].info.type=="struct");
        return false;
    };
    inline bool hasFunc(const std::string& id)
    {
        if(entries.count(id))
            return (entries[id].info.type=="fun");
        return false;
    };
    inline std::string getFuncReturnType(const std::string& id)
    {
        if(entries.count(id))
            return entries[id].info.datatype;
        return "";
    };
    size_t getNumParams(const std::string& func_name)
    {
        if(entries.count(func_name))
            return entries[func_name].symtab->params.size();
        return 0;
    };
};



class Declaration
{
public:
    std::string id;
    int width;
    std::string type;

    Declaration(std::string id):id(id),width(1),type(""){};
    Declaration(std::string id, int width, std::string type)
        :id(id),width(width),type(type){};
};

class DeclarationList
{
private:
    std::vector<Declaration*> vars;
public:
    void add_item(
        Declaration *decln
    );
    int getNumOfVars(){return vars.size();};
    Declaration* getDeclaration(int index){return vars[index];};
    int getFuncOffset(int index);
    int getStructOffset(int index);

    int getTotalWidth();

    int getIndex(const std::string &var_name);
    bool hasItem(Declaration *decln)
    {
        for(const auto var: vars)
            if(var->id==decln->id)
                return true;
        return false;
    }
};

class ParamDeclarationList
{
private:
    std::vector<Declaration*> params;
    std::vector<int> offsets;
public:
    void add_item(Declaration *decln);
    int getNumOfVars(){return params.size();};
    Declaration* getDeclaration(int index){return params[index];};
    int getOffset(int index){return offsets[index];};
    
    int getIndex(const std::string &param_name);
    bool hasItem(Declaration *decln)
    {
        for(const auto param: params)
            if(param->id==decln->id)
                return true;
        return false;
    }
};


class Function
{
public:
    std::string returntype;
    std::string name;
    ParamDeclarationList* params;
    DeclarationList* localvars;
    abstract_astnode* ast;
    Function(){};
    Function(
        std::string &returntype,
        std::string &name,
        ParamDeclarationList* params,
        DeclarationList* localvars,
        abstract_astnode* ast
    ):returntype(returntype), name(name), params(params), localvars(localvars), ast(ast)
    {};

};

class Struct
{
public:
    std::string name;
    DeclarationList* membervars;
    Struct(){};
    Struct(
        std::string name,
        DeclarationList* membervars
    ):name(name), membervars(membervars)
    {};
    // ~Struct()
    // {
    //     delete membervars;
    // }
};


class LocalVarHandler
{
private:
    DeclarationList* m_local_vars;
    ParamDeclarationList* m_func_params;
    GlobalSymTab& m_gst;
    std::string func_name;
    std::string ret_type;
public:
    LocalVarHandler(GlobalSymTab& gst):m_gst(gst){};
    void reset()
    {
        m_local_vars=nullptr;
        m_func_params=nullptr;
        ret_type = "";
    };
    bool getType(const std::string &var_name, std::string &type);
    void setReturnType(std::string& type){ret_type=type;};
    const std::string& getReturnType(){return ret_type;};
    void setCurrentFuncName(std::string& name){func_name=name;};
    const std::string& getCurrentFuncName(){return func_name;};

    bool addParamDecl(Declaration* decln)
    {
        if(m_func_params!=nullptr&&m_func_params->hasItem(decln))
            return false;
        m_func_params->add_item(decln);
        return true;
    }
    bool addLocalVarDecl(Declaration* decln)
    {
        if(
            (m_func_params!=nullptr&&m_func_params->hasItem(decln))
            ||(m_local_vars!=nullptr&&m_local_vars->hasItem(decln))
        )
            return false;
        m_local_vars->add_item(decln);
        return true;
    }
    void setLocalVarDeclList(DeclarationList* local_vars)
    {
        m_local_vars=local_vars;
    };
    void setParamDeclList(ParamDeclarationList* func_params)
    {
        m_func_params=func_params;
    };
    size_t getNumParams()
    {
        return m_func_params->getNumOfVars();
    }
    std::string getParamType(int index)
    {
        return m_func_params->getDeclaration(index)->type;
    }
};
