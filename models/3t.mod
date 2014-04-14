
    p1, p2, p3, c1, c2, c3 :continuous;
    T1 == 8, C1 == 2, D1 == 8,     T2 == 20, C2 == 2, D2 == 20,     T3 == 50, C3 == 2, D3 == 50: discrete; 

automaton sched

loc idle : while true wait { }
    when p1-T1>=0  do {p1'=0, c1'=C1} goto x1R;
    when p2-T2>=0  do {p2'=0, c2'=C2} goto x2R;
    when p3-T3>=0  do {p3'=0, c3'=C3} goto x3R;


loc x3R : while c3>=0 & p3-D3<=0 wait { c3'=-1}
    when c3 == 0  do {} goto idle;
    when c3>=1 & p3-D3==0 do {} goto error;
    when c3>=1&p1-T1>=0   do {p1'=0, c1'=C1} goto x1R3W;
    when c3>=1&p2-T2>=0   do {p2'=0, c2'=C2} goto x2R3W;

loc x2R : while c2>=0 & p2-D2<=0 wait { c2'=-1}
    when c2 == 0  do {} goto idle;
    when c2>=1 & p2-D2==0 do {} goto error;
    when c2>=1&p1-T1>=0   do {p1'=0, c1'=C1} goto x1R2W;
    when p3-T3>=0   do {p3'=0, c3'=C3} goto x2R3W;

loc x2R3W : while c2>=0 & p2-D2<=0 & p3-D3<=0 wait { c2'=-1,c3'=0}
    when c2 == 0  do {} goto x3R;
    when c2>=1 & p2-D2==0 do {} goto error;
    when p3-D3==0 do {} goto error;
    when c2>=1&p1-T1>=0   do {p1'=0, c1'=C1} goto x1R2W3W;

loc x1R : while c1>=0 & p1-D1<=0 wait { c1'=-1}
    when c1 == 0  do {} goto idle;
    when c1>=1 & p1-D1==0 do {} goto error;
    when p2-T2>=0   do {p2'=0, c2'=C2} goto x1R2W;
    when p3-T3>=0   do {p3'=0, c3'=C3} goto x1R3W;

loc x1R3W : while c1>=0 & p1-D1<=0 & p3-D3<=0 wait { c1'=-1,c3'=0}
    when c1 == 0  do {} goto x3R;
    when c1>=1 & p1-D1==0 do {} goto error;
    when p3-D3==0 do {} goto error;
    when p2-T2>=0   do {p2'=0, c2'=C2} goto x1R2W3W;

loc x1R2W : while c1>=0 & p1-D1<=0 & p2-D2<=0 wait { c1'=-1,c2'=0}
    when c1 == 0  do {} goto x2R;
    when c1>=1 & p1-D1==0 do {} goto error;
    when p2-D2==0 do {} goto error;
    when p3-T3>=0   do {p3'=0, c3'=C3} goto x1R2W3W;

loc x1R2W3W : while c1>=0 & p1-D1<=0 & p2-D2<=0 & p3-D3<=0 wait { c1'=-1,c2'=0,c3'=0}
    when c1 == 0  do {} goto x2R3W;
    when c1>=1 & p1-D1==0 do {} goto error;
    when p2-D2==0 do {} goto error;
    when p3-D3==0 do {} goto error;

loc error: while true wait {}

end

init := 
    loc[sched] == idle & 
    p1>=0 & c1==0 &
    p2>=0 & c2==0 &
    p3>=0 & c3==0;
bad := loc[sched]==error;
