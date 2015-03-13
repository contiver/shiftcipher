/* use main(int argc, char *argv[]) */
#define ARGBEGIN	for (argv++, argc--;\
					argv[0] && argv[0][1] && argv[0][0] == '-';\
					argc--, argv++) {\
						char **argv_;\
						if (argv[0][1] == '-' && argv[0][2] == '\0') {\
							argv++;\
							argc--;\
							break;\
						}\
						for (argv[0]++, argv_ = argv; argv[0][0]; argv[0]++) {\
							if (argv_ != argv)\
								break;\
							switch (argv[0][0])

#define ARGEND			}\
			}
