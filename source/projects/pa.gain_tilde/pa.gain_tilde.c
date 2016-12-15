// Copyright (c) 2016 Eliott Paris.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

//! @brief Multiply signal with a smooth transition.

#include <m_pd.h>
#include <math.h>

static t_class *pa_gain_tilde_class;

typedef struct _pa_gain_tilde
{
    t_object    m_obj;

    float       m_gain;
    float       m_gain_to;
    float       m_gain_increment;
    int         m_samps_to_fade;

    t_float     m_f;

} t_pa_gain_tilde;

static void pa_gain_tilde_set_gain(t_pa_gain_tilde *x, float new_gain, float ramp_time_ms)
{
    // gain should be positive
    x->m_gain_to = (new_gain > 0.) ? new_gain : 0.;

    // if the ramp time is positive then calculate the number of samples needed to smooth gain
    x->m_samps_to_fade = (ramp_time_ms > 0) ? (int)(sys_getsr() * 0.001 * ramp_time_ms) : 0;

    // compute gain increment for each samples
    x->m_gain_increment = (x->m_samps_to_fade > 0) ? (x->m_gain_to - x->m_gain) / (float)x->m_samps_to_fade : 0;
}

static t_int *pa_gain_tilde_dsp_perform(t_int *w)
{
    t_pa_gain_tilde   *x   = (t_pa_gain_tilde *)(w[1]);
    t_sample  *in = (t_sample *)(w[2]);
    t_sample  *out = (t_sample *)(w[3]);
    int vecsize = (int)(w[4]);

    while(vecsize--)
    {
        if(x->m_samps_to_fade > 0)
        {
            x->m_gain += x->m_gain_increment;
            x->m_samps_to_fade--;
        }
        else
        {
            x->m_gain = x->m_gain_to;
        }

        *out++ = *in++ * x->m_gain;
    }

    return (w+5);
}

static void pa_gain_tilde_dsp_prepare(t_pa_gain_tilde *x, t_signal **sp)
{
    // reset gain
    pa_gain_tilde_set_gain(x, x->m_gain_to, 0.);

    dsp_add(pa_gain_tilde_dsp_perform, 4,
            x,
            sp[0]->s_vec,
            sp[1]->s_vec,
            sp[0]->s_n);
}

static void *pa_gain_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_gain_tilde *x = (t_pa_gain_tilde *)pd_new(pa_gain_tilde_class);
    if(x)
    {
        x->m_gain = x->m_gain_to = x->m_gain_increment = 0.f;
        x->m_samps_to_fade = 0;

        outlet_new((t_object *)x, &s_signal);
    }

    return (x);
}

static void pa_gain_tilde_free(t_pa_gain_tilde *x)
{
    ;
}

extern void setup_pa0x2egain_tilde(void)
{
    t_class* c = class_new(gensym("pa.gain~"),
                           (t_newmethod)pa_gain_tilde_new, (t_method)pa_gain_tilde_free,
                           sizeof(t_pa_gain_tilde), CLASS_DEFAULT, A_GIMME, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_gain_tilde_dsp_prepare, gensym("dsp"), A_CANT);
        class_addmethod(c, (t_method)pa_gain_tilde_set_gain,    gensym("gain"), A_FLOAT, A_DEFFLOAT, 0);
        CLASS_MAINSIGNALIN(c, t_pa_gain_tilde, m_f);
    }
    pa_gain_tilde_class = c;
}
