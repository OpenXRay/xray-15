	// Protocol for BigMatrix class
	
	use_generic( get,			"get");
	use_generic( put,			"put");	
	use_generic( identity,		"identity");	//Should actually be mapped_generic
	use_generic( plus,			"+");

	def_visible_generic( invert,		"invert");	
	def_visible_generic( transpose,		"transpose");
	def_visible_generic( clear,			"clear");
	def_visible_generic( setSize,		"setSize");

	
