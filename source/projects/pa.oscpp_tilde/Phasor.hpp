/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include <cmath>

namespace paccpp
{
    template<class SampleType>
    class Phasor
    {
    public: // methods
        
        using sample_t = SampleType;
        
        //! Default constructor
        Phasor(sample_t freq = 0.) :
        m_phase(0.),
        m_freq(freq)
        {
            ;
        }
        
        //! @brief Destructor
        ~Phasor() = default;
        
        //! @brief Set the current sampling rate
        //! @details You need to set a valid samplerate before calling any process method
        void setSampleRate(sample_t samplerate)
        {
            m_sr = samplerate;
            computeIncrement();
        }
        
        //! @brief Get the current sampling rate
        sample_t getSampleRate() const
        {
            return m_sr;
        }
        
        //! @brief Set the phase
        void setPhase(double phase)
        {
            while (phase >= 1.) phase -= 1.;
            while (phase < 0.) phase += 1.;
            m_phase = phase;
        }
        
        //! @brief Get the current phase
        double getPhase() const
        {
            return m_phase;
        }
        
        //! @brief Set the frequency
        void setFrequency(double freq)
        {
            m_freq = freq;
            computeIncrement();
        }
        
        //! @brief Get the frequency
        double getFrequency() const
        {
            return m_freq;
        }
        
        //! @brief Increment the phasor and return current phase value
        //! @details Phasor is running at the current frequency.
        //! @return The current phase.
        //! @see setFrequency
        sample_t process()
        {
            // store output value
            sample_t out = m_phase;
            
            // increment phase
            m_phase += m_phase_inc;
            
            // wrap phase between 0. and 1.
            if(m_phase >= 1.f) m_phase -= 1.f;
            if(m_phase < 0.f) m_phase += 1.f;
            
            return out;
        }
        
        //! @brief Process a block of samples
        //! @details Update phasor frequency with the inputs
        //! then increment the phasor and return current phase value
        void process(sample_t const* freqs, sample_t* outs, long vecsize)
        {
            sample_t freq = 0.f;
            
            while(vecsize--)
            {
                freq = *freqs++;
                
                *outs++ = m_phase;
                
                m_phase += (freq / m_sr);
                
                if(m_phase >= 1.f) m_phase -= 1.f;
                if(m_phase < 0.f) m_phase += 1.f;
            }
        }
        
    private: // methods
        
        void computeIncrement()
        {
            m_phase_inc = (m_sr > 0.) ? (m_freq / m_sr) : 0.;
        }
        
    private: // variables
        
        sample_t    m_sr;
        sample_t    m_phase = 0.;
        sample_t    m_freq = 0.;
        sample_t    m_phase_inc = 0.;
    };
}
