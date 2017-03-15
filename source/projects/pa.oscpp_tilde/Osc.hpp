/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "Phasor.hpp"

#include <array>
#include <cmath>

namespace paccpp
{
    // ================================================================================ //
    //                                    COS TABLE                                     //
    // ================================================================================ //
    
    //! @brief A cosine wave table that can be read with a phase between 0. and 1.
    template<class SampleType, size_t TableSize = 512>
    class CosTable
    {
    public: // methods
        
        using sample_t = SampleType;
        
        //! @brief Constructor.
        //! @details Initialize the cosinus table.
        CosTable()
        {
            const size_t tsize = size();
            
            for(size_t i = 0; i < tsize; ++i)
            {
                m_table[i] = cos(2.f * M_PI * i / tsize);
            }
            
            m_table[m_table.size()] = m_table[0];
        }
        
        //! @brief Destructor
        ~CosTable() = default;
        
        //! @brief Returns the size of the array
        size_t size() const
        {
            return m_table.size() - 1;
        }
        
        //! @brief Returns a linear interpolated value given a phase value between 0. and 1.
        sample_t getInterp(sample_t phase) const
        {
            // we cast to int to keep only the integer part of the floating-point number (eg. 3.99 => 3)
            phase *= (size() - 1);
            
            // we cast to int to keep only the integer part of the floating-point number (eg. 3.99 => 3)
            const size_t idx_1 = static_cast<size_t>(phase);
            
            // delta = tphase - integral part of the the floating-point number
            sample_t delta = phase - idx_1;
            
            const sample_t y1 = m_table[idx_1];
            
            // linear interpolation
            // (idx2 can always be idx_1+1 thanks to the additional sample in the table)
            return y1 + delta * (m_table[idx_1+1] - y1);
        }
        
    private: // variables
        
        std::array<sample_t, TableSize+1> m_table;
    };
    
    // ================================================================================ //
    //                                       OSC                                        //
    // ================================================================================ //
    
    template<class SampleType>
    class Osc
    {
    public: // methods
        
        using sample_t = SampleType;
        using costable_t = CosTable<sample_t, 512>;
        
        //! Default constructor
        Osc() = default;
        
        //! Destructor
        ~Osc() = default;
        
        //! @brief Set the current sampling rate
        //! @details You need to set a valid samplerate before calling any process method
        void setSampleRate(sample_t samplerate)
        {
            m_phasor.setSampleRate(samplerate);
        }
        
        //! @brief Set the frequency
        void setFrequency(sample_t freq)
        {
            m_phasor.setFrequency(freq);
        }
        
        //! @brief Get the frequency
        double getFrequency() const
        {
            return m_phasor.getFrequency();
        }
        
        //! @brief Increment the oscillator and return current phase value
        //! @details The Oscillator is running at the current frequency.
        //! @return The current phase.
        //! @see setFrequency
        sample_t process()
        {
            //return m_costable.getInterp(m_phasor.process());
            return m_costable.getInterp(m_phasor.process());
        }
        
        //! @brief Process a block of samples
        //! @details Update oscillator frequency with the inputs
        //! then increment the oscillator phase and return current value
        void process(sample_t const* freqs, sample_t* outs, long vecsize)
        {
            while(vecsize--)
            {
                m_phasor.setFrequency(*freqs++);
                *outs++ = process();
            }
        }
        
    private: // variables
        
        static const costable_t m_costable;
        
        Phasor<sample_t> m_phasor = 0.;
    };
    
    // Les variables statiques doivent être initialisées à l'extérieur de la classe: 
    template<class SampleType>
    typename Osc<SampleType>::costable_t const Osc<SampleType>::m_costable = {};
    
}
