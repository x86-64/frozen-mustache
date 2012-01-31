This is glue code for mustache support in frozen.

Installation
-----------------
1. Compile and install frozen
2. Compile and install mustache-c
3. Download or clone this repo
4. ./configure --prefix=/usr
5. make
6. make install


Usage
-----------------
	
	{
		     class                   = "modules/mustache",
		     template                =                                      # template data
					       (file_t){ ... }                      #  - from file
					       (machine_t){ ... }                   #  - from machine
					       ...
		     },
		     output                  = (hashkey_t)"buffer"                  # output result to, default "output"
	}
