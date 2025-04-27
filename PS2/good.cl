class A {
ana(): Int {
(let x:Int <- 1 in 2)+3
};
}; 

Class BB__ inherits A {
};

(* Class *)
class Empty { };
class Empty2 { };
class Simple inherits Empty { };
class Complex inherits Empty {
    a: Int;
    b: String;
}; 

(* Feature *)
class Feature {
    a: Int;
    b: Int;
    c: String;
    d: Int <- 123;
    e: String <- "Hello";
};

(* Math *)
class Math {
    a: Int <- 1+2*3/100*29;
    b: Int <- (1+2)*3;
};