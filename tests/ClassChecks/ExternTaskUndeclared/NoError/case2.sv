// Multiple extern tasks properly declared
class Driver;
    bit ready;
    
    extern task initialize();
    extern task drive(bit [31:0] value);
endclass

task Driver::initialize();
    ready = 1'b0;
    #10;
    ready = 1'b1;
endtask

task Driver::drive(bit [31:0] value);
    $display("Driving: %h", value);
    #5;
endtask
