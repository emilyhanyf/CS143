Class C {
	f : Int;
	a : Int <- 3 + 2 + f;
	b : Bool;
	c : String;
	init(x : Int, y : Bool) : C {
           {
		a <- x;
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
		a <- l;
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
