// Multiple levels of undefined inheritance
class Level1 extends UndefinedL0;
    int x;
endclass

class Level2 extends UndefinedL1;
    string y;
endclass

class Level3 extends UndefinedL2;
    bit z;
endclass
