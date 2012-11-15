/* (C)2012 Simon Jackson
 * Tweeker Source - The synth for the tweeker */

//follows some copied stuff of my own for templating the synth
//Maybe split into other files later






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

//OSAF FN (flags and function produced)
//0000
//0001 expm1
//0010
//0011 expm1(ix)
//0100
//0101 sinh
//0110
//0111 sin
//1000
//1001 qfn
//1010
//1011 
//1100 log with right input transform (is atanh)
//1101
//1110 atan
//1111

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

float split_mod;

float split_num(float x) {
	s16 i = (s32)x;
	split_mod = (float)(i % 100);
	return (float)(i / 100);
}

float mul(float x) {
	return split_num(x) * split_mod;
}

float div(float x) {
	return split_num(x) * inv(split_mod);
}

float harm(float x) {
	float t = split_num(x);
	return t * split_mod * inv(t + split_mod);
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




/* (C)2012 K Ring Technologies/People Android.
 * Author: Simon Jackson, BEng. (all files)
 * 
 * A state variable filter implementation of modified 1/f
 * pink noise generation. This uses the fact that a
 * double integration of a squared spectrum then rooted
 * is approximate to a single integral.
 */
package uk.co.peopleandroid.m24;

import java.util.Random;

public class Notes {

	final Random rand = new Random();
	final int[] data = new int[256];
	final int shift = 16;
	final int base = 1 << shift;
	final int half = base / 2;
	final char[] root = new char[base];
	int[] sin;
	char[] dials;
	final char[] spectrum = new char[256];
	
	public Notes(int[] sinTable) {
		sin = sinTable;
		int i;
		rand.setSeed(666);//known starting condition
		for(i = 0; i < 256; i++) {
			data[i] = (int)(rand.nextGaussian() * base);
		}
		for(i = 0; i < base; i++) {
			root[i] = (char)(Math.sqrt(i) * 128);
		}
	}
	
	public char[] calc(char[] dialsArray) {
		dials = dialsArray;//0 to 7
		reset();
		int i, j, k;
		for(i = 0; i < 256; i++) {
			filtIn = 0;
			j = data[i];
			k = data[255 - i];
			filtIn += ( ((i * j) >>> shift) * (dials[3] - half) ) >>> shift;
			j = (j < 0)?-(j * j) >>> shift:(j * j) >>> shift;
			filtIn += (j * (dials[2] - half) ) >>> shift;
			k = (k < 0)?-(k * k) >>> shift:(k * k) >>> shift;
			filtIn += (k * (dials[4] - half) ) >>> shift;
			filter();
			if(filtOut < 0) spectrum[i] = (char)(half - root[(char)filtOut]);
			else spectrum[i] = (char)(half + root[(char)filtOut]);
		} 
		//return an un tempo'd and un volumed spectrum
		return spectrum;
	}
	
	int f, q, b, h;
	int filtOut, e1, e2;
	int filtIn;
	
	void reset() {
		e1 = e2 = 0;
	}
	
	void filter() {
		//state variable filter
		f = sin[dials[5]];
		q = (dials[6] >> 2);
		//32k range
		if(q < 0) q = 0;//it's inverse q actually!!
		if(q > base / 2) q = base / 2;
		q = (base / 2 - q) << 2;
		filtOut = e2 + ((f * e1) >>> shift);
		h = filtIn - filtOut - ((q * e1) >>> shift);
		b = ((f * h) >>> shift) + e1;
		e1 = b;
		e2 = filtOut;
	}
}




/* This file is (C)2012 Simon Jackson, BEng.
 * The PM cube shape algorithm (PMCS) is unique
 * and constitutes the main thrust of the
 * copyright requirement. To this end copying
 * the oscillate method is bad. Delete it and
 * make your own character. Open does not mean free.
 * 
 * The dial config
 * ===============
 * 0-7 Note Sequence
 * 8-15 Ratio
 * 16-23 Depth
 * 24 Tempo
 * 25 Pitch
 * 26-28 Oscillator Shaper
 * 29 Sweep
 * 30 Resor
 * 31 Volume
 * 
 * Most dials have a +-1 octave range on tuning. The depth on the oscillator
 * has a little to large range with no centre. The filter sweep has a +-2
 * octave range, but total filter range is 8 octaves. This may clip the sweep
 * at extream settings. The q range of the resor is from 1/2 to infinity.
 * Clipping is possible at high q. Volume envelope modulation is partial.
 * 
 * A natural start position is all dials centred (almost), with ratios
 * slightly right of centre. Set volume bottom right, and tempo bottom left.
 * Make a nice tune on the top row, and fiddle with all the other dials.
 */

package uk.co.peopleandroid.m24;

import uk.co.peopleandroid.renderadapter.AbstractControl;
import uk.co.peopleandroid.renderadapter.RenderAdapter;

public class M24 extends RenderAdapter {
	
	final static int shift = 16;
	final static int base = 1 << shift;
	final static int ratio = 24;
	final static int noteBase = 440;
	final static int sampleRate = 44100;
	final static int lfoHz = 2;
	final static int noteSpread = base/96;
	
	final static int[] exp = new int[base];//frequency map
	final static int[] sin = new int[base];//filter frequency map
	final static int[] iharm = new int[ratio];//harmonic ratios
	int noteOn;
	
	final static double harm[] = {
			2.0/23, 2.0/21, 2.0/19, 2.0/17, 2.0/15, 2.0/13, 2.0/11, 2.0/9,
			2.0/7, 2.0/5, 1.0/2, 2.0/3, 1, 3.0/2, 5.0/2, 7.0/2,
			9.0/2, 11.0/2, 13.0/2, 15.0/2, 17.0/2, 19.0/2, 21.0/2, 23.0/2
	};
	
	public void note(byte on) {
		if(on < 128) {
			on -= 69;//A440 offset
			noteOn = (base>>1) + (on * noteSpread);
			while(noteOn < 0) noteOn += noteSpread << 2;//1 octave
			while(noteOn > base) noteOn -= noteSpread << 2;//1 octave
		}
	}
	
	long tick = 0;
	long innerTick = 0;
	long innerLast = 0;
	final int oneTick = base/96; 
	
	public void tick() {
		tick++;
	}

	long pitchSpeed, pitchCount;
	final static long pitchBase = base * noteBase / sampleRate;//speed for 440Hz
	
	long tempoTemp, tempoSpeed, tempoCount;//lfo locking (use instead of samplCount)
	long tempoCtl;
	final static long tempoBase = base * lfoHz / sampleRate;//lfo at 2Hz
	
	int noteCount, noteVol, lastNote;
	
	//shuffle, pitchbend, sweep, res, env
	final static int[] lfoDo = { 8, 9, 13, 14, 15 };
	final int[] lfoVal = new int[5];
	
	int scaleCentre(int value) {
		return ((value - (base/2)) >>> 2) + (base/2); 
	}
	
	void calcStatics() {
		//calculate static context based on dial positions
		pitchSpeed = (pitchBase * exp[scaleCentre(dials[25])]) >> shift;//+-1 octave
		tempoSpeed = (tempoBase * exp[dials[24]]) >> shift;
		if(!controls.stopped) tempoCount += tempoSpeed;
		innerTick = (int) (tempoCount * 96 >> shift);//tick
		if(innerTick != innerLast) {
			controls.tick();
		}
		if(tick != 0) {//No MIDI ticks yet
			//synchronous tick
			long error = (innerTick - tick);
			error %= 96;//modulo matching
			tempoCount -= error * oneTick;
		}
		for(int i = 0; i < 5; i++) {
			int speed = (iharm[ (dials[lfoDo[i]]*ratio) >> shift ]);//ratio
			tempoTemp = tempoCount * speed;
			lfoVal[i] = (int) (((tempoTemp * (dials[lfoDo[i]+8]-(base >> 1)))
					>>> (shift + 1)) & (base-1));
			//now have lfo counters +-1 octave
		}
		//apply tempo jitter lfo 0
		tempoTemp = tempoCount + lfoVal[0];//index lfo env, for easy sync
		noteCount = (int) (tempoTemp >> shift) & 15;//note
		//synchronous pattern update
		if(noteCount == 0 && lastNote != noteCount) {
			dials = controls.sync();
		}
		lastNote = noteCount;
		//tempo index into note sequence
		noteCount = (noteCount * exp[dials[1]]) >> shift;
		//calc note index
		pitchSpeed = (pitchSpeed * exp[lfoVal[1]+(base >> 1)]) >> shift;//lfo pitch
		pitchSpeed = (pitchSpeed * exp[scaleCentre(spectrum[noteCount])]) >> shift;
		pitchSpeed = (pitchSpeed * exp[noteOn]) >> shift;//MIDI tune
		if(!controls.stopped) pitchCount += pitchSpeed;
		//+- 1 octave on tune
		//total pitch = master+notepot+lfo IMP +- 3 octaves
		//@55Hz with sub from ratio timbre
		//calc volume envelope
		//but first do musical note length
		tempoCtl = exp[spectrum[255 - (noteCount * exp[dials[0]]) >> shift]];
		//add in some extra notes
		tempoTemp *= (tempoCtl + exp[dials[7]]) >> shift;
		if(tempoTemp > 0) noteVol = (int) (base - 1 - (tempoTemp & (base-1)));
		else noteVol = 0;
	}
	
	int oscOut, noteScale, sq;
	
	void oscillate() {
		//the oscillator generator
		//sampPreVol is modulation
		oscOut = 0;
		for(int i = 0; i < 3; i++) {
			noteScale = (int) (pitchCount * (iharm[ (dials[10+i]*ratio) >> shift ]));//ratio
			noteScale += (filtOut * (exp[dials[18+i]])) >>> shift;//depth
			//this gives a modulation index from about a 1/16 to 16
			//not including Q res gain
			noteScale &= (base - 1);//limit to saw
			noteScale += noteScale - base - 1;//2* and centre
			//apply cubic volume reduce and plus harmonics
			sq = ((noteScale * noteScale) >>> shift) - base;
			sq = ((sq * dials[26+i]) >>> shift) + base;
			sq = (sq * noteScale) >>> shift;//normed function
			oscOut += sq;//sum
		}
	}
	
	int l, b, hpfOut, d2, d1;
	final static int setF = 12;
	
	void hpf() {
		//filter out DC
		//it's polite to, and offset may introduce xtra
		l = d2 + (d1 >>> setF);
		hpfOut = oscOut - l - d1;
		b = (hpfOut >>> setF) + d1;
		d1 = b;
		d2 = l;
	}
	
	//sound about 3.0 level here
	
	int filtOut, e2, e1;
	int q, f, h;
	
	void filter() {
		//state variable filter
		f = (int) (lfoVal[2] + (dials[29] - (base/2)) + (pitchCount & (base-1)));//all filter freq factors
		//this could be upto +- 6 octaves, but limit to 4!!
		if(f < 0) f = 0;
		if(f >= base) f = base - 1;
		f = sin[f];
		q = lfoVal[3] + (dials[30] >> 2);
		//32k range
		if(q < 0) q = 0;//it's inverse q actually!!
		if(q > base / 2) q = base / 2;
		q = (base / 2 - q) << 2;
		filtOut = e2 + ((f * e1) >>> shift);
		h = hpfOut - filtOut - ((q * e1) >>> shift);
		b = ((f * h) >>> shift) + e1;
		e1 = b;
		e2 = filtOut;
	}
	
	//3.0+ level here
	
	int clip;
	
	void distort() {
		//round and clip signal
		clip = (filtOut >>> 2);//3 notes
		//1.0- level here
		clip = ((noteVol + lfoVal[4]) * clip) >>> shift;//apply envelope
		//either quiet- or louder+
		//clipping via lfo or q resor makes peak
		//hard clip!! NOT in feedback as would be OTT
		if(clip >= base) clip = base - 1;
		if(clip <= -base) clip = -base + 1;
	}
	
	void envelope() {
		//apply volume
		outputToJack = (short) ((clip * dials[31]) >>> (shift + 1));//extra bit for double pkpk 
	}
	
	short outputToJack;
	
	//=======================================================
	
	/* The following code engages the above code to provide
	 * sound output rendered via calling the above functional
	 * units of the M24. It can be used in general synth
	 * design, and makes no complicated dependencies.
	 */
	
	void makePow() {
		//frequency scaling
		for(int i = 0; i < base; i++) {
			//a four octave range plus or minus
			exp[i] = (int) (Math.pow(2, (i - (base/2)) * 4 / (base/2) ) * base);
		}
		//harmonic ratios
		for(int i = 0; i < ratio; i++)
			iharm[i] = (int) (harm[i] * base);
		double f;
		for(int i = 0; i < base; i++) {
			f = (noteBase * exp[i]);
			//filter mapping of note to play
			//is sin of an exp as to have musical scale tracking
			sin[i] = (int) (2 * Math.sin(Math.PI * f/sampleRate) * base);
		}
	}
	
	char[] dials;
	AbstractControl controls;
	
	boolean running = true;
	Notes n;
	
	public M24() {
		makePow();
		n = new Notes(sin);//add note spectrum generator
	}
	
	public void resetControl(AbstractControl ac) {
		tick = 0;
		innerTick = 0;
		innerLast = 0;
		tempoCount = 0;
		pitchCount = 0;
		noteOn = base/2;//middle note A
		dials = ac.sync();
		controls = ac;//default
	}
	
	char[] spectrum;
	
	public void genSamples(short[] s) {
		spectrum = n.calc(dials);//get note spectrum
		for(int i = 0; i <= s.length; i++) {
			calcStatics();//using sync
			oscillate();
			hpf();
			filter();
			distort();
			envelope();//generation of sync
			s[i] = outputToJack;
		}
	}
}
