#include "expr.hpp"
#include "length.hpp"


Expr*
gather(Expr_seq const& subkeys)
{
  // maintain the largest allowable key buffer
  uint512_t buf = 0;

  // maintain the position to start writing
  int pos = 0;
  for (auto subkey : subkeys) {
    // get the precision of the subkey
    int prec = precision(subkey->type());
    // get the literal expression
    Literal_expr* e = as<Literal_expr>(subkey);
    Value const& val = e->value();
    // FIXME: for now we're only dealing with unsigned integer values
    assert(val.is_integer());
    Integer_value i = val.get_integer();
    // shift the integer over by the amount already written
    i = i << pos;
    // add the length of the current integer to the pos
    pos += prec;
    // log-and the integer into the buffer
    buf &= i;
  }

  std::cout << "gather: " << buf << '\n';

  return nullptr;
}
