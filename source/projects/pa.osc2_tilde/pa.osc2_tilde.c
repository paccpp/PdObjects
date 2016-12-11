/*
// Copyright (c) 2016 Eliott Paris.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

//! @brief A sinusoidal oscillator using linear interpolation on a 512 point buffer (to be optimized)

#include <m_pd.h>

#define _USE_MATH_DEFINES
#include <math.h> // cos...

static t_class *pa_osc2_tilde_class;

#define OSC2_COSTABLE_SIZE 512
static float osc2_cos_table[OSC2_COSTABLE_SIZE];

typedef struct _pa_osc2_tilde
{
    t_object    m_obj;
    float       m_f;
    
    float       m_sr;
    double      m_phase;
    
    t_outlet*   m_out;
    
} t_pa_osc2_tilde;

static void pa_osc2_tilde_fill_cos_table()
{
    int i;
    for(i=0; i < OSC2_COSTABLE_SIZE; i++)
    {
        osc2_cos_table[i] = cosf(2.f * M_PI * i / OSC2_COSTABLE_SIZE);
        //post("%f, ", osc2_cos_table[i]);
    }
}

static t_int *pa_osc2_tilde_perform(t_int *w)
{
    t_pa_osc2_tilde     *x      = (t_pa_osc2_tilde *)(w[1]);
    t_sample            *in     = (t_sample *)(w[2]);
    t_sample            *out    = (t_sample *)(w[3]);
    int                 vecsize = (int)(w[4]);

    // interpolation indexes
    int idx_1, idx_2;
    
    // interpolation values
    float y1, y2, frac;
    
    float *cos_table = osc2_cos_table;
    
    const float sr = x->m_sr;
    float freq;
    float phase = x->m_phase;
    
    float tphase = 0.f; // phase in 0. to OSC2_COSTABLE_SIZE. range
    
    while(vecsize--)
    {
        freq = *in++;
        
        // wrap phase between 0. and 1.
        if(phase >= 1.f) { phase -= 1.f; }
        else if(phase < 0.f) { phase += 1.f; }
        
        tphase = phase * OSC2_COSTABLE_SIZE;
        
        // we cast in int to keep only the integer part of the floating-point number (eg. 3.99 => 3)
        idx_1 = (int)tphase;
        
        // wrap idx_2
        idx_2 = (idx_1 < (OSC2_COSTABLE_SIZE-1)) ? (idx_1+1) : 0;
        
        frac = tphase - idx_1;
        
        y1 = cos_table[idx_1];
        y2 = cos_table[idx_2];
        
        // linear interpolation
        *out++ = y1 + frac * (y2 - y1);
        
        // increment phase
        phase += (freq / sr);
    }
    
    x->m_phase = phase;
    return (w+5);
}

static void pa_osc2_tilde_dsp(t_pa_osc2_tilde *x, t_signal **sp)
{
    x->m_sr = sys_getsr();
    
    // You can reset the phase here
    //x->m_phase = 0.f;
    
    dsp_add(pa_osc2_tilde_perform, 4,
            x,              // object
            sp[0]->s_vec,   // inlet 1
            sp[1]->s_vec,   // outlet 1
            sp[0]->s_n);    // vectorsize
}

static void *pa_osc2_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_osc2_tilde *x = (t_pa_osc2_tilde *)pd_new(pa_osc2_tilde_class);
    if(x)
    {
        x->m_phase = 0.f;
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return (x);
}


static void pa_osc2_tilde_free(t_pa_osc2_tilde *x)
{
    outlet_free(x->m_out);
}

extern void setup_pa0x2eosc2_tilde(void)
{
    t_class* c = class_new(gensym("pa.osc2~"),
                           (t_newmethod)pa_osc2_tilde_new, (t_method)pa_osc2_tilde_free,
                           sizeof(t_pa_osc2_tilde), CLASS_DEFAULT, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_osc2_tilde_dsp, gensym("dsp"), A_CANT);
        CLASS_MAINSIGNALIN(c, t_pa_osc2_tilde, m_f);
    }
    
    pa_osc2_tilde_fill_cos_table();
    
    pa_osc2_tilde_class = c;
}
