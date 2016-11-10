/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A fixed delay (of 44100 samples)

#include <m_pd.h>

static t_class *pa_delay1_tilde_class;

#define DELAY1_MAX_DELAY 44100

typedef struct _pa_delay1_tilde
{
    t_object    m_obj;
    
    float       m_buffer[DELAY1_MAX_DELAY];
    int         m_count;
    
    t_outlet*   m_out;
    
    float       m_f;
    
} t_pa_delay1_tilde;

static void pa_delay1_tilde_clear_buffer(t_pa_delay1_tilde* x)
{
    int i = 0;
    for(; i < DELAY1_MAX_DELAY; ++i)
    {
        x->m_buffer[i] = 0.f;
    }
    
    x->m_count = 0;
}

static t_int *pa_delay1_tilde_perform(t_int *w)
{
    t_pa_delay1_tilde *x = (t_pa_delay1_tilde *)(w[1]);
    t_sample  *in   = (t_sample *)(w[2]);
    t_sample  *out  = (t_sample *)(w[3]);
    int vecsize = (int)(w[4]);
    
    float* const buffer = x->m_buffer;
    int count = x->m_count;
    float* buffer_playhead = NULL;
    float sample_to_write = 0.f;
    
    while(vecsize--)
    {
        // store current input sample to write in the buffer
        sample_to_write = *in++;
        
        // fetch buffer pointer playhead position
        buffer_playhead = buffer + count;
        
        // we read our buffer.
        *out++ = *buffer_playhead;
        
        // then store incoming sample to the buffer.
        *buffer_playhead = sample_to_write;
        
        // wrap counter between buffer boundaries
        if(++count >= DELAY1_MAX_DELAY) count = 0;
    }
    
    x->m_count = count;
    
    return (w+5);
}

static void pa_delay1_tilde_dsp(t_pa_delay1_tilde *x, t_signal **sp)
{
    // as you want:
    pa_delay1_tilde_clear_buffer(x);
    
    dsp_add(pa_delay1_tilde_perform, 4,
            x,              // object
            sp[0]->s_vec,   // inlet 1
            sp[1]->s_vec,   // outlet 1
            sp[0]->s_n);    // vectorsize
}

static void *pa_delay1_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_delay1_tilde *x = (t_pa_delay1_tilde *)pd_new(pa_delay1_tilde_class);
    if(x)
    {
        // reset our buffer.
        pa_delay1_tilde_clear_buffer(x);
        
        // create one signal outlet:
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return (x);
}


static void pa_delay1_tilde_free(t_pa_delay1_tilde *x)
{
    outlet_free(x->m_out);
}

extern void setup_pa0x2edelay1_tilde(void)
{
    t_class* c = class_new(gensym("pa.delay1~"),
                           (t_newmethod)pa_delay1_tilde_new, (t_method)pa_delay1_tilde_free,
                           sizeof(t_pa_delay1_tilde), CLASS_DEFAULT, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_delay1_tilde_dsp, gensym("dsp"), A_CANT);
        CLASS_MAINSIGNALIN(c, t_pa_delay1_tilde, m_f);
    }
    pa_delay1_tilde_class = c;
}
