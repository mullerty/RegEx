#pragma once
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <stack>
#include<unordered_set>
#include "sint_tree.h"
using namespace std;
namespace Regex
{

	template <class T>
	bool vec_contains(vector<T> vec, T el) {
			for (auto i = vec.begin(); i != vec.end(); i++)
				if (*i == el)
					return true;
			return false;
		}
	template <class T>
	bool vec_eq(vector<T> vec1, vector<T> vec2) {
		if (vec1.size() != vec2.size()) return false;
		for (auto i = vec1.begin(); i != vec1.end(); i++)
			if (!vec_contains(vec2, *i))
				return false;
		return true;
	}
	template <class T>
	bool vecofvec_eq(vector<vector<T>> vec1, vector<vector<T>> vec2) {
		if (vec1.size() != vec2.size()) return false;
		for (auto i = vec1.begin(); i != vec1.end(); i++)
			for (auto j = vec2.begin(); j != vec2.end(); j++)
				if (!vec_eq(*j, *i))
					return false;
		return true;
	}

	class State {
	private:
		bool fin;
		int num;
		vector<pair<string, shared_ptr<State>>> transition;
	public:
		State(bool fin = false) : num(0), fin(fin) {}
		State(int num, bool fin = false): num(num), fin(fin) {}
		~State() {}
		void setFin(bool f) { fin = f; }
		void setNum(int n) { num = n; }
		int getNum() { return num; }
		void resetTrans(vector<pair<string, shared_ptr<State>>> trans) { transition = trans; }
		void addTrans(pair<string, shared_ptr<State>> el) { transition.push_back(el); }
		vector<pair<string, shared_ptr<State>>> getTrans() { return transition; }
		bool getFin() { return(fin); }
	};

	class NFA {
	private:
		shared_ptr<State> start;
		unordered_set<string> alphabet;
		void LeafToNFA(string s) {
			shared_ptr<State> fin = make_shared<State>(true);
			shared_ptr<State> start = make_shared<State>();
			pair<string, shared_ptr<State>> elem;
			elem.first = s;
			elem.second = fin;
			vector<pair<string, shared_ptr<State>>> trans;
			trans.push_back(elem);
			start->resetTrans(trans);
			this->start = start;
		}
		void AnyToNFA() {
			shared_ptr<State> fin = make_shared<State>(true);
			shared_ptr<State> start = make_shared<State>();
			pair<string, shared_ptr<State>> elem;
			elem.first = "Any";
			elem.second = fin;
			vector<pair<string, shared_ptr<State>>> trans;
			trans.push_back(elem);
			start->resetTrans(trans);
			this->start = start;
		}
		void OptToNFA(NFA left){
			shared_ptr<State> fin = make_shared<State>(true);
			shared_ptr<State> start = make_shared<State>();
			//start->setNum(++st_amount);
			//fin->setNum(++st_amount);
			pair<string, shared_ptr<State>> elem;
			vector<pair<string, shared_ptr<State>>> trans;
			shared_ptr<State> st;
			elem.first = "eps";
			elem.second = fin;
			trans.push_back(elem);
			st = left.getStart();
			elem.second = st;
			trans.push_back(elem);
			start->resetTrans(trans);
			while (!st->getFin())
				st = st->getTrans()[0].second;
			st->setFin(false);
			trans.clear();
			elem.second = fin;
			trans.push_back(elem);
			st->resetTrans(trans);
			this->start = start;
		}
		void RepToNFA(NFA left, pair<int,int> range) {
			shared_ptr<State> fin = make_shared<State>(true);
			shared_ptr<State> start;
			pair<string, shared_ptr<State>> elem;
			vector<pair<string, shared_ptr<State>>> trans;
			shared_ptr<State> st, fn;
			st = left.getStart();
			elem.first = "eps";
			// нужно проверить границы повторений
			if (range.first == 0 && range.second == 0)
			{
				// S*
				// Создаем новое стартовое состояние
				elem.second = fin;
				trans.push_back(elem);
				elem.second = st;
				trans.push_back(elem);
				start = make_shared<State>();
				start->resetTrans(trans);
				// Создаем новые переходы для старого конечного состояния
				fn = st;
				while (!fn->getFin())
					fn = fn->getTrans()[0].second;
				fn->setFin(false);
				fn->resetTrans(trans);
			}
			else if (range.first > 0 && range.second == 0)
			{
				//S..S+
				NFA right = left;
				NFA el = left;
				/*elem.second = fin;
				trans.push_back(elem);
				elem.second = st;
				trans.push_back(elem);
				start = make_shared<State>();
				start->resetTrans(trans);
				
				fn = st;
				while (!fn->getFin())
					fn = fn->getTrans()[0].second;*/
				int n = range.first;
				while (n > 1) {
					shared_ptr<State> finn = make_shared<State>(true);
					

					--n;
				}
				PosClToNFA(right);
				start = left.getStart();
			}
			else if (range.first > 0 && range.second > 0)
			{
				//S..S
				NFA right = left;
				NFA el = left;
				shared_ptr<State> fn_l, fn_r;
				int n = range.first;
				int m = range.second-n;
				while (n > 0) {
					ConToNFA(left, el);
					--n;
				}
				while (m > 0) {
					ConToNFA(right, el);
					--m;
				}
				fn_r = right.getStart();
				while (!fn_r->getFin())
					fn_r = fn_r->getTrans()[0].second;
				elem.second = fn_r;
				trans.push_back(elem);
				elem.second = right.getStart();
				trans.push_back(elem);
				fn_l = left.getStart();
				while (!fn_l->getFin())
					fn_l = fn_l->getTrans()[0].second;
				fn_l->setFin(false);
				fn_l->resetTrans(trans);
				start = left.getStart();
			}

			this->start = start;
		}
		void PosClToNFA(NFA left){
			shared_ptr<State> fin = make_shared<State>(true);
			shared_ptr<State> start = make_shared<State>();
			//start->setNum(++st_amount);
			//fin->setNum(++st_amount);
			pair<string, shared_ptr<State>> elem;
			vector<pair<string, shared_ptr<State>>> trans;
			shared_ptr<State> st;
			elem.first = "eps";
			st = left.getStart();
			elem.second = st;
			trans.push_back(elem);
			start->resetTrans(trans);
			while (!st->getFin())
				st = st->getTrans()[0].second;
			st->setFin(false);
			trans.clear();
			elem.second = fin;
			trans.push_back(elem);
			elem.second = left.getStart();
			trans.push_back(elem);
			st->resetTrans(trans);
			this->start = start;
		}
		void OrToNFA(NFA left, NFA right){
			shared_ptr<State> fin = make_shared<State>(true);
			shared_ptr<State> start = make_shared<State>();
			//start->setNum(++st_amount);
			//fin->setNum(++st_amount);
			pair<string, shared_ptr<State>> elem;
			vector<pair<string, shared_ptr<State>>> trans;
			shared_ptr<State> st_r, fn_r, st_l, fn_l;
			elem.first = "eps";
			st_l = left.getStart();
			st_r = right.getStart();
			elem.second = st_l;
			trans.push_back(elem);
			elem.second = st_r;
			trans.push_back(elem);
			start->resetTrans(trans);
			fn_l = st_l;
			fn_r = st_r;
			while (!fn_l->getFin())
				fn_l = fn_l->getTrans()[0].second;
			while (!fn_r->getFin())
				fn_r = fn_r->getTrans()[0].second;
			fn_l->setFin(false);
			fn_r->setFin(false);
			trans.clear();
			elem.second = fin;
			trans.push_back(elem);
			fn_l->resetTrans(trans);
			fn_r->resetTrans(trans);
			this->start = start;
		}
		void ConToNFA(NFA left, NFA right){
			shared_ptr<State> st_r, st_l, fn_l;
			st_l = left.getStart();
			st_r = right.getStart();
			fn_l = st_l;
			while (!fn_l->getFin())
				fn_l = fn_l->getTrans()[0].second;
			fn_l->resetTrans(st_r->getTrans());
			fn_l->setFin(false);
			//--st_amount;
			this->start = st_l;
		}
	public:
		NFA() {}
		NFA(Sint_tree tree) : start(nullptr) {
			createNFA(tree);
		}
		~NFA() {}
		void createNFA(Sint_tree tree);
		shared_ptr<State> getStart() { return start; }
		unordered_set<string> getAlphabet() { return alphabet; }
		/*NFA& operator=(const NFA& autom) {
			shared_ptr<State> st = autom.start;
			if (st == nullptr)
				start = nullptr;
			else{
				shared_ptr<State> nst = make_shared<State>();
			}
			return *this;
		}*/
	};

	class DFA {
	private:
		vector<shared_ptr<State>> states;
		bool min = false;
		int st_num;
		unordered_set<string> alphabet;
		void minDFA();
		void createDFA(NFA nfautom);
		void SNnuller(vector<shared_ptr<State>>& tStates) {
			for (auto i: tStates) i->setNum(0);
		}
		void epsClosure(shared_ptr<State> ptr, vector<shared_ptr<State>>& tStates)
		{
			if (!vec_contains(tStates, ptr)) tStates.push_back(ptr);
			
			if ((ptr->getTrans().size() > 0) && (ptr->getNum() == 0))
			{
				ptr->setNum(1);
				if (ptr->getTrans()[0].first == "eps")
					epsClosure(ptr->getTrans()[0].second, tStates);
				if (ptr->getTrans().size() == 2)
					if (ptr->getTrans()[1].first == "eps")
						epsClosure(ptr->getTrans()[1].second, tStates);
			}
		}
		void epsClosure(vector<shared_ptr<State>> ptr, vector<shared_ptr<State>>& tStates)
		{
			for (auto i = ptr.begin(); i != ptr.end(); i++)
			{
				epsClosure(*i, tStates);
				SNnuller(tStates);
			}
				
		}
		void symbClosure(const string c, shared_ptr<State> ptr, vector<shared_ptr<State>>& tStates)
		{
			if (ptr->getTrans().size() > 0)
			{
				if (ptr->getTrans()[0].first == c || ptr->getTrans()[0].first == "Any")
					if (!vec_contains(tStates, ptr)) tStates.push_back(ptr->getTrans()[0].second);
				if (ptr->getTrans().size() == 2)
					if (ptr->getTrans()[1].first == c || ptr->getTrans()[1].first == "Any")
						if (!vec_contains(tStates, ptr)) tStates.push_back(ptr->getTrans()[1].second);
			}
		}
		void symbClosure(const string c, vector<shared_ptr<State>> ptr, vector<shared_ptr<State>>& tStates)
		{
			for (auto i = ptr.begin(); i != ptr.end(); i++)
				symbClosure(c, *i, tStates);
			epsClosure(tStates, tStates);
		}
		bool isFinState(const vector<shared_ptr<State>> ptr){
			for (auto i = ptr.begin(); i != ptr.end(); i++)
					if ((*i)->getFin())
						return true;
			return false;
		}

	public:
		DFA() : st_num(0) {}
		DFA(NFA nfautom): st_num(0) {
			createDFA(nfautom);
			minDFA();
		}
		~DFA() {}

		bool match(const string s);
		string ReFromDFA();
		DFA multiply(const DFA& autom);
		void print();
	};

};

