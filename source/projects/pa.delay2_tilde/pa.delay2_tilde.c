/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A fixed delay (initialized with first argument using dynamic allocation)

#include <m_pd.h>

#include <stdlib.h> // malloc, calloc, free...

static t_class *pa_delay2_tilde_class;

typedef struct _pa_delay2_tilde
{
    t_object    m_obj;
    
    float*      m_buffer;
    int         m_buffersize;
    int         m_count;
    
    t_outlet*   m_out;
    
    float       m_f;
    
} t_pa_delay2_tilde;

static void pa_delay2_tilde_delete_buffer(t_pa_delay2_tilde* x)
{
    if(x->m_buffer)
    {
        // free allocated buffer
        free(x->m_buffer);
        x->m_buffer = NULL;
    }
}

static void pa_delay2_tilde_clear_buffer(t_pa_delay2_tilde* x)
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

static void pa_delay2_tilde_create_buffer(t_pa_delay2_tilde* x)
{
    pa_delay2_tilde_delete_buffer(x);
    
    // allocate buffer
    x->m_buffer = (float*)malloc(sizeof(float) * x->m_buffersize);
    
    if(x->m_buffer)
    {
        pa_delay2_tilde_clear_buffer(x);
    }
    
    // or alloc and init to 0 directly with calloc
    //x->m_buffer = (float*)calloc(x->m_buffersize, sizeof(float));
}

static t_int *pa_delay2_tilde_perform(t_int *w)
{
    t_pa_delay2_tilde *x   = (t_pa_delay2_tilde *)(w[1]);
    t_sample  *in = (t_sample *)(w[2]);
    t_sample  *out = (t_sample *)(w[3]);
    int vecsize = (int)(w[4]);
    
    float* buffer = x->m_buffer;
    int count = x->m_count;
    float* buffer_playhead = NULL;
    float sample_to_write = 0.f;
    
    int buffersize = x->m_buffersize;
    
    while(vecsize--)
    {
        // store current input sample to write in the buffer
        sample_to_write = *in++;
        
        // fetch buffer pointer playhead position
        buffer_playhead = buffer + count;
        
        // we read our buffer.
        *out++ = *buffer_playhead;
        
        // then store incoming sample to the buffer.
        *buffer_playhead = sample_to_write;
        
        // wrap counter between buffer boundaries
        if(++count >= buffersize) count = 0;
    }
    
    x->m_count = count;
    
    return (w+5);
}

static void pa_delay2_tilde_dsp(t_pa_delay2_tilde *x, t_signal **sp)
{
    // as you want :
    //pa_delay2_tilde_clear_buffer(x);
    //x->m_count = 0;
    
    dsp_add(pa_delay2_tilde_perform, 4,
            x,              // object
            sp[0]->s_vec,   // inlet 1
            sp[1]->s_vec,   // outlet 1
            sp[0]->s_n);    // vectorsize
}

static void *pa_delay2_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_delay2_tilde *x = (t_pa_delay2_tilde *)pd_new(pa_delay2_tilde_class);
    
    if(x)
    {
        x->m_buffer = NULL;
        x->m_count = 0;
        
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
        pa_delay2_tilde_create_buffer(x);
        
        // create one signal outlet:
        x->m_out = outlet_new((t_object *)x, &s_signal);
    }
    
    return (x);
}


static void pa_delay2_tilde_free(t_pa_delay2_tilde *x)
{
    pa_delay2_tilde_delete_buffer(x);
    outlet_free(x->m_out);
}

extern void setup_pa0x2edelay2_tilde(void)
{
    t_class* c = class_new(gensym("pa.delay2~"),
                           (t_newmethod)pa_delay2_tilde_new, (t_method)pa_delay2_tilde_free,
                           sizeof(t_pa_delay2_tilde), CLASS_DEFAULT, A_GIMME, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_delay2_tilde_dsp, gensym("dsp"), A_CANT);
        CLASS_MAINSIGNALIN(c, t_pa_delay2_tilde, m_f);
    }
    pa_delay2_tilde_class = c;
}
