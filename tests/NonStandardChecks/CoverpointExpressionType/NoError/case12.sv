typedef enum bit [1:0] { LOW, MID, HIGH } priority_e;
typedef enum bit [2:0] { NO_HAZ, RAW_HAZ, WAW_HAZ } hazard_e;

class instr_t;
  priority_e       prio;
  hazard_e         hazard;
  bit [3:0]        count;
endclass

module cg_class_member_access;
  covergroup cg with function sample(instr_t instr);
    cp_prio   : coverpoint instr.prio;
    cp_hazard : coverpoint instr.hazard;
    cp_count  : coverpoint instr.count;
  endgroup
endmodule
