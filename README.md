# PdObjects

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

## Liens

- ["HOWTO write an External for PureData"](http://pdstatic.iem.at/externals-HOWTO/)
