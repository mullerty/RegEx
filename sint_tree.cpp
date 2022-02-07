#include "sint_tree.h"
using namespace std;
namespace Regex
{
	string getRepStr(const string st, int f, int s) {
		string rep, re;
		re = st;
		int k = re.size();
		if (re[k - 1] == ')') {
			int i = k - 1;
			int p = 0;
			while (re[i] != '(') --i;
			while (i < re.size()) {
				rep += re[i];
				++i;
				++p;
			}
			while (p > 0) {
				re.pop_back();
				--p;
			}
		}
		else  if (re[k - 1] != '+' && re[k - 1] != '?' && re[k - 1] != '|' && re[k - 1] != '-') {
			rep = re[k - 1];
			re.erase(k - 1);
		}	
		else throw "Wrong symbol to repeat!";
		if (f == 0 && s == 0)
		{
			// S*
			re += rep+'+';
			string br = "()";
			br.insert(1, re);
			br += '?';
			re = br;
		}
		else if (f > 0 && s == 0)
		{
			//S..S+
			while (f > 0) {
				re += rep;
				--f;
			}
				
			re += '+';
		}
		else if (f > 0 && s > 0)
		{
			//S..SS?S?
			while (f > 0) {
				re += rep;
				--f;
				--s;
			}
			while (s > 0) {
				re += rep;
				re += '?';
				--s;
			}
		}
		return re;
	}
	bool  Sint_tree::parse_re(const string& re) {
		string n_re;
		int k = 0;
		for (auto ind = re.begin(); ind != re.end(); ind++)
		{
			unique_ptr<Node> tmp = nullptr;
			if (*ind == '{')
			{
				tmp = Repeat().create(ind);
				int f = tmp->getRange().first, s = tmp->getRange().second;
				if (k > 0)
					n_re = getRepStr(n_re, f, s);
				else
					throw "ERROR! Wrong repeat placement!";
			}
			else n_re += *ind;
			++k;
		}
		nodes.emplace_back(make_shared<Op_br>());
		for (string::const_iterator i = n_re.cbegin(); i != n_re.cend(); i++) {
			unique_ptr<Node> tmp = nullptr;
			int nodeTemplateNumber = -1;
			switch (*i)
			{
			case'&': {
				tmp = Screen_elem().create(i);
				break;
			}
			case '(': {

				tmp = NamedGroup("", n_re.cend()).create(i);
				if (tmp == nullptr)
					tmp = Op_br().create(i);
				else
					nodes.emplace_back(make_shared<Op_br>());
				break; }
			case ')': 
				tmp = Cl_br().create(i);
				break;
			case '+':
				tmp = Pos_close().create(i);
				break;
			case '?':
				tmp = Optional().create(i);
				break;
			case '{':
				tmp = Repeat().create(i);
				break;
			case '.': {
				tmp = Any().create(i);
				break; }
			case '<':
				tmp = NamedGroup_ptr("", n_re.cend()).create(i);
				break;
			case '|':
				tmp = Or().create(i);
				break;
			case '-':
				tmp = Concat().create(i);
				break;
			default: {
				tmp = Leaf().create(i);
				break; }
			}
			if (tmp == nullptr) tmp = Leaf().create(i);
			nodes.emplace_back(std::move(tmp));
		}
		nodes.emplace_back(make_shared<Cl_br>());
		return true;
	}

	void Sint_tree::create_tree(bool inverse) {

		stack<int> openBrakets;
		int i = 0;
		while ( i < nodes.size()) {
			string s = nodes[i]->getSymbol();

			if (s == "&") {
				(*nodes[i]).setChild(nodes, i);
			}

			if (s == "(")
				openBrakets.push(i);
			else if (s == ")") {
				int br = openBrakets.top();
				openBrakets.pop();
				int bef = nodes.size();
				child_setter(br, i, inverse);
				i -= bef - nodes.size();
			}
			++i;
		}
		if (!openBrakets.empty()) throw '(';
		if (nodes.size() > 1) throw ')';
		root = nodes[0];
		nodes.clear();
	}

	void Sint_tree::child_setter(int op_br, int cl_br, bool inverse) {

		for (int p = 4; p > 1; p--) {
			for (int i = op_br; i < cl_br; i++) {
				
				if ((*nodes[i]).getPriority() == p)
				{
					int bef = nodes.size();
					(*nodes[i]).setChild(nodes, i);
					cl_br -= bef - nodes.size();
				}
					
			}
		}
		int i = op_br+1;
		while ( i < cl_br) {
			if (i+1 < cl_br && ((((*nodes[i]).getPriority() != 1 && (*nodes[i + 1]).getPriority() != 1)) || ((*nodes[i]).getPriority() == 1 && (*nodes[i]).complited() && ((*nodes[i + 1]).getPriority() == 1 && (*nodes[i + 1]).complited())) || ((*nodes[i]).getPriority() == 1 && (*nodes[i]).complited() && (*nodes[i + 1]).getPriority() != 1))){
				vector<shared_ptr<Node>>::iterator iter = nodes.begin();
				while (*iter != nodes[i+1]) ++iter;
				nodes.emplace(iter, std::make_shared<Concat>());
				(*nodes[i + 1]).setChild(nodes, i + 1, inverse);
				--cl_br;
				--i;
			}
			++i;
		}

		for (int i = op_br; i < cl_br; i++) {

			if ((*nodes[i]).getPriority() == 1)
			{
				int bef = nodes.size();
				(*nodes[i]).setChild(nodes, i);
				cl_br -= bef - nodes.size();
			}
		}

		vector<shared_ptr<Node>>::iterator iter = nodes.begin();
		while (*iter != nodes[op_br]) ++iter;
		nodes.erase(iter);
		--cl_br;
		vector<shared_ptr<Node>>::iterator ipter = nodes.begin();
		while (*ipter != nodes[cl_br]) ++ipter;
		nodes.erase(ipter);
	}
}