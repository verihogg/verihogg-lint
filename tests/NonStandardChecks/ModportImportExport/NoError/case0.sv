interface smart_bus (input logic clk);
    logic [7:0] data;
    logic       valid;
    logic       ready;

    task automatic send_byte(input logic [7:0] byte_val);
        @(posedge clk);
        data  <= byte_val;
        valid <= 1'b1;
        @(posedge clk);
        valid <= 1'b0;
    endtask

    function automatic logic [7:0] get_data();
        return data;
    endfunction

    modport master (
        input  clk, 
        output data,
        output valid,
        input  ready,
        export send_byte,
        import get_data 
    );

    modport slave (
        input  clk,
        input  data,
        input  valid,
        output ready,
        import send_byte,
        export get_data 
    );

endinterface

// Модуль-мастер использует modport master
module bus_master (smart_bus.master bus);
    initial begin
        bus.send_byte(8'hFF);
    end
endmodule

// Модуль-слейв использует modport slave
module bus_slave (smart_bus.slave bus);
    logic [7:0] received;
    always_ff @(posedge bus.clk) begin
        if (bus.valid)
            received <= bus.get_data();
    end
endmodule

module top;
    logic clk = 0;
    always #5 clk = ~clk;

    smart_bus my_bus (.clk(clk));

    bus_master u_master (.bus(my_bus));
    bus_slave  u_slave  (.bus(my_bus));
endmodule