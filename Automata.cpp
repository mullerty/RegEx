#include "Automata.h"
namespace Regex
{
	// Метод НКА
	void NFA::createNFA(Sint_tree tree) {
		stack<NFA> tmpNFA;
		vector<shared_ptr<Node>> nodes = tree.getNodes();
		shared_ptr<Node> node;
		while (!nodes.empty()){
			node = nodes.back();
			string s = node->getSymbol();
			int p = node->getPriority();
			NFA autom = NFA();
			switch (p)
			{
			case 0: {
				if (s.size() > 1)
				{
					autom = tmpNFA.top();
					tmpNFA.pop();
					nodes.pop_back();
				}
				else
				{
					autom.LeafToNFA(s);
					if (!alphabet.contains(s))
						alphabet.insert(s);
				}	
				break;
			}
			case 1: {
				NFA left, right = tmpNFA.top();
				tmpNFA.pop();
				left = tmpNFA.top();
				tmpNFA.pop();
				autom.OrToNFA(left, right);
				break;
			}
			case 2: {
				NFA left, right = tmpNFA.top();
				tmpNFA.pop();
				left = tmpNFA.top();
				tmpNFA.pop();
				autom.ConToNFA(left, right);
				break; }
			case 3: {
				if (s == "."){
					autom.AnyToNFA();
				}
				else if (s == "+"){
					autom.PosClToNFA(tmpNFA.top());
					tmpNFA.pop();
				}
				else if (s == "?") {
					autom.OptToNFA(tmpNFA.top());
					tmpNFA.pop();
				}
				else if (s[0] == '{') {
					autom.RepToNFA(tmpNFA.top(), node->getRange());
					tmpNFA.pop();
				}
				break;}
			default: {
				autom = tmpNFA.top();
				tmpNFA.pop();
				break;}
			}
			tmpNFA.push(autom);
			nodes.pop_back();
		}
		this->start = tmpNFA.top().start;
		tmpNFA.pop();
	};

	// Методы ДКА
	void DFA::createDFA(NFA nfautom) {
		alphabet = nfautom.getAlphabet();
		shared_ptr<State> start = nfautom.getStart();
		vector<shared_ptr<State>> nums;
		vector<pair<shared_ptr<State>, vector<shared_ptr<State>>>> tmp_states;
		// Эпсилон замыкание стартового состояния
		epsClosure(start, nums);
		SNnuller(nums);
		shared_ptr<State> st = make_shared<State>(st_num, isFinState(nums));
		++st_num;
		tmp_states.push_back({ st, nums });
		// Построение состояний и связей в ДКА
		int i = 0;
		while (i < tmp_states.size()) {
			for (auto j = alphabet.begin(); j != alphabet.end(); j++)
			{
				nums.clear();
				symbClosure(*j, tmp_states[i].second, nums);
				auto k = tmp_states.begin();
				bool flag = true;
				if (!nums.empty())
					while (k != tmp_states.end() && flag) {
						flag = !vec_eq(nums, (*k).second);
						st = (*k).first;
						k++;
					}
				else {
					while (k != tmp_states.end() && flag) {
						flag = !(*k).second.empty();
						st = (*k).first;
						k++;
					}
					//if (!flag) tmp_states[i].first->addTrans({ *j, (*k).first });
				}
				if (flag) {
					st = make_shared<State>(st_num, isFinState(nums)); // Проверка на принимающее состояние
					++st_num;
					tmp_states.push_back({ st, nums });
				}
				tmp_states[i].first->addTrans({ *j, st });
			}
			++i;
		}
		for (auto i = tmp_states.begin(); i != tmp_states.end(); i++)
			states.push_back((*i).first);
	};

	/*void DFA::minDFA() {
		if (!min) {
			shared_ptr<State> start = states[0];
			vector<shared_ptr<State>> nums, ngroup;
			vector<pair<shared_ptr<State>, vector<shared_ptr<State>>>> groups, ngroups;
			// Начальное разбиение на F и S-F
			st_num = 0;
			shared_ptr<State> st = make_shared<State>(st_num);
			++st_num;
			groups.push_back({ st, {} });
			st = make_shared<State>(st_num);
			++st_num;
			groups.push_back({ st, {} });
			for (auto i = states.begin(); i != states.end(); i++)
				if ((*i)->getFin())
					groups[1].second.push_back(*i);
				else
					groups[0].second.push_back(*i);

			// Коррекция разбиений, пока новое не будет равно предыдущему
			while (!vec_eq(groups, ngroups)) {
				
				for (auto i = ngroups.begin(); i != ngroups.end(); i++) (*i).first->resetTrans({});
				
				for (auto i = groups.begin(); i != groups.end(); i++)
					for (auto j = alphabet.begin(); j != alphabet.end(); j++)
					{
						//Проверяем если все состояние группы приходят в эту же группу
						bool flag = true;
						for (auto k = (*i).second.begin(); k != (*i).second.end(); k++)
							symbClosure(*j, *k, nums);
						for (auto k = nums.begin(); k != nums.end(); k++)
							if (!vec_contains((*i).second, *k)) flag = false;
						if (flag)
						{
							ngroups.push_back(*i);
							break;
						}

						
						if (flag) {
							st = make_shared<State>(st_num);
							++st_num;
							.push_back({ st, nums });
						}
						tmp_states[i].first->addTrans({ *j, st });
					}
			}
			for (auto i = ngroups.begin(); i != ngroups.end(); i++)
				states.push_back((*i).first);
		}
	};*/

	void DFA::minDFA() {
		vector<tuple<shared_ptr<State>, int, int, map<string, int>>> oldStates;
		int groupsTotal = 1;
		if (states.size() != 1 || !min) {
			bool flag = false;
			// Начальное разбиение на принимающие состояния и все остальные
			bool fin = states[0]->getFin();
			for (auto i : states)
				if (i->getFin() != fin)
				{
					oldStates.push_back({ i, 0, 1, {} });
					flag = true;
				}
				else
					oldStates.push_back({ i, 0, 0, {} });
					
			if (flag)
			{
				++groupsTotal;
				// Корректируем разбиения пока предыдущее разбиение не будет равно текущему
				bool changed = false;
				do {
					changed = false;
					for (auto symbol : alphabet) {
						for (auto& state : oldStates) {
							shared_ptr<State> to;
							for (auto st : get<0>(state)->getTrans()) {
								if (st.first == symbol)
									to = st.second;
							}
							for (auto i : oldStates) {
								if (get<0>(i).get() == to.get()) {
									get<3>(state)[symbol] = get<2>(i);
									break;
								}
							}

						}
						bool addedGroup = false;
						for (int group = 0; group < groupsTotal && !addedGroup; group++) {
							auto i = oldStates.begin();
							for (; i < oldStates.end() && get<2>(*i) != group; i++) {}
							int destination = get<3>(*i)[symbol];
							for (; i < oldStates.end(); i++) {
								if (get<2>(*i) == group && get<3>(*i)[symbol] != destination) {
									get<1>(*i) = get<2>(*i);
									get<2>(*i) = groupsTotal;
									addedGroup = true;
									changed = true;
								}
							}
						}
						if (addedGroup) groupsTotal++;
					}
				} while (changed);
				// Заменяем группы представителями и назначаем принимающее состояние
				states.clear();
				for (int g = 0; g < groupsTotal; g++)
					states.push_back(make_shared<State>());
				for (int g = 0; g < groupsTotal; g++) {
					for (auto& i : oldStates) {
						if (get<2>(i) == g) {
							states[g]->setFin(get<0>(i)->getFin());
							states[g]->setNum(g);
							for (auto symbol : alphabet) {
								states[g]->addTrans({ symbol, states[get<3>(i)[symbol]] });
							}
							break;
						}
					}
				}
			}
			else {
				// Заменяем группы представителями и назначаем принимающее состояние
				states.clear();
				states.push_back(make_shared<State>());
					for (auto& i : oldStates) {
						if (get<2>(i) == 0) {
							states[0]->setFin(get<0>(i)->getFin());
							states[0]->setNum(0);
							for (auto symbol : alphabet) {
								states[0]->addTrans({ symbol, states[0] });
							}
							break;
						}
					}
			}
			
			min = true;
		}
	}

	bool DFA::match(const string s) {
		shared_ptr<State> state = states[0];
		string c;
		for (int i = 0; i != s.size(); i++)
		{
			c = s[i];
			if (alphabet.find(c) == alphabet.end())
				return false;
			for (auto st : state->getTrans()) {
				if (st.first == c)
					state = st.second;
			}
		}
		return state->getFin();
	}

	DFA DFA::multiply(const DFA& autom) {
		auto fSize = autom.states.size(), sSize = states.size();
		auto fBegin = autom.states.begin();
		auto sBegin = states.begin();
		vector<shared_ptr<State>> res_states(fSize * sSize);
		for (int i = 0; i < res_states.size(); i++)
			res_states[i] = make_shared<State>(i);
		unordered_set<string> alphabet = this->alphabet;
		alphabet.insert(make_move_iterator(autom.alphabet.begin()), make_move_iterator(autom.alphabet.end()));
		
		for (auto& i : autom.states)
			for (auto& j : states) {
				int index = i->getNum() * sSize + j->getNum();
				if (i->getFin() && j->getFin()) res_states[index]->setFin(true);
				for (auto& iTrans : i->getTrans()) {
					for (auto& jTrans : j->getTrans())
						if (iTrans.first == jTrans.first)
							res_states[index]->addTrans({ iTrans.first, res_states[iTrans.second->getNum() * sSize + jTrans.second->getNum()] });
				}
			}

		DFA res = DFA();
		res.alphabet = alphabet;
		res.states = move(res_states);
		res.minDFA();
		return res;
	}

	string DFA::ReFromDFA() {
		// Исключение не стартовых и принимающих состояний с переопределением переходов
		// Обработка стартового и принимающих состояний
		return "";
	};

	void DFA::print() {
		for (auto i : states) {
			cout << "{ " << (i->getFin() ? "(" : "") << i->getNum() << (i->getFin() ? ")" : "") << " }" << endl << "transitions:  ";
			for (auto j : i->getTrans())
				cout << endl << " ---( " << j.first << " " << ")--->  { " << (j.second->getFin() ? "(" : "") << j.second->getNum() << (j.second->getFin() ? ")" : "") << " }" << endl;
			cout << endl;
		}
	}
}