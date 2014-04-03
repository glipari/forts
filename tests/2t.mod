    p1, c1, d1, p2, c2, d2: continuous;
    O1 == 0, T1 == 2, C1 == 1, D1 == 2,     O2 == 1, T2 == 4, C2 == 2, D2 == 4 : discrete;

automaton act1
sync: r1;
loc wait_for_offset1 : while p1<=O1 wait {}
    when p1==O1 sync r1 do { p1'= 0 } goto wait_for_period1;
loc wait_for_period1 : while p1<=T1 wait {}
    when p1==T1 sync r1 do { p1'= 0 } goto wait_for_period1;

end

automaton act2
sync : r2;
loc wait_for_offset2 : while p2<=O2 wait {}
    when p2==O2 sync r2 do { p2'= 0 } goto wait_for_period2;
loc wait_for_period2 : while p2<=T2 wait {}
    when p2==T2 sync r2 do { p2'= 0 } goto wait_for_period2;

end

automaton sched

sync : r1, r2;

loc idle: while true wait {}
    when true  sync r1 do {c1'=0, d1'=0} goto x1R;
    when true  sync r2 do {c2'=0, d2'=0} goto x2R;


loc x2R : while c2<=C2 & d2<=D2 wait {}
    when c2 == C2 do {} goto idle;
    when c2<=C2-1 & d2==D2 do {} goto error;
    when c2<=C2-1 & d2<=D2-1 sync r1 do { c1'=0, d1'=0 } goto x1R2W;

loc x1R : while c1<=C1 & d1<=D1 wait {}
    when c1 == C1 do {} goto idle;
    when c1<=C1-1 & d1==D1 do {} goto error;
    when true sync r2 do { c2'=0, d2'=0 } goto x1R2W;


loc x1R2W : while c1<=C1 & d1<=D1 & c2<=C2 & d2<=D2 wait {c2'=0}
    when c1 == C1  do {} goto x2R;
    when c1<=C1-1 & d1==D1 do {} goto error;
    when d2==D2 do {} goto error;


loc error: while true wait {}

end

init := 
    loc[act1] == wait_for_offset1 &
    loc[act2] == wait_for_offset2 &
    loc[sched] == idle & p1 == 0 & p2 == 0 & c1==0 & d1 == 0 & c2==0 & d2 == 0;
    
bad := loc[sched] == error; 
