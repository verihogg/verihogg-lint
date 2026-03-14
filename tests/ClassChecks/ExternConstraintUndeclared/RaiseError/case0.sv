class Packet;
  rand bit [7:0] length;
  constraint c_external;
endclass

constraint Packet::c_external { length > 0; }
