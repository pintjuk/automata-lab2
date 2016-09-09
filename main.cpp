#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdio.h>
#include <string>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <regex>
#include <set>
#include <list>
typedef unsigned int uint;

using namespace std;

bool enable_complete=false;

vector<string> string_split(string s, const char delimiter)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);
    
    vector<string> output;
    
    while (end <= string::npos)
    {
	    output.emplace_back(s.substr(start, end-start));

	    if (end == string::npos)
	    	break;

    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
    
    return output;
}

uint hexstring_to_int(string s)
{
    uint x;   
    std::stringstream ss;
    ss << std::hex << s;
    ss >> x;
    return x;
}

string time_stamp()
{
    time_t now = time(0);
    string lel(ctime(&now));
    replace(lel.begin(), lel.end(), ' ', '-');
    
    return lel.substr(0, lel.size()-1);
}


class method
{
    public:
    string          name;
    vector<string>  nodes;
    string          entry;
    vector<string>  ret;
};

struct flowedge
{
     string from;
     string to;
     bool epsilon;
     string method;
};


pid_t execute_command(const char *path, char * argv[])

{
    pid_t pid = fork();

    if(pid == 0)
    {
        execvp(path, argv);
        exit(0);
    }
    else if(pid ==-1)
    {
        printf("failed to fork!\n");
        exit(0);
    }
    else
    {
        return pid;
    }

}

class flowgraph
{
public:
    
    flowgraph(string filename)
    {
        string module_name;
        string line;
        main=NULL;

        ifstream ctf_getname(filename);
        while (getline(ctf_getname, line))
        {
            istringstream iss(line);
            vector<string> wards;
            vector<string> tempwards = string_split(line.c_str(), ' ');

            for(string ward: tempwards)
            {
                if (ward.size()>0)
                {
                    wards.push_back(ward);
                }
            }
            if ( wards.size() == 0 )
            {
                continue;
            }
            
            if ( wards[0] == "node" )
            {
                string node =wards[1];
                string m_name=wards[2].substr(5, wards[2].size()-6);
                num_nodes++;
                smatch sm;
                regex ismain("(.*)(main)");
                if( regex_match(m_name, sm, ismain) )
                {
                    cout << sm[1];
                    cout << sm[3];
                    module_name=sm[1];
                    main = &methods["main"];
                    methods["main"].name="main";
                    break;
                } 
            }

        }

        ctf_getname.close();
        num_nodes=0;
        ifstream ctf_stream(filename);
        while (getline(ctf_stream, line))
        {
            istringstream iss(line);
            vector<string> wards;
            vector<string> tempwards = string_split(line.c_str(), ' ');

            for(string ward: tempwards)
            {
                if (ward.size()>0)
                {
                    wards.push_back(ward);
                }
            }
            if ( wards.size() == 0 )
            {
                continue;
            }

            if ( wards[0] == "node" )
            {
                string node =wards[1];
                string m_name=wards[2];
                cout << m_name <<"\n";
                smatch sm_remove_module_name;
                regex remove_module_name("meth\\("+ module_name+"([a-zA-Z0-9_-]*)\\)");
                if(!regex_match(m_name, sm_remove_module_name, remove_module_name))
                {
                    continue;
                }
                m_name=sm_remove_module_name[1];
                num_nodes++;
                cout << "-- " << m_name << " " << node << "\n";
                methods[m_name].name=m_name;
                methods[m_name].nodes.push_back(node);
                cout << wards.size() << endl;
                if(wards.size()>3)
                {
                    if( wards[3] == "entry" )
                    {
                        cout << "lelwhat"<<endl;
                        cout << "entry node: " << node << endl; 
                        methods[m_name].entry = node;
                        cout << "actual entry node: " << methods[m_name].entry << endl;
                    }
                    if ( wards[3] == "ret" )
                    {
                        cout << "ret node:" << node << "\n";
                        methods[m_name].ret.push_back(node);
                    }
                }
            }
            else if( wards[0] == "edge" )
            {
                flowedge this_edge;
                this_edge.from = wards[1];
                this_edge.to =  wards[2];
                string m_name = wards[3];
                smatch sm_remove_module_name;
                regex remove_module_name("(?:"+module_name+")?([a-zA-Z0-9_-]*)");
                if(regex_match(m_name, sm_remove_module_name, remove_module_name))
                {
                    this_edge.epsilon = false;
                    cout << sm_remove_module_name[1] << "\n";
                    this_edge.method = sm_remove_module_name[1];
                }
                else
                {
                    this_edge.epsilon = true;
                    this_edge.method = "eps";
                }

                //this_edge.method = wards[3];
                edges.push_back(this_edge);
            }
            for(string ward: wards)
            {
                cout << "<";
                cout << ward;
                cout << ">";
                cout << " ";

            }
            cout << endl;
            
        }

        ctf_stream.close();
    }

    void print_graph()
    {
        pid_t pid = fork();

        if(pid ==0)
        {
            int status;
            ofstream myfile;
            string filename ="dumps/flowgraph-"+time_stamp();
            myfile.open (filename+".gv");
            
            myfile << "digraph G {\n";
            myfile << "\tsize = \"4,4\";\n";
            myfile << "\tshape=circle\n";
            for(flowedge e: edges)
            {
                //cout << e.from << " -> " << e.to << "[label=\""<< e.method <<"\"]\n";
                myfile << "\t\"" << e.from << "\" -> \"" << e.to << "\"[label=\""<< e.method <<"\"]\n";
            }
            
            for (auto& kv : methods) {
                string modname=kv.second.name;
                replace(modname.begin(), modname.end(), '-', '_');
                myfile << "\t\"" << modname << "\"[shape=box, style=filled,color=\".7 .3 1.0\",size =\"3,3\"]\n";
                myfile << "\t\"" << modname << "\" -> \"" << kv.second.entry << "\"\n";
                for(auto r: kv.second.ret)
                {
                    myfile << "\t" << r << "[shape=doublecircle]\n"; 
                }
            }

            myfile << "}\n";

            myfile.close();
            char* arg_v[6];
            arg_v[0]=strdup("dot");
            arg_v[1]=strdup("-Tps");
            arg_v[2]=strdup((filename+".gv").c_str());
            arg_v[3]=strdup("-o");
            arg_v[4]=strdup((filename+".ps").c_str());
            arg_v[5]= NULL;
            pid = execute_command(arg_v[0], arg_v);
            waitpid(pid, &status, 0);
            char* arg_v2[2];
            arg_v2[0]=strdup("evince");
            arg_v2[1]=strdup((filename+".ps").c_str());
            execvp(arg_v2[0], arg_v2);
            exit(0);

        }
        else if(pid ==-1)
        {
            printf("failed to fork!\n");
            exit(0);
        }
        
        
        return;
    }
    
    int num_nodes;
    map <string, method> methods;
    method* main;
    vector<flowedge> edges;
};

struct trans
{
    int from;
    int to;
    string symbol;
};

class dfa
{
public:
    set<int> nodes;
    set<string> symbols;
    vector<trans>   transitions;
    set<int>        F;
    int             s;

    dfa()
    {
    }
    
    void compleate(set<string> methods)
    {
        cout << "&&&&&&&&&&\n";
        vector<trans> toadd;
        for(auto t: transitions)
        {
           // cout <<t.from << " "<< t.symbol <<" "<< t.to << endl;
        } 

        for(int node:nodes)
        {
            for(string method: methods)
            {
                bool covered = any_of(
                        transitions.begin(), 
                        transitions.end(), 
                        [method, node](trans t){return t.from==node && t.symbol==method;});

                if(!covered)
                {
                    cout << node << " ";
                    cout << method << endl;
                    trans newtrans;
                    newtrans.from=node;
                    newtrans.symbol=method;
                    newtrans.to=node;
                    toadd.push_back(newtrans);
                }
            }
        }
        transitions.insert(transitions.end(), toadd.begin(), toadd.end());
    }

    dfa(string filename, set<string> methods)
    {
        ifstream ctf_stream(filename);
        string line;
        while (getline(ctf_stream, line))
        {
            //cout << line << "\n";
            regex first(".*[\\[\\(]q([0-9]+)[\\]\\)]-([a-zA-Z0-9_-]*)->[\\[\\(]q([0-9]+)[\\]\\)]");
            regex start(".*=>[\\[\\(]q([0-9]+)[\\]\\)].*");
            regex final1("=?>?\\(q([0-9]+)\\).*");
            regex final2(".*->\\(q([0-9]+)\\)");
            smatch sm;
            if(regex_match(line, sm,first))
            {
                cout << "first: " << line << "\n";
                trans newtrans;
                newtrans.from = stoi(sm[1]);
                newtrans.symbol = sm[2];
                newtrans.to   = stoi(sm[3]);
                transitions.push_back(newtrans);
                nodes.insert(newtrans.from);
                nodes.insert(newtrans.to);
            }
                       
            if(regex_match(line, sm, start))
            { 
                s=stoi(sm[1]);
                cout <<"start: " << line << "\n";
            }

            if(regex_match(line, sm, final1))
            {
                F.insert(stoi(sm[1]));
                cout << "final1: " << line << "\n";
            }

            if(regex_match(line, sm, final2))
            {   
                F.insert(stoi(sm[1]));
                cout << "final2: " << line << "\n";
            }
            cout << line << "\n";
            
        }
        
        //if(enable_complete)
        {
            compleate(methods);
        }

        for(trans t: transitions)
        {
            symbols.insert(t.symbol);
        }
    }

    
    dfa invert(){
        dfa newdfa;
        newdfa.nodes=nodes;
        newdfa.s=s;
        newdfa.symbols = symbols;
        newdfa.transitions=transitions;
        newdfa.F=nodes;
        for(int e: F)
        {
            newdfa.F.erase(e);
        }
        return newdfa;

    }
    void print_graph()
    {
        pid_t pid = fork();

        if(pid ==0)
        {
            int status;
            ofstream myfile;
            string filename ="dumps/dfa-"+time_stamp();
            myfile.open (filename+".gv");
            
            myfile << "digraph G {\n";
            myfile << "\tsize = \"4,4\";\n";

            myfile << "\trankdir = LR\n";
            myfile << "\tnode [shape = doublecircle];";
            
            for(int e: F)
            {
                myfile << " q"<< e<<";";
            }
            myfile << "\n\tnode [shape = point]; point_q0;\n";
            myfile << "\tnode [shape = circle];\n";
            myfile << "\t point_q0 -> q"<< s << "\n";

            for(trans e: transitions)
            {
                myfile << "\tq" << e.from << " -> q" << e.to << "[label=\""<< e.symbol <<"\"]\n";
            }
            
           
            myfile << "}\n";

            myfile.close();
            char* arg_v[6];
            arg_v[0]=strdup("dot");
            arg_v[1]=strdup("-Tps");
            arg_v[2]=strdup((filename+".gv").c_str());
            arg_v[3]=strdup("-o");
            arg_v[4]=strdup((filename+".ps").c_str());
            arg_v[5]= NULL;
            pid = execute_command(arg_v[0], arg_v);
            waitpid(pid, &status, 0);
            char* arg_v2[2];
            arg_v2[0]=strdup("evince");
            arg_v2[1]=strdup((filename+".ps").c_str());
            execvp(arg_v2[0], arg_v2);
            exit(0);

        }
        else if(pid ==-1)
        {
            printf("failed to fork!\n");
            exit(0);
        }
        
        
        return;
    }

};

class g_symbol
{
public:
    string type;
    string terminal;
    int q1;
    string v;
    int q2;

    friend bool operator<(const g_symbol& l, const g_symbol& r)
    {
        if(l.type=="S" && r.type=="S")
            return 0;
        return std::tie(l.type, l.terminal, l.q1, l.v, l.q2) < std::tie(r.type, r.terminal, r.q1, r.v, r.q2); // keep the same order
    }

    string to_graph_name() const
    {
        if (type=="S")
            return "S";
        else if (type=="triple")
            return std::to_string(q1)+v+std::to_string(q2);
        else if (type=="terminal")
            return terminal;
    }

    string to_string() const
    {
        if (type=="S")
            return "S";
        else if (type=="triple")
            return "["+std::to_string(q1)+", "+v+", "+std::to_string(q2)+ "]";
        else if (type=="terminal")
            return terminal;

    }
};

string rule_to_string(g_symbol sym, vector<g_symbol> l)
{
    string res=sym.to_string();
    for(auto& a: l)
        res += a.to_string();
    return res; 
}

class grammar
{
private:

public:
    map<g_symbol, set<vector<g_symbol>>> rules;
    set<g_symbol> generating_symbol;
    

    const int maxnodes=300;
    int currnode=0;
    set<string> loopavoid;
    string curtab="";

    void lm_derivation(g_symbol sym)
    {
        currnode++;
        if (currnode>maxnodes){
            cout << "...";
            return;
        }
        bool bloopavoid=true;
findrule:
        for(auto& result: rules[sym])
        {
            if(loopavoid.count(rule_to_string(sym, result))!=0 && bloopavoid){
                continue;
            }

            if( all_of( result.begin(), 
                        result.end(), 
                        [this](g_symbol const &ent3)
                {
                    return ent3.type=="terminal" || generating_symbol.count(ent3)==1;
                }))
            {
                loopavoid.insert(rule_to_string(sym, result));
                list<g_symbol> result_list={};
               // cout << sym.to_string() << " => ";
                for(auto& res_elem: result)
                {
                   //cout << res_elem.to_string();
                }
                //cout << endl;
                for(auto& res_elem: result)
                {
                    if(res_elem.type=="terminal")
                    {
                         if (currnode>maxnodes)
                            return;
                        if(res_elem.terminal=="eps"){
                            cout << "\n"<< curtab<< ")";
                            curtab=curtab.substr(0, curtab.size()-1);
                        }
                        else
                        {
                            cout << "(";
                            curtab+="\t";
                        }
                    }
                    else
                    {
                        if (currnode>maxnodes)
                            return;
                        cout <<"\n[q" << res_elem.q1 << "]\t" <<curtab<<res_elem.v;
                        lm_derivation(res_elem);
                    }
                }
                
                return;
            }
        }
        bloopavoid=false;
        goto findrule;
    }

    void print_counter_example()
    {
        g_symbol S;
        S.type="S";
        currnode=0;
        cout << "\n( ";
        lm_derivation(S);
        cout << "\b\b\b\b";
        cout << endl;
    }

    bool is_empty_lang()
    {
        g_symbol S;
        S.type="S";

        return generating_symbol.count(S)==0;
    }
    void dump_graph()
    {

        pid_t pid = fork();
        vector<string> terminals;
        vector<string> triples;
        if(pid ==0)
        {
            int status;
            ofstream myfile;
            string filename ="dumps/grammar-"+time_stamp();
            myfile.open (filename+".gv");
            
            myfile << "digraph G {\n";
            myfile << "\tsize = \"4,4\";\n";

            myfile << "\trankdir = LR\n";
           // myfile << "\tnode [shape = doublecircle];";
            
           /* for(int e: F)
            {
                myfile << " q"<< e<<";";
            }*/
           // myfile << "\n\tnode [shape = point]; point_q0;\n";
          //  myfile << "\tnode [shape = circle];\n";
            //myfile << "\t point_q0 -> q"<< s << "\n";
            
            /*for(trans e: transitions)
            {
                myfile << "\tq" << e.from << " -> q" << e.to << "[label=\""<< e.symbol <<"\"]\n";
            }*/
            for(auto const &ent1: rules )
            {
                for(vector<g_symbol> const &ent2: ent1.second)
                {
                    for(g_symbol const &ent3: ent2)
                    {
                        myfile << "\t\"" << ent1.first.to_graph_name() << "\" -> ";
                        myfile << "\"" << ent3.to_graph_name() << "\"";
                        myfile << "\n";
                    }
                }
            }

           
            myfile << "}\n";

            myfile.close();
            char* arg_v[6];
            arg_v[0]=strdup("dot");
            arg_v[1]=strdup("-Tps");
            arg_v[2]=strdup((filename+".gv").c_str());
            arg_v[3]=strdup("-o");
            arg_v[4]=strdup((filename+".ps").c_str());
            arg_v[5]= NULL;
            pid = execute_command(arg_v[0], arg_v);
            waitpid(pid, &status, 0);
            char* arg_v2[2];
            arg_v2[0]=strdup("evince");
            arg_v2[1]=strdup((filename+".ps").c_str());
            execvp(arg_v2[0], arg_v2);
            exit(0);

        }
        else if(pid ==-1)
        {
            printf("failed to fork!\n");
            exit(0);
        }
        
        
        return;
    }

    void print()
    {
        for(auto const &ent1: rules )
        {
            if(generating_symbol.count(ent1.first)==1)
            {
                cout << ":: ";
            }
            else
            {
                cout << "   ";
            }
            if(ent1.first.type=="S")
            {
                cout<<"S => ";
            }
            else if (ent1.first.type=="triple")
            {
                cout << "[" <<ent1.first.q1<<", "<<ent1.first.v<<", "<<ent1.first.q2<<"] => ";
            }

            for(vector<g_symbol> const &ent2: ent1.second)
            {
                for(g_symbol const &ent3: ent2)
                {
                    if(ent3.type=="S")
                    {
                        cout<<"S";
                    }
                    else if (ent3.type=="triple")
                    {
                        cout << "[" <<ent3.q1<<", "<<ent3.v<<", "<<ent3.q2<<"]";
                    }
                    else if (ent3.type=="terminal")
                    {
                        cout << ent3.terminal;
                    }
                }
                cout << " | ";
            }
            cout << "\n";
        }
    }

    void find_all_generating()
    {
        for(;;)
        {
            bool new_found=false;
            for(auto const &ent1: rules )
            {
                if(generating_symbol.count(ent1.first)==1)
                {
                    continue;
                }

                bool is_generating = any_of(
                        ent1.second.begin(), 
                        ent1.second.end(), 
                        [this](vector<g_symbol> const &ent2)
                {
                    return all_of(
                            ent2.begin(), 
                            ent2.end(), 
                            [this](g_symbol const &ent3)
                    {
                        return ent3.type=="terminal" || generating_symbol.count(ent3)==1;
                    });
                });
            
                if(is_generating)
                {
                    new_found=true;
                    generating_symbol.insert(ent1.first);
                }
            }

            if(!new_found)
            {
                break;
            }
        }
    }
};


grammar fg_dfa_product(flowgraph& fg, dfa& mydfa)
{
    grammar res;
    g_symbol S;
    S.type="S";
    /* 1. For every final state q i ∈ Q F , a production S → [q 0 v 0 q i ], 
     * where v 0 is the entry node of the main method
     */


    for(int qi: mydfa.F)
    {
        g_symbol triple;
        triple.type="triple";
        triple.q1 = mydfa.s;
        triple.v  = (fg.main -> entry);
        cout << "###### " << fg.main -> entry << "\n";
        triple.q2 = qi;
        res.rules[S].insert(vector<g_symbol>{triple});
    }

    /* 2. For every transfer edge v i →
     * − v j of F and every state sequence q a q b ∈
     * Q 2 , a production [q a v i q b ] → [q a v j q b ].
     */
    for(flowedge fe: fg.edges)
    {
        //if(!fe.epsilon)
        //    continue;
        if(fe.method!="eps" && mydfa.symbols.count(fe.method)!=0 )
            continue;

        for(auto qa: mydfa.nodes)
        {
            for(auto qb: mydfa.nodes)
            {
                g_symbol from;
                g_symbol to;
                from.type   = "triple";
                from.q1     = qa;
                from.v      = fe.from; 
                from.q2     = qb;
                to.type     = "triple";
                to.q1     = qa;
                to.v      = fe.to;
                to.q2     = qb;
                res.rules[from].insert(vector<g_symbol>{to});
            }
        }
    }
    /* 3. For every call edge v i −→ v j and every state sequence q a q b q c q d ∈ Q 4 ,
     * a production [q a v i q d ] → [q a m q b ][q b v k q c ][q c v j q d ], where v k is the
     * entry node of method m.
     */

    for(flowedge fe: fg.edges)
    {
        if(fe.method=="eps" || mydfa.symbols.count(fe.method)==0)
            continue;

        for(auto qa: mydfa.nodes)
        {
            for(auto qb: mydfa.nodes)
            {
                for(auto qc: mydfa.nodes)
                {
                    for(auto qd: mydfa.nodes)
                    {
                        g_symbol to2;
                        g_symbol from;
                        g_symbol to1;
                        g_symbol to3;

                        if(fg.methods.count(fe.method)!=0)
                        {
                            string vk = fg.methods[fe.method].entry; 
                            to2.q1      = qb;
                            to2.v       = vk;
                            to2.q2      = qc;
                        }
                        from.type   = "triple";
                        to1.type    = "triple";
                        to2.type    = "triple";
                        to3.type    = "triple";
                        from.q1     = qa;
                        from.v      = fe.from;
                        from.q2     = qd;
                        to1.q1      = qa;
                        to1.v       = fe.method;
                        to1.q2      = qb;
                        to3.q1      = qc;
                        to3.v       = fe.to;
                        to3.q2      = qd;

                        if(fg.methods.count(fe.method)!=0)
                        {
                            res.rules[from].insert(vector<g_symbol>{to1, to2, to3});
                        }
                        else
                        {
                            g_symbol ep;
                            ep.type="terminal";
                            ep.terminal="eps";
                            to3.q1      = qb;
                            to3.q2      = qc;
                            from.q2     = qc;
                            res.rules[from].insert(vector<g_symbol>{to1,ep, to3});
                        }
                    }
                }
            }
        }
    }

    /* 4. For every return node v i ∈ R and every state q j ∈ Q, a production
     * [q j v i q j ] → .
     */
    for(auto& mpair: fg.methods)
    {
        method e= mpair.second; 
        for(string vi: e.ret)
        {
            for(int qj: mydfa.nodes)
            {
                g_symbol from;
                g_symbol to;
                from.type="triple";
                from.q1 = qj;
                from.v  = vi;
                from.q2 = qj;
                
                to.type="terminal";
                to.terminal="eps";
                res.rules[from].insert(vector<g_symbol>{to});
            }
        }
    }

    /*
     * 5 For every transition δ(q i , a) = q j of D, a production [q i a q j ] → a
     */
    for(trans t: mydfa.transitions)
    {
                g_symbol from;
                g_symbol to;
                from.type="triple";
                from.q1 = t.from;
                from.v  = t.symbol;
                from.q2 = t.to;
                
                to.type="terminal";
                to.terminal=t.symbol;
                res.rules[from].insert(vector<g_symbol>{to});

    }

    return res;
}
 

int main( int argc, const char* argv[] )
{
    if(argc<3)
    {
        printf("not enough arguments\n");
        return 0;
    }

    printf( "\nHello World (%s, %s) \n\n", argv[1], argv[2] );

    if(argc>=4)
    {
        cout << "AOEUANOEUETNUEUT!\n";
        cout << argv[3]<<endl;
        if(string(argv[3])=="-c")
        {
            enable_complete=true;
            cout << "ANTINTOINOINI!\n";
        }
    }
    string ctf_file_name= argv[1];
    string spec_file_name= argv[2];

    flowgraph cfg(ctf_file_name);
    cfg.print_graph();
    set<string> methods;
    for(auto& imap: cfg.methods){
        methods.insert(imap.first);
    }
    dfa lelwhat(spec_file_name, methods);
    lelwhat.print_graph();
    lelwhat=lelwhat.invert(); 
    grammar resultgrammer= fg_dfa_product(cfg, lelwhat);
    resultgrammer.dump_graph();
    resultgrammer.find_all_generating();
    resultgrammer.print();
    if(resultgrammer.is_empty_lang())
    {
        cout << "Spec Holds\n";
    }
    else 
    {
        cout << "Spec Does not hold!!!\nHere is a countre example:\n";
        resultgrammer.print_counter_example();
    }
}
