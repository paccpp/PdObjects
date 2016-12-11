/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief Read samples in a Pd array at a given speed

#include <m_pd.h>

typedef struct _pa_readbuffer2
{
    t_object    m_obj;
    
    // phasor
    float       m_phase;
    
    // buffer
    t_symbol*   m_name;
    t_word*     m_buffer;
    int         m_size;
    
    t_outlet*   m_out;
    t_float     m_f;
    
} t_pa_readbuffer2;

static t_class* pa_readbuffer2_tilde_class;

static int pa_readbuffer2_tilde_set_buffer(t_pa_readbuffer2* x, t_symbol* s)
{
    x->m_size = 0;
    x->m_buffer = NULL;
    
    x->m_name = s;
    t_garray* a = NULL;
    
    if(!x->m_name || x->m_name == gensym(""))
    {
        return -1; // no buffer set
    }
    
    a = (t_garray*)pd_findbyclass(x->m_name, garray_class);
    
    if(!a)
    {
        pd_error(x, "pa.readbuffer1~: %s no such array.", x->m_name->s_name);
        return -1;
    }
    
    const int err = garray_getfloatwords(a, &x->m_size, &x->m_buffer);
    if(!err)
    {
        pd_error(x, "pa.readbuffer1~: %s array is empty.", x->m_name->s_name);
        return -1;
    }
    
    // mark the array as used by the DSP
    // doing so will cause the prepare method to be called when the array is removed
    garray_usedindsp(a);
    
    return 0;
}

static t_int* pa_readbuffer2_perform(t_int *w)
{
    t_pa_readbuffer2* x = (t_pa_readbuffer2 *)(w[1]);
    t_sample* in   = (t_sample *)(w[2]);
    t_sample* out  = (t_sample *)(w[3]);
    size_t n       = (size_t)(w[4]);
    float sr       = (float)(w[5]);
    
    float speed = 0.f;
    float freq = 0.f;
    float phase = x->m_phase;
    float tphase = 0.f; // phase in 0. to buffersize. range
    
    // interpolation indexes
    int idx_1, idx_2;
    
    // interpolation values
    float y1, y2, frac;
    
    // buffer
    const int buffersize = x->m_size;
    t_word const* buffer = x->m_buffer;
    
    while(n--)
    {
        speed = *in++;
        
        if(speed != 0.f && buffersize > 0 && x->m_buffer)
        {
            freq = sr / buffersize * speed;
            
            // wrap phase between 0. and 1.
            if(phase >= 1.f) { phase -= 1.f; }
            else if(phase < 0.f) { phase += 1.f; }
            
            tphase = phase * buffersize;
            
            // we cast in int to keep only the integer part of the floating-point number (eg. 3.99 => 3)
            idx_1 = (int)tphase;
            
            // wrap idx_2
            idx_2 = (idx_1 < (buffersize-1)) ? (idx_1+1) : 0;
            
            frac = tphase - idx_1;
            
            y1 = buffer[idx_1].w_float;
            y2 = buffer[idx_2].w_float;
            
            // linear interpolation
            *out++ = y1 + frac * (y2 - y1);
            
            // increment phase
            phase += (freq / sr);
        }
        else
        {
            *out++ = 0.f;
        }
    }
    
    x->m_phase = phase;
    
    return (w+6);
}


static void pa_readbuffer2_tilde_dsp(t_pa_readbuffer2* x, t_signal **sp)
{
    pa_readbuffer2_tilde_set_buffer(x, x->m_name);
    
    dsp_add(pa_readbuffer2_perform, 5,
            (t_int)x,
            (t_int)sp[0]->s_vec,
            (t_int)sp[1]->s_vec,
            (t_int)sp[0]->s_n,
            (t_int)sys_getsr());
}

static void *pa_readbuffer2_tilde_new(t_symbol* buffer_name)
{
    t_pa_readbuffer2* x = (t_pa_readbuffer2 *)pd_new(pa_readbuffer2_tilde_class);
    
    if(x)
    {
        pa_readbuffer2_tilde_set_buffer(x, buffer_name);
        
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return x;
}

static void pa_readbuffer2_tilde_free(t_pa_readbuffer2 *x)
{
    outlet_free(x->m_out);
}

extern void setup_pa0x2ereadbuffer2_tilde(void)
{
    t_class* c = class_new(gensym("pa.readbuffer2~"),
                           (t_newmethod)pa_readbuffer2_tilde_new, (t_method)pa_readbuffer2_tilde_free,
                           sizeof(t_pa_readbuffer2), CLASS_DEFAULT, A_DEFSYM, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_readbuffer2_tilde_dsp,           gensym("dsp"),        A_CANT);
        class_addmethod(c, (t_method)pa_readbuffer2_tilde_set_buffer,    gensym("set"),        A_DEFSYM, 0);
        CLASS_MAINSIGNALIN(c, t_pa_readbuffer2, m_f);
    }
    
    pa_readbuffer2_tilde_class = c;
}
