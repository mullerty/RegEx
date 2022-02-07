#pragma once
#include"Nodes.h"
using namespace std;
namespace Regex
{
	class Sint_tree
	{
	private:
		shared_ptr<Node> root;
		vector<shared_ptr<Node>> nodes;
		void child_setter(int beg, int end, bool inverse);
		void rec_print(int deep, shared_ptr<Node> ptr) const {
			if (ptr->get_right() != nullptr)
				rec_print(deep + 1, ptr->get_right());
			for (int i = 0; i < deep * 5; i++)
				cout << " ";
			cout << ptr->getSymbol() << std::endl;
			if (ptr->get_left() != nullptr)
				rec_print(deep + 1, ptr->get_left());
		}

		void rec_nodes(shared_ptr<Node> ptr) {
			//if ((ptr->getSymbol()[0] == '{') && (ptr->getSymbol().size() > 1))
				//repeatNodes(ptr);
			//else
			    this->nodes.push_back(ptr);
			if (ptr->get_right() != nullptr)
				rec_nodes(ptr->get_right());
			if (ptr->get_left() != nullptr)
				rec_nodes(ptr->get_left());
		}
	public:
		
		Sint_tree(const string& re, bool inv = false) {
			parse_re(re);
 			create_tree(inv);
			print_tree();
			nodes.clear();
		};
		~Sint_tree(){};

		bool parse_re(const string& re);
		void create_tree(bool inv = false);
		void print_tree() const {
			int deep = 0;
			cout << "_________________________________________\n";
			shared_ptr<Node> ptr = root;
			rec_print(0, ptr);

		}
		
		vector<shared_ptr<Node>> getNodes() {
			rec_nodes(root);
			return this->nodes;
		}
	};
}