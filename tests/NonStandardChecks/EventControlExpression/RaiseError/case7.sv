interface clk_if;
    logic clk;
    logic rst_n;
    logic enable;
endinterface

class my_driver;
    virtual clk_if vif;

    task run();
        @(vif)
        $display("triggered");
    endtask
endclass