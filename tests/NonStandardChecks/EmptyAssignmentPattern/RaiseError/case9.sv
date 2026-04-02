module test6;
  typedef struct {
    logic a;
    logic b;
  } inner_t;

  typedef struct {
    inner_t in;
    logic c;
  } outer_t;

  outer_t o;

  initial begin
    o = '{}; 
  end
endmodule