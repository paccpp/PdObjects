/*
// Copyright (c) 2016 Eliott Paris.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

//! @brief A bank of cosine wave oscillators

#include <m_pd.h>
#include <vector>

#include "Osc.hpp"
using paccpp::Osc;

static t_class *pa_oscbank_tilde_class;

typedef struct _pa_oscbank_tilde
{
    t_object    m_obj;

    // store a vector of Osc pointers
    std::vector<Osc<float>*> m_oscbank;

    t_outlet*   m_out;

} t_pa_oscbank_tilde;

static void pa_oscbank_tilde_list(t_pa_oscbank_tilde* x, t_symbol* s, int argc, t_atom* argv)
{
    float sr = sys_getsr();
    
    // reserve space for new oscillators in the vector
    x->m_oscbank.reserve(argc);
    
    for(int i = 0; i < argc; ++i)
    {
        // allocate a new Osc object if needed
        if(x->m_oscbank.size() <= i)
        {
            x->m_oscbank.push_back(new Osc<float>());
            x->m_oscbank[i]->setSampleRate(sr);
        }
        
        if(argv[i].a_type == A_FLOAT)
        {
            x->m_oscbank[i]->setFrequency(argv[i].a_w.w_float);
        }
        else
        {
            x->m_oscbank[i]->setFrequency(0.f);
            error("bad frequency for osc %i, reset to 0Hz", i);
        }
    }
    
    // delete all useless pointers and remove them from the vector
    if(x->m_oscbank.size() > argc)
    {
        for(int i = argc; i < x->m_oscbank.size(); ++i)
        {
            delete x->m_oscbank[i];
        }
        
        x->m_oscbank.erase(x->m_oscbank.begin()+argc, x->m_oscbank.end());
    }
}

static t_int* pa_oscbank_tilde_perform(t_int* w)
{
    t_pa_oscbank_tilde* x = (t_pa_oscbank_tilde*)(w[1]);
    t_sample*       outs = (t_sample *)(w[2]);

    int vecsize = (int)(w[3]);
    
    const size_t osc_count = (!x->m_oscbank.empty()) ? x->m_oscbank.size() : 1ul;
    
    while(vecsize--)
    {
        double out = 0.f;
        
        for(Osc<float>* osc : x->m_oscbank)
        {
            out += osc->process();
        }
        
        *outs++ = out / osc_count;
    }

    return (w+4);
}

static void pa_oscbank_tilde_dsp(t_pa_oscbank_tilde* x, t_signal **sp)
{
    // set samplerate of all oscillators
    const float sr = sys_getsr();
    
    for(Osc<float>* osc : x->m_oscbank)
    {
        osc->setSampleRate(sr);
    }

    dsp_add(pa_oscbank_tilde_perform, 3,
            x,
            sp[0]->s_vec,   // outlet 0
            sp[0]->s_n);    // vectorsize
}

static void *pa_oscbank_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_oscbank_tilde* x = (t_pa_oscbank_tilde*)pd_new(pa_oscbank_tilde_class);
    if(x)
    {
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }

    return (x);
}

static void pa_oscbank_tilde_free(t_pa_oscbank_tilde* x)
{
    outlet_free(x->m_out);

    // free the memory allocated for each Osc objects of the vector
    for(Osc<float>* osc : x->m_oscbank)
    {
        delete osc;
    }
    
    // then clear the vector
    x->m_oscbank.clear();
}

// Note in c++ you need to wrap the setup method in an extern "C" statement.
extern "C"
{
    extern void setup_pa0x2eoscbank_tilde(void)
    {
        t_class* c = class_new(gensym("pa.oscbank~"),
                               (t_newmethod)pa_oscbank_tilde_new, (t_method)pa_oscbank_tilde_free,
                               sizeof(t_pa_oscbank_tilde), CLASS_DEFAULT, A_GIMME, 0);
        if(c)
        {
            class_addmethod(c, (t_method)pa_oscbank_tilde_dsp,  gensym("dsp"),  A_CANT);
            class_addmethod(c, (t_method)pa_oscbank_tilde_list, gensym("list"), A_GIMME, 0);
        }

        pa_oscbank_tilde_class = c;
    }
}
