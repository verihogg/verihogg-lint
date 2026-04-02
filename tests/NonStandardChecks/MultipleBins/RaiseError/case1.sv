module tb_incorrect_1;

    logic clk;
    logic [1:0] data;

    initial clk = 0;
    always #5 clk = ~clk;

    initial begin
        data = 2'b00;
        repeat(10) begin
            @(posedge clk);
            data = $urandom_range(0, 3);
        end
        $finish;
    end

    covergroup cg_bad_1 @(posedge clk);

        cp_data: coverpoint data {

            wildcard bins trans_bad = ({1'bx} => {1'bx});

        }

    endgroup

    cg_bad_1 cg_inst = new();

endmodule