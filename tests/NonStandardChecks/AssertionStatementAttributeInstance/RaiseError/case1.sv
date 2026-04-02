module tb_bad_1;
    logic clk, req, ack;

    my_cover_label : cover property (
        @(posedge clk) req |-> ##[1:3] ack
    );

endmodule