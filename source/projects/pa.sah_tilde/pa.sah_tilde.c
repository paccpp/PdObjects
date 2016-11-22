/*
// Copyright (c) 2016 Eliott Paris.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

//! @brief A Sample And Hold object.
//! @detail Capture and continually output the value of an input signal
//! whenever another "control" signal rises above a specified threshold value.

#include <m_pd.h>

static t_class *pa_sah_tilde_class;

typedef struct _pa_sah_tilde
{
    t_object    m_obj;
    float       m_threshold;
    float       m_hold_value;
    float       m_last_ctrl_sample;
    
    t_inlet*    m_in1;
    t_outlet*   m_out;
    
    t_float     m_f;
    
} t_pa_sah_tilde;

static void pa_sah_tilde_float(t_pa_sah_tilde *x, float f)
{
    x->m_threshold = f;
}

static t_int *pa_sah_tilde_perform(t_int *w)
{
    t_pa_sah_tilde *x   = (t_pa_sah_tilde *)(w[1]);
    t_sample  *in1      = (t_sample *)(w[2]);
    t_sample  *in2      = (t_sample *)(w[3]);
    t_sample  *out      = (t_sample *)(w[4]);
    
    int vectorsize = (int)(w[5]);
    
    const float thresh = x->m_threshold;
    float value;
    float ctrl_sample;
    
    while(vectorsize--)
    {
        value = *in1++;
        ctrl_sample = *in2++;
        
        if(x->m_last_ctrl_sample <= thresh && ctrl_sample > thresh)
        {
            x->m_hold_value = value;
        }
        
        *out++ = x->m_hold_value;
        x->m_last_ctrl_sample = ctrl_sample;
    }

    return (w+6);
}

static void pa_sah_tilde_dsp(t_pa_sah_tilde *x, t_signal **sp)
{
    dsp_add(pa_sah_tilde_perform, 5,
            x,
            sp[0]->s_vec, sp[1]->s_vec,
            sp[2]->s_vec,
            sp[0]->s_n);
}

static void *pa_sah_tilde_new(t_symbol *s, float thresh)
{
    t_pa_sah_tilde *x = (t_pa_sah_tilde *)pd_new(pa_sah_tilde_class);
    if(x)
    {
        x->m_threshold = 0.f;
        x->m_hold_value = 0.f;
        x->m_last_ctrl_sample = 0.f;
        x->m_threshold = thresh;
        
        x->m_in1 = signalinlet_new((t_object *)x, 0);
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return (x);
}

static void pa_sah_tilde_free(t_pa_sah_tilde *x)
{
    // free dynamically allocated IO
    inlet_free(x->m_in1);
    outlet_free(x->m_out);
}

extern void setup_pa0x2esah_tilde(void)
{
    t_class* c = class_new(gensym("pa.sah~"),
                           (t_newmethod)pa_sah_tilde_new, (t_method)pa_sah_tilde_free,
                           sizeof(t_pa_sah_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_sah_tilde_dsp, gensym("dsp"), A_CANT);
        class_addmethod(c, (t_method)pa_sah_tilde_float, gensym("threshold"), A_FLOAT, 0);
        CLASS_MAINSIGNALIN(c, t_pa_sah_tilde, m_f);
    }
    
    pa_sah_tilde_class = c;
}
