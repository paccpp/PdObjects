/*
// Copyright (c) 2016 Eliott Paris.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

//! @brief clip a signal input between a minimum and maximum value.

#include <m_pd.h>

static t_class *pa_clip_tilde_class;

typedef struct _pa_clip_tilde
{
    t_object    m_obj;
    
    float       m_min;
    float       m_max;
    t_outlet*   m_out;
    
    t_float     m_f;
} t_pa_clip_tilde;

static void pa_clip_tilde_set_minmax(t_pa_clip_tilde *x, float min, float max)
{
    if(min <= max)
    {
        x->m_min = min;
        x->m_max = max;
    }
    else
    {
        x->m_min = max;
        x->m_max = min;
    }
}

static void pa_clip_tilde_set_min(t_pa_clip_tilde *x, float value)
{
    pa_clip_tilde_set_minmax(x, value, x->m_max);
}

static void pa_clip_tilde_set_max(t_pa_clip_tilde *x, float value)
{
    pa_clip_tilde_set_minmax(x, x->m_min, value);
}

static t_int *pa_clip_tilde_perform(t_int *w)
{
    t_pa_clip_tilde *x = (t_pa_clip_tilde *)(w[1]);
    t_sample  *in = (t_sample *)(w[2]);
    t_sample  *out = (t_sample *)(w[3]);
    int vectorsize = (int)(w[4]);
    
    const float min = x->m_min;
    const float max = x->m_max;
    float value = 0.f;

    while(vectorsize--)
    {
        value = *in++;
        
        if(value < min) value = min;
        else if(value > max) value = max;
        
        *out++ = value;
    }

    return (w+5);
}

static void pa_clip_tilde_dsp(t_pa_clip_tilde *x, t_signal **sp)
{
    dsp_add(pa_clip_tilde_perform, 4,
            x,
            sp[0]->s_vec,
            sp[1]->s_vec,
            sp[0]->s_n);
}

static void *pa_clip_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_clip_tilde *x = (t_pa_clip_tilde *)pd_new(pa_clip_tilde_class);
    if(x)
    {
        float min = -1.;
        float max = 1.;
        
        // first argument set the minimum value
        if(argc >= 1 && argv[0].a_type == A_FLOAT)
        {
            min = argv[0].a_w.w_float;
        }
        
        // second argument set the maximum value
        if(argc >= 2 && argv[1].a_type == A_FLOAT)
        {
            max = argv[1].a_w.w_float;
        }
        
        pa_clip_tilde_set_minmax(x, min, max);
        
        // creation d'un outlet signal
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return (x);
}

static void pa_clip_tilde_free(t_pa_clip_tilde *x)
{
    outlet_free(x->m_out);
}

extern void setup_pa0x2eclip_tilde(void)
{
    t_class* c = class_new(gensym("pa.clip~"),
                           (t_newmethod)pa_clip_tilde_new, (t_method)pa_clip_tilde_free,
                           sizeof(t_pa_clip_tilde), CLASS_DEFAULT, A_GIMME, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_clip_tilde_dsp, gensym("dsp"), A_CANT);
        class_addmethod(c, (t_method)pa_clip_tilde_set_min, gensym("min"), A_FLOAT, 0);
        class_addmethod(c, (t_method)pa_clip_tilde_set_max, gensym("max"), A_FLOAT, 0);
        CLASS_MAINSIGNALIN(c, t_pa_clip_tilde, m_f);
    }
    pa_clip_tilde_class = c;
}
