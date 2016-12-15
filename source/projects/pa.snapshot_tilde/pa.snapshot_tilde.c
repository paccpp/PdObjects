/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief Converts signal into float at a given time interval.

#include <m_pd.h>
#include <math.h>

typedef struct _pa_snapshot_tilde
{
    t_object    m_obj;
    
    float       m_value;
    float       m_interval_ms;
    
    t_clock*    m_clock;
    t_outlet*   m_outlet;
    
    t_float     m_f;
    
} t_pa_snapshot_tilde;

static t_class *pa_snapshot_tilde_class;

static void pa_snapshot_tilde_bang(t_pa_snapshot_tilde *x)
{
    outlet_float(x->m_outlet, x->m_value);
}

static void pa_snapshot_tilde_tick(t_pa_snapshot_tilde *x)
{
    if(pd_getdspstate() && x->m_interval_ms > 0)
    {
        pa_snapshot_tilde_bang(x);
        
        // schedule the execution of the clock.
        clock_delay(x->m_clock, x->m_interval_ms);
    }
}

t_int *pa_snapshot_tilde_dsp_perform(t_int *w)
{
    t_pa_snapshot_tilde* x = (t_pa_snapshot_tilde *)(w[1]);
    t_sample*   in = (t_sample *)(w[2]);
    size_t vecsize = (size_t)(w[3]);

    while(vecsize--)
    {
        x->m_value = *in++;
    }
    
    return (w+4);
}


static void pa_snapshot_tilde_dsp_prepare(t_pa_snapshot_tilde *x, t_signal **sp)
{
    if(x->m_interval_ms > 0)
    {
        // schedule the execution of the clock.
        clock_delay(x->m_clock, x->m_interval_ms);
        pa_snapshot_tilde_bang(x);
    }
    
    dsp_add(pa_snapshot_tilde_dsp_perform, 3,
            (t_int)x,
            (t_int)sp[0]->s_vec,
            (t_int)sp[0]->s_n);
}

static void *pa_snapshot_tilde_new(t_float interval)
{
    t_pa_snapshot_tilde* x = (t_pa_snapshot_tilde *)pd_new(pa_snapshot_tilde_class);
    
    if(x)
    {
        // argument set the reporting interval in ms (must be positive)
        // 0 means no automatic report
        x->m_interval_ms = interval >= 1.f ? interval : 0.f;
        x->m_value = 0.f;
        
        // create the clock, passing the method to be called by the clock as second parameter
        x->m_clock  = clock_new(x, (t_method)pa_snapshot_tilde_tick);
        x->m_outlet = outlet_new((t_object *)x, &s_list);
    }
    return x;
}

static void pa_snapshot_tilde_free(t_pa_snapshot_tilde *x)
{
    // delete the clock when the object is free
    clock_free(x->m_clock);
    
    outlet_free(x->m_outlet);
}

extern void setup_pa0x2esnapshot_tilde(void)
{
    t_class* c = class_new(gensym("pa.snapshot~"),
                           (t_newmethod)pa_snapshot_tilde_new,
                           (t_method)pa_snapshot_tilde_free,
                           sizeof(t_pa_snapshot_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_snapshot_tilde_dsp_prepare, gensym("dsp"), A_CANT);
        class_addmethod(c, (t_method)pa_snapshot_tilde_bang,        gensym("bang"), 0);
        CLASS_MAINSIGNALIN(c, t_pa_snapshot_tilde, m_f);
    }
    
    pa_snapshot_tilde_class = c;
}
