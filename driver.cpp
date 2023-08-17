#include "scanner.hh"
#include "parser.tab.hh"
#include "ast.hpp"
#include <fstream>
using namespace std;

extern std::vector<std::string> Gstrings;
GlobalSymTab gst, gstfun, gststruct; 
string filename;
extern std::map<string,abstract_astnode*> ast;
std::string CurrFun;
LsymTab *CurrSymTab;
std::map<std::string, std::string> predefined {
            {"printf", "void"},
            {"scanf", "void"},
            {"mod", "int"}
        };
int main(int argc, char **argv)
{
	using namespace std;
	fstream in_file, out_file;
	

	in_file.open(argv[1], ios::in);

	IPL::Scanner scanner(in_file);

	IPL::Parser parser(scanner);

#ifdef YYDEBUG
	parser.set_debug_level(1);
#endif
parser.parse();
// create gstfun with function entries only

for (const auto &entry : gst.entries)
{
	if (entry.second.type == Entry::EntryType::Func)
	gstfun.entries.insert({entry.first, entry.second});
}
// create gststruct with struct entries only

for (const auto &entry : gst.entries)
{
	if (entry.second.type == Entry::EntryType::Struct)
	gststruct.entries.insert({entry.first, entry.second});
}
// start the JSON printing

std::cout<<"\t.text\n\t.section .rodata"<<endl;

for(int label = 0; label<Gstrings.size(); label++){
	std::cout<<".LC"<<label<<":"<<endl;
	std::cout<<"\t.string "<<Gstrings[label]<<endl;
}

for (auto it = gstfun.entries.begin(); it != gstfun.entries.end(); ++it)

{
	std::cout<<"\t.text"<<endl;
	int width = 0;
	CurrSymTab = it->second.symtab;
	CurrFun = it->first;
	for(auto elem = CurrSymTab->elems.begin();elem!=CurrSymTab->elems.end();elem++){
		if(elem->second.scope == "local"){
			width+=elem->second.width;
		}
	}
	cout<<"\t.globl\t"<<it->first<<endl;
	cout<<"\t.type\t"<<it->first<<", @function"<<endl;
	cout << it->first << ":" << endl;
	cout<<"\tpushl\t%ebp\n\tmovl\t%esp, %ebp\n";
	if(width>0)
		cout<<"\tsubl\t$"<<width<<", %esp"<<endl;
	it->second.symtab->print();
	ast[it->first]->print(0);
	cout<<"\tleave\n\tret"<<endl;
	cout<<"\t.size\t"<<it->first<<", .-"<<it->first<<endl;
}
	fclose(stdout);
}



