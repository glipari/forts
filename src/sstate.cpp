#include "sstate.hpp"

bool sstate::contains(const sstate &ss) const
{
  return cvx.contains(ss.cvx);
}
