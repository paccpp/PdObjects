/*
// Copyright (c) 2016 Eliott Paris.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

//! @brief Outputs a ramp between 0. and 1. at a given frequency
//! @details this object is c++ version of the [pa.phasor~] object

#include <m_pd.h>

#include "Phasor.hpp"
using paccpp::Phasor;

static t_class *pa_phasorpp_tilde_class;

typedef struct _pa_phasorpp_tilde
{
    t_object    m_obj;
    
    // store a Phasor pointer
    Phasor<float>* m_phasor;
    
    t_outlet*   m_out;
    t_float     m_f;
    
} t_pa_phasorpp_tilde;

static t_int *pa_phasorpp_tilde_perform(t_int* w)
{
    t_pa_phasorpp_tilde* x = (t_pa_phasorpp_tilde*)(w[1]);
    
    t_sample const* ins  = (t_sample *)(w[2]);
    t_sample*       outs = (t_sample *)(w[3]);
    
    int vecsize = (int)(w[4]);
    
    x->m_phasor->process(ins, outs, vecsize);

    return (w+5);
}

static void pa_phasorpp_tilde_dsp(t_pa_phasorpp_tilde* x, t_signal **sp)
{
    x->m_phasor->setSampleRate(sys_getsr());
    
    dsp_add(pa_phasorpp_tilde_perform, 4,
            x,
            sp[0]->s_vec,   // inlet 0
            sp[1]->s_vec,   // outlet 0
            sp[0]->s_n);    // vectorsize
}

static void *pa_phasorpp_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_phasorpp_tilde* x = (t_pa_phasorpp_tilde*)pd_new(pa_phasorpp_tilde_class);
    if(x)
    {
        // instantiate a new Phasor object
        // Note: dont forget to delete it in the free method !
        x->m_phasor = new Phasor<float>();

        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return (x);
}

static void pa_phasorpp_tilde_free(t_pa_phasorpp_tilde* x)
{
    outlet_free(x->m_out);
    
    // free the memory for the Phasor object
    delete x->m_phasor;
}

// Note in c++ you need to wrap the setup method in an extern "C" statement.
extern "C"
{
    extern void setup_pa0x2ephasorpp_tilde(void)
    {
        t_class* c = class_new(gensym("pa.phasorpp~"),
                               (t_newmethod)pa_phasorpp_tilde_new, (t_method)pa_phasorpp_tilde_free,
                               sizeof(t_pa_phasorpp_tilde), CLASS_DEFAULT, A_GIMME, 0);
        if(c)
        {
            CLASS_MAINSIGNALIN(c, t_pa_phasorpp_tilde, m_f);
            class_addmethod(c, (t_method)pa_phasorpp_tilde_dsp, gensym("dsp"), A_CANT);
        }
        
        pa_phasorpp_tilde_class = c;
    }
}
