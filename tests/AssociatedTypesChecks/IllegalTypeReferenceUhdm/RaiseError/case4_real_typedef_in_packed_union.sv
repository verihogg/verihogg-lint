typedef real my_real_t;

typedef union packed {
  my_real_t x;
  int i;
} bad_union_t;