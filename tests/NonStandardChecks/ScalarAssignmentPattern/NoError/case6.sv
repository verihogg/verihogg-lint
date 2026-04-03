module test7;
    typedef struct { bit f; } my_struct_t;
    my_struct_t s;
    initial begin
        s = '{f:1'b1};
    end
endmodule