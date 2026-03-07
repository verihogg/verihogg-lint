class Packet;
  rand bit [7:0] length;
  extern constraint c_external; // no extern keyword
endclass

// Outside the class
constraint Packet::c_external { length > 0; }
