/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A variable delay line (with control value in samps)

#include <m_pd.h>

#include <stdlib.h> // malloc, calloc, free...

static t_class *pa_delay3_tilde_class;

typedef struct _pa_delay3_tilde
{
    t_object    m_obj;
    
    float*      m_buffer;
    int         m_buffersize;
    
    size_t      m_writer_playhead;
    size_t      m_reader_playhead;
    
    t_outlet*   m_out;
    
    float       m_f;
    
} t_pa_delay3_tilde;

static void pa_delay3_tilde_delete_buffer(t_pa_delay3_tilde* x)
{
    if(x->m_buffer)
    {
        // free allocated buffer
        free(x->m_buffer);
        x->m_buffer = NULL;
    }
}

static void pa_delay3_tilde_clear_buffer(t_pa_delay3_tilde* x)
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

static void pa_delay3_tilde_create_buffer(t_pa_delay3_tilde* x)
{
    pa_delay3_tilde_delete_buffer(x);
    
    // or alloc and init to 0 directly with calloc
    x->m_buffer = (float*)calloc(x->m_buffersize, sizeof(float));
}

static void pa_delay3_tilde_set_size_in_samps(t_pa_delay3_tilde* x, float f)
{
    int delay_samps = (int)f;
    
    // clip to buffersize
    if(delay_samps > x->m_buffersize)
    {
        delay_samps = x->m_buffersize;
    }
    else if(delay_samps < 1)
    {
        delay_samps = 1;
    }
    
    int reader_playhead = x->m_writer_playhead - delay_samps;
    
    if(reader_playhead < 0)
    {
        reader_playhead += x->m_buffersize;
    }
    
    x->m_reader_playhead = reader_playhead;
}

static t_int *pa_delay3_tilde_dsp_perform(t_int *w)
{
    t_pa_delay3_tilde *x   = (t_pa_delay3_tilde *)(w[1]);
    t_sample  *in = (t_sample *)(w[2]);
    t_sample  *out = (t_sample *)(w[3]);
    int vecsize = (int)(w[4]);
    
    float* buffer = x->m_buffer;
    float sample_to_write = 0.f;
    int buffersize = x->m_buffersize;
    
    while(vecsize--)
    {
        // store current input sample to write in the buffer
        sample_to_write = *in++;
        
        // we read our buffer.
        *out++ = buffer[x->m_reader_playhead];
        
        // then store incoming sample to the buffer.
        buffer[x->m_writer_playhead] = sample_to_write;
        
        // wrap counter between buffer boundaries
        if(++x->m_writer_playhead >= buffersize) x->m_writer_playhead = 0;
        if(++x->m_reader_playhead >= buffersize) x->m_reader_playhead = 0;
    }
    
    return (w+5);
}

static void pa_delay3_tilde_dsp_prepare(t_pa_delay3_tilde *x, t_signal **sp)
{
    // as you want :
    //pa_delay3_tilde_clear_buffer(x);
    
    dsp_add(pa_delay3_tilde_dsp_perform, 4,
            x,              // object
            sp[0]->s_vec,   // inlet 1
            sp[1]->s_vec,   // outlet 1
            sp[0]->s_n);    // vectorsize
}

static void *pa_delay3_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_delay3_tilde *x = (t_pa_delay3_tilde *)pd_new(pa_delay3_tilde_class);
    
    if(x)
    {
        x->m_writer_playhead = x->m_reader_playhead = 0;
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
        pa_delay3_tilde_create_buffer(x);
        
        // create one signal outlet:
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return (x);
}


static void pa_delay3_tilde_free(t_pa_delay3_tilde *x)
{
    pa_delay3_tilde_delete_buffer(x);
    outlet_free(x->m_out);
}

extern void setup_pa0x2edelay3_tilde(void)
{
    t_class* c = class_new(gensym("pa.delay3~"),
                           (t_newmethod)pa_delay3_tilde_new, (t_method)pa_delay3_tilde_free,
                           sizeof(t_pa_delay3_tilde), CLASS_DEFAULT, A_GIMME, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_delay3_tilde_dsp_prepare, gensym("dsp"), A_CANT);
        class_addmethod(c, (t_method)pa_delay3_tilde_set_size_in_samps, gensym("size"), A_FLOAT, 0);
        class_addmethod(c, (t_method)pa_delay3_tilde_clear_buffer, gensym("clear"), 0);
        CLASS_MAINSIGNALIN(c, t_pa_delay3_tilde, m_f);
    }
    pa_delay3_tilde_class = c;
}

