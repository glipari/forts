
    p1, p2, p3, c1, c2, c3 , x, y :continuous;
    T1 == 8, C1 == 2, D1 == 8,     T2 == 20, C2 == 2, D2 == 20,     T3 == 50, C3 == 6, D3 == 50, 
    P == 5, Q == 3: discrete;



automaton sched

sync : r1, r2, r3,  empty, go, frozen;

loc idle : while p1-T1<=0 & p2-T2<=0 & p3-T3<=0 wait { c1'=0, c2'=0, c3'=0}
    when p1-T1==0  sync r1 do {p1'=0, c1'=C1} goto f1R;
    when p2-T2==0  sync r2 do {p2'=0, c2'=C2} goto f2R;
    when p3-T3==0  sync r3 do {p3'=0, c3'=C3} goto f3R;


loc x3R : while p1-T1<=0 & p2-T2<=0 & c3>=0 & p3-D3<=0 wait { c1'=0, c2'=0, c3'=-1}
    when c3 == 0  sync empty do {} goto idle;
    when c3>=1 & p3-D3==0 do {} goto error;
    when c3>=1&p1-T1==0   sync r1 do {p1'=0, c1'=C1} goto x1R3W;
    when c3>=1&p2-T2==0   sync r2 do {p2'=0, c2'=C2} goto x2R3W;
    when c3>=1 sync frozen do {} goto f3R;

loc x2R : while p1-T1<=0 & c2>=0 & p2-D2<=0 & p3-T3<=0 wait { c1'=0, c2'=-1, c3'=0}
    when c2 == 0  sync empty do {} goto idle;
    when c2>=1 & p2-D2==0 do {} goto error;
    when c2>=1&p1-T1==0   sync r1 do {p1'=0, c1'=C1} goto x1R2W;
    when p3-T3==0   sync r3 do {p3'=0, c3'=C3} goto x2R3W;
    when c2>=1 sync frozen do {} goto f2R;

loc x2R3W : while p1-T1<=0 & c2>=0 & p2-D2<=0 & p3-D3<=0 wait { c1'=0, c2'=-1, c3'=0}
    when c2 == 0  do {} goto x3R;
    when c2>=1 & p2-D2==0 do {} goto error;
    when p3-D3==0 do {} goto error;
    when c2>=1&p1-T1==0   sync r1 do {p1'=0, c1'=C1} goto x1R2W3W;
    when c2>=1 sync frozen do {} goto f2R3W;

loc x1R : while c1>=0 & p1-D1<=0 & p2-T2<=0 & p3-T3<=0 wait { c1'=-1, c2'=0, c3'=0}
    when c1 == 0  sync empty do {} goto idle;
    when c1>=1 & p1-D1==0 do {} goto error;
    when p2-T2==0   sync r2 do {p2'=0, c2'=C2} goto x1R2W;
    when p3-T3==0   sync r3 do {p3'=0, c3'=C3} goto x1R3W;
    when c1>=1 sync frozen do {} goto f1R;

loc x1R3W : while c1>=0 & p1-D1<=0 & p2-T2<=0 & p3-D3<=0 wait { c1'=-1, c2'=0, c3'=0}
    when c1 == 0  do {} goto x3R;
    when c1>=1 & p1-D1==0 do {} goto error;
    when p3-D3==0 do {} goto error;
    when p2-T2==0   sync r2 do {p2'=0, c2'=C2} goto x1R2W3W;
    when c1>=1 sync frozen do {} goto f1R3W;

loc x1R2W : while c1>=0 & p1-D1<=0 & p2-D2<=0 & p3-T3<=0 wait { c1'=-1, c2'=0, c3'=0}
    when c1 == 0  do {} goto x2R;
    when c1>=1 & p1-D1==0 do {} goto error;
    when p2-D2==0 do {} goto error;
    when p3-T3==0   sync r3 do {p3'=0, c3'=C3} goto x1R2W3W;
    when c1>=1 sync frozen do {} goto f1R2W;

loc x1R2W3W : while c1>=0 & p1-D1<=0 & p2-D2<=0 & p3-D3<=0 wait { c1'=-1, c2'=0, c3'=0}
    when c1 == 0  do {} goto x2R3W;
    when c1>=1 & p1-D1==0 do {} goto error;
    when p2-D2==0 do {} goto error;
    when p3-D3==0 do {} goto error;
    when c1>=1 sync frozen do {} goto f1R2W3W;

loc f3R : while p1-T1<=0 & p2-T2<=0 & p3-D3<=0 wait { c1'=0, c2'=0, c3'=0}
    when p3-D3==0 do {} goto error;
    when p1-T1==0   sync r1 do {p1'=0, c1'=C1} goto f1R3W;
    when p2-T2==0   sync r2 do {p2'=0, c2'=C2} goto f2R3W;
    when true sync go do {} goto x3R;

loc f2R : while p1-T1<=0 & p2-D2<=0 & p3-T3<=0 wait { c1'=0, c2'=0, c3'=0}
    when p2-D2==0 do {} goto error;
    when p1-T1==0   sync r1 do {p1'=0, c1'=C1} goto f1R2W;
    when p3-T3==0   sync r3 do {p3'=0, c3'=C3} goto f2R3W;
    when true sync go do {} goto x2R;

loc f2R3W : while p1-T1<=0 & p2-D2<=0 & p3-D3<=0 wait { c1'=0, c2'=0, c3'=0}
    when p2-D2==0 do {} goto error;
    when p3-D3==0 do {} goto error;
    when p1-T1==0   sync r1 do {p1'=0, c1'=C1} goto f1R2W3W;
    when true sync go do {} goto x2R3W;

loc f1R : while p1-D1<=0 & p2-T2<=0 & p3-T3<=0 wait { c1'=0, c2'=0, c3'=0}
    when p1-D1==0 do {} goto error;
    when p2-T2==0   sync r2 do {p2'=0, c2'=C2} goto f1R2W;
    when p3-T3==0   sync r3 do {p3'=0, c3'=C3} goto f1R3W;
    when true sync go do {} goto x1R;

loc f1R3W : while p1-D1<=0 & p2-T2<=0 & p3-D3<=0 wait { c1'=0, c2'=0, c3'=0}
    when p1-D1==0 do {} goto error;
    when p3-D3==0 do {} goto error;
    when p2-T2==0   sync r2 do {p2'=0, c2'=C2} goto f1R2W3W;
    when true sync go do {} goto x1R3W;

loc f1R2W : while p1-D1<=0 & p2-D2<=0 & p3-T3<=0 wait { c1'=0, c2'=0, c3'=0}
    when p1-D1==0 do {} goto error;
    when p2-D2==0 do {} goto error;
    when p3-T3==0   sync r3 do {p3'=0, c3'=C3} goto f1R2W3W;
    when true sync go do {} goto x1R2W;

loc f1R2W3W : while p1-D1<=0 & p2-D2<=0 & p3-D3<=0 wait { c1'=0, c2'=0, c3'=0}
    when p1-D1==0 do {} goto error;
    when p2-D2==0 do {} goto error;
    when p3-D3==0 do {} goto error;
    when true sync go do {} goto x1R2W3W;

loc error: while true wait {}

end

automaton server

sync: r1, r2, r3, empty, go, frozen;
loc idle: while true wait{x'=0, y'=0}
    when true sync r1 do { x'=0, y'=0 } goto active;
    when true sync r2 do { x'=0, y'=0 } goto active;
    when true sync r3 do { x'=0, y'=0 } goto active;
loc active: while y-x <= P-Q wait {x'=0}
    when true sync r1 do { } goto active;
    when true sync r2 do { } goto active;
    when true sync r3 do { } goto active;
    when true sync go do {} goto executing;
loc executing: while x <= Q wait {}
    when true sync r1 do { } goto executing;
    when true sync r2 do { } goto executing;
    when true sync r3 do { } goto executing;
    when x == Q sync frozen do {} goto recharging;
    when 5*x >= 3*y sync empty do {  } goto empty;
    when 5*x <= 3*y-1 sync empty  do { x'=0,y'=0 } goto idle;
    when x <= Q-1 sync frozen do { } goto active;
loc recharging: while y <= P wait {x'=0}
    when true sync r1 do { } goto recharging;
    when true sync r2 do { } goto recharging;
    when true sync r3 do { } goto recharging;
    when y == P do {x'=0,y'=0} goto active;
loc empty: while 5*x >= 3*y wait {x'=0}
    when true sync r1 do { } goto active;
    when true sync r2 do { } goto active;
    when true sync r3 do { } goto active;
    when 5*x == 3*y do {} goto idle;
end

init := 
    loc[sched] == idle & 
    loc[server] == idle & 
    p1>=0 & c1==0 &
    p2>=0 & c2==0 &
    p3>=0 & c3==0 & x==0 & y==0;
bad := loc[sched]==error;
