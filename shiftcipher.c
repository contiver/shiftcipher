#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *argv0; /* required by usage() */

#define NELEMS(array) (sizeof(array)/sizeof(array[0]))
#define INIT_SIZE     1024
#define EN_ALPH_SIZE  26
#define EN_INDEX      0.0654967 /* English index of coincidence */

/* Function prototypes */
static void decrypt(const char *message);
static void freqdec(const char *m, int key);
static void dec(const char *m, int key);
static void encrypt(char *message, int key);
static char shiftforward(char c, int key);
static char shiftbackward(char c, int key);
static void resetfrequencies(void);
static void analyzefrequencies(void);
static void bruteforce(char *m);
static void validatekey(int key);
static int  mod(int number, int modulus);
static void die(const char *errstr, ...);
static void usage(void);
static int getinput(char **s, FILE *stream);

/* Globals */
static char   *bestguess; /* most likely correct decryption */
static char   *plaintext; /* where dec() stores the deciphered letters */
static double freqcount[EN_ALPH_SIZE];
static double mindiff = INFINITY;

/* frequencies taken from norvig.com/mayzner.html */
static const double en_letterfreq[] = {
	/* a     b       c       d       e       f       g        h      i */
	8.04,   1.48,   3.34,   3.82,  12.49,   2.40,   1.87,   5.05,   7.57,
	/* j     k       l       m        n      o       p        q      r */
	0.16,   0.54,   4.07,   2.51,   7.23,   7.64,   2.14,   0.12,   6.28,
	/* s     t       u       v        w      x       y       z */
	6.51,   9.28,   2.73,   1.05,   1.68,   0.23,   1.66,   0.09
};

void
bruteforce(char *m){
	int k;

	for(k = 1; k < EN_ALPH_SIZE; k++){
		dec(m, k);
		puts(plaintext);
	}
}

void
usage(void){
	die("usage: %s [-e key] [-d (key)] [-b]\n", argv0);
}

void
encrypt(char *message, int key){
	for(; *message != '\0' && *message != '\n'; message++){
		*message = shiftforward(*message, key);
	}
	*message = '\0'; /* erase \n if any */
}

void
validatekey(int key){
	if(key <= 0 || key >= EN_ALPH_SIZE)
		die("Error: key %d invalid. The key must be in the range [1,%d]\n",
				key, EN_ALPH_SIZE);
}

void
decrypt(const char *message){
	int k;

	for(k = 1; k < EN_ALPH_SIZE; k++){
		resetfrequencies();
		freqdec(message, k);
		analyzefrequencies();
	}
}

void
resetfrequencies(void){
	unsigned int i;
	for(i = 0; i < NELEMS(freqcount); i++)
		freqcount[i] = 0;
}

void
analyzefrequencies(void){
	int i;
	double ic = 0; /* Index of coincidence */

	for(i = 0; i < EN_ALPH_SIZE; i++)
		freqcount[i] /= 100.0;
	for(i = 0; i < EN_ALPH_SIZE; i++)
		ic += freqcount[i] * en_letterfreq[i];

	double diff = EN_INDEX - ic;
	if(diff < mindiff){
		char *temp;
		mindiff   = diff;
		temp      = bestguess;
		bestguess = plaintext;
		plaintext = temp;
	}
}

void
dec(const char *m, int key){
	char *cp = plaintext;

	for(; *m != '0' && *m != '\n'; m++){
		*cp++ = shiftbackward(*m, key);
	}
	*cp = '\0';
}

/* Same as dec, but counting letter frequency in between */
void
freqdec(const char *m, int key){
	int i;
	char *cp = plaintext;

	for(; *m != '0' && *m != '\n'; m++){
		*cp = shiftbackward(*m, key);
		i = isupper(*cp) ? *cp - 'A' : *cp - 'a';
		freqcount[i]++;
		cp++;
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
die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

int
getinput(char **s, FILE *stream){
	int size = INIT_SIZE/2;
	int read = 0;
	*s = NULL;

	for(;;){
		size *= 2;
		*s = realloc(*s, size);
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
	char *message;
	int msgsize;
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
			validatekey(key);
			encrypt(message, key);
			puts(message);
			return EXIT_SUCCESS;
		}
		usage();
	}

	if(strcmp("-d", argv[1]) == 0){
		if(argc > 2){
			key = strtoul(argv[2], &end, 10);
			if(*end != '\0')
				usage();
			validatekey(key);
			msgsize = getinput(&message, stdin);
			plaintext = malloc(msgsize+1);
			dec(message, key);
			puts(plaintext);
		} else {
			msgsize = getinput(&message, stdin);
			bestguess = malloc(msgsize+1);
			plaintext = malloc(msgsize+1);
			decrypt(message);
			puts(bestguess);
		}
	} else if(strcmp("-b", argv[1]) == 0){
		msgsize = getinput(&message, stdin);
		plaintext = malloc(msgsize+1);
		bruteforce(message);
	} else {
		usage();
	}

	return EXIT_SUCCESS;
}
