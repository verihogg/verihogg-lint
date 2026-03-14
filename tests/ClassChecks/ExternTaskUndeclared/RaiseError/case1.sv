// Outer task not declared extern in class
class MyTask;
   bit done;
   task execute();
endclass

task MyTask::execute();
    done = 1'b1;
    #10;
    done = 1'b0;
endtask
