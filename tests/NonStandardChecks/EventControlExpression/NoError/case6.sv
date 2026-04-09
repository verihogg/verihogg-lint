interface clk_if;
    logic clk;
    logic rst_n;
    logic enable;
endinterface

class my_driver;
    virtual clk_if vif;

    task wait_for_enable();
        @(posedge vif.enable);
        $display("enable asserted");
    endtask

    task wait_clk(int n = 1);
        repeat(n) @(posedge vif.clk);
    endtask

    task wait_reset_done();
        @(posedge vif.rst_n);
        $display("reset released at time %0t", $time);
    endtask

    task run();
        wait_reset_done();
        wait_clk(5);
        wait_for_enable();
    endtask
endclass

module tb_correct;
    clk_if dut_if();
    my_driver drv;

    initial begin
        dut_if.clk    = 0;
        dut_if.rst_n  = 0;
        dut_if.enable = 0;

        drv     = new();
        drv.vif = dut_if;

        fork
            forever #5 dut_if.clk = ~dut_if.clk;
            begin
                #20 dut_if.rst_n  = 1;
                #30 dut_if.enable = 1;
            end
            drv.run();
        join_any
        #100 $finish;
    end
endmodule