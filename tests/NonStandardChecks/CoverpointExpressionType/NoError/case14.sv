typedef enum bit [4:0] { GPR0, GPR1, GPR2 } reg_e;
typedef enum bit [1:0] { NOSIGN, POS, NEG } sign_e;

class instr_t;
  reg_e   rs1;
  sign_e  rs1_sign;
  bit [2:0] rd;
endclass

class processor_t;
  instr_t instr;

  function void build_instr_list();
    instr_t instr;
    instr = new();
  endfunction

  covergroup instr_cg with function sample(instr_t instr);
    cp_rs1      : coverpoint instr.rs1;
    cp_rs1_sign : coverpoint instr.rs1_sign;
    cp_rd       : coverpoint instr.rd;
  endgroup
endclass
