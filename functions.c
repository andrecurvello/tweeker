/* (C)2012 Simon Jackson
 * Tweeker Source - The synth for the tweeker */

#define name(x,fn)	{&x,fn}

float square(float x) {
	return x * x;
}

/* use initial estimate and y'=y*(3-x*y*y)/2 with iterations */
float irt(float x) {
    	// Watchdog triggers after 16 seconds when not cleared
	// So place here just in case any code uses intense calculaton.
#ifdef USE_WATCHDOG
    	WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK;
#else
    	WDTCTL = WDTPW + WDTHOLD;
#endif
	u32 i;
        float x2;
        const float threehalfs = 1.5F;
	x2 = x * 0.5F;
        i  = * ( u32 * ) &x;                       // evil floating point bit level hacking
        i  = 0x5f3759df - ( i >> 1 );
        x  = * ( float * ) &i;
	for(i = 0; i < 4; i++)
        	x  *= ( threehalfs - ( x2 * square(x) ) );   //iteration
        return x;
}

float inv(float x) {
	x = irt(x);
	return square(x);
}

float sqrt(float x) {
	return x * irt(x);
}

float half(float x, s8 sgn) {		/* x/(1+sqrt(1+x*x)) */
	return x * inv(1.0F + sqrt(1.0F + (sgn > 0 ? square(x) : -square(x))));
}

float halfa(float x) {
	return half(x, 1);
}

//cosine estimate 0 to pi = 0 to 1
float cost(float x) {
	s32 y = (s32)x;
	x -= (float)y;
	return (1.0F - 2.0F * x) * (x & 1 == 0 ? 1.0F : -1.0F);//triangle
}

//p and q range 0 to 1
float polyterm(float x, float p, float q) {
	float acc = 0.0F;
	float mul = 1.0F;
	float harm;
	float corr;
	u16 start;
	for(start = 1; start < 64; start ++) {
		harm = inv((float)start);
		//p as over fact(n) to n
		corr = 4.0F * p * (1.0F - p);//p correction (symmetric)
		mul *= x * (p + (1.0F - p) * harm + corr * ((1.0F - p) * harm - p)));
		acc += mul * ((1.0F - p) + p * harm + corr * (p * harm - (1.0F - p))) * cost(q * ((float)start - 1.0F));
        };
	return acc;
}

float log(float x) { //base e
	float sign = 0.0F;
	if(x == 1.0F) return 0.0F;
	if(x > 1.0F) {
		x = irt(x);
		sign = 1.0F;
	}
	x = irt(irt(irt(irt(x))));//symmetry and double roots
	return polyterm((x-1.0F), 1.0F, 1.0F) * (16.0F + sign);
}

float atan(float x) {
	return polyterm(half(half(x, 1), 1), 1.0F, 0.5F) * 4.0F;
}

float rel(float x) {
	return sqrt(1.0F - square(x));
}

float exp(float x) {
	return polyterm(x, 0.0F, 0.0F) + 1.0F;
}

float qfn(float x) {
	return polyterm(x, 0.5F, 0.0F);
}

float pfn(float x) {
	return polyterm(x, 0.5F, 0.5F);
}

float ein(float x) {
	return qfn(x) + log(x);
}

float lin(float x) {
	return ein(log(x));
}

//extra eight functions
//on root
float halfs(float x) {
	return half(x, -1);
}

float halfc(float x) {
	return rel(x) * inv(x + 1);
}
//on logs
float asin(float x) {
	return 2.0F * atan(halfs(x));
}

float acos(float x) {
	return 2.0F * atan(halfc(x));
}
//on exps
float sin(float x) {
	return polyterm(x, 0.0F, 0.5F);
}

float cos(float x) {
	return sin(x + 1.57079632679F);
}
//on xtra
float tan(float x) {
	return sin(x) * inv(cos(x));
}

//TODO:Add gamma iterate


//TODO:add function table of names and fnptr
struct token tokens[] = {
//name definitions
//basic 16 functions
name("ACS", acos),
name("ASN", asin),
name("ATN", atan),
name("COS", cos),
name("REL", rel),
name("EIN", ein),
name("EXP", exp),
name("INV", inv),
name("IRT", irt),
name("LIN", lin),
name("LOG", log),
name("PFN", pfn),
name("QFN", qfn),
name("SIN", sin),
name("SQR", sqrt),
name("TAN", tan)
//next 16 functions

//end name definitions
};

//TODO:add group definitions
