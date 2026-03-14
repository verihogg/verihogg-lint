// Typo in interface name
interface class Processor;
    pure virtual function void process();
endclass

class MyProcessor implements Processer;
    function void process();
        $display("Processing");
    endfunction
endclass
