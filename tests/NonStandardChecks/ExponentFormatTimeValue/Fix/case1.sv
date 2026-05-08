`timescale 1ns/1ps

module uart_driver_bad (
    input  logic       clk,
    input  logic       rst_n,
    output logic       tx,
    input  logic [7:0] data,
    input  logic       send
);

    localparam real BAUD_PERIOD_NS = 1.0e6;

    task automatic send_bit(input logic b);
        tx = b;
        #(1.0e6ns);
    endtask

endmodule
