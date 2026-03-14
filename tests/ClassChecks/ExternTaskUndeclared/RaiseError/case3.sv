class Parent;
    // Declare the virtual task as extern
    virtual task run();
endclass

class Child extends Parent;
endclass

// Define the extern virtual task for the Child class
task Child::run();
    $display("Child running");
endtask

// Test Module to invoke the run task
module Test;
    initial begin
        Child childObj = new();
        childObj.run(); // Outputs: "Child running"
    end
endmodule
