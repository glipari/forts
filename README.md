FOrmal Real-Time Scheduler (FORTS)
==================================

- Authors: Youcheng Sun and Giuseppe Lipari
- E-mail: <techieyoucheng@gmail.com>

Introduction
------------

FORTS is a model checker on Linear Hybrid Automata (LHA) that I write
in C++. The basic parts of FORTS compose a parser to read LHA models
in script inputs and an engine to perform reachability analysis for
the safety property. It is specialized for Real-Time Scheduling
problems.

References
----------

- "A Weak Simulation Relation for Real-Time Schedulability Analysis of
  Global Fixed Priority Scheduling Using Linear Hybrid Automata"
  (2014) by Youcheng Sun and Giuseppe Lipari

- "Component-Based Analysis of Hierarchical Scheduling Using Linear
  Hybrid Automata" (2014) by Youcheng Sun, Giuseppe Lipari, Romain
  Soulat, Laurent Fribourg and Nicolas Markey


Compilation
-----------

		automake --add-missing 
		autoreconf
		./configure
		make

Examples
--------

Examples on input models for FORTS can be found in directory "models".
To perform a plain reachability analysis in the LHA model, simply run

		./efforts input-file

or

        ./efforts -t origin input-file

For Multiprocessor Global Fixed-Priority Schedulability Analysis, the
python script "forts-gen-smp-sched.py" generates the scheduling model
in LHA given an input description

	python forts-gen-smp-sched.py num-of-processors task-set-description-in-json output-file-name

To trigger the efficient schedulability analysis with "slack-time weak
simulation relation", run

        ./efforts -t widened_ex input-file

Options for running FORTS
-------------------------

	-s max_states_num

set up an upper bound on the number of states in the state space,
and an early termination will happen if the number of generated
states exceeds this bound.

	-b max_steps

set up the step limit for state space exploration, and an early
termination will happen if the current step exceeds this number.
