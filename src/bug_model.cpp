#include "model.hpp"


//void Model::BUG() {
//  Valuations param_vars;
//  for ( auto &x : parameters) {
//    param_vars[x.name] = x.min;
//  }
//  Valuations old = dvars;
//  dvars.insert(params_vars.begin(), params.end());
//  check_consistency();
//  dvars = old;
//  // The above part is only for consistency check?
//  // I really forget ...
//
//  // The param space is first divided into a set of integer points
//  vector<Valuations> points, passed_points;
//
//  // to initialise the reference point
//  Valuations v;
//  for ( auto & x : parameters)
//    v.insert(pair<string, int>(x.name, x.min));
//
//  while (points.size() != 0) {
//    auto x = points.at(0);
//    Bad.clear();
//    Unknown.clear();
//    Good.clear();
//
//    if ( bug_in_a_tile(x)) {
//      passed_points.push_back(points.at(0));
//      vector<Valuations> next_points = increase_by_one_step(x);
//      points.erase(points.begin());
//      
//      for (auto &y : next_points) {
//        if( (not existing_point(y, points)) and ( not existing_point(y, passed_points))
//            points.push_back(y);
//      }
//      continue;
//    }
//
//    BUG(x);
//
//    // Sadly, we simply copy the above codes inside if condition and paste them here
//    passed_points.push_back(points.at(0));
//    vector<Valuations> next_points = increase_by_one_step(x);
//    points.erase(points.begin());
//      
//    for (auto &y : next_points) {
//      if( (not existing_point(y, points)) and ( not existing_point(y, passed_points))
//        points.push_back(y);
//        }
//        }
//
//
//  }
//
//
//
//}
