// Package scope undeclared extern task
package test_pkg;
    class Monitor;
        int counter;
        task monitor();
    endclass
endpackage

task test_pkg::Monitor::monitor();
    test_pkg::Monitor.counter++;
    #1;
endtask
