/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief Access samples of a Pd array

#include <m_pd.h>

typedef struct _pa_readbuffer1
{
    t_object    m_obj;
    
    t_symbol*   m_name;
    t_word*     m_buffer;
    int         m_size;
    
    t_float     m_f;
    
} t_pa_readbuffer1;

static t_class *pa_readbuffer1_tilde_class;

static int pa_readbuffer1_tilde_set_buffer(t_pa_readbuffer1* x, t_symbol* s)
{
    x->m_size = 0;
    x->m_buffer = NULL;
    
    x->m_name = s;
    t_garray* a = NULL;
    
    if(!x->m_name || x->m_name == gensym(""))
    {
        pd_error(x, "pa.readbuffer1~: array name can not be empty");
        x->m_name = gensym("");
        return -1;
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
    
    return 0;
}

static t_int* pa_readbuffer1_perform(t_int *w)
{
    t_pa_readbuffer1* x = (t_pa_readbuffer1 *)(w[1]);
    t_sample* in   = (t_sample *)(w[2]);
    t_sample* out  = (t_sample *)(w[3]);
    size_t n       = (size_t)(w[4]);
    
    const int size = x->m_size;
    t_word const* buffer = x->m_buffer;
    int index;
    
    while(n--)
    {
        if(size > 0 && x->m_buffer)
        {
            index = (int)(*in++ * size);
            
            while(index < 0) { index += size; }
            while(index >= size) { index -= size; }
            
            *out++ = buffer[index].w_float;
        }
        else
        {
            *out++ = 0.f;
        }
    }
    
    return (w+5);
}


static void pa_readbuffer1_tilde_dsp(t_pa_readbuffer1* x, t_signal **sp)
{
    pa_readbuffer1_tilde_set_buffer(x, x->m_name);
    
    dsp_add(pa_readbuffer1_perform, 4,
            (t_int)x,
            (t_int)sp[0]->s_vec,
            (t_int)sp[1]->s_vec,
            (t_int)sp[0]->s_n);
}

static void *pa_readbuffer1_tilde_new(t_symbol* buffer_name)
{
    t_pa_readbuffer1* x = (t_pa_readbuffer1 *)pd_new(pa_readbuffer1_tilde_class);
    
    if(x)
    {
        pa_readbuffer1_tilde_set_buffer(x, buffer_name);
        
        outlet_new((t_object *)x, &s_signal);
    }
    
    return x;
}

static void pa_readbuffer1_tilde_free(t_pa_readbuffer1 *x)
{
    ;
}

extern void setup_pa0x2ereadbuffer1_tilde(void)
{
    t_class* c = class_new(gensym("pa.readbuffer1~"),
                           (t_newmethod)pa_readbuffer1_tilde_new, (t_method)pa_readbuffer1_tilde_free,
                           sizeof(t_pa_readbuffer1), CLASS_DEFAULT, A_DEFSYM, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_readbuffer1_tilde_dsp,           gensym("dsp"),        A_CANT);
        class_addmethod(c, (t_method)pa_readbuffer1_tilde_set_buffer,    gensym("set"),        A_SYMBOL, 0);
        CLASS_MAINSIGNALIN(c, t_pa_readbuffer1, m_f);
    }
    
    pa_readbuffer1_tilde_class = c;
}
