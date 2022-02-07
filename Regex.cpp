
#include "Automata.cpp"
#include "sint_tree.cpp"
#include "sint_tree.h"
#include "Automata.h"

namespace Regex
{
	class Re {
	private:
		string reg;
		DFA autom;
		bool compiled;

		void compile(string r) {
			if (!compiled)
			{
				autom = DFA(NFA(Sint_tree(r)));
				compiled = true;
			}
		}
	public:
		Re(string r) : reg(r), compiled(false) {
			compile(r);
		}
		Re(DFA a) : autom(a), compiled(true) {}
		~Re() {}

		vector<string> findall(string str) {
			if (!compiled) compile(reg);
			vector<string> substrings;
			int i = 0, j, k;
			while (i < str.length())
			{
				j = i;
				while (j < str.length())
				{
					k = i;
					string substr = "";
					while (k <= j)
					{
						substr += str[k];
						++k;
					}
					if (autom.match(substr)) {
						substrings.push_back(substr);
						for (k = i; k < j + 1; k++) str.erase(k);
						j = i;
					}
					else j++;
				}
				++i;
			}
			return substrings;
		}
		string rec_expr() {
			if (!compiled) compile(reg);
			//return autom.ReFromDFA();
			return reg;
		}
		void inverse() {
			if (reg == "") reg = autom.ReFromDFA();
			autom = DFA(NFA(Sint_tree(reg, true)));
		}
		/*DFA intesections(DFA nautom) {
			if (!compiled) compile(reg);
			autom.multiply(nautom).print();
		}*/
		bool intesections(string s, string re) {
			if (!compiled) compile(reg);
			DFA nautom = DFA(NFA(Sint_tree(s)));
			DFA res = autom.multiply(nautom);
			res.print();
			return res.match(re);
		}
		
		bool match(string str) {
			if (!compiled) compile(reg);
			return autom.match(str);
		}
		void print_DFA() {
			if (!compiled) compile(reg);
			autom.print();
		}
	};
	
}