/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A signal driven variable delay line.

#include <m_pd.h>

#include <stdlib.h> // malloc, calloc, free...

static t_class *pa_delay4_tilde_class;

typedef struct _pa_delay4_tilde
{
    t_object    m_obj;
    
    float*      m_buffer;
    int         m_buffersize;
    size_t      m_writer_playhead;
    
    t_inlet*    m_in;
    t_outlet*   m_out;
    
    float       m_f;
    
} t_pa_delay4_tilde;

static void pa_delay4_tilde_delete_buffer(t_pa_delay4_tilde* x)
{
    if(x->m_buffer)
    {
        // free allocated buffer
        free(x->m_buffer);
        x->m_buffer = NULL;
    }
}

static void pa_delay4_tilde_clear_buffer(t_pa_delay4_tilde* x)
{
    int i = 0;
    if(x->m_buffer)
    {
        for(; i < x->m_buffersize; ++i)
        {
            x->m_buffer[i] = 0.f;
        }
    }
}

static void pa_delay4_tilde_create_buffer(t_pa_delay4_tilde* x)
{
    pa_delay4_tilde_delete_buffer(x);
    
    // alloc and init to 0 directly with calloc
    x->m_buffer = (float*)calloc(x->m_buffersize, sizeof(float));
}

static float get_buffer_value(t_pa_delay4_tilde* x, int idx)
{
    const int buffersize = x->m_buffersize;
    
    while(idx < 0) idx += buffersize;
    while(idx >= buffersize) idx -= buffersize;
    
    return x->m_buffer[idx];
}

static float linear_interp(float y1, float y2, float delta)
{
    return y1 + delta * (y2 - y1);
}

static t_int *pa_delay4_tilde_dsp_perform(t_int *w)
{
    t_pa_delay4_tilde *x   = (t_pa_delay4_tilde *)(w[1]);
    t_sample  *in1 = (t_sample *)(w[2]);
    t_sample  *in2 = (t_sample *)(w[3]);
    t_sample  *out = (t_sample *)(w[4]);
    int vecsize = (int)(w[5]);
    
    float* buffer = x->m_buffer;
    float sample_to_write = 0.f;
    const int buffersize = x->m_buffersize;
    
    float delay_size_samps = 0.f;
    int reader_playhead = 0;
    
    // interpolation values
    float y1, y2, delta;
    
    while(vecsize--)
    {
        // store current input sample to write in the buffer
        sample_to_write = *in1++;
        
        // get new delay size value (in samps)
        delay_size_samps = *in2++;
        
        // clip delay size between buffer boundaries.
        if(delay_size_samps >= buffersize)
        {
            delay_size_samps = buffersize - 1;
        }
        else if(delay_size_samps < 1.f)
        {
            delay_size_samps = 1.f;
        }
        
        delta = delay_size_samps - (int)delay_size_samps;
        
        reader_playhead = x->m_writer_playhead - (int)delay_size_samps;
        
        y1 = get_buffer_value(x, reader_playhead);
        y2 = get_buffer_value(x, reader_playhead - 1);
        
        // we read our buffer.
        *out++ = linear_interp(y1, y2, delta);
        
        // without interpolation:
        //*out++ = y1;
        
        // then store incoming sample to the buffer.
        buffer[x->m_writer_playhead] = sample_to_write;
        
        // wrap writer playhead between buffer boundaries.
        if(++x->m_writer_playhead >= buffersize) x->m_writer_playhead = 0;
    }
    
    return (w+6);
}

static void pa_delay4_tilde_dsp_prepare(t_pa_delay4_tilde *x, t_signal **sp)
{
    // as you want :
    //pa_delay4_tilde_clear_buffer(x);
    
    dsp_add(pa_delay4_tilde_dsp_perform, 5,
            x,              // object
            sp[0]->s_vec,   // inlet 1
            sp[1]->s_vec,   // inlet 2
            sp[2]->s_vec,   // outlet 1
            sp[0]->s_n);    // vectorsize
}

static void *pa_delay4_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_delay4_tilde *x = (t_pa_delay4_tilde *)pd_new(pa_delay4_tilde_class);
    
    if(x)
    {
        x->m_writer_playhead = 0;
        x->m_buffer = NULL;
        
        int buffersize = sys_getsr() * 0.1; // default to 100ms
        
        if(argc >= 1 && argv->a_type == A_FLOAT)
        {
            if(argv->a_w.w_float > 0)
            {
                buffersize = (int)argv->a_w.w_float;
            }
            else
            {
                pd_error((t_object*)x, "buffer size must be > 1");
            }
        }
        
        x->m_buffersize = buffersize;
        
        // reset our buffer.
        pa_delay4_tilde_create_buffer(x);
        
        // create delay size control signal inlet
        x->m_in = signalinlet_new((t_object*)x, 0.f);
        
        // create one signal outlet:
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return (x);
}


static void pa_delay4_tilde_free(t_pa_delay4_tilde *x)
{
    pa_delay4_tilde_delete_buffer(x);
    inlet_free(x->m_in);
    outlet_free(x->m_out);
}

extern void setup_pa0x2edelay4_tilde(void)
{
    t_class* c = class_new(gensym("pa.delay4~"),
                           (t_newmethod)pa_delay4_tilde_new, (t_method)pa_delay4_tilde_free,
                           sizeof(t_pa_delay4_tilde), CLASS_DEFAULT, A_GIMME, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_delay4_tilde_dsp_prepare, gensym("dsp"), A_CANT);
        class_addmethod(c, (t_method)pa_delay4_tilde_clear_buffer, gensym("clear"), 0);
        CLASS_MAINSIGNALIN(c, t_pa_delay4_tilde, m_f);
    }
    pa_delay4_tilde_class = c;
}

