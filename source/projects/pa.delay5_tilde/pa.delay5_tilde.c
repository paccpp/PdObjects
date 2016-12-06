/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A single writer / multiple readers delay line.

#include <m_pd.h>

#include <stdlib.h> // malloc, calloc, free...

static t_class *pa_delay5_tilde_class;

typedef struct _pa_delay5_tilde
{
    t_object    m_obj;

    float*      m_buffer;
    size_t      m_buffersize;
    size_t      m_writer_playhead;
    int         m_number_of_readers;

    t_inlet**   m_inlets;
    t_outlet**  m_outlets;
    t_int*      m_dspvec;

    float       m_f;

} t_pa_delay5_tilde;

static void delete_buffer(t_pa_delay5_tilde* x)
{
    if(x->m_buffer)
    {
        // free allocated buffer
        free(x->m_buffer);
        x->m_buffer = NULL;
    }
}

static void clear_buffer(t_pa_delay5_tilde* x)
{
    if(x->m_buffer)
    {
        for(int i = 0; i < x->m_buffersize; ++i)
        {
            x->m_buffer[i] = 0.f;
        }
    }
}

static void create_buffer(t_pa_delay5_tilde* x)
{
    delete_buffer(x);

    // allocate and clear buffer
    x->m_buffer = (float*)calloc(x->m_buffersize, sizeof(float));
}

//! @brief returns a buffer value at a given index position.
//! @details idx will be wrapped between low and high buffer boundaries in a circular way.
static float get_buffer_value(t_pa_delay5_tilde* x, int idx)
{
    const size_t buffersize = x->m_buffersize;

    // wrap idx between low and high buffer boundaries.
    while(idx < 0) idx += buffersize;
    while(idx >= buffersize) idx -= buffersize;

    return x->m_buffer[idx];
}

static float linear_interp(float y1, float y2, float delta)
{
    return y1 + delta * (y2 - y1);
}

static t_int *pa_delay5_tilde_perform(t_int *w)
{
    t_pa_delay5_tilde *x = (t_pa_delay5_tilde *)(w[1]);
    int vecsize = (int)(w[2]);
    t_sample* first_input = (t_sample *)(w[3]);

    t_sample* inputs[x->m_number_of_readers];
    t_sample* outputs[x->m_number_of_readers];

    for(int i = 0; i < x->m_number_of_readers; ++i)
    {
        inputs[i] = (t_sample*)(w[i + 4]);
        outputs[i] = (t_sample*)(w[i + 4 + x->m_number_of_readers]);
    }

    float y1, y2, delta;
    float delay_size_samps = 0.f;
    float delay_sizes[x->m_number_of_readers];
    float* buffer = x->m_buffer;
    const size_t buffersize = x->m_buffersize;
    float sample_to_write = 0.f;
    int reader;

    for(int i = 0; i < vecsize; ++i)
    {
        // store current input sample to write it later in the buffer.
        sample_to_write = *first_input++;

        // we first need to store delay sizes samples because they may be overriden by outputs
        for(int j = 0; j < x->m_number_of_readers; ++j)
        {
            // get new delay size value.
            delay_sizes[j] = inputs[j][i];
        }

        for(int j = 0; j < x->m_number_of_readers; ++j)
        {
            // get new delay size value.
            delay_size_samps = delay_sizes[j];

            // clip delay size to buffersize - 1
            if(delay_size_samps >= buffersize)
            {
                delay_size_samps = buffersize - 1;
            }
            else if(delay_size_samps < 1.) // read first implementation : 0 samps delay = max delay
            {
                delay_size_samps = 1.;
            }

            // extract the fractional part
            delta = delay_size_samps - (int)delay_size_samps;

            reader = x->m_writer_playhead - (int)delay_size_samps;

            // Reading our buffer.
            y1 = get_buffer_value(x, reader);
            y2 = get_buffer_value(x, reader - 1);

            // with linear interpolation
            outputs[j][i] = linear_interp(y1, y2, delta);
        }

        // then store incoming sample to the buffer.
        buffer[x->m_writer_playhead] = sample_to_write;

        // increment then wrap counter between buffer boundaries
        if(++x->m_writer_playhead >= x->m_buffersize) x->m_writer_playhead -= x->m_buffersize;
    }

    return (w + (4 + (x->m_number_of_readers * 2)));
}

static void pa_delay5_tilde_dsp(t_pa_delay5_tilde *x, t_signal **sp)
{
    x->m_dspvec[0] = (t_int)x;
    x->m_dspvec[1] = (t_int)sp[0]->s_n;
    x->m_dspvec[2] = (t_int)sp[0]->s_vec;

    for(int i = 0; i < (x->m_number_of_readers * 2); ++i)
    {
        x->m_dspvec[3 + i] = (t_int)sp[i+1]->s_vec;
    }

    dsp_addv(pa_delay5_tilde_perform, (3 + (x->m_number_of_readers * 2)), x->m_dspvec);
}

static void* pa_delay5_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_delay5_tilde *x = (t_pa_delay5_tilde *)pd_new(pa_delay5_tilde_class);

    if(x)
    {
        x->m_buffer = NULL;
        x->m_writer_playhead = 0;
        int ndelay = 1;

        t_int buffersize = sys_getsr() * 0.1; // default to 100ms

        // init buffersize
        if(argc >= 1 && argv->a_type == A_FLOAT)
        {
            if(argv->a_w.w_float > 0)
            {
                buffersize = (t_int)argv->a_w.w_float;
            }
            else
            {
                pd_error((t_object*)x, "buffer size must be > 1");
            }
        }

        // init number of delays
        if(argc >= 2 && (argv+1)->a_type == A_FLOAT)
        {
            ndelay = (argv+1)->a_w.w_float;
            if(ndelay < 1)
            {
                ndelay = 1;
            }
        }

        x->m_buffersize = buffersize;
        x->m_number_of_readers = ndelay;

        // init dsp vector
        // object + vecsize + default inlet + inlets + outlets
        x->m_dspvec = (t_int*)malloc(sizeof(t_int) * (3 + (x->m_number_of_readers * 2)));

        // init inputs/outputs
        x->m_inlets = (t_inlet**)malloc(sizeof(t_inlet*) * x->m_number_of_readers);
        x->m_outlets = (t_outlet**)malloc(sizeof(t_outlet*) * x->m_number_of_readers);

        for(int i = 0; i < x->m_number_of_readers; i++)
        {
            x->m_inlets[i] = signalinlet_new((t_object*)x, 0.f);
            x->m_outlets[i] = outlet_new((t_object *)x, &s_signal);
        }

        // reset our buffer.
        create_buffer(x);
    }

    return (x);
}


static void pa_delay5_tilde_free(t_pa_delay5_tilde *x)
{
    // free each inlet/outlet
    for(int i = 0; i < x->m_number_of_readers; i++)
    {
        inlet_free(x->m_inlets[i]);
        outlet_free(x->m_outlets[i]);
    }

    // free io array
    free(x->m_inlets);
    free(x->m_outlets);

    free(x->m_dspvec);

    delete_buffer(x);
}

extern void setup_pa0x2edelay5_tilde(void)
{
    t_class* c = class_new(gensym("pa.delay5~"),
                           (t_newmethod)pa_delay5_tilde_new, (t_method)pa_delay5_tilde_free,
                           sizeof(t_pa_delay5_tilde), CLASS_DEFAULT, A_GIMME, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_delay5_tilde_dsp, gensym("dsp"), A_CANT);
        class_addmethod(c, (t_method)clear_buffer, gensym("clear"), 0);
        CLASS_MAINSIGNALIN(c, t_pa_delay5_tilde, m_f);
    }
    pa_delay5_tilde_class = c;
}
