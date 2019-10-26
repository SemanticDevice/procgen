# procgen
A place to play around with graphical procedural generation

## beat_square

This pattern appears visually random, but once in a while particles align to form visible square  boundaries. I am guessing this has something to do with integer truncation in velocity  initialization.

![beat_square pattern example](https://github.com/SemanticDevice/procgen/blob/master/beat_square.gif)

## beat_circle

Somewhat similar to the beat_square pattern, but velocities are deliberately chosen from a discrete range with values that are multiples of each other. Particles also wrap around at a circle boundary instead of a square.

![beat_circle pattern example](https://github.com/SemanticDevice/procgen/blob/master/beat_circle.gif)