typedef enum bit [2:0] { S_IDLE, S_REQ, S_ACK, S_DONE } state_e;
typedef bit [7:0] byte_t;

module cg_typedef_integral;
  covergroup cg with function sample(state_e st, byte_t data);
    cp_st   : coverpoint st;
    cp_data : coverpoint data;
  endgroup
endmodule
