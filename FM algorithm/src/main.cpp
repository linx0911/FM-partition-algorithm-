#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <map>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <iomanip>
#include <unistd.h>


using namespace std;

class Cell{
public:
    Cell(string, int, int);
    void add_net(string);
    void change_way();
    void change_gain(int);
    void reset();
    void modify();
    void clean();
    string name;
    int size, pin, way, gain;
    bool lock, change;
    vector<string> net_list;
};

Cell::Cell(string name, int size, int way){
    this->name = name;
    this->size = size;
    this->lock = false;
    this->change = false;
    this->way = way;
    this->gain = 0;
    this->pin = 0;
}

void Cell::modify(){
    this->change = true;
}

void Cell::clean(){
    this->change = false;
}

void Cell::reset(){
    this->lock = false;
    this->clean();
    this->gain = 0;
}

void Cell::add_net(string net_name){
    this->net_list.push_back(net_name);
}

void Cell::change_way(){
    this->way = (this->way + 1) % 2;
    this->lock = true;
}

void Cell::change_gain(int gain){
    this->gain = gain;
}

class Net{
public:
    Net(string, vector<Cell*>);
    void add_set(int);
    void diff_set(int);
    void modify();
    void clean();
    void reset();
    string name;
    vector<Cell*> cell_list;
    int in[2];
    bool change;
};

Net::Net(string name, vector<Cell*> cell_list){
    this->name = name;
    this->cell_list = cell_list;
    this->in[0] = 0;
    this->in[1] = 0;
    this->change = false;  
    this->clean();
    vector<Cell*>::iterator iter;
    for(iter = this->cell_list.begin(); iter != this->cell_list.end(); ++iter){
        this->add_set((*iter)->way);
    }
}

void Net::modify(){
    this->change = true;
}

void Net::clean(){
    this->change = false;
}

void Net::reset(){
    this->clean();
}


void Net::add_set(int flag){
    this->in[flag]++;
}

void Net::diff_set(int flag){
    this->in[flag]--;
}


class Bucket_list{
public:
    Bucket_list(){};
    Bucket_list(int, int, int);
    void insert(Cell*);
    void del_cell(Cell*);
    Cell* get_max_cell(int&,int);
    void update_cell(int,Cell*);
    int find(int,Cell*);
    int size, pmax, length, max_gain, cell_num, way;//area, pmax, pmax * 2 + 1, MAX Gain, # of cell, set
    vector<vector<Cell*> > gain_list;
};

Bucket_list::Bucket_list(int size, int pmax, int way){
    this->size = size;
    this->pmax = pmax;
    this->way = way;
    this->cell_num = 0;
    this->max_gain = -pmax*2;
    this->length = this->pmax * 2 + 1;
    gain_list.resize(this->length);
    for(int i = 0; i < this->length; ++i){
        gain_list.push_back(vector<Cell*>());
    }
}

int Bucket_list::find(int gain_index, Cell* cell){
    for(int i = 0; i < this->gain_list.at(gain_index).size(); ++i){
        if(this->gain_list.at(gain_index).at(i)->name == cell->name)
            return i;
    }
}

void Bucket_list::del_cell(Cell* cell){
    this->size -= cell->size;
    this->cell_num--;
    int gain_index = cell->gain + this->pmax;
    int index = this->find(gain_index, cell);
    this->gain_list.at(gain_index).erase(this->gain_list.at(gain_index).begin()+index);
}

void Bucket_list::insert(Cell* cell){
    this->size += cell->size;
    this->cell_num++;
    this->gain_list.at(cell->gain + this->pmax).push_back(cell);
    if(this->max_gain < cell->gain) this->max_gain = cell->gain;
}

Cell* Bucket_list::get_max_cell(int& max_gain,int another_size){
    double constraint = (this->size + another_size)/10.0;
    if(this->cell_num < 0) return new Cell(" ", -1, false);
    bool check = true;
    for(int j = this->max_gain + this->pmax; j >= 0; j--){
        max_gain = j - pmax;
        int len = this->gain_list.at(j).size();
        for(int i = 0; i < len; i++){
            Cell* tmp_cell = this->gain_list.at(j).at(i);
            if(fabs( (this->size - tmp_cell->size) - (another_size + tmp_cell->size)) < constraint) return tmp_cell;
        }
        if(check && len != 0){
            this->max_gain = max_gain;
            check = false;
        }
    }
    max_gain = -pmax*2 - 100;
    return new Cell(" ", -1, false);
}

void Bucket_list::update_cell(int step, Cell* move_cell){
    this->del_cell(move_cell);
    move_cell->change_gain(move_cell->gain + step);
    this->insert(move_cell);
}


class Cell_Map{
public:
    Cell_Map();
    map<string, Cell*> cell_map;
    vector<string> group[2];
    Cell* find(string);
    void change_way(string);
    void change_gain(string,int);
    void insert(string,int,int);
    void partition();
    void reset();
    int number, size, group_size[2];
};

Cell_Map::Cell_Map(){
    this->size = 0;
    this->number = 0;
    this->group_size[0] = 0;
    this->group_size[1] = 0;
}

Cell* Cell_Map::find(string name){
    map<string, Cell*>::iterator iter;
    iter = this->cell_map.find(name);
    if(iter == this->cell_map.end()) return new Cell(" ", -1, -1);
    return (iter->second);
}

void Cell_Map::insert(string name, int size, int way){
    Cell *cell = new Cell(name, size, way);
    this->cell_map.insert(pair<string, Cell*>(cell->name, cell));
    this->number++;
    this->size += cell->size;
}

void Cell_Map::change_way(string name){
    Cell* cell = this->find(name);
    cell->change_way();
}

void Cell_Map::change_gain(string name, int new_gain){
    Cell* cell = this->find(name);
    cell->change_gain(new_gain);
}

void Cell_Map::partition(){
    map<string, Cell*>::iterator iter;
    for(iter = this->cell_map.begin(); iter != this->cell_map.end(); ++iter){
        this->group[iter->second->way].push_back(iter->second->name);
        this->group_size[iter->second->way] += iter->second->size;
    }
}

void Cell_Map::reset(){
    map<string, Cell*>::iterator cell;
    for(cell = this->cell_map.begin(); cell != this->cell_map.end(); ++cell)
        (cell->second)->reset();
}

class Net_Map{
public:
    Net_Map();
    map<string, Net*> net_map;
    void insert(string, vector<Cell*>);
    void reset();
    int get_cut();
    Net* find_net(string);
    int number;
};

Net_Map::Net_Map(){
    this->number = 0;
}

void Net_Map::insert(string name, vector<Cell*> cell_list){
    Net* net = new Net(name, cell_list);
    this->net_map.insert(pair<string, Net*>(net->name, net));
    this->number++;
}

Net* Net_Map::find_net(string name){
    map<string, Net*>::iterator iter;
    iter = this->net_map.find(name);
    if(iter == this->net_map.end()) return(new Net("NULL", vector<Cell*>() ));
    return (iter->second);
}

int Net_Map::get_cut(){
    int cut = 0;
    map<string, Net*>::iterator iter;
    for(iter = this->net_map.begin(); iter != this->net_map.end(); ++iter){
        if(iter->second->in[0] != 0 && iter->second->in[1] != 0) cut++;
    }
    return cut;
}

void Net_Map::reset(){
    map<string, Net*>::iterator iter;
    for(iter = this->net_map.begin(); iter != this->net_map.end(); ++iter){
        iter->second->reset();
    }
}

/******GLOBAL VARIABLE******/
Cell_Map Cell_map;
Net_Map Net_map;
int pmax;
Bucket_list blist[2];
fstream fp;
map<string, Cell> Cell_record;
map<string, Net> Net_record;
clock_t compute_s, compute_e, o_s, o_e;
double computing_time, IO_time;
/******GLOBAL VARIABLE******/


void input(string cell_name, string net_name){
    o_s = clock();
    ifstream fin1(cell_name.c_str(), ios::in);
    string str;
    int way, tag = 0, size;
    double Size[2], initial_cut = 1.0 + 0.1;
    Size[0] = Size[1] = 0.0;
    vector<string> Cell_name;
    vector<int> Cell_size;
    while (getline(fin1, str)){
        string token, name;
        istringstream delim(str);
        getline(delim, token,' ');
        name = token;
        getline(delim, token,' ');
        size = atoi(token.c_str());
        Cell_name.push_back(name);
        Cell_size.push_back(size);
        Size[1] += size;
    }

    for(int i = 0; i < Cell_size.size(); i++){
        if(Size[0] < Size[1] * initial_cut) way = 0;
        else way = 1;
        Cell_map.insert(Cell_name[i], Cell_size[i], way);
        Cell cell = Cell(Cell_name[i], Cell_size[i], way);
        Cell_record.insert(pair<string, Cell>(cell.name, cell));;
    	Size[0] += Cell_size[i];
    	Size[1] -= Cell_size[i];
    }

    map<string, Cell*>::iterator o;
    fin1.close();
    ifstream fin2(net_name.c_str(), ios::in);
    string net_tmp;
    while(fin2>>net_tmp != NULL){
        string name, tmp;
        fin2>>name;
        fin2>>tmp;
        vector<Cell*> tmp_cell_list;
        while(fin2>>tmp != NULL){
            if(tmp == "}") break;
            int flag = 0;
            for(int i = 0; i < tmp_cell_list.size() && flag == 0; i++)
                if(tmp_cell_list.at(i)->name == tmp)  flag = 1;
            if(flag == 1) continue;
            Cell* tmp_cell =  Cell_map.find(tmp);
            tmp_cell->add_net(name);
            tmp_cell->pin++;
            if(pmax < tmp_cell->pin) pmax = tmp_cell->pin;
            tmp_cell_list.push_back(tmp_cell);
        }
        Net_map.insert(name, tmp_cell_list);
        Net* tmp_net = Net_map.find_net(name);
        Net_record.insert(pair<string, Net>(tmp_net->name, *tmp_net));;
    }
    map<string, Net*>::iterator iter;
    fin2.close();
    o_e = clock();
    IO_time += (o_e-o_s)/(double)(CLOCKS_PER_SEC);
}

void update_Map(){
    map<string, Cell>::iterator cell_iter;
    map<string, Net>::iterator net_iter;
    for(cell_iter = Cell_record.begin(); cell_iter != Cell_record.end(); ++cell_iter){
        Cell* cell_find = Cell_map.find(cell_iter->second.name);
        cell_find->way = cell_iter->second.way;
    }
    for(net_iter = Net_record.begin(); net_iter != Net_record.end(); ++net_iter){
        Net* net_find = Net_map.find_net(net_iter->second.name);
        net_find->in[0] = net_iter->second.in[0];
        net_find->in[1] = net_iter->second.in[1];
    }
}

void setup(){
    update_Map();
    Bucket_list t[2];
    t[0] = Bucket_list(0, pmax, 0);
    t[1] = Bucket_list(0, pmax, 1);
    Cell_map.reset();
    Net_map.reset();
    map<string, Net*>::iterator iter;
    map<string, Cell*>::iterator cell_iter;
    for(iter = Net_map.net_map.begin(); iter != Net_map.net_map.end(); ++iter){
        Net* net = iter->second;
        vector<Cell*>::iterator c_iter;
        for(c_iter = net->cell_list.begin(); c_iter != net->cell_list.end(); ++c_iter){
            Cell* cell = *c_iter;
            if(net->in[cell->way] == 1) cell->gain++;
            if(net->in[(cell->way+1)%2] == 0) cell->gain--;
        }
    }
    for(cell_iter = Cell_map.cell_map.begin(); cell_iter != Cell_map.cell_map.end(); ++cell_iter){
        Cell* cell = (cell_iter->second);
        t[cell->way].insert(cell);
    }
    blist[0] = t[0];
    blist[1] = t[1];
}

void update_one(Net* net, int update_num, int way){
    vector<Cell*>::iterator cell_iter;
    for(cell_iter = net->cell_list.begin(); cell_iter != net->cell_list.end(); ++cell_iter){
        if((*cell_iter)->way == way && (*cell_iter)->lock == false){
            (*cell_iter)->modify();
            blist[(*cell_iter)->way].update_cell(update_num, *cell_iter);
            return;
        }
    }
}

void update_all(Net* net, int update_num){
    vector<Cell*>::iterator cell_iter;
    for(cell_iter = net->cell_list.begin(); cell_iter != net->cell_list.end(); ++cell_iter){
        if((*cell_iter)->lock == true) continue;
        (*cell_iter)->modify();
        blist[(*cell_iter)->way].update_cell(update_num, *cell_iter);
    }
    return;
}

void update_Gain(Cell* base_cell){
    int F = base_cell->way, T = (base_cell->way + 1) % 2;
    blist[F].del_cell(base_cell);
    blist[T].size += base_cell->size;
    base_cell->change_way();
    base_cell->modify();
    vector<string>::iterator net_iter;
    for(net_iter = base_cell->net_list.begin(); net_iter != base_cell->net_list.end(); ++net_iter){
        Net* tmp_net = Net_map.find_net(*net_iter);
        tmp_net->modify();
        if(tmp_net->in[T] == 0) update_all(tmp_net, 1); //increment gains of all free cells
        else if(tmp_net->in[T] == 1) update_one(tmp_net, -1, T);//decrement gain of the only T cell on n
        tmp_net->diff_set(F);
        tmp_net->add_set(T);
        if(tmp_net->in[F] == 0) update_all(tmp_net, -1);//decrement gains of all free cells
        else if(tmp_net->in[F] == 1) update_one(tmp_net, 1, F);//increment gain of the only F cell on n
    }
}

void output(string output_name){
    compute_s = clock();
    Cell_map.partition();
    int cut_number = Net_map.get_cut();
    compute_e = clock();
    computing_time += (compute_e-compute_s)/(double)(CLOCKS_PER_SEC);
    o_s = clock();
    cout<<"Final Cutsize = "<<cut_number<<endl;
    fp.open(output_name.c_str(), ios::out);
    fp<<"cut_size "<<cut_number<<endl;
    fp<<"A "<<Cell_map.group[0].size()<<endl;
    for(int i = 0; i < Cell_map.group[0].size(); i++) fp<<Cell_map.group[0].at(i)<<endl;
    fp<<"B "<<Cell_map.group[1].size()<<endl;
    for(int i = 0; i < Cell_map.group[1].size(); i++) fp<<Cell_map.group[1].at(i)<<endl;
    fp.close();
    o_e = clock();
    IO_time += (o_e-o_s)/(double)(CLOCKS_PER_SEC);
}

void update_record(){
    map<string, Cell*>::iterator cell_iter;
    map<string, Cell>::iterator cell_find;
    map<string, Net*>::iterator net_iter;
    map<string, Net>::iterator net_find;
    for(cell_iter = Cell_map.cell_map.begin(); cell_iter != Cell_map.cell_map.end(); ++cell_iter){
        if(cell_iter->second->change == false) continue;
        cell_iter->second->clean();
        cell_find = Cell_record.find(cell_iter->second->name);
        cell_find->second.way = (cell_iter->second->way);
    }
    for(net_iter = Net_map.net_map.begin(); net_iter != Net_map.net_map.end(); ++net_iter){
        if(net_iter->second->change == false) continue;
        net_iter->second->clean();
        net_find = Net_record.find(net_iter->second->name);
        net_find->second.in[0] = net_iter->second->in[0];
        net_find->second.in[1] = net_iter->second->in[1];
    }
}

void printUsage(string str){
	cerr<<"Usage: "<<str<<"[-c cells_file] [-n nets_file] [-o output_file]"<<endl;
}

void parseCmd(int argc, char **argv){
	cout<<"---------------Parameter Analyzer---------------"<<endl;
	if(argc < 7){
		printUsage(string(argv[0]));
		exit(EXIT_FAILURE);
	}
	char opt = 0;
	while((opt = getopt(argc, argv, "o:c:n:")) != -1){
		switch(opt){
			case 'o': 
			case 'c': 
			case 'n':
				cout<<"opt = "<<opt<<endl;
				cout<<"optarg = "<<optarg<<endl;
				break;
			default:
				printUsage(string(argv[0]));
				exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char **argv){
	parseCmd(argc, argv);
    computing_time = 0.0;
    IO_time = 0.0;
    input(argv[2], argv[4]);
    cout<<"------------------------------------------------"<<endl;
    cout<<"Total Cell number = "<<Cell_map.number<<endl;
    cout<<"Total Net number = "<<Net_map.number<<endl;
    compute_s = clock();

    while(1){
        setup();
        int partial_sum = 0, max_sum = -100000000;
        while(blist[0].cell_num > 0 || blist[1].cell_num > 0){
            int max_gain = -100000000, tmp;
            Cell* base_cell;
            for(int i = 0; i < 2; i++){
                Cell* tmp_cell = blist[i].get_max_cell(tmp, blist[(i+1)%2].size);
                if(tmp > max_gain){
                    base_cell = tmp_cell;
                    max_gain = tmp;
                }
            }
            partial_sum += max_gain;
            if(partial_sum < 0) break;
            update_Gain(base_cell);
            if(partial_sum > max_sum && partial_sum >= 0){
                max_sum = partial_sum;
                update_record();
            }          
        }
        if(max_sum <= 0) break;
    }
    compute_e = clock();
    computing_time += (compute_e-compute_s)/(double)(CLOCKS_PER_SEC);
    output("../output/" + string(argv[6]));
    cout<<"IO time = "<<fixed<<setprecision(5)<<IO_time<<" , Computing time = "<<fixed<<setprecision(5)<<computing_time<<endl;
    cout<<"Total Runtime = "<<fixed<<setprecision(5)<<IO_time + computing_time<<endl;
    cout<<"------------------------------------------------"<<endl;	
    return 0;
}
