
    p1, c1 :continuous;
    r1 == 0,T1 == 8, C1 == 2, D1 == 8: discrete; 

automaton sched

loc idle : while true wait { }
    when p1-T1>=0  do {p1'=0, r1'=C1} goto x1R;


loc x1R : while c1<=1 & p1-D1<=0 wait { }

loc error: while true wait {}

end

init := 
    loc[sched] == idle & 
    p1>=0 & c1==0;
bad := loc[sched]==error;
