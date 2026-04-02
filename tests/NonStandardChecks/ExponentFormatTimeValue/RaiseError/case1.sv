`timescale 1ns/1ps

module uart_driver_bad (
    input  logic       clk,
    input  logic       rst_n,
    output logic       tx,
    input  logic [7:0] data,
    input  logic       send
);

    localparam real BAUD_PERIOD_NS = 1.0e6;

    typedef enum logic [1:0] {
        IDLE  = 2'b00,
        START = 2'b01,
        DATA  = 2'b10,
        STOP  = 2'b11
    } state_t;

    state_t       state;
    logic [7:0]   shift_reg;
    integer       bit_cnt;

    task automatic send_bit(input logic b);
        tx = b;
        #(1.0e6ns);
    endtask

endmodule
