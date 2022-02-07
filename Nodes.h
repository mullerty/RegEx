#pragma once
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <stack>

using namespace std;
namespace Regex
{

	class Node
	{
	private:
		shared_ptr<Node> parent;
	public:
		Node() : parent() {}
		virtual ~Node() {}
		virtual unique_ptr<Node> create(string::const_iterator& i) = 0;
		virtual void setChild(vector<shared_ptr<Node>>&, int i, bool inverse = false) = 0;
		virtual bool complited() = 0;
		void setParent(shared_ptr<Node> iter) { parent = iter; }
		virtual shared_ptr<Node>  get_left() { return nullptr; }
		virtual shared_ptr<Node> get_right() { return nullptr; }
		virtual int getPriority() = 0;
		virtual string getSymbol() = 0;
		virtual pair<int, int> getRange() = 0;
		shared_ptr<Node> getParent() { return parent; }
	};

	class Leaf : public Node
	{
	private:
		const string symb;
	public:
		Leaf(const string symb = "") :Node(), symb(symb) {}
		virtual ~Leaf()override {}
		virtual unique_ptr<Node> create(std::string::const_iterator& iter) override {
			string s;
			s += *iter;
			return make_unique<Leaf>(s);
		}

		virtual void setChild(vector<shared_ptr<Node>>&, int i, bool inverse = false) { return; };

		virtual bool complited() override {
			return true;
		}
		virtual int getPriority() override {
			return 0;
		}
		virtual string getSymbol() override {
			return symb;
		}
		virtual pair<int, int> getRange() { return {}; }

	};

	class Meta :public Node
	{

	protected:
		std::shared_ptr<Node> left_;
		std::shared_ptr<Node> right_;
	public:
		Meta() :Node() {
			left_ = nullptr;
			right_ = nullptr;
		};
		virtual ~Meta()override {};
		virtual std::shared_ptr<Node> get_left() override { return left_; }
		virtual std::shared_ptr<Node> get_right() override { return right_; }
	};

	class Any : public Meta
	{
	public:
		Any() :Meta() {};
		virtual unique_ptr<Node> create(string::const_iterator& iter) override {
			if (*iter++ == '.') {
				--iter;
				return  std::make_unique<Any>();
			}
			else {
				--iter;
				return nullptr;
			}
		}

		virtual std::string getSymbol() override {
			return ".";
		}
		virtual int getPriority() override {
			return 3;
		}
		virtual void setChild(vector<shared_ptr<Node>>&, int i, bool inverse = false) { return; };
		virtual bool complited() override {
			return true;
		}
		virtual pair<int, int> getRange() { return {}; }
	};

	class Op_br : public Meta
	{
	public:
		Op_br() : Meta() {};
		virtual unique_ptr<Node> create(string::const_iterator& iter) override {
			if (*iter++ == '(') {
				--iter;
				return  make_unique<Op_br>();
			}
			else {
				--iter;
				return nullptr;
			}
		}
		virtual string getSymbol() override {
			return "(";
		}
		virtual void setChild(vector<shared_ptr<Node>>&, int i, bool inverse = false) { return; };
		virtual int getPriority() override {
			return 0;
		}
		virtual pair<int, int> getRange() { return {}; }
		virtual bool complited() override {
			return false;
		}
	};

	class Cl_br :public  Meta
	{
	public:
		Cl_br() : Meta() {};
		virtual unique_ptr<Node> create(string::const_iterator& iter) override {
			if (*iter++ == ')') {
				--iter;
				return  make_unique<Cl_br>();
			}
			else {
				--iter;
				return nullptr;
			}
		}
		virtual string getSymbol() override {
			return ")";
		}
		virtual void setChild(vector<shared_ptr<Node>>&, int i, bool inverse = false) { return; };
		virtual int getPriority() override {
			return 0;
		}
		virtual pair<int, int> getRange() { return {}; }
		virtual bool complited() override {
			return false;
		}
	};

	class Screen_elem :public Meta
	{
	public:
		Screen_elem() :Meta() {};
		virtual std::unique_ptr<Node> create(std::string::const_iterator& iter) override {
			std::string tmp;
			tmp += *iter++;

			if (tmp == "&") {
				--iter;
				return  make_unique<Screen_elem>();
			}
			else {
				iter -= 1;
				return nullptr;
			}
		}
		virtual std::string getSymbol() override {
			return "&";
		}
		virtual int getPriority() override {
			return 5;
		}
		virtual void setChild(vector<shared_ptr<Node>>& ptr, int i, bool inverse = false) override {
			if (!complited()) {

				++i;
				right_ = make_shared<Leaf>((ptr[i])->getSymbol());
				--i;
				right_->setParent(ptr[i]);

				vector<shared_ptr<Node>>::iterator iter = ptr.begin();
				while (*iter != ptr[i + 1]) ++iter;
				ptr.erase(iter);

			}
		}
		virtual bool complited() override {
			return right_ != nullptr;
		}
		virtual pair<int, int> getRange() { return {}; }
	};

	class NamedGroup_ptr : public Meta
	{
	private:
		string::const_iterator eos_;
		string nameOfGroup_;
		static int links;
	public:
		NamedGroup_ptr(string name = "", const string::const_iterator end = {}) :Meta(), eos_(end), nameOfGroup_(name) {};
		~NamedGroup_ptr() {};
		unique_ptr<Node> create(string::const_iterator& iter) override {
			string nameOfGroup_ = "";
			int count = 0;
			if (*iter == '<') {
				++iter;
				++count;
				while (iter != eos_ && (isalpha(*iter) || *iter == '>'))
				{
					if (*iter == '>' && count > 0) {

						return make_unique<NamedGroup_ptr>(nameOfGroup_);
					}
					nameOfGroup_ += *iter;
					count++;
					iter++;
				}
			}
			iter -= count;
			return nullptr;
		}
		virtual void setChild(vector<shared_ptr<Node>>&, int i, bool inverse = false) { return; };
		virtual string getSymbol() override {
			return "<" + nameOfGroup_ + ">";
		}
		virtual int getPriority() override {
			return 0;
		}
		virtual bool complited() override {
			return true;
		}
		virtual pair<int, int> getRange() { return {}; }
	};

	class NamedGroup :public Meta
	{
	private:
		const string::const_iterator eos_;
		string nameOfGroup_;
		map<string, string> group;
	public:
		NamedGroup(string name = "", const string::const_iterator end = {}) :Meta(), eos_(end), nameOfGroup_(name) {
			if (group.contains(name) && name != "") {
				throw "repeated usage name of group";
			}
			else if (name != "") {
				group[name] = "";
			}
		};
		~NamedGroup() {}
		virtual int getPriority() override {
			return 4;
		}
		virtual unique_ptr<Node> create(string::const_iterator& iter) override {
			string nameOfGroup_ = "";
			string tmp;
			bool correct = false;
			int count = 0;
			for (int i = 0; i < 2; i++) {
				if (iter != eos_) {
					count++;
					tmp += *iter++;
				}
				else break;
			}
			if (tmp == "(<") {
				while (iter != eos_ && !correct)
				{

					if (*iter == '>' && count > 2) {
						count = 0;
						while (iter != eos_ && !correct)
						{
							if (*iter == ')' && count > 0 && *(iter - 1) != '&')
								correct = true;
							else if (*iter == ')')
								return nullptr;
							else
							{
								++iter;
								++count;
							}
						}
					}
					else if (isalpha(*iter))
					{
						nameOfGroup_ += *iter;
						++iter;
						++count;
					}
					else break;
				}
			}
			iter -= count;
			return correct ? std::make_unique<NamedGroup>(nameOfGroup_) : nullptr;
		}
		virtual std::string getSymbol() override {
			return "(<" + nameOfGroup_ + ">";
		}
		virtual void setChild(vector<shared_ptr<Node>>& ptr, int i, bool inverse = false) override {
			if (!complited())
				if ((*ptr[i + 1]).complited()) {
					right_ = ptr[i + 1];
					right_->setParent(ptr[i]);

					vector<shared_ptr<Node>>::iterator iter = ptr.begin();
					while (*iter != ptr[i + 1]) ++iter;
					ptr.erase(iter);
				}
				else
					throw "Incorrect usage of operation. Some of operations have less operands then they should.";
		}
		virtual bool complited() override {
			return right_ != nullptr;
		}
		virtual pair<int, int> getRange() { return {}; }
	};

	class Optional : public Meta
	{
	public:
		Optional() :Meta() {};

		virtual unique_ptr<Node> create(string::const_iterator& iter) override {
			if (*iter++ == '?') {
				--iter;
				return  make_unique<Optional>();
			}
			else {
				--iter;
				return nullptr;
			}
		}

		virtual string getSymbol() override {
			return "?";
		}

		virtual int getPriority() override {
			return 3;
		}

		virtual void setChild(vector<shared_ptr<Node>>& ptr, int i, bool inverse = false) override {
			if (!complited())
				if ((*ptr[i - 1]).complited()) {
					left_ = ptr[i - 1];
					left_->setParent(ptr[i]);

					vector<shared_ptr<Node>>::iterator iter = ptr.begin();
					while (*iter != ptr[i - 1]) ++iter;
					ptr.erase(iter);
				}
				else
					throw "Incorrect usage of operation. Some of operations have less operands then they should.";
		}
		virtual pair<int, int> getRange() { return {}; }
		bool complited() override {
			return left_ != nullptr;
		}

	};
	
	class Repeat : public Meta
	{
	private:
		pair<int, int> range;
	public:
		Repeat(pair<int, int> range = {}) :Meta(), range(range) {};
		virtual ~Repeat() override {}
		virtual unique_ptr<Node> create(string::const_iterator& iter) override {
			int count = 0;
			std::string first, second;
			if (*iter == '{') {
				++iter;
				++count;
				while (std::isdigit(*iter))
				{
					first += *iter++;

					count++;
				}
				if (*iter == ',') {
					++iter;
					++count;
					while (std::isdigit(*iter))
					{
						second += *iter++;
						count++;
					}
					if (*iter == '}') {
						std::pair<int, int> dia;
						if (first.empty())
							dia.first = 0;
						else
							dia.first = std::stoi(first);
						if (second.empty())
							dia.second = 0;
						else
						{
							dia.second = std::stoi(second);
							if (dia.second == 0)
								throw "Wrong value of repeat diapason.";
						}
						if ((dia.first < 0) | (dia.second < 0))
							throw "Wrong value of repeat diapason.";
						if (dia.first && dia.second && dia.first > dia.second)
							throw "left value of repeat diapason greater than right value.";

						return std::make_unique<Repeat>(dia);

					}
				}
			}
			iter -= count;
			return nullptr;
		}
		virtual string getSymbol() override {
			string s;
			s += "{" + std::to_string(range.first) + ", ";
			if (range.second != 0) s += std::to_string(range.second);
			s += "}";
			return s;
		}
		virtual int getPriority() override {
			return 3;
		}
		virtual void setChild(vector<shared_ptr<Node>>& ptr, int i, bool inverse = false) override {
			if (!complited())
				if ((*ptr[i - 1]).complited()) {
					left_ = ptr[i - 1];
					left_->setParent(ptr[i]);

					vector<shared_ptr<Node>>::iterator iter = ptr.begin();
					while (*iter != ptr[i - 1]) ++iter;
					ptr.erase(iter);
				}
				else
					throw "Incorrect usage of operation. Some of operations have less operands then they should.";
		}
		virtual bool complited() override {
			return left_ != nullptr;
		}
		virtual pair<int, int> getRange() { return range; }
	};

	class Pos_close : public Meta
	{
	public:
		Pos_close() :Meta() {};
		virtual unique_ptr<Node> create(string::const_iterator& iter) override {
			if (*iter++ == '+') {
				--iter;
				return  make_unique<Pos_close>();
			}
			else {
				--iter;
				return nullptr;
			}
		}
		virtual string getSymbol() override {
			return "+";
		}
		virtual int getPriority() override {
			return 3;
		}
		virtual void setChild(vector<shared_ptr<Node>>& ptr, int i, bool inverse = false) override {
			if (!complited())
				if ((*ptr[i - 1]).complited()) {
					left_ = ptr[i - 1];
					left_->setParent(ptr[i]);

					vector<shared_ptr<Node>>::iterator iter = ptr.begin();
					while (*iter != ptr[i - 1]) ++iter;
					ptr.erase(iter);
				}
				else
					throw "Incorrect usage of operation. Some of operations have less operands then they should.";
		}
		virtual bool complited() override {
			return left_ != nullptr;
		}
		virtual pair<int, int> getRange() { return {}; }
	};

	class Or :public Meta
	{
	public:
		Or() :Meta() {}
		virtual unique_ptr<Node> create(string::const_iterator& iter) override {
			if (*iter++ == '|') {
				--iter;
				return  make_unique<Or>();
			}
			else {
				--iter;
				return nullptr;
			}
		}
		virtual string getSymbol() override {
			return "|";
		}
		virtual int getPriority() override {
			return 1;
		}
		virtual bool complited() override {
			return left_ != nullptr && right_ != nullptr;
		}
		virtual pair<int, int> getRange() { return {}; }
		virtual void setChild(vector<shared_ptr<Node>>& ptr, int i, bool inverse = false) override {
			if (!complited())
			{
				shared_ptr<Node> leftNode = ptr[i - 1];
				shared_ptr<Node> rightNode = ptr[i + 1];
				if (leftNode->complited() && rightNode->complited()) {
					right_ = !inverse ? rightNode : leftNode;
					right_->setParent(ptr[i]);
					left_ = !inverse ? leftNode : rightNode;
					left_->setParent(ptr[i]);

					vector<shared_ptr<Node>>::iterator iter = ptr.begin();
					while (*iter != rightNode) ++iter;
					ptr.erase(iter++);
					vector<shared_ptr<Node>>::iterator ipter = ptr.begin();
					while (*ipter != leftNode) ++ipter;
					ptr.erase(ipter);
				}
				else
					throw "Incorrect usage of operation. Some of oherations have less operands then they should.";
			}
		}
	};

	class Concat :public Meta
	{
	public:
		Concat() :Meta() {};
		virtual int getPriority() override {
			return 2;
		}
		virtual std::unique_ptr<Node> create(string::const_iterator& iter) override {
			if (*iter++ == '-') {
				--iter;
				return  make_unique<Concat>();
			}
			else {
				--iter;
				return nullptr;
			}
		}
		virtual std::string getSymbol() override {
			return "-";
		}
		virtual bool complited() override {
			return left_ != nullptr && right_ != nullptr;
		}
		virtual pair<int, int> getRange() { return {}; };
		virtual void setChild(vector<shared_ptr<Node>>& ptr, int i, bool inverse = false) override {
			if (!complited())
			{
				shared_ptr<Node> leftNode = ptr[i - 1];
				shared_ptr<Node> rightNode = ptr[i + 1];
				if (leftNode->complited() && rightNode->complited()) {
					right_ = !inverse ? rightNode : leftNode;
					right_->setParent(ptr[i]);
					left_ = !inverse ? leftNode : rightNode;
					left_->setParent(ptr[i]);

					vector<shared_ptr<Node>>::iterator iter = ptr.begin();
					while (*iter != rightNode) ++iter;
					ptr.erase(iter);
					vector<shared_ptr<Node>>::iterator ipter = ptr.begin();
					while (*ipter != leftNode) ++ipter;
					ptr.erase(ipter);
				}
				else
					throw "Incorrect usage of operation. Some of oherations have less operands then they should.";
			}
		}
	};

}