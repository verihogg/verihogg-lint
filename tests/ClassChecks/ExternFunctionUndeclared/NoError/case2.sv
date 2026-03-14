// Multiple extern functions properly declared
class Logger;
    bit enabled;
    
    extern function void log_info(string msg);
    extern function void log_error(string msg);
endclass

function void Logger::log_info(string msg);
    $display("INFO: %s", msg);
endfunction

function void Logger::log_error(string msg);
    $display("ERROR: %s", msg);
endfunction
