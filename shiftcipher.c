#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_SIZE     512
#define EN_ALPH_SIZE  26
#define EN_INDEX      0.0654967 /* English index of coincidence */

/* Function prototypes */
static int    analyzefrequencies(const double *freqcount);
static void   bruteforce(const char *m);
static void   dec(const char *m, int key);
static void   decrypt(const char *message);
static void   die(const char *errstr, ...);
static void   encrypt(char *message, int key);
static int    getinput(char **s, FILE *stream);
static double *letterfreq(const char *m);
static int    mod(int number, int modulus);
static char   shiftbackward(char c, int key);
static char   shiftforward(char c, int key);
static void   usage(void);
static void   validatekey(int key);
static void   *xcalloc(size_t nmemb, size_t size);
static void   *xmalloc(size_t size);
static void   *xrealloc(void *p, size_t len);

/* Globals */
static char   *argv0;       /* required by usage() */
static char   *plaintext;   /* where dec() stores the decrypted letters */
static double mindiff = INFINITY;

/* frequencies taken from norvig.com/mayzner.html and divided by 100 */
static const double en_letterprob[] = {
	/* a       b      c       d       e       f       g       h       i */
	0.0804, 0.0148, 0.0334, 0.0382, 0.1249, 0.0240, 0.0187, 0.0505, 0.0757,
 	/* j      k       l       m       n       o       p       q       r */
	0.0016, 0.0054, 0.0407, 0.0251, 0.0723, 0.0764, 0.0214, 0.0012, 0.0628,
	/* s      t       u       v       w       x       y       z */
	0.0651, 0.0928, 0.0273, 0.0105, 0.0168, 0.0023, 0.0166, 0.0009
};

void
bruteforce(const char *m){
	for(int k = 1; k < EN_ALPH_SIZE; k++){
		dec(m, k);
		puts(plaintext);
	}
}

void
encrypt(char *message, int key){
	validatekey(key);

	for(; *message != '\0' && *message != '\n'; message++){
		*message = shiftforward(*message, key);
	}

	*message = '\0'; /* erase '\n' if any */
}

void
validatekey(int key){
	if(key < 0 || key >= EN_ALPH_SIZE)
		die("Error: key %d invalid. The key must be in the range [0,%d]\n",
				key, EN_ALPH_SIZE - 1);
}

void
decrypt(const char *message){
	double *freqcount = letterfreq(message);
	int probablekey = analyzefrequencies(freqcount);
	free(freqcount);
	dec(message, probablekey);
}

double *
letterfreq(const char *m){
	int i;
	int msgsize = 0;
	double  *freqcount = xcalloc(EN_ALPH_SIZE, sizeof(*freqcount));

	for(; *m != '\0' && *m != '\n'; m++){
		i = isupper(*m) ? *m - 'A' : *m - 'a';
		freqcount[i]++;
		msgsize++;
	}

	for(i = 0; i < EN_ALPH_SIZE; i++)
		freqcount[i] /= msgsize;

	return freqcount;
}

int
analyzefrequencies(const double *freqcount){
	int i, k;
	double ic;       /* Index of coincidence */
	int probablekey; /* key guessed by frequency analysis */

	for(k = 1; k < EN_ALPH_SIZE; k++){
		ic = 0.0;
		for(i = 0; i < EN_ALPH_SIZE; i++){
			ic += en_letterprob[i] * freqcount[(i + k) % EN_ALPH_SIZE];
		}
		double diff = EN_INDEX - ic;
		if(diff < mindiff){
			mindiff = diff;
			probablekey = k;
		}
	}

	return probablekey;
}

void
dec(const char *m, int key){
	char *cp = plaintext;

	for(; *m != '0' && *m != '\n'; m++){
		*cp++ = shiftbackward(*m, key);
	}
	*cp = '\0';
}

char
shiftforward(char c, int key){
	if(isupper(c))
		return ((c - 'A' + key) % EN_ALPH_SIZE) + 'A';
	return ((c - 'a' + key) % EN_ALPH_SIZE) + 'a';
}

char
shiftbackward(char c, int key){
	if(isupper(c))
		return mod(c - 'A' - key, EN_ALPH_SIZE) + 'A';
	return mod(c - 'a' - key, EN_ALPH_SIZE) + 'a';
}

int
mod(int n, int m){
	int r = n % m;
	return r < 0 ? r + m : r;
}

void
usage(void){
	die("usage: %s (-e key | -d [key] | -b)\n", argv0);
}

void
die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void *
xrealloc(void *p, size_t len) {
	if((p = realloc(p, len)) == NULL)
		die("%s: could not realloc() %d bytes\n", argv0, len);

	return p;
}

void *
xmalloc(size_t size) {
	void *p = malloc(size);

	if(!p)
		die("%s: could not malloc() %d bytes\n", argv0, size);

	return p;
}

void *
xcalloc(size_t nmemb, size_t size) {
	void *p = calloc(nmemb, size);

	if(!p)
		die("%s: could not calloc() %d bytes\n", argv0, nmemb*size);

	return p;
}

/* read input from stream into s, return bytes read */
int
getinput(char **s, FILE *stream){
	int size = INIT_SIZE / 2;
	int read = 0;
	*s = NULL;

	for(;;){
		if(read + INIT_SIZE > size){
			size *= 2;
			*s = xrealloc(*s, size);
		}
		if(fgets(*s + read, INIT_SIZE, stream) == NULL)
			break;
		read += strlen(*s + read);
		if((*s)[read - 1] == '\n')
			break;
	}

	return read;
}

int
main(int argc, char *argv[]){
	unsigned long key;
	unsigned int msgsize;
	char *message;
	char *end;

	argv0 = *argv; /* Keep program name for usage() */

	if(argc < 2)
		usage();

	if(strcmp("-e", argv[1]) == 0){
		if(argc > 2){
			key = strtoul(argv[2], &end, 10);
			if(*end != '\0')
				usage();
			getinput(&message, stdin);
			encrypt(message, key);
			puts(message);
			free(message);
			return EXIT_SUCCESS;
		}
		usage();
	}

	msgsize = getinput(&message, stdin);
	plaintext = xmalloc(msgsize);
	if(strcmp("-d", argv[1]) == 0){
		if(argc > 2){
			key = strtoul(argv[2], &end, 10);
			if(*end != '\0')
				usage();
			validatekey(key);
			dec(message, key);
		} else {
			decrypt(message);
		}
		puts(plaintext);
	} else if(strcmp("-b", argv[1]) == 0){
		bruteforce(message);
	} else {
		usage();
	}

	free(plaintext);
	free(message);
	return EXIT_SUCCESS;
}
