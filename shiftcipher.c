#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "arg.h"

static char *argv0; /* required by usage() */

#define NELEMS(array) (sizeof(array)/sizeof(array[0]))
#define INIT_SIZE 1024
#define EN_ALPH_SIZE 26
#define EN_INDEX 0.0654967 /* English index of coincidence */

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

/* Globals */
const double en_letterfreq[] = {
	/* a      b       c       d       e        f       g       h       i */
	8.167,  1.492,  2.782,  4.253,  12.702,  2.228,  2.015,  6.094,  6.966,
	/* j      k       l       m       n        o       p       q       r */
	0.153,  0.772,  4.025,  2.406,   6.749,  7.507,  1.929,  0.095,  5.987,
	/* s      t       u       v       w        x       y       z         */
	6.327,  9.056,  2.758,  0.978,   2.360,  0.150,   1.974,  0.074
};
static char *bestguess; /* Most probable correct decryption */
static char *plaintext; /* where dec() stores the deciphered letters */
static double freqcount[EN_ALPH_SIZE];
static double mindiff = INFINITY;

int
main(int argc, char *argv[]){
	unsigned long key;
	char fixme[INIT_SIZE] = {0}; /* TODO malloc for arbitrary size string! */

	argv0 = *argv; /* Keep program name for usage() */

	if(argc < 2)
		usage();

	fgets(fixme, INIT_SIZE, stdin);

	plaintext = malloc(INIT_SIZE);
	bestguess = malloc(INIT_SIZE);
	ARGBEGIN{
	case 'd':
		if(argc > 1){
			key = strtoul(argv[1], NULL, 10);
			validatekey(key);
			dec(fixme, key);
			puts(plaintext);
		} else {
			decrypt(fixme);
			puts(bestguess);
		}
		break;
	case 'b':
		bruteforce(fixme);
		break;
	case 'e':
		if(argc > 1){
			key = strtoul(argv[1], NULL, 10);
			validatekey(key);
			encrypt(fixme, key);
			puts(fixme);
		}
		break;
	default:
		usage();
	}ARGEND;

	free(plaintext);
	free(bestguess);

	return EXIT_SUCCESS;
}

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
		mindiff = diff;
		temp = bestguess;
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
