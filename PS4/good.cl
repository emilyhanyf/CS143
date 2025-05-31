Class Order {
	a : Int <- 1;
	b : Int <- 2;
	c : Int <- 3;
};

Class Breski {
	erer : Int <- let j : Int <- 5 in {let k : Int <- 5 in {let l : Int <- 5 in {j + k + l;};};};
};

Class Bro1 {
	a : Int <- 1;
};

Class Bro2 {
	a : Int <- 1 + 2;
};

Class Bro3 {
	a : Int;
	b : Int <- 1 + a;
};

Class C {
	f : Int;
	a : Int <- 3 + 2 + f;
	b : Bool;
	erer : Int <- let j : Int <- 5 in {let k : Int <- 5 in {let l : Int <- 5 in {j + k + l;};};};
	c : String;
	init(x : Int, y : Bool) : C {
           {
		a <- x + 3 + 2  + f;
		b <- y;
		self;
           }
	};
};

Class B inherits C {
	l : Int;
	p : Int <- 1;
	init(l : Int, p : Bool) : C {
           {
			let jit : Int <- 2 in {
				a <- l + jit;
			};
		b <- p;
		self;
           }
	};
	bro() : Int {
		1
	};
};

Class F {
	init() : String {
		"string"
	};
};

Class D inherits B {
	init(l : Int, p : Bool) : C {
           {
		a <- l;
		b <- p;
		self;
           }
	};
};

Class Main {
	main():C {
	  (new C).init(1,true)
	};
};
