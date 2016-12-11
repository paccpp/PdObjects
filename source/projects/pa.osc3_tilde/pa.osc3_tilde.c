/*
// Copyright (c) 2016 Eliott Paris.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

//! @brief A sinusoidal oscillator using linear interpolation on a 512+1 point buffer

#include <m_pd.h>

#define _USE_MATH_DEFINES
#include <math.h> // cos...

static t_class *pa_osc3_tilde_class;

#define OSC3_COSTABLE_SIZE 512
static float osc3_cos_table[OSC3_COSTABLE_SIZE+1]; // last sample = first sample

typedef struct _pa_osc3_tilde
{
    t_object    m_obj;
    float       m_f;
    
    float       m_sr;
    double      m_phase;
    
    t_outlet*   m_out;
    
} t_pa_osc3_tilde;

static void pa_osc3_tilde_fill_cos_table()
{
    int i;
    for(i=0; i < OSC3_COSTABLE_SIZE; i++)
    {
        osc3_cos_table[i] = cosf(2.f * M_PI * i / OSC3_COSTABLE_SIZE);
    }
    
    osc3_cos_table[OSC3_COSTABLE_SIZE] = osc3_cos_table[0]; // last sample = first sample
}

static void pa_osc3_tilde_reset_phase(t_pa_osc3_tilde *x, float f)
{
    const int tsize = (OSC3_COSTABLE_SIZE-1);
    float phase = f * tsize;
    
    // wrap between table boundaries
    while(phase >= tsize) { phase -= tsize; }
    while(phase < 0) { phase += tsize; }
    
    x->m_phase = phase;
}

static t_int *pa_osc3_tilde_perform(t_int *w)
{
    t_pa_osc3_tilde     *x      = (t_pa_osc3_tilde *)(w[1]);
    t_sample            *in     = (t_sample *)(w[2]);
    t_sample            *out    = (t_sample *)(w[3]);
    int                 vecsize = (int)(w[4]);
    
    // interpolation index
    int idx_1;
    
    const int tsize = (OSC3_COSTABLE_SIZE-1);
    
    const float sr = x->m_sr;
    
    // interpolation values
    float y1, delta;
    float *cos_table = osc3_cos_table;
    float freq;
    float tphase = x->m_phase; // phase in 0. to OSC3_COSTABLE_SIZE. range
    
    while(vecsize--)
    {
        freq = *in++;
        
        // wrap between table boundaries
        if(tphase >= tsize) { tphase -= tsize; }
        else if(tphase < 0.f) { tphase += tsize; }
        
        // we cast to int to keep only the integer part of the floating-point number (eg. 3.99 => 3)
        idx_1 = (int)tphase;
        
        // delta = tphase - integral part of the the floating-point number
        delta = tphase - idx_1;
        
        y1 = cos_table[idx_1];
        
        // linear interpolation
        // (idx_2 can allways be idx_1+1 thanks to the additionnal sample in the buffer)
        *out++ = y1 + delta * (cos_table[idx_1+1] - y1);
        
        // increment phase
        tphase += (freq / sr) * tsize;
    }
    
    x->m_phase = tphase;
    return (w+5);
}

static void pa_osc3_tilde_dsp(t_pa_osc3_tilde *x, t_signal **sp)
{
    x->m_sr = sys_getsr();
    
    // You can reset the phase here
    //x->m_phase = 0.f;
    
    dsp_add(pa_osc3_tilde_perform, 4,
            x,              // object
            sp[0]->s_vec,   // inlet 1
            sp[1]->s_vec,   // outlet 1
            sp[0]->s_n);    // vectorsize
}

static void *pa_osc3_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_osc3_tilde *x = (t_pa_osc3_tilde *)pd_new(pa_osc3_tilde_class);
    if(x)
    {
        x->m_phase = 0.f;
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return (x);
}


static void pa_osc3_tilde_free(t_pa_osc3_tilde *x)
{
    outlet_free(x->m_out);
}

extern void setup_pa0x2eosc3_tilde(void)
{
    t_class* c = class_new(gensym("pa.osc3~"),
                           (t_newmethod)pa_osc3_tilde_new, (t_method)pa_osc3_tilde_free,
                           sizeof(t_pa_osc3_tilde), CLASS_DEFAULT, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_osc3_tilde_dsp, gensym("dsp"), A_CANT);
        class_addmethod(c, (t_method)pa_osc3_tilde_reset_phase, gensym("phase"), A_DEFFLOAT, 0);
        CLASS_MAINSIGNALIN(c, t_pa_osc3_tilde, m_f);
    }
    
    pa_osc3_tilde_fill_cos_table();
    
    pa_osc3_tilde_class = c;
}
