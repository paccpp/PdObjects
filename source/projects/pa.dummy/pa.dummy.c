/*
 // Copyright (c) 2016 Eliott Paris, paccpp, Université Paris 8.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

// A dummy object that select outlet based on input atoms.

#include <m_pd.h> // puredata header

static t_class *pa_dummy_class;

typedef struct t_pa_dummy
{
    t_object    m_obj;          // pd object - always placed in first in the object's struct
    
    t_outlet*   m_out_bang;
    t_outlet*   m_out_float;
    t_outlet*   m_out_sym;
    
} t_pa_dummy;

static void pa_dummy_bang(t_pa_dummy *x)
{
    post("bang method called");
    outlet_bang(x->m_out_bang);
}

static void pa_dummy_float(t_pa_dummy *x, float f)
{
    post("float method called");
    outlet_float(x->m_out_float, f);
}

static void pa_dummy_anything(t_pa_dummy *x, t_symbol* s, int argc, t_atom *argv)
{
    post("anything method called with symbol : %s", s->s_name);
    
    int i = 0;
    for(; i < argc; ++i)
    {
        if(argv[i].a_type == A_FLOAT)
        {
            outlet_float(x->m_out_float, argv[i].a_w.w_float);
        }
        else if(argv[i].a_type == A_SYMBOL)
        {
            outlet_symbol(x->m_out_sym, argv[i].a_w.w_symbol);
        }
    }
}

static void pa_dummy_list(t_pa_dummy *x, t_symbol* s, int argc, t_atom *argv)
{
    post("list method called");
    
    int i = 0;
    for(; i < argc; ++i)
    {
        if(argv[i].a_type == A_FLOAT)
        {
            outlet_float(x->m_out_float, argv[i].a_w.w_float);
        }
        else if(argv[i].a_type == A_SYMBOL)
        {
            outlet_symbol(x->m_out_sym, argv[i].a_w.w_symbol);
        }
    }
}

static void *pa_dummy_new(t_symbol *name, int argc, t_atom *argv)
{
    t_pa_dummy *x = (t_pa_dummy *)pd_new(pa_dummy_class);
    
    if(x)
    {
        post("Dummy : number of args = %i", argc);
        
        int i = 0;
        for(; i < argc; ++i)
        {
            if(argv[i].a_type == A_FLOAT)
            {
                float flottant = argv[i].a_w.w_float;
                post("arg %i (float) = %f", i, flottant);
            }
            else if(argv[i].a_type == A_SYMBOL)
            {
                t_symbol* s = argv[i].a_w.w_symbol;
                post("arg %i (symbol) = %s", i, s->s_name);
            }
        }
        
        inlet_new((t_object*)x, &x->m_obj.ob_pd, 0, 0); // declare a new general inlet
        
        x->m_out_bang   = outlet_new((t_object*)x, gensym("bang"));
        x->m_out_float  = outlet_new((t_object*)x, gensym("float"));
        x->m_out_sym    = outlet_new((t_object*)x, NULL);
    }
    
    return (x);
}

static void pa_dummy_free(t_pa_dummy *x)
{
    ; // nothing to be done here.
}

extern void setup_pa0x2edummy(void)
{
    t_class* c = class_new(gensym("pa.dummy"),
                           (t_newmethod)pa_dummy_new, (t_method)pa_dummy_free,
                           sizeof(t_pa_dummy), CLASS_DEFAULT, A_GIMME, 0);
    if(c)
    {
        class_addmethod(c, (t_method)pa_dummy_bang,     gensym("bang"),     0);
        class_addmethod(c, (t_method)pa_dummy_float,    gensym("float"),    A_FLOAT, 0);
        class_addmethod(c, (t_method)pa_dummy_list,     gensym("list"),     A_GIMME, 0);
        class_addmethod(c, (t_method)pa_dummy_anything, gensym("anything"), A_GIMME, 0);
    }
    
    pa_dummy_class = c;
}
