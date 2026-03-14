// Extending from package where class doesn't exist
package other_pkg;
    class Helper;
        int id;
    endclass
endpackage

class Processor extends other_pkg::NonExistentClass;
    bit enabled;
endclass
