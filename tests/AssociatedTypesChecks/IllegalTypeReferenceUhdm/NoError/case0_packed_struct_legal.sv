typedef logic [7:0] byte_t;
typedef bit [3:0] nibble_t;

typedef struct packed {
  byte_t a;
  nibble_t b;
  int i;
} good_struct_t;