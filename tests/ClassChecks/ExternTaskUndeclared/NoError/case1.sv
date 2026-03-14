// Properly declared extern task
class TimedRun;
    bit finished;
    
    extern task run_timed(int duration);
endclass

task TimedRun::run_timed(int duration);
    repeat(duration) #1;
    finished = 1'b1;
endtask
