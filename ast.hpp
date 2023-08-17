#pragma once

#include <bits/stdc++.h>
   #include <cstdlib>


enum typeExp 
{
    //statement_astnode children
    
    EMPTY,
    SEQ,
    ASSIGN_S,
    RETURN,
    IF,
    WHILE,
    FOR,
    PROCCALL,

    //exp_astnode children
    

    OP_BINARY_NODE,
    OP_UNARY_NODE,
    ASSIGN_E,
    FUNCALL,
    INT,
    FLOAT,
    STRING,
    
    //ref_astnode children
    
    IDENTIFIER,
    ARRAY_REF,
    MEMBER_NODE,
    ARROW_NODE,

};

enum UNARY_OP
{
    TO_FLOAT,
    TO_INT,
    MINUS,
    NOT,
    ADDRESS,
    DEREF,
    PP
};

enum BINARY_OP
{
    OR,
    AND,
    
    EQ_INT,
    EQ_FLOAT,
    NE_INT,
    NE_FLOAT,

    LT_INT,
    LT_FLOAT,
    GT_INT,
    GT_FLOAT,
    
    GE_INT,
    GE_FLOAT,
    LE_INT,
    LE_FLOAT,

    PLUS_INT,
    PLUS_FLOAT,
    MINUS_INT,
    MINUS_FLOAT,

    MULT_INT,
    MULT_FLOAT,
    DIV_INT,
    DIV_FLOAT
};


class abstract_astnode
{
    protected:
        abstract_astnode();
    public:
        virtual void print(int blanks) = 0;
        typeExp astnode_type;
};

class exp_astnode: public abstract_astnode
{
    protected:
        exp_astnode();
    public:
        std::string iid;
        std::string type;
        bool isLval;
        virtual void print(int blanks) = 0;
};

class statement_astnode: public abstract_astnode
{
    protected:
        statement_astnode();
    public:
        virtual void print(int blanks) = 0;
};

class ref_astnode: public exp_astnode
{
    protected:
        ref_astnode();
    public:
        virtual void print(int blanks) = 0;
};

//START statement_astnode children

class empty_astnode: public statement_astnode
{
    public:
        empty_astnode();
        virtual void print(int blanks) override;
};

class seq_astnode: public statement_astnode
{   
    protected:
        std::vector<statement_astnode*> stmts;
    public:
        seq_astnode(){};
        seq_astnode(std::vector<statement_astnode*> &stmts);
        virtual void print(int blanks) override;
};

class assignS_astnode: public statement_astnode
{
    protected:
        exp_astnode* left;
        exp_astnode* right;
    public:
        assignS_astnode(exp_astnode* left, exp_astnode* right);
        virtual void print(int blanks) override;
};

class return_expnode: public statement_astnode
{
    protected:
        exp_astnode* ret_exp;
    public:
        return_expnode(exp_astnode* ret_exp);
        virtual void print(int blanks) override;
};

class if_astnode: public statement_astnode
{   
    protected:
        exp_astnode* if_condition;
        statement_astnode* then_stmt;
        statement_astnode* else_stmt;
    public:
        if_astnode(exp_astnode* if_condition, statement_astnode* then_stmt, statement_astnode* else_stmt);
        virtual void print(int blanks) override;
};

class while_astnode: public statement_astnode
{
    protected:
        exp_astnode* condition;
        statement_astnode* stmts;
    public:
        while_astnode(exp_astnode* condition, statement_astnode* stmts);
        virtual void print(int blanks) override;
};

class for_astnode: public statement_astnode
{
    protected:
        exp_astnode* init;
        exp_astnode* condition;
        exp_astnode* update;
        statement_astnode* stmts;
    public:
        for_astnode(
            exp_astnode* init,
            exp_astnode* condition,
            exp_astnode* update,
            statement_astnode* stmts
        );
        virtual void print(int blanks) override;
};

class proccall_astnode: public statement_astnode
{
    protected:
        exp_astnode* fname;
        std::vector<exp_astnode*> exps;
    public:
        proccall_astnode(exp_astnode* fname);
        proccall_astnode(exp_astnode* fname, std::vector<exp_astnode*> &exps);
        virtual void print(int blanks) override;

};

//END statement_astnode children


//START exp_astnode children

class op_binary_astnode: public exp_astnode
{
    protected:
        BINARY_OP op;
        exp_astnode* left;
        exp_astnode* right;
    public:
        op_binary_astnode(BINARY_OP op, exp_astnode* left, exp_astnode* right);
        virtual void print(int blanks) override;
    protected:
};

class op_unary_astnode: public exp_astnode
{
    protected:
        exp_astnode* child_exp;
    public:
        UNARY_OP op;
        op_unary_astnode(UNARY_OP val, exp_astnode* child);
        virtual void print(int blanks) override;
    protected:
};

class assignE_astnode: public exp_astnode
{
    protected:
        exp_astnode* left;
        exp_astnode* right;
    public:
        assignE_astnode(exp_astnode* left, exp_astnode* right);
        virtual void print(int blanks) override;
        exp_astnode* get_left();
        exp_astnode* get_right();
};

class funcall_astnode: public exp_astnode
{
    protected:
        exp_astnode* fname;
        std::vector<exp_astnode*> exps;
    public:
        funcall_astnode(exp_astnode* fname);
        funcall_astnode(exp_astnode* fname, std::vector<exp_astnode*> &exps);
        virtual void print(int blanks) override;
    protected:
};

class intconst_astnode: public exp_astnode
{
    protected:
        int val;
    public:
        intconst_astnode(std::string &val);
        int getVal(){return val;};
        virtual void print(int blanks) override;
    protected:
};

class floatconst_astnode: public exp_astnode
{
    protected:
        float val;
    public:
        floatconst_astnode(std::string &val);
        virtual void print(int blanks) override;
    protected:
};

class stringconst_astnode: public exp_astnode
{
        int label;
        std::string val;
    public:
        stringconst_astnode(std::string &val);
        virtual void print(int blanks) override;
    protected:
};

//END exp_astnode children

//START ref_astnode children

class identifier_astnode: public ref_astnode
{
    protected:
    public:
        std::string val;
        identifier_astnode(std::string &val);
        virtual void print(int blanks) override;
    protected:
};

class arrayref_astnode: public ref_astnode
{
    protected:
        exp_astnode* array;
        exp_astnode* index;
    public:
        arrayref_astnode(exp_astnode* array, exp_astnode* index);
        virtual void print(int blanks) override;
    protected:
};

class member_astnode: public ref_astnode
{
    protected:
        exp_astnode* expr;
        identifier_astnode* id;     
    public:
        member_astnode(exp_astnode* expr, identifier_astnode* id);
        virtual void print(int blanks) override;
    protected:
};

class arrow_astnode: public ref_astnode
{
    protected:
        exp_astnode* expr;
        identifier_astnode* id;  
    public:
        arrow_astnode(exp_astnode* expr, identifier_astnode* id);
        virtual void print(int blanks) override;
    protected:
};

//END ref_astnode children