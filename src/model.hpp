#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <vector>
#include <string>
#include <memory>

#include "sstate.hpp"
#include "automaton.hpp"
#include "statistics.hpp"
#include "time_abstract_state.hpp"

typedef constraint_node constraint;

class Combined_edge;

#define MODEL Model::get_instance()


struct model_stats {
    int total_states = 0; 
    int eliminated = 0; 
    int past_elim_from_next = 0;
    int past_elim_from_current = 0;
    int past_elim_from_space = 0;

    void print() const;
};


//struct Parameter {
//    std::string name;
//    int min;
//    int max;
//    Parameter(std::string n, int mi, int ma);
////    Parameter();
//};

enum SYMBOLIC_STATE_TYPE { ORIGIN, WIDENED, WIDENED_EX, WIDENED_MERGE, BOX_WIDENED, DBM, OCT}; 

class Model {
protected:
  
    bool merge;
    enum SYMBOLIC_STATE_TYPE sstate_type = ORIGIN;
    // continuous vars for this model
    VariableList cvars;
    // discrete variables for this model
    Valuations dvars;

    // the initial constraints
    constraint              init_constraint; 

    // the set of automatons
    std::vector<automaton>  automata;

    /** The symbolic state space */
    std::list<std::shared_ptr<Symbolic_State> > Space;

    model_stats stats;
    TimeStatistic contains_stat;
    TimeStatistic post_stat;

    int max_states_num = 0;
    int max_steps= 0;

    Model();

    static Model *the_instance; 

bool contained_in(const std::shared_ptr<Symbolic_State> &ss, const std::list<std::shared_ptr<Symbolic_State> > &lss, const std::list<std::shared_ptr<Symbolic_State> >::iterator & curr);
    bool contained_in(const std::shared_ptr<Symbolic_State> &ss, const std::list<std::shared_ptr<Symbolic_State> > &lss);
    bool merged_in(const std::shared_ptr<Symbolic_State> &ss, const std::list<std::shared_ptr<Symbolic_State> > &lss);
    int remove_included_sstates_in_a_list(const std::shared_ptr<Symbolic_State> &ss, std::list<std::shared_ptr<Symbolic_State> > &lss);
    int invalidate_included_sstates_in_a_list(const std::shared_ptr<Symbolic_State> &ss, std::list<std::shared_ptr<Symbolic_State> > &lss);

    void output_stats(const int time_spent) const;

public:
    Model(const Model &other) = delete;
    Model &operator=(const Model &other) = delete;

    // singleton
    static Model& get_instance(); 
    // reset the singleton
    static void reset(); 
    // print (for debugging)
    void print() const;
    // checks that everything is ok
    void check_consistency();

    // building functions 
    void add_automaton(const automaton &a); 
    void set_init(const constraint &ini); 
    void add_cvar(const std::string &cv); 
    void add_dvar(const std::string &dv, int value); 

    VariableList get_cvars() const { return cvars; }

    // TBM: Given a initial sstate, performs a continuous step
    // void continuous_step(Symbolic_State &ss);
    // TBM: given an initial sstate and a combined edge, performs a discrete step
    //void discrete_step(Symbolic_State &ss, Combined_edge &edges);
    void discrete_step(std::shared_ptr<Symbolic_State> &pss, Combined_edge &edges);

    // Maybe will become private:
    // performs a step in the exploration of the state space
    //std::vector<Symbolic_State> Post(const Symbolic_State& ss);
    std::vector<std::shared_ptr<Symbolic_State> > Post(const std::shared_ptr<Symbolic_State>& pss);
    std::vector<std::shared_ptr<Symbolic_State> > discrete_steps(const std::shared_ptr<Symbolic_State>& pss);

    // TBM 
    // PPL::NNC_Polyhedron get_invariant_cvx(Symbolic_State &ss);

    // Initial symbolic state
    std::shared_ptr<Symbolic_State> init_sstate();

    // TBM: if the current state is bad
    //bool is_bad(const Symbolic_State &ss);

    // throws an exception if the automaton is not found
    automaton& get_automaton_by_name(const std::string name);

    // throws an exception if the automaton is not found
    automaton& get_automaton_at(unsigned i) { return automata.at(i); }

    /** 
	To explore Space according to breadth first search. 
	This is the function that performs reachability analysis.
    */
    void SpaceExplorer();

    std::list<std::shared_ptr<Symbolic_State> > &get_all_states() { return Space; }


    /** Return the meomey used for symbolic states in Space. */
    int total_memory_in_bytes() const;

    /** Print the symbolic state space "Space" into a file. */
    void print_log(const std::string fname= ".log") const;

    void set_sstate_type(enum SYMBOLIC_STATE_TYPE t);


    /** This part is designed for efficient schedulability test of G-FP algorithm on sporadic tasks. */
    void TbT();

    void set_max_states_num(const int num) {  max_states_num = num;}
    void set_max_steps(const int num) {  max_steps = num;}




    /********************************* Model with parameters ***************************/
protected:
//    bool on_the_fly = false;
//
//    std::vector<PPL::Pointset_Powerset<PPL::NNC_Polyhedron> > good_tiles;
//    std::vector<PPL::NNC_Polyhedron> bad_tiles;
    std::vector<Parameter> parameters; 
//
//    std::vector<Time_Abstract_State> UNReach, Acyclic, BS;
//    std::vector<std::pair<Time_Abstract_State, Time_Abstract_State> > Contained;
//
//    bool contained_in_then_store(const std::shared_ptr<Symbolic_State> &ss, const std::list<std::shared_ptr<Symbolic_State> > &lss);
//    void build_a_good_tile();
//    void build_a_bad_tile();
//    PPL::NNC_Polyhedron trace_to_cvx(const Trace& tr);
//    void map_to_parameters(PPL::NNC_Polyhedron &poly) const;
//    void map_to_parameters(PPL::NNC_Polyhedron &poly, const VariableList& cvars);
//    void map_to_parameters(PPL::Pointset_Powerset<PPL::NNC_Polyhedron> &poly, const VariableList& cvars);
//    bool in_a_tile(const Valuations &v) const;
//    std::pair<PPL::NNC_Polyhedron, PPL::NNC_Polyhedron> unreach_trace_to_cvx(const Trace& tr);
//
//
//    std::shared_ptr<Symbolic_State> beep_init_sstate();
//
//    bool contained_in_then_store(const std::shared_ptr<Symbolic_State> &ss, const std::list<std::shared_ptr<Symbolic_State> > &lss, PPL::NNC_Polyhedron& conjunction_part);
//
public:
//
//    /** Model with parameters */
    void add_param(const Parameter &param);
//    const std::vector<Parameter>& get_parameters() const;
//    void BEEP(const Valuations &pi0);
//    void BEEP();
//    bool is_parameter(const std::string& s) const;
//    Parameter get_parameter_by_name(const std::string& s) const;
//    void print_points(std::string fname) const;
//
//
//    bool backtrack(const std::shared_ptr<Symbolic_State> &ss, std::vector<std::shared_ptr<Symbolic_State> >& dominating, std::vector<std::shared_ptr<Symbolic_State> >& dominated) const;
//
//
//    bool be_outside(const int i, const double v, const PPL::Pointset_Powerset<PPL::NNC_Polyhedron> &domain) const;
//
//    void grow_by_steps(PPL::Pointset_Powerset<PPL::NNC_Polyhedron> &good_tile, const std::shared_ptr<Symbolic_State> &dominating, const std::shared_ptr<Symbolic_State> &dominated, const Valuations &pi0, const double step = 1.0) const;
//
//    void cut_tile(const int i, const int i_trial_upper_bound, const bool up, const int i_lower_bound, const bool down, PPL::Pointset_Powerset<PPL::NNC_Polyhedron> &trial_refined_tile) const;
//
//    int dominated_relation(const std::shared_ptr<Symbolic_State> &ss, const std::list<std::shared_ptr<Symbolic_State> > &li, std::vector<std::shared_ptr<Symbolic_State> >& dominating, std::vector<std::shared_ptr<Symbolic_State> >& dominated) const;
//
//    Parameter valuation_index_to_parameter(const Valuations& v, const int index) const;
//    std::vector<Valuations> increase_by_one_step(const Valuations& x) const;
//    bool existing_point(const Valuations &v, const std::vector<Valuations> &vv) const;
//
//
//    /** Efficient BEEP method **/
//    void efficient_BEEP();
//    void efficient_BEEP(const Valuations &pi0);
//    bool incompatible(const PPL::Constraint &cs, const Valuations &pi0) const;
//    PPL::Constraint negate_cs(const PPL::Constraint &c1, const Valuations &ref) const;
//    PPL::Constraint negate_cs(const PPL::Constraint &c1) const;
//    void grow_by_steps(PPL::NNC_Polyhedron &good_tile, const std::shared_ptr<Symbolic_State> &dominating, const std::shared_ptr<Symbolic_State> &dominated, const Valuations &pi0, const double step = 1.0) const;
//    void cut_tile(const int i, const int i_trial_upper_bound, const bool up, const int i_lower_bound, const bool down, PPL::NNC_Polyhedron &trial_refined_tile) const;
//    bool be_outside(const int i, const double v, const PPL::NNC_Polyhedron &domain) const;
//    std::vector<PPL::NNC_Polyhedron > efficient_good_tiles;
//    void efficient_print_points(std::string fname) const;
//    bool efficient_in_a_tile(const Valuations &v) const;
//    
//
//    /************** The BUG (bad/unknown/good) algorithm for parameter synthesis ********/
//
//
//    void set_bound(int b); 
//    /** Bounded Counter-Example approach for parameter synthesis. */
//    void CE();
//    void unconstrain_to_parameters(PPL::NNC_Polyhedron &poly);
//    int bound = 0;
//    std::shared_ptr<Symbolic_State> init_param_sstate();
//    PPL::NNC_Polyhedron param_region;
//    PPL::NNC_Polyhedron param_domain;
//    int get_cvar_index(const std::string& s) const;
//    void print_cvx(const PPL::NNC_Polyhedron &cvx, const std::string &fname) const;
//    std::list<PPL::NNC_Polyhedron> get_incl_constraints(const PPL::NNC_Polyhedron& inf, const PPL::NNC_Polyhedron& sup) const;
//    PPL::Pointset_Powerset<PPL::NNC_Polyhedron> union_ces;
//    std::list<PPL::NNC_Polyhedron> ces;
//    std::list<PPL::NNC_Polyhedron> ending_points;
//    //PPL::Pointset_Powerset<PPL::NNC_Polyhedron> big_ending_point;
//    bool contained_in(const PPL::NNC_Polyhedron &c, const std::list<PPL::NNC_Polyhedron> &lcs);
//    int remove_contained_elements(const PPL::NNC_Polyhedron &c, std::list<PPL::NNC_Polyhedron> &lcs);
//    //PPL::Pointset_Powerset<NNC_Polyhedron> union_ces;
//    //PPL::Pointset_Powerset<NNC_Polyhedron> diff_ces;
//    void refine_with_counter_example(const PPL::NNC_Polyhedron& ce, std::list<std::shared_ptr<Symbolic_State> > &lss);
//    void refine_with_counter_examples(const PPL::Pointset_Powerset<PPL::NNC_Polyhedron>& uc, std::list<std::shared_ptr<Symbolic_State> > &lss);
//    void print_points_ce(std::string fname) const;

};

#endif
