// Nested class task not declared extern
class Outer;
    class Inner;
       int flag;
       task do_work();
    endclass
endclass

task Outer::Inner::do_work();
    flag = 1'b1;
    #5;
    flag = 1'b0;
endtask
