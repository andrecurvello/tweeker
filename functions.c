/* (C)2012 Simon Jackson
 * Tweeker Source - The synth for the tweeker */

#define name(x,fn)	{&x,fn}

//TODO: use symetries for speed, and signed integers

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
        i  = * ( long * ) &x;                       // evil floating point bit level hacking
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
	return x * inv(1.0F + sqrt(1.0F + (sgn > 1? square(x) : -square(x))));
}

float halfa(float x) {
	return half(x, 1);
}

//TODO:make using 3 param generator.

float eq(float x, s8 over, s8 sq, s8 alt, s8 fact) { //base e exponential and Q+
	float acc = 0;
	float lacc;
	float mul = x;
	float harm = 1;
	u16 start = 1;
	if(sq != 0) x = square(x);
	x = (alt != 0 ? -x : x);
	do {
		lacc = acc;
		acc += mul * (over == 0 ? 1.0F : harm);
		start += sq + 1;
		harm = inv((float)start);
		mul *= x * (fact == 0 ? 1.0F : harm * (sq == 0 ? 1.0F : inv(start - 1)));
        } while(lacc != acc && start < 200);//term limit
	return acc;
}

float log(float x) { //base e
	x = irt(irt(irt(x)));//symetry and double triple roots
	return -eq((x-1.0F) * inv(x+1.0F), 1, 1, 0, 0) * 16.0F;
}

float atan(float x) {
	return eq(half(half(x, 1), 1), 1, 1, 1, 0) * 4.0F;
}

float circ(float x) {
	return sqrt(1.0F - square(x));
}

float exp(float x) {
	return eq(x, 0, 0, 0, 1) + 1.0F;
}

float qfn(float x) {
	return eq(x, 1, 0, 0, 1);
}

float invw(float x) {
	return x * exp(x);
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
	return circ(x) * inv(x + 1);
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
	return eq(x, 0, 1, 1, 1);
}

float cos(float x) {
	return sin(x + 1.57079632679F);
}
//on xtra
float tan(float x) {
	return sin(x) * inv(cos(x));
}

float entropy(float x) {
	return x * log(inv(x)) * 1.44269504089F;//base 2
}

//TODO:Add gamma iterate

//TODO:add function table of names and fnptr
struct token tokens[] = {
//name definitions
name("COS", cos),
name("SIN", sin)
//end name definitions
};
