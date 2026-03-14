// Missing extern declaration in package scope
package test_pkg;
    class Randomizer;
       rand real value;
       constraint missing_extern;
    endclass // Randomizer
endpackage

constraint test_pkg::Randomizer::missing_extern {
    value > 0.0;
    value < 1.0;
}
