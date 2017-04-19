/*
// Copyright (c) 2016 Eliott Paris.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

//! @brief Cosine wave oscillator
//! @details this object is c++ version of the [pa.osc3~] object

#include <m_pd.h>

#include "Osc.hpp"
using paccpp::Osc;

static t_class *pa_oscpp_tilde_class;

typedef struct _pa_oscpp_tilde
{
    t_object    m_obj;

    // store an Osc pointer
    Osc<float>* m_osc;

    t_outlet*   m_out;
    t_float     m_f;

} t_pa_oscpp_tilde;

static t_int *pa_oscpp_tilde_perform(t_int* w)
{
    t_pa_oscpp_tilde* x = (t_pa_oscpp_tilde*)(w[1]);

    t_sample const* ins  = (t_sample *)(w[2]);
    t_sample*       outs = (t_sample *)(w[3]);

    int vecsize = (int)(w[4]);

    x->m_osc->process(ins, outs, vecsize);

    return (w+5);
}

static void pa_oscpp_tilde_dsp(t_pa_oscpp_tilde* x, t_signal **sp)
{
    x->m_osc->setSampleRate(sys_getsr());

    dsp_add(pa_oscpp_tilde_perform, 4,
            x,
            sp[0]->s_vec,   // inlet 0
            sp[1]->s_vec,   // outlet 0
            sp[0]->s_n);    // vectorsize
}

static void *pa_oscpp_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_oscpp_tilde* x = (t_pa_oscpp_tilde*)pd_new(pa_oscpp_tilde_class);
    if(x)
    {
        // instantiate a new Osc object
        // Note: dont forget to delete it in the free method !
        x->m_osc = new Osc<float>();

        x->m_out = outlet_new((t_object *)x, &s_signal);
    }

    return (x);
}

static void pa_oscpp_tilde_free(t_pa_oscpp_tilde* x)
{
    outlet_free(x->m_out);

    // free the memory for the Osc object
    delete x->m_osc;
}

// Note in c++ you need to wrap the setup method in an extern "C" statement.
extern "C"
{
    extern void setup_pa0x2eoscpp_tilde(void)
    {
        t_class* c = class_new(gensym("pa.oscpp~"),
                               (t_newmethod)pa_oscpp_tilde_new, (t_method)pa_oscpp_tilde_free,
                               sizeof(t_pa_oscpp_tilde), CLASS_DEFAULT, A_GIMME, 0);
        if(c)
        {
            CLASS_MAINSIGNALIN(c, t_pa_oscpp_tilde, m_f);
            class_addmethod(c, (t_method)pa_oscpp_tilde_dsp, gensym("dsp"), A_CANT);
        }

        pa_oscpp_tilde_class = c;
    }
}
