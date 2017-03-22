# PdObjects
[![Unix Build Status](https://travis-ci.org/paccpp/PdObjects.svg?branch=master)](https://travis-ci.org/paccpp/PdObjects) [![Windows Build status](https://ci.appveyor.com/api/projects/status/qvt1p0h499eiu47p/branch/master?svg=true)](https://ci.appveyor.com/project/eliottparis/pdobjects)

Ce répertoire regroupe les externals pour [Pure Data](https://puredata.info/) du cours [paccpp](https://github.com/paccpp/paccpp).

Pour installer ce répertoire et compiler les externals, se reporter au répertoire [PdStarterKit](https://github.com/paccpp/PdStarterKit) qui possède la même structure.

## Objets

| Nom       | déscription |
|-----------|-------------|
|[pa.dummy](source/projects/pa.dummy)           | Select outlet based on input atoms |
|[pa.count~](source/projects/pa.count_tilde)    | Outputs a signal increasing by 1 for each sample elapsed |
|[pa.phasor~](source/projects/pa.phasor_tilde)  | Outputs a ramp between 0. and 1. at a given frequency |
|[pa.osc1~](source/projects/pa.osc1_tilde)      | A simple sinusoidal oscillator |
|[pa.osc2~](source/projects/pa.osc2_tilde)      | A sinusoidal oscillator using linear interpolation on a 512 point buffer |
|[pa.osc3~](source/projects/pa.osc3_tilde)      | A sinusoidal oscillator using linear interpolation on a 512+1 point buffer |
|[pa.delay1~](source/projects/pa.delay1_tilde)  | A fixed delay using static memory allocation |
|[pa.delay2~](source/projects/pa.delay2_tilde)  | A fixed delay using dynamic memory allocation |
|[pa.sah~](source/projects/pa.sah_tilde)        | Sample and hold a signal according to a trigger |
|[pa.clip~](source/projects/pa.clip_tilde)      | Clip signal between minimum and maximum values |
|[pa.delay3~](source/projects/pa.delay3_tilde)  | A variable delay line |
|[pa.delay4~](source/projects/pa.delay4_tilde)  | A signal driven variable delay line |
|[pa.delay5~](source/projects/pa.delay5_tilde)  | A single writer / multiple readers delay line |
|[pa.readbuffer1~](source/projects/pa.readbuffer1_tilde)  | Access samples of a Pd array |
|[pa.readbuffer2~](source/projects/pa.readbuffer2_tilde)  | Read samples in a Pd array at a given speed |
|[pa.snapshot~](source/projects/pa.snapshot_tilde)  | Converts signal into float at a given time interval|
|[pa.gain~](source/projects/pa.gain_tilde)  | Multiply signal with a smooth transition|
|[pa.phasorpp~](source/projects/pa.phasorpp_tilde)  | `c++` version of the [pa.phasor~](source/projects/pa.phasor_tilde) object |

## Liens

- paccpp wiki => ["Anatomie-d'un-objet-PureData"](https://github.com/paccpp/paccpp/wiki/Anatomie-d'un-objet-PureData)
- ["HOWTO write an External for PureData"](http://pdstatic.iem.at/externals-HOWTO/)
